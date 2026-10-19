# ISSUE-116: a console encoding error in the historical XNA oracle

The repair campaign reran the singular-affine inversion on genuine Microsoft XNA
4.0 under the existing Wine/.NET prefix on 2026-10-19. CNA already agrees with XNA:
M22 is positive infinity, M42 is negative infinity, and the other fourteen
components are NaN. The historical demo incorrectly expected finite 8 and -8.

The original probe printed floats with `{R}` using the current culture. That
culture's infinity symbol is U+221E, but the console encoding is Windows-1252.
Its best-fit conversion produces ASCII `8`. Thus the old transcript was not
evidence of finite values. Explicit classifications, equality checks and IEEE-754
bits disambiguate the result; they must take precedence over formatted glyphs.

## Reproduction

`MatrixInvertOracle.cs` preserves the diagnostic probe. Compile it with `mcs
-sdk:4 -platform:x86` against the installed genuine `Microsoft.Xna.Framework.dll`,
write the executable to the existing ignored `build-probe` directory, and execute
it in a Wine prefix containing XNA 4.0/.NET. Use the private display runner, with
`WAYLAND_DISPLAY` empty for Wine, never the owner's desktop. The probe constructs
no graphics or audio objects. Microsoft assemblies are external prerequisites;
none are redistributed here.

Observed assembly identity:

```text
Microsoft.Xna.Framework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=842cf8be1de50553
SHA-256: 8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4
```

## Captured evidence

The initial repair probe (same operations and diagnostics as the preserved source)
exited 0 with empty stderr. Raw output remains in the ignored artifact
`build/linux-full/reports/repair-baseline-xna-matrix-bits.log`. Relevant stdout:

```text
culture=en-US outputCodePage=1252
positive-infinity-symbol UTF16: 221E
value bits: FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 7F800000 FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 FF800000 FFC00000 FFC00000
value M22PositiveInfinity=True M42NegativeInfinity=True M22Equals8=False M42EqualsMinus8=False
value original-format: M22=8 M42=-8
out bits: FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 7F800000 FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 FFC00000 FF800000 FFC00000 FFC00000
out M22PositiveInfinity=True M42NegativeInfinity=True M22Equals8=False M42EqualsMinus8=False
out original-format: M22=8 M42=-8
```

NaN classification is the portable expectation; the observed NaN payload/sign is
not a new cross-platform requirement. The signs of the two infinities are part
of this specific reproduction. No CNA runtime change is warranted for ISSUE-116.
