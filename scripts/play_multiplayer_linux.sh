#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
export POKE_NET_SERVER="${POKE_NET_SERVER:-127.0.0.1}"
exec "$project_root/scripts/play_linux.sh" "$@"
