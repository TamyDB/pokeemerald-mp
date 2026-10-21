#!/usr/bin/env bash
# Start without a display, using separate save/config files.
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
port_dir="$project_root/build/linux-port"
test_dir="$project_root/build/linux-smoke"
[[ -x "$port_dir/pokeemerald" ]] || { echo 'Compile o executável primeiro.' >&2; exit 1; }
mkdir -p "$test_dir"
cp "$port_dir/BG.png" "$port_dir/BG1.png" "$port_dir/Border.png" "$test_dir/"
cd "$test_dir"
status=0
timeout -k 2s 8s env -u POKE_NET_SERVER SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy SDL_RENDER_DRIVER=software \
    "$port_dir/pokeemerald" > smoke.log 2>&1 || status=$?
if [[ "$status" != 124 ]]; then
    cat smoke.log
    echo "Falha: o processo terminou antes do timeout (status $status)." >&2
    exit 1
fi
rg -q '\[AgbMain\] Task_Scene1_Load reached!' smoke.log
rg -q '\[AgbMain\] frame 300' smoke.log
echo "Inicialização sem janela aprovada; log: $test_dir/smoke.log"
