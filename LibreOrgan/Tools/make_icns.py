#!/usr/bin/env python3
"""Builds Source/Icons/LibreOrgan.icns from the per-size PNG icons.

Pure Python (zlib only, no Pillow), so it runs on Windows, Linux and macOS alike.

Entries written:
  16, 32, 48, 128 px  classic 24-bit RLE data with an 8-bit mask
                      (is32/s8mk, il32/l8mk, ih32/h8mk, it32/t8mk) - every
                      macOS since 10.0 renders these exactly; the Finder of
                      macOS 10.7-10.13 renders PNG data in the newer 'icp4'
                      and 'icp5' entries as pixel garbage, so those two are
                      not written at all.
  128, 256, 512, 1024 PNG data (ic07, ic08, ic09, ic10; macOS 10.7+)
  @2x 16, 32, 128, 256  PNG data of 32, 64, 256, 512 px (ic11-ic14; 10.8+)

Usage: python make_icns.py <Source/Icons dir> <AppIcon.png (1024)> <out.icns>
The icons dir must hold icon-16/32/48/64/128/256/512.png (8-bit RGBA, not
interlaced). The result is parsed back and the classic entries are decoded and
compared with the PNG pixels before the file is written.
"""
import os
import struct
import sys
import zlib


def read_png(path):
    data = open(path, 'rb').read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise SystemExit(path + ': not a PNG')
    pos = 8
    idat = b''
    width = height = None
    while pos < len(data):
        (length,) = struct.unpack('>I', data[pos:pos + 4])
        kind = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IHDR':
            width, height, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', body)
            if depth != 8 or ctype != 6 or interlace != 0:
                raise SystemExit('%s: need 8-bit RGBA, not interlaced (depth %d, type %d, interlace %d)'
                                 % (path, depth, ctype, interlace))
        elif kind == b'IDAT':
            idat += body
        elif kind == b'IEND':
            break
    raw = zlib.decompress(idat)
    stride = width * 4
    out = bytearray()
    prev = bytearray(stride)
    p = 0
    for _ in range(height):
        f = raw[p]
        line = bytearray(raw[p + 1:p + 1 + stride])
        p += 1 + stride
        for i in range(stride):
            a = line[i - 4] if i >= 4 else 0
            b = prev[i]
            c = prev[i - 4] if i >= 4 else 0
            if f == 1:
                line[i] = (line[i] + a) & 255
            elif f == 2:
                line[i] = (line[i] + b) & 255
            elif f == 3:
                line[i] = (line[i] + ((a + b) >> 1)) & 255
            elif f == 4:
                pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 255
            elif f != 0:
                raise SystemExit(path + ': unknown PNG filter')
        out += line
        prev = line
    return width, height, bytes(out), data


def rle_encode(channel):
    # The icns RLE: a header byte below 0x80 = that many + 1 literal bytes follow;
    # 0x80 and above = the next byte repeated (header - 0x80 + 3) times.
    out = bytearray()
    i, n = 0, len(channel)
    while i < n:
        run = 1
        while i + run < n and channel[i + run] == channel[i] and run < 130:
            run += 1
        if run >= 3:
            out.append(0x80 + run - 3)
            out.append(channel[i])
            i += run
            continue
        j = i
        while j < n and j - i < 128:
            if j + 2 < n and channel[j] == channel[j + 1] == channel[j + 2]:
                break
            j += 1
        out.append(j - i - 1)
        out += channel[i:j]
        i = j
    return bytes(out)


def rle_decode(data, count):
    out = bytearray()
    i = 0
    while len(out) < count:
        h = data[i]
        i += 1
        if h < 0x80:
            out += data[i:i + h + 1]
            i += h + 1
        else:
            out += bytes([data[i]]) * (h - 0x80 + 3)
            i += 1
    return bytes(out[:count]), i


def classic_entries(size, rgba, data_type, mask_type):
    pixels = size * size
    channels = [bytes(rgba[c::4]) for c in range(3)]   # R, G, B planes
    data = b''.join(rle_encode(ch) for ch in channels)
    if data_type == b'it32':
        data = b'\0\0\0\0' + data                       # the 128 px entry starts with four zero bytes
    mask = bytes(rgba[3::4])
    # Verify the round trip before anything is written.
    body = data[4:] if data_type == b'it32' else data
    pos = 0
    for c in range(3):
        decoded, used = rle_decode(body[pos:], pixels)
        if decoded != channels[c]:
            raise SystemExit('RLE round trip failed for %s channel %d' % (data_type.decode(), c))
        pos += used
    return [(data_type, data), (mask_type, mask)]


def entry(kind, data):
    return kind + struct.pack('>I', len(data) + 8) + data


def main():
    if len(sys.argv) != 4:
        raise SystemExit(__doc__)
    icons, app_icon, out_path = sys.argv[1:]
    png = {}
    for size in (16, 32, 48, 64, 128, 256, 512):
        png[size] = read_png(os.path.join(icons, 'icon-%d.png' % size))
    png[1024] = read_png(app_icon)
    for size, (w, h, _, _) in png.items():
        if (w, h) != (size, size):
            raise SystemExit('icon-%d.png is %dx%d' % (size, w, h))
    entries = []
    entries += classic_entries(16, png[16][2], b'is32', b's8mk')
    entries += classic_entries(32, png[32][2], b'il32', b'l8mk')
    entries += classic_entries(48, png[48][2], b'ih32', b'h8mk')
    entries += classic_entries(128, png[128][2], b'it32', b't8mk')
    for kind, size in ((b'ic07', 128), (b'ic08', 256), (b'ic09', 512), (b'ic10', 1024),
                       (b'ic11', 32), (b'ic12', 64), (b'ic13', 256), (b'ic14', 512)):
        entries.append((kind, png[size][3]))
    body = b''.join(entry(kind, data) for kind, data in entries)
    blob = b'icns' + struct.pack('>I', len(body) + 8) + body
    with open(out_path, 'wb') as f:
        f.write(blob)
    print('%s: %d bytes, %d entries: %s' % (out_path, len(blob), len(entries),
                                             ' '.join(kind.decode() for kind, _ in entries)))


if __name__ == '__main__':
    main()
