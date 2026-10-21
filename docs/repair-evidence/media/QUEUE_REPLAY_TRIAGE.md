# ISSUE-058: queue-owned Song replay

## Preserved historical reproducer

`MediaQueueReplayProbe.cpp` is byte-identical to the original
`build/linux-full/reports/checkpoint-473-hierarchy-next-queue-replay-probe.cpp`.
SHA256: `21b83a50399533beca4ff5739d601aeb15c42c91547303f5a811113e72b21406`.
Keep both the independently caller-owned control and the queue-owned input, the
128-character name and the original `assets/music/rising.wav`. The old result
is caller exit0 versus queue SIGABRT with ASan heap-use-after-free. Its recorded
build used an instrumented probe and an uninstrumented CNA library. Do not claim
it was a fully instrumented CNA run, or reuse the old `/tmp` build procedure.

## Current read-only source audit (2026-10-21)

On CNA8d73b19c0 with the uncommitted ISSUE-011 ownership candidate, the same
mechanism remains in `modules/media/src/Xna/MediaPlayer.cpp::Play(Song*)`:
`queue_.Clear()` destroys the argument's owning `unique_ptr` before both
`LoadSong(song)` and `PlaySong(song)`. Saving only the name before Clear would
leave the later playback use dangling. `previousSong` also points into the old
queue; decide its event-comparison result while those objects are still alive.

The collection overload has the same ownership boundary: SongCollection stores
non-owning Song pointers, so a collection built from queue entries can be
invalidated by Clear before iteration. Cover that alias case while fixing this
root cause, rather than leaving a second overload with the same lifetime error.
No current runtime reproduction or repair is claimed by this source audit.

## Reference evidence and repair constraints

- Local FNA at b35512475ed7980169574d2c40927381c1764d5a, `src/Media/MediaPlayer.cs`,
  has the same Clear/Load/Play order, but managed references keep the Song alive.
  A literal ordering port does not preserve that lifetime guarantee in C++.
- Local XNA decompilation at xna4-decomp bb5eb888053aca8009197552fe6c29add2c2ae2e,
  `reference/xna4/decompiled/windows/Microsoft.Xna.Framework/Microsoft.Xna.Framework.Media/`:
  MediaPlayer.Play delegates to MediaQueue.Play; the latter validates the Song
  and passes its native Handle to MediaQueue_PlaySong. The queue indexer creates
  a new managed Song wrapper. There is no managed destruction of the supplied
  wrapper before extracting its handle. This is source/IL-derived evidence,
  not an acoustic or genuine Windows playback validation.
- CNA's existing MediaPlayerTest.PlayEnqueuesADuplicateNotTheOriginalInstance
  protects duplication and independent play counts. Preserve that behavior and
  the caller-owned control. PlaySong writes duration and play count, so blindly
  replacing every caller input with a queue copy could alter observable mutation.
- Protect first/non-first queue aliases, collection aliases and existing event
  delivery. Preserve exceptions and unrelated ordering; do not fold separate
  null/empty-collection, ActiveSongIndex or playback-service differences into
  this memory-safety repair without independent evidence.

## Next executable evidence

After the serial ISSUE-011 external gate, compile the unchanged original probe
inside the existing build-probe directory against the existing CNA build-asan
closure. Verify instrumentation of MediaPlayer, MediaQueue and Song, source and
library hashes, and run caller/queue in separate processes with dummy audio,
ASan/leaks/UBSan and no suppressions. Record the actual diagnostic before any
repair. Then add native regressions and repair the smallest lifetime boundary.
