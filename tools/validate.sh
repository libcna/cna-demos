#!/usr/bin/env bash
# SPDX-License-Identifier: MS-PL
# The standard end-of-batch check (developer tool, Linux):
#   build -> --validate -> ctest -> smoke runs with reports -> regenerate docs/COVERAGE.md
#
# Usage: tools/validate.sh [preset]            (default preset: linux-full)
# Environment: CNA_ROOT (default ../cna), SKIP_GL=1 to skip the Xvfb OPENGLES3 run,
# VISUAL_REGRESSION=1 to compare approved screenshots, and JOBS to override build/test
# parallelism (default: nproc).
# Reports are written to build/<preset>/reports/.
set -euo pipefail

preset="${1:-linux-full}"
root="$(cd "$(dirname "$0")/.." && pwd)"
cna_root="${CNA_ROOT:-$root/../cna}"
build="$root/build/$preset"
bin="$build/bin/cna-demos"
reports="$build/reports"
jobs="${JOBS:-$(nproc)}"
mkdir -p "$reports"
# A smoke process killed before writing its JSON must not leave a previous run's report
# looking current. Keep deliberately named baseline archives; invalidate only current result/stderr outputs.
rm -f "$reports/results-SOFTWARE.json" "$reports/results-OPENGLES3.json" \
      "$reports/smoke-SOFTWARE.stderr.log" "$reports/smoke-OPENGLES3.stderr.log"

export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/tmp/cna-demos-xdg}"
export XDG_DATA_HOME="${XDG_DATA_HOME:-$reports/xdg-data}"
mkdir -p -m 700 "$XDG_RUNTIME_DIR"
mkdir -p "$XDG_DATA_HOME"

echo "== build ($preset)"
if ! cmake --build --preset "$preset" -j"$jobs" > "$reports/build.log" 2>&1; then
    cat "$reports/build.log"
    exit 1
fi
if grep -E "error:|warning:|FAILED:" "$reports/build.log"; then
    echo "build produced errors/warnings (see $reports/build.log)"
    exit 1
fi

echo "== validate"
"$bin" --validate | tail -1

echo "== cna-examples migration ledger"
migration_args=(--exe "$bin")
if [[ -d "$root/../cna-examples/src" ]]; then
    migration_args+=(--legacy-root "$root/../cna-examples")
fi
python3 "$root/tools/cna_examples_migration_audit.py" "${migration_args[@]}"

echo "== ctest"
if ! ctest --test-dir "$build" -j"$jobs" --output-on-failure > "$reports/ctest.log" 2>&1; then
    cat "$reports/ctest.log"
    exit 1
fi
grep -E "tests passed|tests failed|\*\*\*Failed|Not Run" "$reports/ctest.log" || true

# Keep stderr separate: CNA logger writes can land inside buffered stdout assertion lines.
# Preserve both streams verbatim; compare_checkpoints.py reads the stdout assertion log.
results=()
ran_gl=0
echo "== smoke: SOFTWARE"
SDL_AUDIO_DRIVER=dummy "$bin" --smoke --quiet --renderer SOFTWARE --report "$reports/results-SOFTWARE.json" \
    2> "$reports/smoke-SOFTWARE.stderr.log" | tee "$reports/smoke-SOFTWARE.log" | grep -A50 "^Summary"
results+=(--results "SOFTWARE=$reports/results-SOFTWARE.json")

if [[ "${SKIP_GL:-0}" != "1" ]] && "$bin" --renderers | grep -qx OPENGLES3 && command -v xvfb-run >/dev/null; then
    echo "== smoke: OPENGLES3 (Xvfb)"
    # DISPLAY alone does not prevent SDL selecting an inherited Wayland session.
    SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY \
        xvfb-run -a -s "-screen 0 1280x1024x24" \
        "$bin" --smoke --quiet --renderer OPENGLES3 --report "$reports/results-OPENGLES3.json" \
        2> "$reports/smoke-OPENGLES3.stderr.log" | tee "$reports/smoke-OPENGLES3.log" | grep -A50 "^Summary"
    results+=(--results "OPENGLES3=$reports/results-OPENGLES3.json")
    ran_gl=1
fi

echo "== coverage"
python3 "$root/tools/coverage_report.py" --exe "$bin" --cna-root "$cna_root" "${results[@]}" --output "$root/docs/COVERAGE.md"

if [[ "${VISUAL_REGRESSION:-0}" == "1" ]]; then
    echo "== visual-regression unit tests"
    python3 "$root/tools/test_visual_regression.py"
    visual_args=(--exe "$bin" --references "$root/references" --output "$reports/visual-regression"
                 --renderer SOFTWARE)
    if [[ "$ran_gl" == "1" ]]; then
        visual_args+=(--renderer OPENGLES3)
        echo "== visual regression: SOFTWARE + OPENGLES3 (Xvfb)"
        SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY \
            xvfb-run -a -s "-screen 0 1280x1024x24" \
            python3 "$root/tools/visual_regression.py" "${visual_args[@]}"
    else
        echo "== visual regression: SOFTWARE"
        SDL_AUDIO_DRIVER=dummy python3 "$root/tools/visual_regression.py" "${visual_args[@]}"
    fi
fi
