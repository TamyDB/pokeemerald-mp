#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$project_root/build/linux-port"
if [[ ! -x ./pokeemerald ]]; then
    echo 'Executável ainda não compilado. Execute scripts/build_linux.sh.' >&2
    exit 1
fi
exec ./pokeemerald "$@"
