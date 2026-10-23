# ISSUE-075: C offset receive must preserve bytes outside the packet

**Resolved:** CNA `40f286237cdbf7bdc05eb0a2ee9c9ee584eb677f`, validated below.

## Read-only preparation during the ISSUE-051 checkpoint

CNA base `e2074613f`, demos `77346b7`. No ISSUE-075 source/test change or new focused
repair claim yet. The full ISSUE-050 baseline reports `c_api.net.local_gamer_packets`
FAILED with its existing ISSUE-075 marker. The fresh complete ISSUE-051 SOFTWARE sweep
reproduces exactly one failure out of28 assertions: preserving the packet's surrounding bytes.

The offset C route allocates a zero-initialized vector of the caller's full capacity,
calls the canonical ReceiveData(vector, offset, sender), then copies **the whole vector**
back whenever a packet was received. The canonical method writes only the packet span.
The plain C receive route already copies only the returned byte count.

The public C header defines offset as the destination position where writing begins.
The existing external assertion uses a sentinel-filled eight-byte buffer, receives a
four-byte packet at offset2 and requires both the two-byte prefix and suffix to remain
unchanged. Its historical issue described an earlier twelve-byte reproducer; retain that
history. The current native NetSmoke checks only the payload span, so zeroed surrounding
bytes escape detection. Native LocalNetworkGamer tests separately protect rejection without
dequeue and canonical offset copying.

After the ISSUE-051 freeze, strengthen the native C receive test with nonzero sentinels,
including the rejected request and successful packet boundaries. Demonstrate the failure
against unchanged production code. Repair the C copy range locally; preserve all capacity,
offset, ownership, packet-consumption and sender-publication semantics. Then validate native
network/C controls and the original external scenario before retiring its marker.

This is a C adapter finding, not a new online/service or physical networking validation claim.
Do not edit executable inputs while the ISSUE-051 full sweeps are in progress.

## Native witness and candidate (after the051 checkpoint)

The051 checkpoint completed, with unchanged CTest statuses and all275 artifact hashes verified,
and was published as CNA `6767bda37`, demos `edc44ee`. The freeze ended before this work.
The native C NetSmoke now checks complete sentinel-filled buffers at offsets2,0 and4, including
rejection without mutation and an empty queue after each successful receive. Against unchanged
production code it fails specifically at the offset2 boundary assertion. The existing native
suite's preceding checks still pass (`repair-packet-range-before.log/xml`).

The candidate changes only the C adapter's memcpy to copy the received byte count at the offset.
Native network semantics, validation, dequeue and sender ownership remain unchanged. Build of
the C controls and CnaNetTests passes;21 nodiscard warnings are in unchanged NetworkSessionTests
exception assertions, while the strict C adapter/test build has no diagnostics. Full native
network validation passes549/549 in46.642s. CApi_NetSmoke and GeneratedRoutesSmoke both pass
on SOFTWARE and OPENGLES3. These C packet checks are renderer-independent; no GPU or external
service validation is inferred from their labels. The full existing consumer rebuild passes.
The unchanged external demo passes28/28 assertions under both renderer selections; exact names
and order match the051 checkpoint and precisely its one preservation failure disappears.
Only afterwards are marker075 and its obsolete restriction clause removed. Final build passes;
catalog719 has zero problems and changes only075 metadata. All275 consumer artifacts are frozen
in repair-packet-range-full-artifacts.sha256. Complete SOFTWARE finishes719 IDs:
647PASS/21FAIL/48SKIP/3NOTRUN. Exact comparison to051 changes only075 and removes its one failure
(40->39 failed assertions); every other status, marker, check count, result message and failure
multiplicity is unchanged. OPENGLES3 also completes719 IDs:
689PASS/20FAIL/7SKIP/3NOTRUN, with the same exclusive075 delta and39->38 failed assertions.
Final serial CTest (659.97s) matches every preceding051 status:735 registered,732 enabled,
678PASS/48SKIP/6 unchanged FAIL/3 disabled; DELTAS[]. All275 hashes match after both sweeps
and CTest. The executable freeze is released. No visual, sanitizer, full native corpus,
browser, external service or additional-platform validation is claimed for this adapter repair.
The six CTest residuals remain visible: c_api.content.content_manager,
c_api.graphics.resource_lifecycle, content.content_manager.read_asset_ownership,
content.content_manager.several_managers, content.content_manager.unload_disposes and
graphics.occlusion_query.visible_pixels. The shared tree includes inherited content/graphics
candidates; they are not part of this repair or its commit.

Against Complete1128, full SOFTWARE has93FAIL->PASS and OPENGLES3 has96FAIL->PASS.
The sole original PASS->FAIL remains c_api.graphics.resource_lifecycle, already present
before075. Runtime knownIssue markers change114->23 on SOFTWARE and115->22 on OPENGLES3.
Reports: `build/linux-full/reports/repair-packet-range-*`.
