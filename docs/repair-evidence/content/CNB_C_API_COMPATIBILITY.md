# ISSUE-110: preserve the separate C ABI contract

2026-10-20. The first full boundary SOFTWARE sweep exposed an actual regression:
`c_api.content.cnb_foundations` changes PASS to FAIL at its original stored-codec
assertion. The other status change is the intended105 improvement. Full counts are
624PASS/44FAIL/48SKIP/three manual, not the expected625/43. Category c_api exits1;
the strict driver stops before GLES and final CTest. This failed run remains under
`repair-content-boundaries-full-*`; no successful final checkpoint exists yet.

This is not an incorrect C demo expectation. `docs/c-api/CNB.md` explicitly lists
the None codec's disregard of both declared sizes as an established ABI contract;
native `CnbSmoke.c` also requires it. The public C header's general safety wording
did not state that exception, and source/docs incorrectly claimed universal parity
with the C++ helper. The C API's existing compressed-codec checks already distinguish
None deliberately. Preserving that documented C behavior is within this repair;
there is no reason to weaken the corrected C++ allocation/size contract.

The adapter candidate passes actual stored length as both size/ceiling to the strict
C++ helper for None only. It still uses the same helper/copy implementation, retains
all pointer/capacity/error ordering, and does not duplicate decompression. Compressed
codecs keep declared values and their exact error mapping. C documentation now states
the difference explicitly, including that callers wanting a None allocation ceiling
must check stored_byte_count themselves. The C ABI and demo assertion are unchanged.

Before proof: unchanged native pure-C CnbSmoke compiled with C17/Wall/Wextra/Werror
against the frozen external boundary libraries exits8 at the original None assertion.
Exact audit errors[]; all319 external hashes unchanged. Candidate compiles the actual
CnaCApiCnb.cpp with canonical strict flags, substitutes only its object in a separate
copy of the existing C API link closure, and checks linked library identity. Normal
extended CnbSmoke passes. Added UINT64_MAX/zero-input cases protect ignoring declared
sizes without attempting huge allocations. No original assertion was removed.
Sanitizer candidate also passes. The unchanged external C demo separately passes
all25 original ordered assertions against both normal and sanitized adapters.
Native REQUIRE count grows203 to207 (including the unchanged macro definition);
all original checks remain. Build/linked-identity/exact-label audits and319 artifact
hashes pass. Only the adapter TU and C fixture are instrumented, not the entire
linked dependency closure. Repair/tests/documentation committed CNA `c57ecab8c`.

Native canonical CNA currently has CNA_BUILD_C_API=OFF; the external demo tree has
it ON. Editing these C-only files does not change the three frozen native root-batch
sources or running native build. External artifacts remain unchanged. Next perform
a canonical external build after013/014 native proof,
and rerun all affected C scenarios/full renderer gates. Do not close110 based only
on the C++ focused passes. Reports `repair-cnb-c-api-compatibility-{before,normal,sanitizer}-*`.
