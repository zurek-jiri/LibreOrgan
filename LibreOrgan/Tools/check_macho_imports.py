#!/usr/bin/env python3
"""Lists the symbols a Mach-O executable imports from libc++ / libc++abi / libSystem,
with the dylib each one is bound to - to check an Intel build meant for old macOS
before anyone starts it there ("dyld: Symbol not found").

Usage: python check_macho_imports.py <LibreOrgan.app/Contents/MacOS/LibreOrgan> [--all]

Thin x86_64 and arm64 binaries and fat binaries are read (every slice). Without
--all only the C++ standard library imports (std::__1::..., operator new/delete,
__cxa_*) are listed. Known late-comers are flagged:
  std::__1::__hash_memory                      libc++ of macOS 15
  std::__1::bad_function_call::~bad_function_call (key function)   libc++ of macOS 15
  std::__1::__libcpp_verbose_abort             13.4
  std::__1::bad_optional_access / bad_variant_access / bad_any_cast typeinfo and vtable  10.14
  std::__1::__fs::filesystem::...              10.15
  aligned operator new/delete                  10.13
"""
import struct
import sys

LATE = {
    '__ZNSt3__113__hash_memoryEPKvm': 'macOS 15',
    '__ZNSt3__117bad_function_callD0Ev': 'macOS 15 (key function)',
    '__ZNSt3__117bad_function_callD1Ev': 'macOS 15 (key function)',
    '__ZNSt3__117bad_function_callD2Ev': 'macOS 15 (key function)',
    '__ZTVNSt3__117bad_function_callE': 'macOS 15 (key function)',
    '__ZTINSt3__117bad_function_callE': 'macOS 15 (key function)',
    '___cxa_init_primary_exception': 'macOS 15 (libc++abi, make_exception_ptr)',
    '__ZNSt3__121__libcpp_verbose_abortEPKcz': 'macOS 13.4',
    '__ZTVNSt3__119bad_optional_accessE': 'macOS 10.14',
    '__ZTINSt3__119bad_optional_accessE': 'macOS 10.14',
    '__ZTVNSt3__118bad_variant_accessE': 'macOS 10.14',
    '__ZTINSt3__118bad_variant_accessE': 'macOS 10.14',
    '__ZTVNSt3__112bad_any_castE': 'macOS 10.14',
    '__ZTINSt3__112bad_any_castE': 'macOS 10.14',
    '__ZnwmSt11align_val_t': 'macOS 10.13 (aligned new)',
    '__ZdlPvSt11align_val_t': 'macOS 10.13 (aligned delete)',
    '__ZnamSt11align_val_t': 'macOS 10.13 (aligned new[])',
    '__ZdaPvSt11align_val_t': 'macOS 10.13 (aligned delete[])',
    '__ZdlPvmSt11align_val_t': 'macOS 10.13 (aligned sized delete)',
}

MH_MAGIC_64 = 0xfeedfacf
FAT_MAGIC = 0xcafebabe
LC_SYMTAB = 0x2
LC_LOAD_DYLIB = 0xc
LC_LOAD_WEAK_DYLIB = 0x80000018
LC_REEXPORT_DYLIB = 0x8000001f
LC_LOAD_UPWARD_DYLIB = 0x80000023
N_TYPE = 0x0e
N_UNDF = 0x0
N_EXT = 0x01


def slices(data):
    (magic,) = struct.unpack('>I', data[:4])
    if magic == FAT_MAGIC:
        (count,) = struct.unpack('>I', data[4:8])
        for i in range(count):
            cputype, _, offset, size, _ = struct.unpack('>IIIII', data[8 + i * 20:28 + i * 20])
            yield cputype, data[offset:offset + size]
    else:
        (cputype,) = struct.unpack('<I', data[4:8])
        yield cputype, data


def imports(data):
    magic, cputype, _, _, ncmds, _, _, _ = struct.unpack('<IIIIIIII', data[:32])
    if magic != MH_MAGIC_64:
        raise SystemExit('not a 64-bit Mach-O slice (magic %08x)' % magic)
    pos = 32
    dylibs = []
    symtab = None
    for _ in range(ncmds):
        cmd, size = struct.unpack('<II', data[pos:pos + 8])
        if cmd in (LC_LOAD_DYLIB, LC_LOAD_WEAK_DYLIB, LC_REEXPORT_DYLIB, LC_LOAD_UPWARD_DYLIB):
            (name_offset,) = struct.unpack('<I', data[pos + 8:pos + 12])
            name = data[pos + name_offset:pos + size].split(b'\0')[0].decode()
            dylibs.append(name + (' (weak)' if cmd == LC_LOAD_WEAK_DYLIB else ''))
        elif cmd == LC_SYMTAB:
            symtab = struct.unpack('<IIII', data[pos + 8:pos + 24])
        pos += size
    if symtab is None:
        raise SystemExit('no LC_SYMTAB')
    symoff, nsyms, stroff, strsize = symtab
    strings = data[stroff:stroff + strsize]
    result = []
    for i in range(nsyms):
        n_strx, n_type, n_sect, n_desc, _ = struct.unpack('<IBBHQ', data[symoff + i * 16:symoff + i * 16 + 16])
        if (n_type & N_TYPE) != N_UNDF or not (n_type & N_EXT):
            continue
        name = strings[n_strx:strings.index(b'\0', n_strx)].decode()
        ordinal = (n_desc >> 8) & 0xff
        weak = bool(n_desc & 0x0040)   # N_WEAK_REF
        dylib = dylibs[ordinal - 1] if 0 < ordinal <= len(dylibs) else ('self' if ordinal == 0 else 'ordinal %d' % ordinal)
        result.append((name, dylib, weak))
    return result


def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    show_all = '--all' in sys.argv
    data = open(sys.argv[1], 'rb').read()
    bad = 0
    for cputype, blob in slices(data):
        arch = {0x01000007: 'x86_64', 0x0100000c: 'arm64'}.get(cputype, hex(cputype))
        rows = imports(blob)
        print('== %s: %d imported symbols' % (arch, len(rows)))
        for name, dylib, weak in sorted(rows):
            cxx = name.startswith('__ZNSt3__1') or name.startswith('__ZSt') or name.startswith('__Zn') \
                or name.startswith('__Zd') or name.startswith('___cxa') or name.startswith('__ZT')
            flag = LATE.get(name)
            if flag:
                bad += 1
            if show_all or cxx or flag:
                print('  %-70s %s%s%s' % (name, dylib, ' weak' if weak else '', ('   <-- needs ' + flag) if flag else ''))
    print('late symbols: %d' % bad)
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
