#!/usr/bin/env python3
"""Start a private relay, run real-client integration, and always stop it."""
import argparse
from pathlib import Path
import socket
import subprocess
import time

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--client", default="build/pc-net/relay-test")
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    sock.bind(("127.0.0.1", 0))
    port = str(sock.getsockname()[1])
server = subprocess.Popen([str(root / "build/pc-net/pokeemerald-server"), port])
try:
    time.sleep(0.1)
    if server.poll() is not None:
        raise SystemExit("Relay failed to start")
    subprocess.run([str(root / args.client), "127.0.0.1", port], check=True, timeout=40)
finally:
    server.terminate()
    server.wait(timeout=5)
