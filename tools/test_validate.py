#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Check validate.sh's failure propagation without compiling or running CNA."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class ValidateScriptTests(unittest.TestCase):
    def run_validation(self, failure="", warning=False):
        with tempfile.TemporaryDirectory(prefix="cna-validate-test-") as directory:
            root = Path(directory)
            (root / "tools").mkdir()
            (root / "commands").mkdir()
            (root / "build/linux-full/bin").mkdir(parents=True)
            reports = root / "build/linux-full/reports"
            reports.mkdir()
            for renderer in ("SOFTWARE", "OPENGLES3"):
                (reports / f"results-{renderer}.json").write_text("stale report")
            archive = reports / "baseline-SOFTWARE.json"
            archive.write_text("archived report")
            shutil.copyfile(Path(__file__).with_name("validate.sh"), root / "tools/validate.sh")
            # These are command stubs, not a build tree or compiled probe.
            command = '''#!/usr/bin/env python3
import os, sys
from pathlib import Path
name = Path(sys.argv[0]).name
stage = name
if name == "cna-demos":
    stage = "validate" if "--validate" in sys.argv else "smoke"
with open(os.environ["TRACE"], "a") as trace:
    trace.write(stage + "\\n")
if os.environ["FAILURE"] == stage:
    print("A deliberately unformatted failure")
    sys.exit(9)
if name == "cmake" and os.environ["WARNING"] == "1":
    print("warning: a build warning")
if name == "ctest":
    print("100% tests passed")
if stage == "smoke":
    print("Summary: stub run")
'''
            for relative in ("commands/cmake", "commands/ctest", "build/linux-full/bin/cna-demos"):
                path = root / relative
                path.write_text(command)
                path.chmod(0o755)
            (root / "tools/coverage_report.py").write_text(
                'import os\nwith open(os.environ["TRACE"], "a") as f: f.write("coverage\\n")\n')
            trace = root / "trace"
            env = dict(os.environ, PATH=str(root / "commands") + os.pathsep + os.environ["PATH"],
                       TRACE=str(trace), FAILURE=failure, WARNING=str(int(warning)),
                       SKIP_GL="1", JOBS="1", XDG_RUNTIME_DIR=str(root / "xdg"))
            result = subprocess.run(["bash", str(root / "tools/validate.sh")], cwd=root,
                                    env=env, capture_output=True, text=True)
            # The stubs never emit JSON. In particular, a smoke-stage crash must leave no old
            # result behind, and SKIP_GL must not leave stale GL evidence. Archives survive.
            for renderer in ("SOFTWARE", "OPENGLES3"):
                self.assertFalse((reports / f"results-{renderer}.json").exists())
            self.assertEqual(archive.read_text(), "archived report")
            return result, trace.read_text().splitlines()

    def test_success_reaches_coverage(self):
        result, stages = self.run_validation()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(stages, ["cmake", "validate", "ctest", "smoke", "coverage"])

    def test_failure_stops_at_each_stage(self):
        order = ["cmake", "validate", "ctest", "smoke"]
        for index, stage in enumerate(order):
            with self.subTest(stage=stage):
                result, stages = self.run_validation(failure=stage)
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(stages, order[:index + 1])

    def test_build_warning_stops_validation(self):
        result, stages = self.run_validation(warning=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(stages, ["cmake"])


if __name__ == "__main__":
    unittest.main()
