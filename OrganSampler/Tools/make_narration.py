"""The narration of the instruction video (0.8.206): narration.txt ("ms<TAB>sentence" per "say" step of the
video script) -> one spoken line per sentence (edge-tts, a neural voice; needs the internet) placed at its time
into narration.wav (48 kHz stereo, 16-bit). A sentence that would start while the previous one still speaks is
moved after it (0.4 s gap) and reported, so the script's waits can be lengthened.
Usage: python -I make_narration.py <narration.txt> <out folder> <ffmpeg.exe> [voice] [rate]
"""
import asyncio, os, subprocess, sys, wave
import numpy as np
import edge_tts
if sys.platform == 'win32':
    # The proactor loop's teardown prints "Event loop is closed" through aiohttp's connector on Windows.
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

narr, out, ffmpeg = sys.argv[1], sys.argv[2], sys.argv[3]
voice = sys.argv[4] if len(sys.argv) > 4 else 'en-GB-SoniaNeural'
rate = sys.argv[5] if len(sys.argv) > 5 else '-4%'
RATE = 48000
lines = []
for raw in open(narr, encoding='utf-8'):
    raw = raw.rstrip('\n')
    if '\t' not in raw: continue
    ms, text = raw.split('\t', 1)
    if text.strip(): lines.append((int(ms), text.strip()))
if not lines:
    print('no narration lines'); sys.exit(0)


async def synth(text, path):
    await edge_tts.Communicate(text, voice, rate=rate).save(path)

def synth_with_retries(text, path):
    # The service now and then answers with no audio (NoAudioReceived): a few tries with a pause between them.
    import time
    for attempt in range(6):
        try:
            asyncio.run(synth(text, path))
            if os.path.getsize(path) > 0: return
        except Exception as error:
            print('  try %d failed: %s' % (attempt + 1, error))
        time.sleep(2.0 + attempt)
    raise RuntimeError('no speech for: ' + text[:60])

clips = []
for i, (ms, text) in enumerate(lines):
    mp3 = os.path.join(out, 'say%03d.mp3' % i)
    wav = os.path.join(out, 'say%03d.wav' % i)
    synth_with_retries(text, mp3)
    subprocess.run([ffmpeg, '-y', '-loglevel', 'error', '-i', mp3, '-ar', str(RATE), '-ac', '2', wav], check=True)
    with wave.open(wav, 'rb') as w:
        data = np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).reshape(-1, 2).astype(np.float32) / 32768.0
    clips.append((ms, text, data))

placed, cursor = [], 0.0
for ms, text, data in clips:
    start = ms / 1000.0
    seconds = len(data) / RATE
    if start < cursor:
        print('moved %5.1f s -> %5.1f s (%.1f s long): %s' % (start, cursor, seconds, text[:60]))
        start = cursor
    placed.append((start, data))
    cursor = start + seconds + 0.4
    print('%6.1f - %6.1f s  %s' % (start, start + seconds, text[:70]))

total = int((cursor + 1.0) * RATE)
mix = np.zeros((total, 2), dtype=np.float32)
for start, data in placed:
    a = int(start * RATE)
    mix[a:a + len(data)] += data
peak = float(np.max(np.abs(mix))) if total else 0.0
if peak > 0.98: mix *= 0.98 / peak
with wave.open(os.path.join(out, 'narration.wav'), 'wb') as w:
    w.setnchannels(2); w.setsampwidth(2); w.setframerate(RATE)
    w.writeframes((mix * 32767.0).astype(np.int16).tobytes())
print('narration.wav: %.1f s, %d sentence(s), voice %s' % (total / RATE, len(placed), voice))
