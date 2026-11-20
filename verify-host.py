# SPDX-License-Identifier: GPL-3.0-only
import csv, io, pathlib, struct, subprocess, sys, hashlib, tarfile
import tempfile

def gnu(n):
    h=5381
    for c in n.encode('ascii'): h=(h*33+c)&0xffffffff
    return h
if len(sys.argv) != 3:
    raise SystemExit("Usage: verify-host.py HOST_TGZ desktop_settings.fap (ARM nm/objcopy on PATH)")
host = pathlib.Path(sys.argv[1])
assert hashlib.sha256(host.read_bytes()).hexdigest() == 'c8798c2bf3478a31da18405f159598f8b708a553b5dadbbddbf318c7954baea7'
with tarfile.open(host, 'r:gz') as t:
    dfu = t.extractfile('f7-update-hybridus-lto-safe-20261118/firmware.dfu').read()
assert hashlib.sha256(dfu).hexdigest() == '111b7b3182def7c7abce82a1d3821aa1a15a5df139b6dd19c4cb09bae1137141'
magic, version, total, targets = struct.unpack_from('<5sBIB', dfu)
assert (magic, version, targets) == (b'DfuSe', 1, 1)
start, size = struct.unpack_from('<II', dfu, 285)
assert (start, size) == (0x08000000, 872916)
payload = dfu[293:293+size]
assert hashlib.sha256(payload).hexdigest() == '6b195b417321e49dd23a4ae1b2db02384743c26e3c5998898954c728cf5d5d44'
root = pathlib.Path(__file__).resolve().parent
stock = list(csv.DictReader(io.StringIO((root/'source/targets/f7/api_symbols.csv').read_text())))
custom=['hybridus_vgm_settings_'+s for s in ('get','set','init','save')]+['hybridus_color_get','hybridus_color_set']
all_names={gnu(r['name']):r['name'] for r in stock if r['status']=='+' and r['entry'] in ('Function','Variable')}
all_names.update({gnu(s):s for s in custom})
offsets=[o for o in range(0,len(payload)-8,4) if struct.unpack_from('<I',payload,o)[0]==gnu(custom[0])]
def pair(o): return struct.unpack_from('<II',payload,o)
def api_ptr(o): return start <= pair(o)[1] < start + size or 0x20000000 <= pair(o)[1] < 0x20040000
matches=[]
for center in offsets:
    if not api_ptr(center): continue
    s=e=center
    while s>=8 and 0<pair(s-8)[0]<pair(s)[0] and api_ptr(s-8): s-=8
    while e+16<=len(payload) and pair(e+8)[0]>pair(e)[0] and api_ptr(e+8): e+=8
    table=dict(pair(o) for o in range(s,e+1,8))
    if all(gnu(n) in table and (table[gnu(n)] & 1) for n in custom) and len(set(table)&all_names.keys())>1000:
        matches.append((s,e,table))
assert len(matches)==1
s,e,table=matches[0]
print(f'Firmware export table: 0x{start+s:08x}, {len(table)} rows; {len(set(table)&all_names.keys())} names matched against pinned SDK and six Hybridus names.')
for n in custom: print(f'{n} {table[gnu(n)]:#010x}')
for arg in sys.argv[2:]:
    p=pathlib.Path(arg).resolve()
    syms=sorted(line.split()[0] for line in subprocess.check_output(
        ['arm-none-eabi-nm','-P','-u',p], text=True).splitlines() if line)
    missing=[n for n in syms if gnu(n) not in table]
    print(p.name, len(syms),'imports; missing:', missing)
    with tempfile.TemporaryDirectory() as tmp:
        m=pathlib.Path(tmp)/'meta'
        subprocess.run(['arm-none-eabi-objcopy','--dump-section',f'.fapmeta={m}',p,pathlib.Path(tmp)/'copy'],check=True)
        raw=m.read_bytes()
    magic, version, minor, major, target, stack, app=struct.unpack_from('<IIHHHHI',raw)
    print(f'F{target}/API {major}.{minor}; app {app>>16}.{app&65535}; stack {stack} bytes; manifest {version}')
    assert (magic, version, major, minor, target, stack, app, raw[52])==(0x52474448,1,88,27,7,1024,2,1)
    assert not missing and len(set(map(gnu,syms)))==len(syms)
    assert set(custom)-{'hybridus_vgm_settings_init'} == {n for n in syms if n.startswith(('hybridus_vgm_settings_', 'hybridus_color_'))}
