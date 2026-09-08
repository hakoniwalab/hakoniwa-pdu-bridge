from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).with_name("hako.py")
SPEC = importlib.util.spec_from_file_location("hako_bridge_tool", MODULE_PATH)
assert SPEC is not None and SPEC.loader is not None
HAKO = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = HAKO
SPEC.loader.exec_module(HAKO)


class FoundationInstallTests(unittest.TestCase):
    def test_dependency_receipt_reads_contract_fields(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            prefix = Path(temp_dir)
            receipt = (
                prefix
                / "share"
                / "hakoniwa"
                / "receipts"
                / "hakoniwa-pdu-endpoint.yaml"
            )
            receipt.parent.mkdir(parents=True)
            receipt.write_text(
                """schema_version: 1
component:
  id: hakoniwa-pdu-endpoint
  version: 1.0.0
  source_revision: "def456"
build_limits:
  asset_num: 16
artifacts:
  - path: "lib/libhakoniwa_pdu_endpoint.so"
    kind: library
""",
                encoding="utf-8",
            )

            dependency = HAKO._read_dependency_receipt(
                prefix,
                "hakoniwa-pdu-endpoint",
            )

            self.assertEqual(dependency["version"], "1.0.0")
            self.assertEqual(dependency["source_revision"], "def456")
            self.assertEqual(dependency["build_limits"]["asset_num"], 16)

    def test_bridge_artifacts_include_runtime_and_package_surfaces(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            prefix = Path(temp_dir)
            cmake_dir = prefix / "lib" / "cmake" / "hakoniwa_pdu_bridge"
            cmake_dir.mkdir(parents=True)
            config_dir = (
                prefix
                / "share"
                / "hakoniwa-pdu-bridge"
                / "config"
                / "web_bridge_fleets"
            )
            config_dir.mkdir(parents=True)
            (prefix / "bin").mkdir()
            (prefix / "bin" / "hakoniwa-pdu-web-bridge").write_text(
                "",
                encoding="utf-8",
            )

            artifacts = HAKO._bridge_artifacts(prefix)

            self.assertIn(
                (Path("bin/hakoniwa-pdu-web-bridge"), "executable"),
                artifacts,
            )
            self.assertIn(
                (Path("lib/cmake/hakoniwa_pdu_bridge"), "cmake-package"),
                artifacts,
            )


class StateDirectoryTests(unittest.TestCase):
    def context(self, root, state=None):
        manifest = root / "hakoniwa-build.yaml"
        manifest.write_text("version: 1\n", encoding="utf-8")
        return HAKO.create_context(manifest, root, state)

    def write(self, ctx):
        return HAKO.write_resolved(ctx)

    def receipt(self, ctx, root):
        from unittest.mock import patch
        install = root / "install"
        (install / "lib/cmake/hakoniwa_pdu_bridge").mkdir(parents=True, exist_ok=True)
        ctx.endpoint_root = None
        ctx.core_root = None
        with patch.object(HAKO, "_command_output", return_value="test-revision"):
            receipt = HAKO.write_receipt(ctx, install)
        return receipt.parent / "resolved" / "hakoniwa-pdu-bridge-core.yaml"

    def test_default_state_remains_repository_local(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            ctx = self.context(root)
            self.assertEqual(self.write(ctx), root / ".hako/resolved-build.yaml")

    def test_two_states_and_receipts_do_not_use_legacy_state(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            legacy = root / ".hako/resolved-build.yaml"
            legacy.parent.mkdir()
            legacy.write_text("legacy sentinel", encoding="utf-8")
            contexts = [self.context(root, root / name) for name in ("state-a", "state-b")]
            contexts[0].build_dir = root / "build-a"
            contexts[1].build_dir = root / "build-b"
            first = self.write(contexts[0])
            before = first.read_bytes()
            second = self.write(contexts[1])
            self.assertEqual(first.read_bytes(), before)
            self.assertNotEqual(first.read_bytes(), second.read_bytes())
            for ctx in (contexts[0], contexts[1], contexts[0]):
                self.assertEqual(self.receipt(ctx, root).read_bytes(),
                                 (ctx.hako_state_dir / "resolved-build.yaml").read_bytes())
            self.assertEqual(legacy.read_text(), "legacy sentinel")

    def test_relative_state_is_resolved_from_invocation_directory(self):
        import os
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            repo = root / "repo"
            repo.mkdir()
            previous = Path.cwd()
            try:
                os.chdir(root)
                ctx = self.context(repo, Path("relative-state"))
            finally:
                os.chdir(previous)
            self.assertEqual(ctx.hako_state_dir, root / "relative-state")
            self.assertEqual(self.write(ctx), root / "relative-state/resolved-build.yaml")


if __name__ == "__main__":
    unittest.main()
