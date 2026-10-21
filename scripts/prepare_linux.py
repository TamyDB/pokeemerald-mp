#!/usr/bin/env python3
"""Prepare an isolated, pinned SDL2 port with this workspace's network layer."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
COMMIT = "db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46"
ROM_SHA1 = "f3ae088181bf583e55daf962a92bb46f4f1d07b7"


def replace_once(path, old, new):
    text = path.read_text()
    if text.count(old) != 1:
        raise RuntimeError(f"Unexpected upstream source: {path}: {old!r}")
    path.write_text(text.replace(old, new, 1))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, required=True, help="Clean upstream checkout at the pinned commit")
    parser.add_argument("--rom", type=Path, required=True, help="Local Emerald USA/Europe ROM for identity verification")
    args = parser.parse_args()
    if hashlib.sha1(args.rom.read_bytes()).hexdigest() != ROM_SHA1:
        raise SystemExit("ROM does not match the supported Emerald USA/Europe revision")
    commit = subprocess.check_output(["git", "-C", str(args.source), "rev-parse", "HEAD"], text=True).strip()
    if commit != COMMIT:
        raise SystemExit(f"Expected upstream commit {COMMIT}, got {commit}")
    if subprocess.check_output(["git", "-C", str(args.source), "status", "--porcelain"], text=True).strip():
        raise SystemExit("Upstream checkout must be clean")
    target = ROOT / "build/linux-port"
    if target.exists():
        raise SystemExit(f"{target} already exists; preserve or move it before preparing again")
    shutil.copytree(args.source, target, ignore=shutil.ignore_patterns(".git"))
    for directory in ("src/net", "include/net"):
        shutil.copytree(ROOT / directory, target / directory)
    (target / "UPSTREAM_COMMIT").write_text(COMMIT + "\n")
    (target / "ROM_SHA1").write_text(ROM_SHA1 + "\n")
    replace_once(target / "Makefile_pc", "CPPFLAGS := -std=gnu99", "CPPFLAGS := -DMULTIPLAYER -Iinclude/net -DHAS_FCNTL=1 -DHAS_POLL=1 -DHAS_SOCKLEN_T=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1 -std=gnu99")
    replace_once(target / "Makefile_pc", "-lm -lGL -lGLU", "-lm -lGL -lGLU -lpthread")
    replace_once(target / "Makefile_pc", "pkg-config --cflags sdl2 SDL2_image", "pkg-config --cflags sdl2")
    replace_once(target / "Makefile_pc", "$(shell pkg-config --libs sdl2 SDL2_image)", "$(shell pkg-config --libs sdl2) -lSDL2_image")
    # Upstream's game compiler runs cpp/cc1 separately. ENet uses normal system headers.
    with (target / "Makefile_pc").open("a") as makefile:
        makefile.write('''
$(C_BUILDDIR)/net/enet/%.o: src/net/enet/%.c
	@mkdir -p $(dir $@)
	$(CC) -m32 -std=gnu99 -Iinclude/net -DHAS_FCNTL=1 -DHAS_POLL=1 -DHAS_SOCKLEN_T=1 -DHAS_INET_PTON=1 -DHAS_INET_NTOP=1 -DHAS_MSGHDR_FLAGS=1 -c $< -o $@
''')
    replace_once(target / "src/overworld.c", '#include "global.h"', '#include <stdlib.h>\n#include "global.h"\n#include "net/net_client.h"')
    replace_once(target / "src/overworld.c", "    OverworldBasic();\n    if (fading)", '''    OverworldBasic();
    {
        static bool8 attempted;
        const char *address = getenv("POKE_NET_SERVER");
        if (!attempted && address != NULL)
        {
            attempted = TRUE;
            InitNetworkClient(address, 25001);
        }
        UpdateNetworkClient();
    }
    if (fading)''')
    replace_once(target / "include/event_object_movement.h", "void RemoveObjectEventByLocalIdAndMap(", "void RemoveNetworkObjectEvent(u8 objectEventId);\nvoid RemoveObjectEventByLocalIdAndMap(")
    replace_once(target / "src/event_object_movement.c", "void RemoveObjectEventByLocalIdAndMap(", '''void RemoveNetworkObjectEvent(u8 objectEventId)
{
    if (objectEventId < OBJECT_EVENTS_COUNT && gObjectEvents[objectEventId].active)
        RemoveObjectEvent(&gObjectEvents[objectEventId]);
}

void RemoveObjectEventByLocalIdAndMap(''')
    platform = target / "src/platform/sdl2.c"
    platform.write_text('#include <setjmp.h>\n#include "net/net_client.h"\n' + platform.read_text())
    replace_once(platform, "int DoMain(void *param);", "static jmp_buf sGameExit;\nstatic SDL_atomic_t sNetworkShutdown;\nint DoMain(void *param);")
    replace_once(platform, "    //StoreSaveFile();", "    SDL_AtomicSet(&sNetworkShutdown, 1);\n    SDL_SemPost(vBlankSemaphore);\n    SDL_WaitThread(mainLoopThread, NULL);\n    //StoreSaveFile();")
    replace_once(platform, "    AgbMain();", "    if (setjmp(sGameExit) == 0)\n        AgbMain();\n    ShutdownNetworkClient();")
    replace_once(platform, "    SDL_SemWait(vBlankSemaphore);", "    if (SDL_AtomicGet(&sNetworkShutdown))\n        longjmp(sGameExit, 1);\n    SDL_SemWait(vBlankSemaphore);\n    if (SDL_AtomicGet(&sNetworkShutdown))\n        longjmp(sGameExit, 1);")
    print(f"Prepared {target}; ROM verified, not copied or modified.")
    print(f"Build: make -C {target} -f Makefile_pc NATIVE_LINUX=1 -j4")


if __name__ == "__main__":
    main()
