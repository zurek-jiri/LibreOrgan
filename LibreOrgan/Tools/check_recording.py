"""The recording of the instruction video checked for discontinuities before it is mixed (1 Nov 2026, the user:
"zvuk hlavně musí být bez glitchů! Jeden jsem v našem minulém videu slyšel"). Two findings count:
  - a HOLE: a run of 8 or more exactly-zero samples while the sound before and after it is above -40 dBFS
    (the first take of 1 Nov 2026 had 46 zeros = 1 ms inside a chord: the recorder flushed frames a route had
    not written yet);
  - a JUMP: a step between two samples above -50 dBFS that is more than 10 times the mean step of the 5 ms
    on BOTH sides of it (a note's attack or a smooth rise is no jump: its steps on the far side are as big).
Prints every finding with its time and sample, exits 1 on any, 0 on a clean file.
Usage: python check_recording.py <audio.wav>
"""
import sys, wave
import numpy as np

path = sys.argv[1]
with wave.open(path, 'rb') as w:
    rate, channels, width = w.getframerate(), w.getnchannels(), w.getsampwidth()
    raw = w.readframes(w.getnframes())
if width == 2:
    data = np.frombuffer(raw, dtype=np.int16).astype(np.float32) / 32768.0
elif width == 3:
    b = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
    data = ((b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)) - ((b[:, 2] & 0x80) << 17)).astype(np.float32) / 8388608.0
else:
    data = np.frombuffer(raw, dtype=np.float32)
data = data.reshape(-1, channels)
side = max(8, int(rate * 0.005))   # 5 ms
findings = []
for ch in range(channels):
    x = data[:, ch]
    # holes: runs of exact zeros between sound
    zero = x == 0.0
    edges = np.diff(zero.astype(np.int8))
    starts = np.nonzero(edges == 1)[0] + 1
    ends = np.nonzero(edges == -1)[0] + 1
    for s in starts:
        e = ends[ends > s]
        if len(e) == 0: continue
        e = e[0]
        if e - s < 8: continue
        before = np.abs(x[max(0, s - rate // 20):s]).max() if s > 0 else 0.0
        after = np.abs(x[e:e + rate // 20]).max() if e < len(x) else 0.0
        if before > 0.01 and after > 0.01:
            findings.append((s / rate, ch, 'hole of %d zero samples (%.1f ms) between sound at %.3f and %.3f' % (e - s, (e - s) * 1000.0 / rate, before, after), int(s)))
    # jumps against both sides
    step = np.abs(np.diff(x))
    kernel = np.ones(side, dtype=np.float32) / side
    full = np.convolve(step, kernel, mode='full')
    mean_before = full[:len(step)]            # the 5 ms ending at this step
    mean_after = full[side:side + len(step)]  # the 5 ms starting after it
    if len(mean_after) < len(step): mean_after = np.pad(mean_after, (0, len(step) - len(mean_after)), mode='edge')
    big = (step > 3.2e-3) & (step > 10.0 * np.maximum(mean_before, 1e-6)) & (step > 10.0 * np.maximum(mean_after, 1e-6))
    last = -rate
    for i in np.nonzero(big)[0]:
        if i - last < rate * 0.05: continue
        last = i
        findings.append((i / rate, ch, 'jump %+.4f -> %+.4f (steps around it %.5f / %.5f)' % (x[i], x[i + 1], mean_before[i], mean_after[i]), int(i)))
findings.sort()
for t, ch, what, i in findings[:40]:
    print('discontinuity at %8.3f s (sample %d) channel %d: %s' % (t, i, ch, what))
print('%s: %d discontinuity(ies) in %.1f s' % (path, len(findings), len(data) / rate))
sys.exit(1 if findings else 0)
