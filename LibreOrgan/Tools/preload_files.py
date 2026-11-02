"""Reads every file under the folders given once, so the operating system's cache holds them - the
instruction video's organs load from the cache then (the user, 2 Nov 2026: "Předloudujeme to těsně před
začátkem videa"). Eight readers, 8 MB reads; prints the bytes and the speed.
Usage: python -I preload_files.py <folder> [<folder> ...]"""
import os, sys, time, threading, queue

folders = sys.argv[1:]
files = queue.Queue()
total = 0
for folder in folders:
    if os.path.isfile(folder):
        total += os.path.getsize(folder)
        files.put(folder)
        continue
    for root, _, names in os.walk(folder):
        for name in names:
            path = os.path.join(root, name)
            try: total += os.path.getsize(path)
            except OSError: continue
            files.put(path)
done = [0]
lock = threading.Lock()

def reader():
    while True:
        try: path = files.get_nowait()
        except queue.Empty: return
        try:
            with open(path, 'rb', buffering=0) as f:
                while True:
                    chunk = f.read(8 << 20)
                    if not chunk: break
                    with lock: done[0] += len(chunk)
        except OSError: pass

started = time.time()
threads = [threading.Thread(target=reader) for _ in range(8)]
for t in threads: t.start()
while any(t.is_alive() for t in threads):
    time.sleep(5)
    elapsed = time.time() - started
    print('%.1f of %.1f GB read, %.0f MB/s' % (done[0] / 2**30, total / 2**30, done[0] / 2**20 / max(1e-6, elapsed)), flush=True)
for t in threads: t.join()
elapsed = time.time() - started
print('preloaded %.1f GB in %.0f s = %.0f MB/s' % (done[0] / 2**30, elapsed, done[0] / 2**20 / max(1e-6, elapsed)))
