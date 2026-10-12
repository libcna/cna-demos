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
    def run_validation(self, failure="", warning=False, split_streams=False, with_gl=False):
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
    stage = "renderers" if "--renderers" in sys.argv else ("validate" if "--validate" in sys.argv else "smoke")
with open(os.environ["TRACE"], "a") as trace:
    trace.write(stage + "\\n")
if name == "xvfb-run":
    os.environ["DISPLAY"] = ":private-test-xvfb"
    os.execv(sys.argv[4], sys.argv[4:])  # skip -a -s and its screen argument
if stage == "renderers":
    print("OPENGLES3")
    sys.exit(0)
if stage == "smoke" and "OPENGLES3" in sys.argv:
    if (os.environ.get("WAYLAND_DISPLAY") is not None or
        os.environ.get("SDL_VIDEO_DRIVER") != "x11" or
        os.environ.get("DISPLAY") != ":private-test-xvfb"):
        print("GL inherited a desktop driver instead of the private X11 display", file=sys.stderr)
        sys.exit(12)
if os.environ["FAILURE"] == stage:
    print("A deliberately unformatted failure")
    sys.exit(9)
if name == "cmake" and os.environ["WARNING"] == "1":
    print("warning: a build warning")
if name == "ctest":
    print("100% tests passed")
if stage == "smoke":
    if os.environ["SPLIT_STREAMS"] == "1":
        sys.stdout.write("  CHECK FAILED: expected ")
        sys.stdout.flush()
        sys.stderr.write("[ERROR][APPLICATION] native diagnostic\\n")
        sys.stderr.flush()
        sys.stdout.write("assertion\\n")
        sys.stdout.flush()
    print("Summary: stub run")
'''
            for relative in ("commands/cmake", "commands/ctest", "commands/xvfb-run", "build/linux-full/bin/cna-demos"):
                path = root / relative
                path.write_text(command)
                path.chmod(0o755)
            (root / "tools/coverage_report.py").write_text(
                'import os\nwith open(os.environ["TRACE"], "a") as f: f.write("coverage\\n")\n')
            trace = root / "trace"
            env = dict(os.environ, PATH=str(root / "commands") + os.pathsep + os.environ["PATH"],
                       TRACE=str(trace), FAILURE=failure, WARNING=str(int(warning)),
                       SPLIT_STREAMS=str(int(split_streams)),
                       SKIP_GL=str(int(not with_gl)), JOBS="1", XDG_RUNTIME_DIR=str(root / "xdg"),
                       WAYLAND_DISPLAY="cna-test-inherited-wayland", SDL_VIDEO_DRIVER="wayland",
                       DISPLAY=":inherited-test-display")
            result = subprocess.run(["bash", str(root / "tools/validate.sh")], cwd=root,
                                    env=env, capture_output=True, text=True)
            # The stubs never emit JSON. In particular, a smoke-stage crash must leave no old
            # result behind, and SKIP_GL must not leave stale GL evidence. Archives survive.
            for renderer in ("SOFTWARE", "OPENGLES3"):
                self.assertFalse((reports / f"results-{renderer}.json").exists())
            self.assertEqual(archive.read_text(), "archived report")
            if split_streams:
                self.assertEqual((reports / "smoke-SOFTWARE.log").read_text(),
                                 "  CHECK FAILED: expected assertion\nSummary: stub run\n")
                self.assertEqual((reports / "smoke-SOFTWARE.stderr.log").read_text(),
                                 "[ERROR][APPLICATION] native diagnostic\n")
            return result, trace.read_text().splitlines()

    def test_success_reaches_coverage(self):
        result, stages = self.run_validation()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(stages, ["cmake", "validate", "ctest", "smoke", "coverage"])

    def test_xvfb_overrides_an_inherited_wayland_driver(self):
        result, stages = self.run_validation(with_gl=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(stages, ["cmake", "validate", "ctest", "smoke", "renderers",
                                  "xvfb-run", "smoke", "coverage"])

    def test_smoke_preserves_separate_streams(self):
        result, stages = self.run_validation(split_streams=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(stages[-1], "coverage")

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
