"""Local network settings: validation, overrides, and no shell evaluation."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ConfigTest(unittest.TestCase):
    def run_config(self, content, overrides=None):
        with tempfile.TemporaryDirectory() as directory:
            config = Path(directory) / 'multiplayer.env'
            config.write_text(content)
            env = dict(os.environ)
            for key in ('POKE_NET_SERVER', 'POKE_NET_PORT'):
                env.pop(key, None)
            env.update(overrides or {})
            return subprocess.run(
                ['bash', '-c', 'set -eu; source scripts/multiplayer_config.sh; '
                 'LoadMultiplayerConfig "$1"; printf "%s:%s" "$POKE_NET_SERVER" "$POKE_NET_PORT"',
                 'config-test', str(config)], cwd=ROOT, env=env, capture_output=True, text=True)

    def test_local_config(self):
        result = self.run_config('POKE_NET_SERVER=127.0.0.1\nPOKE_NET_PORT=25001\n')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, '127.0.0.1:25001')

    def test_environment_overrides(self):
        result = self.run_config('POKE_NET_SERVER=127.0.0.1\nPOKE_NET_PORT=25001\n',
                                 {'POKE_NET_SERVER': '192.0.2.1', 'POKE_NET_PORT': '25002'})
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, '192.0.2.1:25002')

    def test_no_private_defaults(self):
        self.assertNotEqual(self.run_config('').returncode, 0)
        self.assertNotEqual(self.run_config('POKE_NET_SERVER=127.0.0.1\n').returncode, 0)

    def test_port_validation(self):
        for port in ('0', '65536', '-1', 'abc', '$(false)', '1;false'):
            with self.subTest(port=port):
                self.assertNotEqual(self.run_config(f'POKE_NET_SERVER=127.0.0.1\nPOKE_NET_PORT={port}\n').returncode, 0)
        for port in ('1', '65535'):
            self.assertEqual(self.run_config(f'POKE_NET_SERVER=127.0.0.1\nPOKE_NET_PORT={port}\n').returncode, 0)

    def test_address_validation(self):
        for address in ('256.1.1.1', 'localhost', '1.2.3', '$(false)', '127.0.0.1;false'):
            with self.subTest(address=address):
                self.assertNotEqual(self.run_config(f'POKE_NET_SERVER={address}\nPOKE_NET_PORT=25001\n').returncode, 0)

    def test_normalize_and_crlf(self):
        result = self.run_config('# example\r\nPOKE_NET_SERVER=127.000.0.1\r\nPOKE_NET_PORT=02501\r\n')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, '127.0.0.1:2501')

    def test_config_is_not_executed(self):
        with tempfile.TemporaryDirectory() as directory:
            marker = Path(directory) / 'must-not-exist'
            result = self.run_config(f'POKE_NET_SERVER=$(touch {marker})\nPOKE_NET_PORT=25001\n')
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse(marker.exists())


if __name__ == '__main__':
    unittest.main()
