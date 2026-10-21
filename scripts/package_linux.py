#!/usr/bin/env python3
"""Package an already-built Linux checkpoint without ROMs or user save files."""
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile

root = Path(__file__).resolve().parents[1]
version = sys.argv[1] if len(sys.argv) == 2 else ""
if not re.fullmatch(r"v[0-9]+\.[0-9]+\.[0-9]+-alpha\.[0-9]+", version):
    raise SystemExit("Usage: python3 scripts/package_linux.py v0.1.0-alpha.1")
port = root / "build/linux-port"
if (port / "pokeemerald").read_bytes()[:5] != b"\x7fELF\x01":
    raise SystemExit("Expected Linux ELF32 executable")
for name in ("src/net/net_client.c", "src/net/net_sync.c", "include/net/net_client.h"):
    if (root / name).read_bytes() != (port / name).read_bytes():
        raise SystemExit(f"Port is out of date: {name}")
commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=root, text=True).strip()
if subprocess.check_output(["git", "status", "--porcelain", "--untracked-files=normal"], cwd=root, text=True).strip():
    raise SystemExit("Commit the checkpoint before packaging")
if subprocess.check_output(["git", "rev-list", "-n", "1", version], cwd=root, text=True).strip() != commit:
    raise SystemExit("Version tag must identify the current checkpoint")
output = root / "build/releases" / f"pokeemerald-linux-{version}"
if output.exists():
    raise SystemExit(f"Output already exists: {output}")
output.mkdir(parents=True)
for name in ("pokeemerald", "BG.png", "BG1.png", "Border.png", "LICENSE"):
    shutil.copy2(port / name, output / name)
shutil.copy2(root / "src/net/enet/LICENSE", output / "LICENSE-ENet")
(output / "play.sh").write_text('''#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
export POKE_NET_SERVER="${POKE_NET_SERVER-127.0.0.1}"
exec ./pokeemerald "$@"
''')
(output / "play.sh").chmod(0o755)
(output / "README.txt").write_text('''Build experimental Linux i386, protocolo ENet 2.
Execute ./play.sh. Conecta à VM ao entrar no overworld, na porta UDP 25001.
Outro servidor: POKE_NET_SERVER=127.0.0.1 ./play.sh
Modo offline: POKE_NET_SERVER= ./play.sh
Requer bibliotecas SDL2, SDL2_image, OpenGL/GLU e libc de 32 bits instaladas.
Não é um AppImage e não inclui bibliotecas de sistema, ROM ou saves.
Use uma cópia/pasta diferente para cada jogador local; o save fica nesta pasta.
Setas: mover; Z/X: A/B; Enter: Start; barra invertida: Select; A/S: L/R.
Não há reconexão automática. O polling pausa em menus e batalhas.
Sprites e gameplay multiplayer ainda precisam de validação manual.
Consulte ROADMAP.md e docs/server.md no repositório para o status da rede pública.
''')
manifest = {
    "version": version, "source_commit": commit, "protocol": 2,
    "port_commit": (port / "UPSTREAM_COMMIT").read_text().strip(),
    "target": "linux-i386", "files": {},
}
for path in sorted(output.iterdir()):
    manifest["files"][path.name] = hashlib.sha256(path.read_bytes()).hexdigest()
(output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
archive = output.with_suffix(output.suffix + ".tar.gz")
with tarfile.open(archive, "w:gz") as tar:
    tar.add(output, arcname=output.name)
print(archive)
