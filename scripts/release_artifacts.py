"""Version and package the exact images built by CI; no flashing or publishing."""
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
version = os.environ.get('BUILD_VERSION', 'dev')
if not re.fullmatch(r'dev|v\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?', version):
    raise SystemExit('BUILD_VERSION must be dev or vMAJOR.MINOR.PATCH[-suffix]')
if '--set-version' in sys.argv:
    config = ROOT / 'firmware/src/config.h'
    text, count = re.subn(r'(#define\s+FIRMWARE_VERSION\s+)"[^"]+"',
                         lambda m: m[1] + json.dumps(version), config.read_text())
    if count != 1:
        raise SystemExit('Expected exactly one firmware version definition')
    config.write_text(text)
    raise SystemExit(0)

build = ROOT / 'firmware/.pio/build'
out = ROOT / 'release'
out.mkdir(exist_ok=True)
files = {}
for env, name in [('wt32_sc01_plus', 'firmware'), ('wt32_sc01_plus', 'littlefs'),
                  ('wt32_sc01_plus', 'bootloader'), ('wt32_sc01_plus', 'partitions'),
                  ('bench', 'firmware')]:
    src = build / env / (name + '.bin')
    dest = out / f'pitclaw-{env}-{name}-{version}.bin'
    if not src.is_file() or not src.stat().st_size:
        raise SystemExit(f'Missing build artifact: {src}')
    shutil.copyfile(src, dest)
    files[dest.name] = hashlib.sha256(dest.read_bytes()).hexdigest()
manifest = {'version': version,
            'commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
            'board': 'wt32_sc01_plus', 'sha256': files,
            'initial_flash': 'Use PlatformIO upload and uploadfs from this commit; individual images are not merged factory images.',
            'update': 'Firmware-only OTA preserves LittleFS. Uploading littlefs.bin erases settings and cook data.',
            'qualification': 'Draft: physical acceptance required; see docs/production-review.md.'}
(out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
(out / 'SHA256SUMS').write_text(''.join(f'{digest}  {name}\n' for name, digest in files.items()))
for name in ['build-guide.md', 'production-review.md']:
    shutil.copyfile(ROOT / 'docs' / name, out / name)

# Keep the illustrated guide usable in the flat release attachment bundle.
for illustration in (ROOT / 'docs/images/build-guide').glob('*.svg'):
    shutil.copyfile(illustration, out / illustration.name)
guide = out / 'build-guide.md'
guide.write_text(guide.read_text(encoding='utf-8').replace('images/build-guide/', ''), encoding='utf-8')
