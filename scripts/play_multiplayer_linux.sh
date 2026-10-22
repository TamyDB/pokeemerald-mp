#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
source "$project_root/scripts/multiplayer_config.sh"
LoadMultiplayerConfig "${POKE_NET_CONFIG:-$project_root/config/multiplayer.env}"
exec "$project_root/scripts/play_linux.sh" "$@"
