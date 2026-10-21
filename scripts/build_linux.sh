#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
port_dir="$project_root/build/linux-port"
if [[ ! -f "$port_dir/UPSTREAM_COMMIT" ]]; then
    echo 'Prepare o port primeiro; veja docs/linux-playable.md.' >&2
    exit 1
fi
# Fedora multilib: select i686 metadata rather than the host's x86_64 libraries.
if [[ -d /usr/lib/pkgconfig ]]; then
    export PKG_CONFIG_LIBDIR=/usr/lib/pkgconfig:/usr/share/pkgconfig
fi
pkg-config --exists sdl2 || {
    echo 'Faltam os pacotes de desenvolvimento SDL2/SDL2_image de 32 bits.' >&2
    exit 1
}
# Keep the isolated port in sync with the versioned network implementation.
cp -ru "$project_root/src/net/." "$port_dir/src/net/"
cp -ru "$project_root/include/net/." "$port_dir/include/net/"
make -C "$port_dir" -f Makefile_pc NATIVE_LINUX=1 -j"${JOBS:-4}"
file "$port_dir/pokeemerald"
ldd "$port_dir/pokeemerald"
echo "Execute: $project_root/scripts/play_linux.sh"
