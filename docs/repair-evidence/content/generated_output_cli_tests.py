"""ISSUE-047 real compiler regression checks; pass the fixture executable as argv[1].

Tests retain their normal failing exit status on an unrepaired CNA. Compile the
fixture in the existing build-probe directory, never in the temporary data tree.
"""

import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


COMPILER = Path(sys.argv.pop(1)).resolve()
MANIFEST = ".cna-content-manifest.json"


class GeneratedOutputCliTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cna-generated-output-cli-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / "source"
        self.output = self.root / "output"
        self.source.mkdir()
        self.output.mkdir()
        self.input = self.source / "route.generated-output"
        self.write_input("report.txt", b"generated report\n")
        self.manual = self.output / "manual.txt"
        self.manual.write_bytes(b"unowned bytes\n")

    def write_input(self, name, payload):
        self.input.write_bytes(name.encode("utf-8") + b"\n" + payload)

    def invoke(self, *arguments, expected=0):
        completed = subprocess.run(
            [str(COMPILER), *map(str, arguments)],
            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            check=False, timeout=30,
        )
        self.assertEqual(completed.returncode, expected, completed.stdout)
        return completed.stdout

    def build(self):
        return self.invoke("build", self.source, "-o", self.output, "--workers", "1", "--explain")

    def entry(self):
        document = json.loads((self.output / MANIFEST).read_text(encoding="utf-8"))
        self.assertEqual(len(document["assets"]), 1)
        entry = document["assets"][0]
        self.assertEqual(entry["nodeId"], "route")
        return entry

    def test_directory_publication_noop_repair_and_clean(self):
        authored = self.input.read_bytes()
        self.assertIn("[BUILD] route", self.build())
        report = self.output / "report.txt"
        compiled = self.output / "route.cnb"
        self.assertEqual(report.read_bytes(), b"generated report\n")
        self.assertTrue(compiled.is_file())
        entry = self.entry()
        self.assertEqual(len(entry["dependencies"]), 1)
        self.assertEqual(entry["dependencies"][0]["identity"], self.input.name)
        self.assertEqual(len(entry["deploymentFiles"]), 1)
        deployment = entry["deploymentFiles"][0]
        self.assertEqual(deployment["sourceRoot"], "")
        self.assertEqual(deployment["source"], "report.txt")
        self.assertEqual(deployment["path"], "report.txt")
        self.assertEqual(deployment["sha256"], hashlib.sha256(report.read_bytes()).hexdigest())
        manifest = (self.output / MANIFEST).read_bytes()
        compiled_bytes = compiled.read_bytes()
        self.assertNotIn(str(self.root).encode(), manifest)
        self.assertIn("[SKIP] route", self.build())
        self.assertEqual((self.output / MANIFEST).read_bytes(), manifest)
        self.assertEqual(compiled.read_bytes(), compiled_bytes)

        report.write_bytes(b"tampered\n")
        self.assertIn("[BUILD] route", self.build())
        self.assertEqual(report.read_bytes(), b"generated report\n")
        self.assertEqual((self.output / MANIFEST).read_bytes(), manifest)
        report.unlink()
        self.assertIn("[BUILD] route", self.build())
        self.assertEqual(report.read_bytes(), b"generated report\n")
        self.assertEqual((self.output / MANIFEST).read_bytes(), manifest)

        self.invoke("clean", self.output)
        self.assertFalse(report.exists())
        self.assertFalse(compiled.exists())
        self.assertFalse((self.output / MANIFEST).exists())
        self.assertEqual(self.input.read_bytes(), authored)
        self.assertEqual(self.manual.read_bytes(), b"unowned bytes\n")

    def test_changed_generated_name_collects_only_obsolete_owned_output(self):
        self.build()
        self.write_input("replacement.txt", b"replacement report\n")
        self.assertIn("[BUILD] route", self.build())
        self.assertFalse((self.output / "report.txt").exists())
        self.assertEqual((self.output / "replacement.txt").read_bytes(), b"replacement report\n")
        self.assertEqual(self.entry()["deploymentFiles"][0]["path"], "replacement.txt")
        self.assertEqual(self.manual.read_bytes(), b"unowned bytes\n")
        self.assertIn("[SKIP] route", self.build())

    def test_single_file_build_publishes_related_file(self):
        authored = self.input.read_bytes()
        compiled = self.output / "direct.cnb"
        self.invoke("build", self.input, "-o", compiled)
        self.assertTrue(compiled.is_file())
        self.assertEqual((self.output / "report.txt").read_bytes(), b"generated report\n")
        self.assertEqual(self.input.read_bytes(), authored)
        self.assertEqual(self.manual.read_bytes(), b"unowned bytes\n")

    def test_nested_generated_path_is_owned_in_place_and_cleaned(self):
        self.write_input("Reports/route.txt", b"nested generated report\n")
        authored = self.input.read_bytes()
        self.build()
        report = self.output / "Reports" / "route.txt"
        self.assertEqual(report.read_bytes(), b"nested generated report\n")
        self.assertFalse((self.output / "route.txt").exists())
        self.assertEqual(len(self.entry()["dependencies"]), 1)
        deployment = self.entry()["deploymentFiles"][0]
        self.assertEqual(deployment["source"], "Reports/route.txt")
        self.assertEqual(deployment["path"], "Reports/route.txt")
        self.assertIn("[SKIP] route", self.build())
        self.invoke("clean", self.output)
        self.assertFalse(report.exists())
        self.assertFalse((self.output / "route.cnb").exists())
        self.assertEqual(self.input.read_bytes(), authored)
        self.assertEqual(self.manual.read_bytes(), b"unowned bytes\n")

    def test_clean_refuses_modified_generated_file_without_partial_deletion(self):
        self.build()
        compiled = self.output / "route.cnb"
        before_compiled = compiled.read_bytes()
        before_manifest = (self.output / MANIFEST).read_bytes()
        report = self.output / "report.txt"
        report.write_bytes(b"edited by user\n")
        self.invoke("clean", self.output, expected=1)
        self.assertEqual(report.read_bytes(), b"edited by user\n")
        self.assertEqual(compiled.read_bytes(), before_compiled)
        self.assertEqual((self.output / MANIFEST).read_bytes(), before_manifest)
        self.assertEqual(self.manual.read_bytes(), b"unowned bytes\n")

    def test_invalid_fixture_input_publishes_no_owned_files(self):
        self.input.write_bytes(b"no report filename delimiter")
        self.invoke("build", self.source, "-o", self.output, "--workers", "1", expected=1)
        self.assertFalse((self.output / "route.cnb").exists())
        self.assertFalse((self.output / "report.txt").exists())
        self.assertFalse((self.output / MANIFEST).exists())
        self.assertEqual(self.manual.read_bytes(), b"unowned bytes\n")


if __name__ == "__main__":
    unittest.main(verbosity=2)
