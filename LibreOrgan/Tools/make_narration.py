"""The narration of the instruction video (0.8.206; Kokoro since 1 Nov 2026): narration.txt ("ms<TAB>sentence" per
"say" step of the video script) -> every sentence spoken at its time into narration.wav (48 kHz stereo, 16-bit).

The voice is Kokoro-82M (local, open source; the user chose "bm_lewis" - the only one he found natural), run in
its own venv (C:\\Users\\doma\\kokoro-venv, Python 3.12, eSpeak NG on PATH). Pauses are OURS, not the voice's: a
sentence is cut at " - " (0.6 s of silence), at sentence ends (0.5 s) and at colons (0.4 s), each piece spoken
alone so the intonation lands on a complete phrase; nothing is compressed, the level is a plain peak of -6 dBFS.
A sentence that would start while the previous one still speaks is moved after it (0.4 s gap) and reported, so
the script's waits can be lengthened. The old edge-tts path stays behind --engine edge.
Usage: python make_narration.py <narration.txt> <out folder> <ffmpeg.exe> [voice] [speed] [--engine kokoro|edge]
"""
import os, re, subprocess, sys, wave
import numpy as np

args = [a for a in sys.argv[1:] if not a.startswith('--')]
engine = 'edge' if '--engine' in sys.argv and sys.argv[sys.argv.index('--engine') + 1] == 'edge' else 'kokoro'
narr, out, ffmpeg = args[0], args[1], args[2]
voice = args[3] if len(args) > 3 else ('bm_lewis' if engine == 'kokoro' else 'en-GB-SoniaNeural')
speed = args[4] if len(args) > 4 else ('1.0' if engine == 'kokoro' else '-4%')
RATE = 48000
PAUSE = {'dash': 0.5, 'stop': 0.4, 'colon': 0.3}   # shortened 1 Nov 2026: Kokoro's pace plus 0.6/0.5/0.4 outran the script's waits

lines = []
for raw in open(narr, encoding='utf-8'):
    raw = raw.rstrip('\n')
    if '\t' not in raw: continue
    ms, text = raw.split('\t', 1)
    if text.strip(): lines.append((int(ms), text.strip()))
if not lines:
    print('no narration lines'); sys.exit(0)


def pieces(text):
    """(phrase, pause after it) - the phrases the voice speaks one by one, the silences between them ours."""
    out = []
    for part in re.split(r'\s+-\s+', text):
        chunks = re.split(r'(?<=[.!?])\s+|\s+\.\.\.\s+', part)
        for chunk in chunks:
            for i, sub in enumerate(re.split(r':\s+', chunk)):
                sub = sub.strip()
                if not sub: continue
                if not re.search(r'[.!?]$', sub): sub += '.'
                out.append([sub, PAUSE['colon']])
            if out: out[-1][1] = PAUSE['stop']
        if out: out[-1][1] = PAUSE['dash']
    if out: out[-1][1] = PAUSE['stop']
    return out


def resample(data, rate):
    if rate == RATE: return data
    n = int(round(len(data) * RATE / float(rate)))
    x = np.linspace(0.0, len(data) - 1, n)
    return np.interp(x, np.arange(len(data)), data).astype(np.float32)


if engine == 'kokoro':
    from kokoro import KPipeline
    pipeline = KPipeline(lang_code='b' if voice.startswith('b') else 'a')

    def speak(text):
        parts = [np.zeros(0, dtype=np.float32)]
        for phrase, pause in pieces(text):
            for _, _, wav in pipeline(phrase, voice=voice, speed=float(speed)):
                parts.append(resample(np.asarray(wav, dtype=np.float32), 24000))
            parts.append(np.zeros(int(pause * RATE), dtype=np.float32))
        mono = np.concatenate(parts)
        return np.stack([mono, mono], axis=1)
else:
    import asyncio, edge_tts
    if sys.platform == 'win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    def speak(text):
        mp3 = os.path.join(out, 'say_tmp.mp3'); wav = os.path.join(out, 'say_tmp.wav')
        for attempt in range(6):
            try:
                asyncio.run(edge_tts.Communicate(text.replace(': ', '. '), voice, rate=speed).save(mp3))
                if os.path.getsize(mp3) > 0: break
            except Exception as error:
                print('  try %d failed: %s' % (attempt + 1, error))
        subprocess.run([ffmpeg, '-y', '-loglevel', 'error', '-i', mp3, '-ar', str(RATE), '-ac', '2', wav], check=True)
        with wave.open(wav, 'rb') as w:
            return np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).reshape(-1, 2).astype(np.float32) / 32768.0

clips = [(ms, text, speak(text)) for ms, text in lines]
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
if peak > 0.0: mix *= 0.5 / peak   # -6 dBFS peak, no compression: a soft voice; the mix sets the final level
with wave.open(os.path.join(out, 'narration.wav'), 'wb') as w:
    w.setnchannels(2); w.setsampwidth(2); w.setframerate(RATE)
    w.writeframes((mix * 32767.0).astype(np.int16).tobytes())
print('narration.wav: %.1f s, %d sentence(s), voice %s (%s), ends at %.1f s' % (total / RATE, len(placed), voice, engine, cursor))
