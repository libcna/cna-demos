# Contributing

Thank you for helping CNA Demos grow. The short version: **one concept per demo, real CNA API
only, honest results, English everywhere.**

## Before you start

* Read [docs/PROJECT.md](docs/PROJECT.md) (why) and [docs/DEMO_AUTHORING.md](docs/DEMO_AUTHORING.md)
  (how). Architecture changes: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
* Check [docs/ROADMAP.md](docs/ROADMAP.md) for the area you want to work on and
  `cna-demos --search` for existing demos.

## Adding demos

1. One `.cpp` per demo under `demos/<category>/...`; the path is the ID.
2. Metadata complete and accurate (`.apis` spelled as `Type::Member` with XNA/CNA names).
3. Build, `cna-demos --validate`, run the demo, look at it (screenshots for graphical demos),
   `ctest`.
4. Regenerate the coverage report if you have a CNA checkout:
   `python3 tools/coverage_report.py --exe <build>/bin/cna-demos --cna-root ../cna`.

## Rules

* **English** in code, comments, identifiers, docs, CLI text, commit messages. Exception: text
  that is itself the subject of a localisation/Unicode demo, marked as such.
* **No invented API.** If CNA does not have it, there is no demo for it.
* **No inflation.** No per-constant demos, no renamed copies, no placeholders or TODO demos.
* **No silent workarounds** for CNA bugs: record them in [docs/CNA_ISSUES.md](docs/CNA_ISSUES.md).
* **Assets**: original or clearly licensed; provenance in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
* **Style**: follow the existing demos -- CNA's brace style, 4-space indent, `final` demo classes,
  anonymous namespaces, comments that explain CNA behaviour rather than C++. Each source file
  starts with `// SPDX-License-Identifier: MS-PL`.
* **Commits**: one coherent change per commit, imperative subject line
  (`Add SpriteBatch rotation and flip demos`), body explaining what and why.

## Licence

By contributing you agree that your contribution is licensed under the Microsoft Public License
(Ms-PL), the licence of this repository.
