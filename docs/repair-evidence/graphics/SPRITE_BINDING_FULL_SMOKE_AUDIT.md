# Sprite binding full-smoke delta audit

## Scope and acceptance boundary

The existing full `repair-staging-lifecycle-full` checkpoint is the comparison
baseline, not an assumption that all interim candidates are accepted. Current
reports are `build/linux-full/reports/repair-sprite-binding-full-*`.
All719 permanent demo identities must be present on each renderer. Failed results
remain failed, including newly executed incorrect expectations; this audit does
not add knownIssue markers, suppress assertions or declare the campaign green.

SOFTWARE completed761459ms with635PASS/33FAIL/48SKIP/3NOT RUN. The preceding
checkpoint was628PASS/40FAIL/48SKIP/3NOT RUN: ten FAIL-to-PASS changes and three
PASS-to-FAIL changes. OPENGLES3 completed263603ms with675PASS/34FAIL/7SKIP/3NOT RUN,
versus667PASS/42FAIL/7SKIP/3NOT RUN: eleven FAIL-to-PASS and three PASS-to-FAIL.
Both inventories are complete. All frozen sources/artifacts remain unchanged;
peak372277248 bytes and every memory event counter is zero. The observer exits0
for complete observations, explicitly `accepted:false`, not a green-suite claim.
Visual comparisons independently
passed all six references with zero differing pixels/tolerance; every screenshot
was individually inspected.

## All16 SOFTWARE delta rows

| Demo | Delta | Evidence / disposition |
|---|---|---|
| `audio.dynamic_sound_effect_instance.streaming` | FAIL→PASS;038 marker removed | Validated038 repair `85a23569c`; both original disposed-stream refusals now pass. |
| `c_api.audio.spatial_state` | FAIL→PASS;073 marker removed | Validated073 repair `514a6a8c2`; failed Pan assignment no longer mutates public state. |
| `c_api.content.content_manager` | PASS→FAIL | Exactly two existing Effect-after-Unload/destruction assertions fail, matching the previous011 focused observation. Owner C lifetime contract remains unresolved; no waiver. |
| `c_api.graphics.effect_core` | PASS→FAIL | Previously compiled-off27-check refusal branch; now compiled-on34 checks, two incorrect expectations. See `COMPILED_EFFECT_C_EXPECTATIONS.md`; not corrected or accepted yet. |
| `c_api.graphics.pbr_effect_material_state` | FAIL→PASS;070 marker removed | Validated070 repair `1c8bbb6b8`; all four cloned-material assertions now pass. |
| `c_api.graphics.resource_lifecycle` | PASS→FAIL | Original callback-before-disposed assertion, already reproduced in082 focused comparison. C timing choice pending; no waiver. |
| `content.content_manager.read_asset_ownership` | FAIL→PASS | Pending011 candidate now records the disposable; this improvement alone does not close011. |
| `content.content_manager.several_managers` | FAIL→PASS | Pending content candidate enforces disposal refusal.010 remains without a final disposition until the complete related batch is accepted. |
| `content.content_manager.unload_disposes` | FAIL→PASS | Pending011 value-resource disposal candidate. No isolated closure inferred. |
| `graphics.shader_effect.clone_state` |055 marker removed; status unchanged | Validated055 repair `1ba26ed18`; SOFTWARE still skips unavailable custom effects, not a new runtime pass. |
| `graphics.sprite_batch.begin_end_rules` | Still FAIL; one added callback-state failure | Original open-batch-accounting failure remains. Added082 callback-before-disposed failure is identical to the four-renderer focused observation; neither hidden nor accepted. |
| `graphics.sprite_batch.immediate_mode.sampler_state` | FAIL→PASS;004 marker removed | Validated004 repair `9359a26b6`; original second-sprite linear-sampler assertion now passes. |
| `media.media_player.play_song` |5→6 checks; remains PASS | Demos commit `af5a986` added the title-relative Song path assertion for015; all old checks remain. Not a tolerance change or timing-based waiver. |
| `media.video.uri_and_metadata` | FAIL→PASS;101 marker removed | Validated Linux URI repair `abbd8a4ea`; all four prior failures gone. The separately documented native-Windows codec requirement remains pending. |
| `media.video_player.pause_and_loop` | FAIL→PASS;099 marker removed | Validated099 repair `abbd8a4ea`; all six previous guard/lifecycle failures gone. |
| `media.video_player.track_selection` | FAIL→PASS;100 marker removed | Validated100 repair `abbd8a4ea`; Video-level track override now passes. |

The raw comparison JSON also compares exact check counts, messages, knownIssue
values and failed assertion labels, not just status totals. All other703 rows
retain those compared fields. The table attributes cumulative differences since
the older full checkpoint; it does not count them as new ISSUE-003 repairs.

OPENGLES3 has the same16 changed identities. Fifteen complete delta objects are
byte-equivalent when serialized as JSON. Its sole different delta is
`graphics.shader_effect.clone_state`: instead of a preserved SOFTWARE skip, GLES
changes FAIL-to-PASS and removes all three old clone transform/pixel/disposal
failures, validating the previously committed055 repair. There is no additional
unattributed GLES-only delta.

## Remaining gate

The full scope is now inactive/dead and the original tool66351 terminated0.
Exercise FNA3D/WebGPU with the unchanged22 behavior assertions after only
broadening their runtime renderer gates; compile the changed D3D9 path where
available. Correct and validate the demonstrated C compiled-effect expectation
errors before claiming that external regression delta resolved. The separate
011/082 policy choices remain open regardless of003's results.
