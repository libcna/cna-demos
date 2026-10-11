# SPDX-License-Identifier: MS-PL
"""Original stage and profile sources for the nested coordinator example."""
from pathlib import Path


def write_all(assets_root: Path) -> None:
    root = assets_root / 'nested_builds'
    root.mkdir(parents=True, exist_ok=True)
    (root / 'arena.stage').write_text('3 guard.profile\n', encoding='utf-8')
    (root / 'guard.profile').write_text('Guard 40\n', encoding='utf-8')
