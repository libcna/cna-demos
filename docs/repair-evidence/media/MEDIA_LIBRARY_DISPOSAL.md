# ISSUE-098: disposed media library objects refuse their members

2026-10-23. Baseline CNA `290378cba`; repair CNA `45f704c04`.

## Reference

The decompiled Windows XNA 4.0 `Microsoft.Xna.Framework.Media` types settle the contract:

- `Song`, `Album`, `Artist`, `Genre`, `Playlist`, `Picture` and `PictureAlbum` call
  `ThrowIfDisposed()` (`ObjectDisposedException(GetType().Name, ...)`) first in every property,
  in `GetAlbumArt`/`GetThumbnail`/`GetImage`, in both `Equals` overloads, `GetHashCode` and
  `ToString`. Only `IsDisposed` and `Dispose` stay usable; `Dispose` is repeatable.
- The seven collections check disposal before `Count`, the indexer and `GetEnumerator`.
- `MediaLibrary` checks it before its nine properties, `GetPictureFromToken` and both
  `SavePicture` overloads. Its `Dispose` does not dispose the child views it handed out.
- `operator ==` is `object.Equals(first, second)`: the same object is equal without being asked,
  otherwise `first.Equals(second)`. `Dispose` releases the native handle, so a disposed argument
  compares equal only by reference.
- `MediaPlayer.Play(Song)` is `MediaQueue.Play`, which refuses a song without a valid handle
  with `InvalidOperationException` before the queue changes.

The demos' assertions (`media.song.properties`, `media.media_library.music_index`,
`media.media_library.save_picture`) were already written to this contract; none changed.

## Repair (CNA `45f704c04`)

A shared `CNA::Internal::Media::ThrowIfDisposed` guards every member listed above; the shared
collection base carries its collection's name and guards `Count`, the indexer (before its bounds
check) and enumeration. Equality is reference-first, and a disposed argument equals only itself.
CNA's own internal paths no longer go through the guarded getters: `MediaPlayer` copies queued
songs and counts plays through friendship, `MediaLibrary` appends to Saved Pictures through the
album's fields, and `Album` compares and hashes its artist by name. `MediaPlayer::Play(Song*)`
refuses a disposed song as XNA's queue does. The C API reports each refusal as
`CNA_RESULT_INVALID_STATE`; its two media smokes asserted the old live-after-dispose behaviour
and were corrected to the reference above.

## Validation

- Native `CnaMediaTests` 364/364, nine new tests. Against the unrepaired sources, eight fail
  with every refusal count at 0; the two that pass both before and after guard the internal
  paths. The library test saves below a private temporary root, so an unguarded `SavePicture`
  cannot write into CNA's checked-in fixtures. `build-asan` (ASan, UBSan, leaks) 364/364.
- `CnaContentTests` 2,065 PASS / 4 unchanged SKIP; all 132 C API tests; the C API coverage,
  limitations, release-gate, generated-route and export-count gates.
- Demos, with the tree rebuilt against the repair: the three demos pass on SOFTWARE and OPENGLES3
  with `Song 16/16`, `Album 13/13, Artist 8/8, Genre 8/8, Playlist 8/8`, the five collections
  `2/2`, `MediaLibrary 12/12`, `Picture 12/12, PictureAlbum 9/9` and both picture collections
  `2/2` (`repair-098-parts/`). The other 14 media demos are unchanged against the
  020/029/062 closure sweep. Their three `knownIssue` markers are removed; the three CTests and
  both catalog gates pass.
