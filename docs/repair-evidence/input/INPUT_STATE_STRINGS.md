# Genuine input-state string reference — 2026-10-23

The inherited `InputStateStringsOracle.cs` was reviewed and executed unchanged
against the installed genuine Microsoft XNA 4.0 assembly under Wine/.NET 4.
All 70 checks passed: all 32 mouse-button combinations, the default mouse state,
and connected/disconnected gamepad states, under both en-US and cs-CZ cultures.
The compiler used `/warnaserror+` and x86; no physical input device was required.

Evidence: `build/linux-full/reports/repair-inherited-input-reference-*`.
Compile: 2,847 ms; execution: 884 ms; memory peak: 385,355,776 bytes;
no memory limit or OOM event. This is Wine reference validation, not a native
Windows or C API pass. The oracle is retained to support the museum's open
MouseState/GamePadState string finding; this reference-only commit does not
claim that CNA's C adapter has been repaired.
