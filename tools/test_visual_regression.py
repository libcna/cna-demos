#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Unit tests for deterministic screenshot comparison."""

from pathlib import Path
import tempfile
import unittest

from PIL import Image

from visual_regression import compare_images, selected_references


class VisualRegressionTests(unittest.TestCase):
    def write_image(self, path: Path, size=(2, 1), pixels=None) -> None:
        image = Image.new("RGBA", size, (10, 20, 30, 255))
        if pixels is not None:
            image.putdata(pixels)
        image.save(path)

    def test_identical_images_pass_exact_comparison(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.png"
            actual = Path(directory) / "actual.png"
            self.write_image(reference)
            self.write_image(actual)
            result = compare_images(reference, actual, 0, 0)
            self.assertTrue(result.passed)
            self.assertEqual(result.differing_pixels, 0)

    def test_tolerance_and_pixel_budget_are_independent(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.png"
            actual = Path(directory) / "actual.png"
            self.write_image(reference)
            self.write_image(actual, pixels=[(12, 20, 30, 255), (10, 25, 30, 255)])
            self.assertEqual(compare_images(reference, actual, 2, 0).differing_pixels, 1)
            self.assertTrue(compare_images(reference, actual, 2, 1).passed)
            self.assertFalse(compare_images(reference, actual, 4, 0).passed)

    def test_mismatch_writes_highlight_mask(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.png"
            actual = Path(directory) / "actual.png"
            difference = Path(directory) / "difference.png"
            self.write_image(reference)
            self.write_image(actual, pixels=[(10, 20, 30, 255), (255, 20, 30, 255)])
            result = compare_images(reference, actual, 0, 0, difference)
            self.assertFalse(result.passed)
            self.assertEqual(result.max_channel_difference, 245)
            self.assertEqual(list(Image.open(difference).getdata()),
                             [(0, 0, 0, 0), (255, 0, 0, 255)])

    def test_dimension_mismatch_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.png"
            actual = Path(directory) / "actual.png"
            self.write_image(reference)
            self.write_image(actual, size=(3, 1))
            result = compare_images(reference, actual, 255, 999)
            self.assertFalse(result.passed)
            self.assertIn("size differs", result.message)

    def test_reference_selection_is_sorted_and_filterable(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for renderer, demo_id in (("SOFTWARE", "b"), ("SOFTWARE", "a"),
                                      ("OPENGLES3", "z")):
                path = root / renderer / f"{demo_id}.png"
                path.parent.mkdir(parents=True, exist_ok=True)
                self.write_image(path)
            selected = selected_references(root, ["SOFTWARE"], [])
            self.assertEqual([item[1] for item in selected], ["a", "b"])
            requested = selected_references(root, ["OPENGLES3"], ["z"])
            self.assertEqual(requested[0][:2], ("OPENGLES3", "z"))


if __name__ == "__main__":
    unittest.main()
