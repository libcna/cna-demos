# ISSUE-020: compiled-media relocation audit

Read-only follow-up on CNA8d73b19c0 with the ISSUE-011 candidate present,
2026-10-21. This is not a new runtime pass or an implemented repair.

The retained reproduction is `../api-shape/CompiledSongRootProbe.cpp` and reports
`repair-compiled-song-root-before{,-build}.log`. It loads the same compiled song
under the content build's own root successfully, then fails when the manager is
rooted one directory higher. The issue matrix records that measured baseline;
the reproduction and asset bytes must remain unchanged when testing a repair.

## Current source evidence

- `modules/content/src/Xna/ContentMediaLoaders.cpp`,
  `ResolveMediaStreamPathEXT`: still calls
  `ResolveExistingAssetPath(BuildAssetPath(reference))`; it does not use the
  compiled asset's directory. Song and Video share this resolver.
- `modules/content/include/CNA/Content/Cnb/CnbMediaCodec.hpp`: the media schema is
  version1. Song/Video `streamReference` comes from the CNB `XREF` table.
- `CnbDocument.hpp::CnbExternalReference::logicalName` documents the XREF name as
  exactly what would be supplied to `ContentManager::Load`, relative and without
  `..` segments. Simply interpreting every existing XREF as asset-relative would
  change established version1 semantics.
- `CnbDocument.hpp::CnbMetadata::contentName` is explicitly optional provenance,
  diagnostic for every asset type, not a load-bearing identity. The media encoder
  accepts an empty contentName. Reconstructing a required relocation base solely
  from this metadata would therefore introduce a new, undocumented dependency
  and leave valid metadata-free files unresolved.
- `modules/content/src/Pipeline/XnbOutputContentPipeline.cpp` already has
  `MediaPathRelativeToAsset`: at compile time, it converts the root-relative media
  reference using the actual logical asset name. Both XNB Song and Video writers
  use it. The XNB runtime readers resolve their asset-relative media reference via
  `ResolveContainedPathRelativeToFile`, including containment checks.

## Required next evidence before selecting the repair

Expand the focused reproduction to both Song and Video, narrow/parent manager
roots, renamed or nested deployment directories, and conflicting files at the
old root. Protect existing version1 loads, metadata-free containers, external
reference inspection and containment/refusal behavior. Do not hide relocation
failure behind a narrower root or fallback to an unrelated same-named file.

Determine the smallest explicit, backward-compatible reference contract using
the existing codec/format policy. A versioned media-schema change may be needed;
it has not been selected or implemented. Do not repurpose optional diagnostic
metadata or silently reinterpret old bytes. Keep ISSUE-101's separate Video URI/
deferred-failure behavior out of this root-resolution repair unless shared
evidence proves the same cause.
