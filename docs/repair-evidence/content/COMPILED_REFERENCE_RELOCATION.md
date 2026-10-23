# ISSUE-020: compiled references follow a relocated content build

2026-10-23. Baseline CNA `fe2eb8f33`. Continues
[COMPILED_MEDIA_RELOCATION_TRIAGE.md](COMPILED_MEDIA_RELOCATION_TRIAGE.md) and the retained
reproduction `../api-shape/CompiledSongRootProbe.cpp`.

## Reference

XNA resolves every external reference from the referring asset's own directory:
`ContentReader.GetPathToReference` combines the asset name's directory with the reference, and
`GetAbsolutePathToReference` (used by `SongReader` and `VideoReader`) then roots that under the
manager's root. A content build may therefore sit in any subdirectory of the manager's root.

A `.cnb` names a reference (`XREF`) relative to the root it was compiled against, and its `CMET`
chunk records the asset's own compiled name (`contentName`), which the pipeline always writes
(`EncodeSongToCnb(song, logicalName)` and the model/video encoders).

## Decision

The triage warned against making optional metadata load-bearing for files that lack it and
against silently reinterpreting existing bytes. The repair does neither: XREF bytes keep their
meaning, and the only new reading is the one the triage asked to protect old loads from.

- When the name an asset is loaded by ends in its `contentName` at a `/` boundary, the remaining
  prefix is where the build sits; it is prepended to each reference. This is exactly XNA's result.
- When the names are equal -- every load that worked before -- the prefix is empty and nothing
  changes. Without a `contentName`, or with one the loaded name does not end in, the reference
  stays relative to the manager's root, as before.
- A same-named file at the manager's root is not taken for the asset's stream; a test proves it.
- References stay validated by `CnbLogicalNameProblem()` and `ContentManager`'s containment, so a
  prefix cannot lead outside the root.

No format version changed. `docs/cnb-format.md` (`XREF`, `CMET`) documents the rule, replacing
the recompile that living-room-simulator R-28 needed.

## Repair (CNA `3da9ea645`)

`detail::CnbReferenceNameEXT` in `BuiltinContentLoaders.cpp`, applied by the Song and Video
loaders to their stream reference and by the model loader to every texture and external-effect
reference of schema 1 and 2 models.

## Validation

- Five new `CnbRelocatedReferenceTest` cases: 1 PASS (the unchanged root-relative control) /
  4 FAIL before -- song and video "not found beside it", V1/V2 model textures not found -- and
  5/5 after, including the decoy file at the manager's root. CNB codec and model V2 manager tests
  52/52; the whole content suite 2,062 PASS / 4 unchanged SKIP (2,057 + 5), through the private
  runner from the source directory.
- Demos: `media.song.compiled_song` and `media.video_player.play_video` keep their original
  narrow-root assertions and each add one: a manager rooted above the media build loads the
  same song/video.
- With the demos tree rebuilt, the `audio`, `content` (73), `media` (17), `graphics.model` (30) and
  `c_api` (177) categories on SOFTWARE and OPENGLES3 match the ISSUE-010 closure sweep per ID
  except the intended check-count increases (`repair-batch3-focused-parts/`); the two media demos
  pass with 4 and 5 checks.
- Complete checkpoint with the demos tree rebuilt from the tree committed as CNA `8842b9a50`:
  719-ID sweeps SOFTWARE 654/14/48/3 and OPENGLES3 696/13/7/3 against the ISSUE-010 closure change
  only `sensor_lifecycle` FAIL-to-PASS and the intended check counts (`compiled_song` 3 -> 4,
  `play_video` 4 -> 5, `from_stream` 8 -> 9); serial CTest 684 PASS/48 SKIP/0 FAIL/3 disabled
  (`repair-batch3-closure-{full,ctest}*`). Closed with CNA `da7388fa9`.
