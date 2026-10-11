#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Regression cases for the checkpoint review that caught a masked network failure."""
import contextlib
from collections import Counter
import io
import json
from pathlib import Path
import tempfile
import unittest

from compare_checkpoints import compare, failed_assertions, read_results


class CheckpointComparisonTests(unittest.TestCase):
    def test_child_failure_is_not_hidden_by_unchanged_expected_failure_status(self):
        with tempfile.TemporaryDirectory() as directory:
            before, after = (Path(directory) / name for name in ("before.log", "after.log"))
            before.write_text("host:   CHECK FAILED: known reliable loss\n")
            after.write_text(before.read_text() + "joiner:   CHECK FAILED: baseline delivery\n")
            results = {"net.example": {"status": "FAILED", "knownIssue": "ISSUE-030", "checks": 0}}
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                status = compare(results, results, failed_assertions(before), failed_assertions(after))
            self.assertEqual(status, 1)
            self.assertIn("ADDED FAILED ASSERTION (1x): baseline delivery", output.getvalue())

    def test_multiplicity_and_removed_assertions_require_review(self):
        rows = {"example": {"status": "FAILED"}}
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(compare(rows, rows, Counter(problem=1), Counter(problem=2)), 1)
            self.assertEqual(compare(rows, rows, Counter(problem=1), Counter()), 1)
            self.assertEqual(compare(rows, rows, Counter(problem=1), Counter(problem=1)), 0)

    def test_new_passing_demo_is_allowed_but_missing_id_or_status_change_is_not(self):
        old = {"old": {"status": "PASSED", "checks": 1}}
        new = dict(old, added={"status": "PASSED", "checks": 7})
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(compare(old, new, Counter(), Counter()), 0)
            self.assertEqual(compare(new, old, Counter(), Counter()), 1)
            self.assertEqual(compare(old, {"old": {"status": "FAILED"}}, Counter(), Counter()), 1)

    def test_duplicate_report_id_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "results.json"
            row = {"id": "same", "status": "PASSED"}
            path.write_text(json.dumps({"schema": "cna-demos-results/1", "results": [row, row]}))
            with self.assertRaisesRegex(ValueError, "duplicate"):
                read_results(path)


if __name__ == "__main__":
    unittest.main()
