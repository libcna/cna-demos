# Genuine XNA Dot/Cross product oracle

`ProductOracle.cs` passes 8,748 checks against Microsoft's actual XNA 4.0 Windows
x86 assembly (2026-10-19). Compile with `mcs -sdk:4 -platform:x86` referencing the
installed assembly, then run through CNA's private display wrapper, existing Wine
prefix and empty `WAYLAND_DISPLAY`. Microsoft binaries are not redistributed.
Logs: `build/linux-full/reports/repair-123-xna*`.

Eighteen vector samples form 324 pairs. Every pair checks Vector2/3/4 Dot value,
output and component-output aliases, and Vector3 Cross value/separate output/both
input aliases/source preservation. Exact bits are compared except for NaN payloads,
where only NaN classification is asserted. The samples include ordinary mixed
signs, all unit axes, both zero signs, huge cancelling products, tiny products,
maximum finite values, infinity and NaN.

The root cause is premature float product rounding before cancellation. Keeping
products and accumulation wide until the final float matches all measured cases.
Cross also needs XNA's original Y subtraction order; negating the reversed
subtraction, although algebraically equivalent, returns the wrong zero sign.
FNA's current Cross uses that reversed negation, so actual XNA is decisive here.
Output components must all be computed before writing an aliased destination.

Four new native tests reproduce four failures (929 passes/two unchanged missing
fixture skips/four failures). The strict C17 VectorSmoke fails only its new wide
product stage against the frozen external runtime. After the local repair all
933 native math tests pass, with the same two skips. All 566 registered vector and
Matrix/Plane/bounding tests pass (116.24s). Runtime/native commit: `0e7be08e0`.
Wider glTF/avatar validation exposed and preserved a catalog fingerprint regression;
see [the investigation](AVATAR_REGRESSION.md). Its compatibility repair is separate,
not a golden-value update. The complete corrected consumer run is in progress.
External binary validation and issue closure remain pending; original demo
assertions and knownIssue markers are unchanged at this checkpoint.
