# Changelog

All notable changes to cna-demos. The format follows [Keep a Changelog](https://keepachangelog.com/);
versions follow Semantic Versioning (pre-1.0: a minor version may change the CLI or metadata).

## [Unreleased]

### Added
- Project foundation: CMake integration with a CNA source checkout (`CNA_ROOT_DIR`, presets
  `default`, `windowless`, `linux-multi`, `linux-full`), Ms-PL licence, documentation set.
- Demo framework: self-registering registry, `DemoInfo` metadata, console/Game/standalone demo
  kinds, `DemoGame` base class (need checks, automated frame limit, Escape to exit, screenshots),
  `DemoContext` (output, checks, seed), runner with PASSED/FAILED/SKIPPED/NOT RUN results.
- `cna-demos` executable: interactive terminal browser and CLI (`--list`, `--categories`,
  `--search`, `--info`, `--run-category`, `--run-tag`, `--run-all`, `--smoke`, `--catalog-json`,
  `--list-ids`, `--validate`, `--renderers`, `--renderer`, `--frames`, `--seed`, `--screenshot`,
  `--report`).
- CTest integration: CLI tests and per-demo tests discovered at ctest time.
- Coverage report generator (`tools/coverage_report.py`) and deterministic asset generator
  (`tools/generate_assets.py`).
- Pilot demos in seven categories (math, core, graphics, content, audio, input, platform),
  including one standalone demo.
