#!/usr/bin/env bash
# SPDX-License-Identifier: MS-PL
# The standard end-of-batch check (developer tool, Linux):
#   build -> --validate -> ctest -> smoke runs with reports -> regenerate docs/COVERAGE.md
#
# Usage: tools/validate.sh [preset]            (default preset: linux-full)
# Environment: CNA_ROOT (default ../cna), SKIP_GL=1 to skip the Xvfb OPENGLES3 run.
# Reports are written to build/<preset>/reports/.
set -euo pipefail

preset="${1:-linux-full}"
root="$(cd "$(dirname "$0")/.." && pwd)"
cna_root="${CNA_ROOT:-$root/../cna}"
build="$root/build/$preset"
bin="$build/bin/cna-demos"
reports="$build/reports"
mkdir -p "$reports"

export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/cna-demos-xdg}"
mkdir -p -m 700 "$XDG_RUNTIME_DIR"

echo "== build ($preset)"
cmake --build --preset "$preset" -j"$(nproc)" 2>&1 | grep -E "error|warning:|FAILED" && { echo "build produced errors/warnings"; exit 1; } || true

echo "== validate"
"$bin" --validate | tail -1

echo "== ctest"
ctest --test-dir "$build" -j"$(nproc)" --output-on-failure | grep -E "tests passed|tests failed|\*\*\*Failed|Not Run" || true

results=()
echo "== smoke: SOFTWARE"
SDL_AUDIO_DRIVER=dummy "$bin" --smoke --quiet --renderer SOFTWARE --report "$reports/results-SOFTWARE.json" | grep -A50 "^Summary"
results+=(--results "SOFTWARE=$reports/results-SOFTWARE.json")

if [[ "${SKIP_GL:-0}" != "1" ]] && "$bin" --renderers | grep -qx OPENGLES3 && command -v xvfb-run >/dev/null; then
    echo "== smoke: OPENGLES3 (Xvfb)"
    SDL_AUDIO_DRIVER=dummy xvfb-run -a -s "-screen 0 1280x1024x24" \
        "$bin" --smoke --quiet --renderer OPENGLES3 --report "$reports/results-OPENGLES3.json" | grep -A50 "^Summary"
    results+=(--results "OPENGLES3=$reports/results-OPENGLES3.json")
fi

echo "== coverage"
python3 "$root/tools/coverage_report.py" --exe "$bin" --cna-root "$cna_root" "${results[@]}" --output "$root/docs/COVERAGE.md"
