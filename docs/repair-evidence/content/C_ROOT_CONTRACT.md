# ISSUE-039/103: owner-approved C RootDirectory contract correction

## Decision and scope

On 2026-10-20 the owner explicitly selected XNA/C++ compatibility for the C API:
reject root assignments while assets are cached, and correct the public C
documentation and demo. The previous C contract allowed root changes while
preserving cached assets. Its conflicting assertions were legitimate under that
old contract, not originally a demo mistake. Historical failure evidence remains
in `RUNTIME_OWNERSHIP_TRIAGE.md` and the full-regression reports.

The existing underlying repair is CNA `583af2a5e`. Both native property overloads
reject any assignment with loaded assets, including the identical value; genuine
XNA evidence and the FNA divergence are recorded in the ownership triage. Eight C
load routes already preserve exception identity through repair `68a3c7b1f`.
This follow-up changes the public C contract and its tests, not the runtime guard
or exception mapping.

## Preservation of the oracle

The C demo now checks same, different and empty-root assignment refusal, exact
`CNA_RESULT_INVALID_STATE` / state category / nonempty message, canonical
`System.InvalidOperationException`, and unchanged original root. A cached texture
is still loaded and checked after refusal. An explicit `Unload` precedes the
successful root assignment.

Every original pixel, font, effect, borrowed-manager, copied-string, new-load,
post-unload/destruction, error-identity and release-order assertion remains.
The original 54 check-label multiset is preserved after exactly two authorized
label/ordering corrections:

- Changing roots **without** unloading becomes changing roots **after** unloading.
- Loading the cached name after changing roots becomes loading it after
  **refused** root changes, before the explicit unload.

The retained cache still has its original 64x64 dimensions. The new root still
resolves the 128x32 sprite strip. Missing `textures/checker` after cache clearing
still must return IO with `ContentLoadException`. Thirteen additive checks bring
the total from 54 to 67. No tolerance, pixel, authored effect value, font metric
or lifetime check is relaxed.

The native `ContentSmoke` adds same/different/empty-root and structured-exception
checks. It temporarily removes its own fixture file, loads the cached texture,
and restores the file before checking success: this proves refusal did not
silently evict the cache. Manifest-root transitions explicitly unload first;
the original independently owned texture is read after each added unload.

## Focused evidence (full checkpoint still pending)

- Original strict-C `ContentSmoke`: exit 2 on SOFTWARE and OPENGLES3, reaching the
  manifest-root transition after all earlier content routes. Original logs retained.
- Updated strict-C `ContentSmoke`: exit 0 on both renderers, compiled C17 with
  `-Wall -Wextra -Wpedantic -Werror`.
- Separately named actual demo executable: **67/67 PASS** on both renderers.
  The 319 artifacts of the simultaneous ISSUE-016 sweep were not replaced.
- Read-only comparison with the original campaign baseline verifies all 54
  original labels (two explicit mappings above), exactly 13 additions, and no
  missing labels on either renderer. Reports:
  `repair-content-c-root-demo-oracle-audit.json`.
- Logs: `repair-content-c-root-native-{before,candidate}-*`,
  `repair-content-c-root-demo-*`.

No issue is closed or known-issue marker removed at this focused checkpoint.
The canonical external rebuild and full regression must follow the completion
of the frozen ISSUE-016 sweep. That sweep still records the previous C contract
residual; it is not retroactively relabeled green.

## Canonical follow-up

After ISSUE-016 completed, the full external rebuild and marker rebuild passed.
The actual registered C demo now passes67/67 on SOFTWARE and OPENGLES3, before
and after removal of marker103 in `aab4bf3`. Every label exactly matches the
already audited 67-check candidate. The C++ cache/root scenario retains5/5.
Contract/source correction commits are CNA `124281bff` and demos `dab9d4f`.
Reports: `repair-xml-substrate-focused{,-markers}-audit.json`.

The full XML/C-root combined checkpoint is running. It requires the old C-root
residual to disappear, not become an allowed difference. Historical039/103 remain
open until this combined regression completes.
