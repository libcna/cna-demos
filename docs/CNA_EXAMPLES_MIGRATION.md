# cna-examples migration audit

The checked-in JSON ledger accounts for the live legacy catalog at source level: **13 areas, 79 categories, 248/248 registered screens**. Each row records its source hash, APIs/behavior, lifecycle, semantic destination evidence, disposition and limitations.

## Dispositions

| Disposition | Screens |
|---|---:|
| `ALREADY_COVERED` | 183 |
| `COVERED_BY_MULTIPLE` | 39 |
| `EXTEND_EXISTING` | 10 |
| `INTENTIONALLY_NOT_MIGRATED` | 1 |
| `MANUAL_INTEGRATION` | 11 |
| `MIGRATE_NEW` | 3 |
| `OBSOLETE` | 1 |

Unaccounted screens: **0**.

## Added stable IDs

- `devices.camera.live_capture`
- `devices.desktop.file_dialog`
- `devices.desktop.message_box`
- `devices.desktop.system_tray`
- `devices.desktop.url_launcher`
- `devices.sensors.live_readings`
- `graphics.render_target.fade_transition`
- `input.keyboard.typematic_repeat`
- `input.mouse.capture`
- `input.mouse.global_position_and_warp`
- `media.media_player.unsupported_opus`

## Existing IDs extended

- `audio.sound_effect_instance.apply_3d`
- `content.xnb.same_assets`
- `graphics.render_target.render_target2d`
- `graphics.sprite_batch.draw_texture`
- `graphics.texture2d.set_data`
- `input.gamepad.device_info`
- `input.gamepad.state_values`
- `input.keyboard.get_state`
- `input.mouse.get_state`
- `input.touch.capabilities`
- `input.touch.touch_values`

## Evidence and retirement rule

Run `tools/cna_examples_migration_audit.py --legacy-root ../cna-examples --exe build/linux-full/bin/cna-demos` to re-enumerate the live legacy checkout, verify all source hashes and validate every destination ID. Full build, CTest, renderer-sweep and exact-visual results are recorded in `HANDOFF.md` after the final run.

No legacy application architecture, source file or asset was copied from cna-examples. The ledger derives titles/descriptions and source facts from that MIT-licensed checkout. New demos are fresh Ms-PL implementations. One real external MonoGame LZX fixture was imported from CNA's Ms-PL test corpus with exact provenance and SHA-256 in `assets/xnb/monogame/PROVENANCE.json`.

## Final validation

The warning-free Linux build validates 730/0 catalog entries. All 735 enabled CTests pass; the
eleven manual tests are disabled. Complete renderer sweeps are SOFTWARE 674 passed / 0 failed / 45
skipped / 11 not run and OPENGLES3 712 / 0 / 7 / 11. All six approved visual references match
with zero differing pixels. Comparison with the 719-demo pre-migration baseline confirms every
existing status and failed-assertion message/multiplicity is unchanged; only the eleven new IDs
and reviewed additional checks appear.

Real cameras, sensors, controllers and desktop host effects were not claimed as hardware-validated
without those devices and human interaction. Windows, macOS, Android, OPENGL33 and SDL_GPU were not
part of this Linux SOFTWARE/OPENGLES3 checkpoint. The manual demos retain explicit activation,
unsupported-platform handling and cleanup for those environments.
