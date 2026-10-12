# CNA issues found by cna-demos

One purpose of cna-demos is to expose real CNA problems. A demo that reveals one does **not**
work around it silently: the problem is minimised, recorded here, and the demo either keeps
failing visibly or is marked with the issue ID in its `restrictions` metadata.

Each entry records the affected CNA area, the demo(s) involved, reproduction steps, observed
and expected behaviour, the environment, and the investigation status. IDs are permanent
(`CNA-DEMOS-ISSUE-NNN`); a fixed issue stays listed under "Resolved".

CNA revision used unless stated otherwise: `10a4ef1cb4cd832f761536d77588000104947f8d`
(CNA 0.1.0, "chore(release): prepare v0.1.0").

---

## Open

### CNA-DEMOS-ISSUE-057 -- Content staging scavenger warns during normal concurrent lifecycle

- **Area:** CNA content tools, `tools/common/CnaContentStaging.hpp`, consumed by `cna-content`
  and the content-project build. Build-time diagnostic race; no runtime demo knownIssue marker.
- **Original observation:** full470-query, demo source `a699a2d`, completed all seven scheduled
  build operations, then `tools/validate.sh` stopped at its unchanged warning gate before
  metadata validation, CTest or smoke. Content-project output included
  `error: warning: content staging scavenger: cna_content_stage_v1_306788_18dd6c565a238173_0:
  owner metadata is absent or is not a regular file`. Both wrapper exits were1.
  The actual candidate under `/tmp/cna_content_staging_v1` was absent when later inspected.
- **Independent reproduction:** compile `/tmp/cna-staging-concurrency-probe.cpp` with GCC14.2,
  C++23, `-O2 -pthread -I../cna/tools/common`. It directly uses the unchanged CNA tool header.
  One thread creates/destroys1,000 `ContentBuildStagingDirectory` objects in an initially empty
  private parent; another repeatedly calls `ScavengeContentStagingDirectories` there. No malformed
  staging fixture is fabricated and the production staging root is never scanned by the probe.
  Run it only with a fresh private parent; the archived Python driver creates one per trial.
- **Observed:** all three bounded trials reproduce the exact missing-owner-metadata diagnostic,
  respectively3,175/3,079/3,089 times. Each completes1,000 owners with zero owner errors, zero
  reported removals and an empty private parent afterwards. Other observed race diagnostics
  concern vanished directory validation and incomplete/unreadable metadata. Each probe exits1
  deliberately for the reproduced condition, with empty stderr. A sequential control creates
  another1,000 owners and scans while each complete owner is alive and after destruction:
  zero scan errors, zero diagnostics, final parent empty, exit0.
- **Expected:** overlapping normal CNA staging lifecycles should not produce malformed-owner
  warnings merely because another builder is publishing or removing its private staging state.
  Conservative cleanup must remain safe; suspicious unrelated directories should still be
  preserved and reported as appropriate. A warning-free successful consumer build should not
  depend on the scheduling of independent content-tool invocations.
- **Mechanism and limits:** enumeration, directory validation, metadata stat/size/read and lease
  handling are separate filesystem operations. Creation exposes the candidate before its lease
  and atomic metadata publication; destruction removes the metadata/directory. The scanner reads
  metadata before checking age or claiming the lease. The isolated probe establishes diagnostics
  during legitimate concurrent lifecycle operations. The original build's precise interleaving,
  including creation versus teardown, was not observed. No asset corruption or unsafe removal
  is claimed. The older checkpoint437 refusal involved the related vanished-directory message;
  its original logs remain separate and are not retroactively a reproduction of this exact run.
- **Evidence/environment:** Linux x86_64, GCC14.2, CNA tools observed at clean
  `eff31310179168fb4f47258e6226a5234c2140d0`; relevant header snapshots and hashes are archived.
  Original build/refusal/exits and post-refusal selected artifacts:
  `build/linux-full/reports/checkpoint-470-query-initial-staging-refusal-*`.
  Probe sources, executables, original streams, compiler logs, private-root identities and review:
  `build/linux-full/reports/checkpoint-470-query-staging-probe-*`.
  All35selected shipped binaries and four existing assets remained unchanged through the probes.
  This is a source/selected-artifact observation, not an atomic full dependency snapshot.
- **Recurrence on471-punctual:** initial focused build22225, demo HEADc173c11 plus the new
  uncommitted punctual receiver, compiles/links33scheduled operations but exits1 at the
  unchanged diagnostic gate (CMake exit0, gate1). The content-project step reports the same
  missing-owner condition for `cna_content_stage_v1_477930_18dd5445705a3d88_0`. No new demo
  metadata/native/CTest/smoke run follows that refusal. Both relevant tool headers are byte-
  identical to the original independent probe; CNA remains cleaneff3131017 and all3909watched
  build inputs stay unchanged. The exact candidate and its owner file are absent at later
  inspection; creation versus teardown is still unobserved. Original build, source/input
  observations, headers and post-build artifact hashes are preserved separately under
  `build/linux-full/reports/checkpoint-471-punctual-initial-staging-refusal-*`. No directory
  is removed, warning suppressed or CNA source changed. The fresh watched consistency retry
  completes7scheduled operations with CMake0/gate0 and all3909inputs unchanged. Focused
  SW/GL and27targeted CTests then pass (GL8pixels, exactSWskip, actual image inspected).
  This permits native validation but does not resolve the staging race.
- **Handling:** keep the warning gate and original failure visible, retry the unchanged documented
  full validation after recording the defect, and retain this issue even if scheduling permits a
  clean retry. The eleven OcclusionQuery controls already pass on both renderers independently.
  No sibling source was changed, unknown staging directory deleted or upstream message sent.

### CNA-DEMOS-ISSUE-056 -- File stream position becomes -1 after a short EOF read

- **Area:** sharp-runtime `System::IO::FileStream::Read/getPositionProperty`, exposed through
  CNA StorageContainer file streams and `cna_storage_stream_get_position`.
- **Reproduction:** the extension of `c_api.storage.save_game` is independently
  compiled as `/tmp/cna-storage-eof-probe` using existing C target inputs. It seeks to byte16
  of the original24-byte save and requests12bytes into physically allocated guarded storage.
  A second read at EOF and a position query follow. A separate `/tmp/cna-canonical-eof-probe`
  uses direct FileStream with a unique4-byte temporary file, requesting6bytes, then querying
  position after both short and empty reads. Sources/builders/runners are archived below.
- **Observed:** C returns SUCCESS with position-1 instead of24. Exactly one of31child checks
  fails; all23original controls plus seven additions pass, including exact tail bytes, untouched
  guards, empty EOF output, rewind/full reread, parent release guards and stale-handle output.
  Both requested renderer environments agree. The direct FileStream probe reads4bytes and
  reports-1after both reads instead of4, while every data/guard/rewind/reread control passes.
  Both programs return1 specifically for the defect; stderr is empty. No Game/rendering claim.
- **Expected:** an open file stream remains positioned at the actual end after a short/empty
  EOF read; its position getter must not publish an internal std::fstream failure sentinel as
  a successful byte offset. The C adapter should retain the same canonical stream semantics.
- **Investigation:** FileStream::Read uses fstream.read/gcount and leaves EOF/fail state set;
  getPositionProperty directly calls tellg. CNA's generated C getter forwards that value with
  SUCCESS. Direct reproduction isolates the underlying stream path from the C adapter and
  rules out the demo's source/output buffers. Seek from Begin clears native state and restores
  successful reading; that positive control does not weaken the failing position assertion.
- **Environment/evidence:** Linux x86_64/GCC14.2, existing linux-full runtime artifacts from
  complete466-transfer (CNA59598bfac), observed clean sharp-runtime c20373bae. Later source
  review observes CNA714f9f76a; no atomic input/source snapshot is claimed. All35selected
  shipped binaries remain unchanged through probe builds/runs. Immutable stdout/stderr/JSON,
  direct/C sources, helper scripts, source hashes and exact23+8child review are in
  `build/linux-full/reports/checkpoint-466-transfer-eof-*`. No sibling source changes or
  upstream messages. The catalog extension now retains the failing assertion with knownIssue metadata; its native
  build, both focused selections and relevant CTest WILL_FAIL contract pass as documented
  in checkpoint-467-focused-* (30positive child checks and only the confirmed position failure). The later read-only extension preserves this failure and adds seven positive controls
  (37passes/one failure,38total) on both requested selections, in checkpoint-467-scene-focused-*.
  Reproduce via `cna-demos c_api.storage.save_game --automated --renderer SOFTWARE` after building.

### CNA-DEMOS-ISSUE-001 -- `libcna.so` fails to link with CMake 3.28 (duplicate symbols)

- **Area:** build system -- CNA's shared runtime library (`CNA_SHARED_LIBRARY`,
  `cmake/SharedRuntimeLibrary.cmake`).
- **Demos:** none specifically; it blocks building any consumer.
- **Environment:** Ubuntu 24.04, CMake 3.28.3 (distribution package), GCC 14.2, lld (CNA's
  default linker choice), multi-renderer build
  `OPENGLES3;OPENGL33;SDL_RENDERER;VULKAN;SDL_GPU;SOFTWARE;HEADLESS;STUB`,
  `CNA_CNAEXT=ON`, `CNA_DEVICES=ON`, CNA added with `add_subdirectory()`.
- **Reproduction:**
  1. Configure cna-demos with `/usr/bin/cmake` 3.28.3 and `-DCNA_SHARED_LIBRARY=ON`
     (CNA's own default on Linux with CMake >= 3.27).
  2. `cmake --build <dir> --target cna-demos`.
- **Observed:** the `CNA/libcna.so` link fails with hundreds of
  `ld.lld: error: duplicate symbol: ...` (first ones: `CNA::Internal::Xnb::XnbAssemblyQualifier`,
  `CNA::Internal::Audio::DecodeMsAdpcm`, `CNA::Internal::Media::MediaLibraryIndex::...`,
  `Microsoft::Xna::Framework::Audio::SoundBank::...`). The generated link line lists
  `libcna_runtime.a` and `libcna_content.a` twice inside the first `--whole-archive` group, and
  `libcna_media.a`/`libcna_audio.a` and the input/graphics/renderer archives twice inside later
  groups -- the repetition CMake emits for static libraries in a dependency cycle. Inside
  `--whole-archive` every repetition re-adds every object, hence the duplicates.
- **Expected:** CNA builds on every CMake version its configuration admits. `CNA_SHARED_LIBRARY`
  defaults to ON for CMake >= 3.27, and `cmake_minimum_required` is 3.20.
- **Cross-check:** the identical configuration with CMake 4.4.4 (`pip install cmake`) generates a
  link line with every archive exactly once and links successfully.
- **Likely cause (not verified):** CMake 3.29 changed link-line de-duplication (policy CMP0156),
  so 3.27/3.28 still repeat cyclic static libraries. Which exact CMake version is the first good
  one, and whether CNA's default option set (CNAEXT/DEVICES off, single renderer) also
  reproduces on 3.28, has not been tested.
- **Workaround in cna-demos:** `cmake/CnaDemosCna.cmake` sets `CNA_SHARED_LIBRARY=OFF` (static
  CNA) when CMake is older than 3.29 and the user did not choose a value. With CMake >= 3.29 CNA's
  default is kept. This is a build-mode choice, not a change to CNA.
- **Suggested upstream fix:** raise the `CNA_SHARED_LIBRARY` support gate in CNA's
  `CMakeLists.txt` to the first CMake that de-duplicates correctly, or de-duplicate the
  WHOLE_ARCHIVE list in `cmake/SharedRuntimeLibrary.cmake` so cycle repetition cannot occur.
- **Status:** documented; not yet reported upstream.

### CNA-DEMOS-ISSUE-002 -- `Curve::Evaluate` with Step keys compares the absolute position with 1.0

- **Area:** math -- `Microsoft::Xna::Framework::Curve` (`modules/math/src/Curve.cpp`, the
  `CurveContinuity::Step` branch of the in-range evaluation, around line 288).
- **Demo:** `math.curve.step_continuity` (marked `knownIssue`).
- **Reproduction:**
  ```cpp
  Curve c;
  c.getKeysProperty().Add(CurveKey(1.0f, 10.0f, 0.0f, 0.0f, CurveContinuity::Step));
  c.getKeysProperty().Add(CurveKey(2.0f, 20.0f, 0.0f, 0.0f, CurveContinuity::Step));
  c.Evaluate(1.5f);   // returns 20
  ```
- **Observed:** 20 -- the *next* key's value. The implementation returns `next.Value` whenever
  `position >= 1.0f` and `prev.Value` otherwise, i.e. it compares the absolute curve position with
  1.0 instead of the position's fraction within the segment. Segments that lie entirely below
  position 1.0 behave as documented (keys at 0 and 0.5: `Evaluate(0.25)` returns the first key's
  value), which is why the problem only appears for curves whose keys extend past 1.0.
- **Expected:** 10. The XNA 4.0 documentation of `CurveContinuity.Step` (xna4-spec,
  `Microsoft.Xna.Framework/CurveContinuity.xml`): *"Interpolation cannot be used between this
  CurveKey and the next. Specifying a position between the two points returns this point."*
- **Environment:** platform-independent (pure math); CNA 0.1.0.
- **Notes:** the code has the shape of FNA's `Curve.GetCurvePosition`, which CNA ports; FNA's source
  was not available in this environment to confirm it carries the same test. Whether genuine XNA 4.0
  (its IL) behaves like its documentation has **not** been verified here either; CNA's policy is
  that XNA's measured behaviour wins over FNA. CNA has no unit test that evaluates a Step curve.
- **Status:** open; minimal reproduction above; not reported upstream yet.

### CNA-DEMOS-ISSUE-003 -- `SpriteBatch` does not leave its texture in `GraphicsDevice::Textures[0]`

- **Area:** graphics -- `Microsoft::Xna::Framework::Graphics::SpriteBatch`
  (`modules/graphics/src/Xna/SpriteBatch.cpp`): sprites are submitted through the renderer's own
  sprite path, which never assigns the device's `TextureCollection`.
- **Demo:** `graphics.texture2d.set_data_rectangle` follows the XNA rule (it unsets `Textures[0]`
  before `SetData`) and checks that `SetData` on a bound texture throws -- which CNA implements --
  but it cannot show the situation XNA programmers actually run into.
- **Reproduction:**
  ```cpp
  spriteBatch.Begin();
  spriteBatch.Draw(canvas, Vector2::Zero, Color::White);
  spriteBatch.End();
  device.getTexturesProperty()[0];             // nullptr in CNA
  canvas.SetData(pixels.data(), count);       // succeeds in CNA
  ```
- **Observed:** `Textures[0]` is null after `End` (SOFTWARE and OPENGLES3), so a following
  `SetData` on the last drawn texture succeeds.
- **Expected:** XNA 4.0's SpriteBatch assigns `GraphicsDevice.Textures[0]` for every batch it
  draws (CNA's own comment in `SpriteBatch.cpp` says "XNA assigns Textures[0] after the pass"), so
  after `End` the last sprite texture is still set on the device and `SetData` on it throws
  `InvalidOperationException` ("You may not call SetData on a resource while it is actively set on
  the GraphicsDevice") until the program unsets it. CNA's `Texture::ThrowIfDataTransferResourceInUseEXT`
  implements that throw for textures set explicitly.
- **Impact:** low. XNA programs keep working on CNA (they already unset the texture). Programs
  written against CNA can call `SetData` where XNA would throw, and code that reads
  `Textures[0]` after drawing sprites sees null.
- **Environment:** CNA 0.1.0, Linux, SOFTWARE and OPENGLES3 renderers.
- **Status:** open; observed behaviour above; not reported upstream yet.

### CNA-DEMOS-ISSUE-004 -- Immediate `SpriteBatch` ignores sampler states set between `Draw` calls

- **Area:** graphics -- `SpriteBatch` with `SpriteSortMode::Immediate`
  (`modules/graphics/src/Xna/SpriteBatch.cpp`, `Begin`): the batch's `SamplerState` is passed to the
  sprite renderer once, in `Begin` (`renderer_->SetSamplerState(...)`), and `flushSingle` draws
  each sprite with it without consulting `GraphicsDevice::SamplerStates`.
- **Demo:** `graphics.sprite_batch.immediate_mode.sampler_state` (marked `knownIssue`); its sibling
  `graphics.sprite_batch.immediate_mode.blend_state` passes.
- **Reproduction:**
  ```cpp
  spriteBatch.Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::PointClamp,
                    nullptr, nullptr);
  spriteBatch.Draw(checker, Rectangle(20, 40, 370, 370), Color::White);
  device.getSamplerStatesProperty()(0, SamplerState::LinearClamp);
  spriteBatch.Draw(checker, Rectangle(410, 40, 370, 370), Color::White);   // still point-sampled
  spriteBatch.End();
  ```
- **Observed:** both sprites are point-sampled (only the checker's two colours along a row) on
  SOFTWARE and OPENGLES3. A `BlendState` set on the device between the same two calls *is*
  honoured, so the problem is specific to sampler state.
- **Expected:** XNA 4.0, `SpriteSortMode.Immediate` (xna4-spec `SpriteSortMode.xml`): *"Begin will
  apply new graphics device settings, and sprites will be drawn within each Draw call."* Each Draw
  therefore draws with the device's current state, so the second sprite is filtered linearly --
  changing states or effect parameters between sprites is what Immediate mode is for.
- **Environment:** CNA 0.1.0, Linux, SOFTWARE and OPENGLES3 renderers.
- **Status:** open; minimal reproduction above; not reported upstream yet.

### CNA-DEMOS-ISSUE-005 -- `Color` is polymorphic, so XNA-style custom vertex structs silently break

- **Area:** math/graphics -- `Microsoft::Xna::Framework::Color` derives from
  `PackedVector::IPackedVectorT<UInt32>`, whose `getPackedValueProperty`/`setPackedValueProperty`
  are virtual, so a `Color` object is 16 bytes (vtable pointer + packed value) instead of XNA's
  4-byte value. The other packed vectors share the design. `GraphicsDevice::DrawUserPrimitives`
  with a `VertexDeclaration` passes a caller's non-polymorphic struct through as raw bytes and
  only rejects a struct that is itself polymorphic -- not one that *contains* a polymorphic member.
- **Demo:** `graphics.vertex_declaration.custom_vertex` stores the colour as its packed
  `std::uint32_t` and says why (`restrictions`).
- **Reproduction:**
  ```cpp
  struct MyVertex { Vector3 Position; float Glow; Color Tint; };   // XNA: 20 bytes; CNA: 40
  VertexDeclaration decl{
      VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
      VertexElement(12, VertexElementFormat::Single, VertexElementUsage::TextureCoordinate, 1),
      VertexElement(offsetof(MyVertex, Tint), VertexElementFormat::Color, VertexElementUsage::Color, 0)};
  device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices, 0, count, decl);
  ```
- **Observed:** `sizeof(MyVertex)` is 40 and `offsetof(MyVertex, Tint)` 16, while the declaration's
  computed stride is 20; the draw reads every vertex at the wrong stride (and the colour element
  would read the vtable pointer) and renders garbage -- no exception, on SOFTWARE and OPENGLES3.
- **Expected:** XNA's `Color` is a 4-byte struct (its `PackedValue` is the whole object), and
  embedding it in custom vertex structures is the standard XNA pattern (vertex colours alongside
  positions and normals). Porting such code to CNA should work, or fail loudly.
- **Possible directions (for CNA's maintainers):** make `Color` a 4-byte standard-layout type that
  satisfies the packed-vector contract without virtual dispatch; or, at least, reject vertex
  types that are not trivially copyable in the raw `DrawUserPrimitives`/`VertexBuffer::SetData`
  templates (`std::is_trivially_copyable_v` is false for a struct containing a `Color`), turning
  silent garbage into a compile-time error.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux x86_64 (sizes are ABI-dependent but never 4).
- **Status:** open; reproduced as above; not reported upstream yet.

### CNA-DEMOS-ISSUE-006 -- No way to ask whether the renderer applies `MipMapLevelOfDetailBias`

- **Area:** graphics -- `SamplerState::MipMapLevelOfDetailBias` on the OpenGL ES renderers
  (`OPENGLES3`, `WEBGL2`), and the capability API (`CNA::RendererFeature`,
  `GraphicsDevice::SupportsCapability`).
- **Demo:** `graphics.sampler_state.mip_level_control.lod_bias` (skips on GL ES renderers, detected
  through `RendererFeature::ShaderDialectGlslEs` as a proxy).
- **Observed:** on OPENGLES3 a LOD bias of +1 has no effect (copies drawn at 1x, 1/2 and 1/4 size
  read mip levels 0, 1, 2 instead of 1, 2, 3); no exception and no log message. SOFTWARE applies it.
- **Documented by CNA:** `docs/sampler-state-support.md` -- "OpenGL ES has no
  `GL_TEXTURE_LOD_BIAS`", and the parity ledger: "LOD bias is unavailable on GL ES exactly as in
  FNA3D". So the behaviour itself is a known platform limitation, not a defect.
- **The gap:** XNA always honours the bias (Direct3D 9 sampler state), and CNA exposes many
  per-renderer facts through `RendererFeature`/`GraphicsCapability`, but none for this one, so a
  program cannot adapt (for example by biasing in its own shader) without hard-coding renderer
  names.
- **Suggestion:** a `RendererFeature` (or `GraphicsCapability`) entry for sampler LOD bias,
  reported `Unsupported` on GL ES; optionally a one-time warning when a non-zero bias is set there.
- **Same gap, second case:** `PbrEffect` (CNAEXT) is documented in its header as rendered "as an
  untextured/unlit fallback" on SOFTWARE, HEADLESS and the native 2D renderer, and no
  `RendererFeature` reports it either; `graphics.pbr_effect.metallic_roughness` skips by renderer
  name there.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-007 -- `ColorMatrixEffect` is silently ignored by GPU renderers

- **Area:** graphics -- `ColorMatrixEffect` (CNAEXT) passed to `SpriteBatch::Begin` on a GPU
  renderer; EasyGL's `EasyGLSpriteBatchRenderer::SetCustomEffect` stores it and the sprite flush
  draws with the stock sprite shader.
- **Demo:** `graphics.color_matrix_effect.sepia_and_invert` (runs on SOFTWARE; skips elsewhere).
- **Reproduction:** on OPENGLES3,
  ```cpp
  ColorMatrixEffect sepia(device);
  sepia.SetColorMatrix(/* sepia */);
  spriteBatch.Begin(SpriteSortMode::Deferred, &BlendState::Opaque, nullptr, nullptr, nullptr, &sepia);
  spriteBatch.Draw(texture, destination, Color::White);
  spriteBatch.End();
  ```
- **Observed:** no exception; the sprite is drawn with its original colours.
- **Expected:** the header documents ColorMatrixEffect as "unsupported by GPU renderers unless they
  explicitly add the same contract", and its own rationale is that a renderer should not "silently
  ignore" what it cannot do. CNA refuses other unsupported requests explicitly (e.g. compiled
  effects throw `NotSupportedException` where `CompiledEffects` is false). A refusal (or a
  capability flag) would let programs fall back, e.g. to a `ShaderEffect`.
- **Environment:** CNA 0.1.0, Linux, OPENGLES3 (EasyGL) under Xvfb.
- **Status:** open; not reported upstream yet.

### CNA-DEMOS-ISSUE-008 -- Public headers leak unprefixed macros (`YES`, `NO`, `nothing`, ...)

- **Area:** sharp-runtime (CNA's C#-runtime dependency) -- `SharpRuntime/Prop.hpp` defines
  object-like macros with ordinary names at global scope: `YES`, `NO`, `nothing`, `static0`,
  `static1`, `ref0`, `ref1`, `getter0`, `getter1`, `setter0`, `setter1`, `member0`, `member1`,
  `constret0`, `constret1`, `constmet0`, `constmet1`, `IF_0`, `IF_1`, `DDATA`, `DGETTER`, ...
  CNA's public `Microsoft/Xna/Framework/Content/ContentManager.hpp` includes it, so every
  translation unit of a game (through `Game.hpp`) receives them.
- **Demo:** found while writing `input.keyboard.state_values`, whose local variable `nothing`
  (an empty `KeyboardState`) expanded to nothing; the variable was renamed.
- **Reproduction:**
  ```cpp
  #include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
  enum class Answer { YES, NO };        // error: expected identifier before numeric constant
  int main() { int nothing = 0; return nothing; }   // error: expected unqualified-id
  ```
- **Observed:** ordinary identifiers in user code fail to compile with confusing diagnostics
  (GCC 14: "declaration does not declare anything", "expected identifier before numeric
  constant"). An identifier used where an empty token sequence is still valid C++ would change
  meaning silently.
- **Expected:** a library's public headers should not reserve common words; XNA (C#) has no
  macros at all, so ported code freely uses names such as `YES`/`NO` in enums.
- **Suggestion:** prefix the helper macros (e.g. `SHARP_PROP_YES`) or keep them in an internal
  header that public headers do not include, and `#undef` the helpers after use where possible.
- **Environment:** CNA 0.1.0 with sharp-runtime 0.1.0 (`c20373b`), GCC 14.2, Linux.
- **Status:** open (low impact, compile-time only); not reported upstream yet.

### CNA-DEMOS-ISSUE-009 -- `HapticDevice`'s default constructor cannot be used outside CNA

- **Area:** input -- `CNA::Input::HapticDevice` (CNAEXT, `CNA/Input/HapticDevice.hpp`). The class
  holds `std::unique_ptr<CNA::Platform::IPlatformHapticDevice>`, an incomplete type in the public
  header, and declares `HapticDevice() = default;` inline. The destructor and the move operations
  are defined in `HapticDevice.cpp`, where the type is complete; the default constructor is not.
- **Demos:** `input.haptics.rumble`, `input.haptics.effects` hold the device in
  `std::optional<HapticDevice>` (filled from `Haptics::OpenEXT`) and say why in a comment.
- **Reproduction:**
  ```cpp
  #include "CNA/Input/HapticDevice.hpp"
  int main() { CNA::Input::HapticDevice device; return device.IsOpenEXT() ? 1 : 0; }
  ```
- **Observed:** GCC 14: `invalid application of 'sizeof' to incomplete type
  'CNA::Platform::IPlatformHapticDevice'` (`~unique_ptr` is needed by the inline constructor); the
  same for a class with a `HapticDevice` member. The header documents the constructor as
  "Constructs a closed haptic device".
- **Expected:** a closed `HapticDevice` can be created by user code (for example as a member that
  `Haptics::OpenEXT` later fills), like `MouseCursor`, whose default constructor is out of line.
- **Suggested fix (low risk):** declare `HapticDevice();` in the header and define
  `HapticDevice::HapticDevice() = default;` in `HapticDevice.cpp`.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open; not reported upstream yet.

### CNA-DEMOS-ISSUE-010 -- Storage reports errors with C++ standard exceptions, not XNA's types

- **Area:** storage -- `StorageContainer` and `StorageDevice::DeleteContainer`. Argument errors
  (empty names, paths that leave the container) throw `std::invalid_argument`; a failing
  filesystem operation such as `DeleteDirectory` on a non-empty directory lets
  `std::filesystem::filesystem_error` escape. Neither derives from `System::Exception`.
  (Other parts of CNA, for example `StorageContainer::OpenFile` with `FileMode::CreateNew`, do
  throw `System::IO::IOException`; across CNA's modules both conventions occur.)
- **Demos:** `storage.storage_container.path_rules` and `storage.storage_container.directories`
  catch `std::exception`, the common base, and say why.
- **Reproduction:**
  ```cpp
  container->CreateDirectory("replays");
  (void)container->CreateFile("replays/race1.rpl");
  try { container->DeleteDirectory("replays"); }
  catch (const System::IO::IOException&) { /* XNA's type -- not reached */ }
  // terminates with std::filesystem::filesystem_error
  try { (void)container->FileExists(""); }
  catch (const System::ArgumentException&) { /* not reached: std::invalid_argument */ }
  ```
- **Observed:** `filesystem error: cannot remove: Directory not empty [<absolute path>]` and
  `Parameter file must contain a value.` as `std::invalid_argument`.
- **Expected:** XNA reports these as `IOException` and `ArgumentNullException`/`ArgumentException`
  (FNA: `Directory.Delete`, `ArgumentNullException`), and CNA mirrors the .NET hierarchy in
  sharp-runtime (`System::IO::IOException`, `System::ArgumentException`). Code ported from XNA
  that catches those types misses these errors. The containment refusals are a deliberate CNA
  hardening (documented in the source); their exception type could still be `ArgumentException`.
- **Same pattern in `ContentManager`:** `Load<T>` on a disposed manager throws
  `std::runtime_error("ContentManager has been disposed.")`, while XNA (and CNA's own
  `ReadAsset<T>`) throws `ObjectDisposedException`.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-011 -- `ContentManager::Unload` does not dispose loaded assets

- **Area:** content -- `ContentManager::Unload()` (`ContentManager.cpp`) only clears the cache
  (`loadedAssets_.clear()`). Its header says it "unloads all cached assets and frees the associated
  resources". A texture handle the game still holds stays alive and usable.
- **Demo:** `content.content_manager.unload_disposes` (knownIssue; expected failure).
  `content.content_manager.cache_and_unload` shows the part that works: after `Unload`, the next
  `Load` reads the file again.
- **Reproduction:**
  ```cpp
  ContentManager level(&game.getServicesProperty(), "assets");
  const Texture2D checker = level.Load<Texture2D>("textures/checker");
  level.Unload();
  checker.getIsDisposedProperty();   // false in CNA; true in XNA
  ```
- **Expected (XNA 4.0):** "Disposes all data that was loaded by this ContentManager" -- every
  `IDisposable` asset it loaded is disposed, whether or not the game still references it. XNA
  games rely on it to release a level's textures and sounds in one call; in CNA the memory is only
  released when the game drops every handle (CNA's `Texture2D` copies are handles to one texture).
  A related consequence of the handle model: two `Load` calls return two handles that share the
  texture but not `GraphicsResource::Name`/`Tag`, where XNA returns the same object.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux, SOFTWARE and OPENGLES3.
- **Status:** open; not reported upstream yet.

- **ReadAsset default-recorder extension (438 batch):** `content.content_manager.read_asset_ownership`
  now reads the same original packed XNB with an empty recorder after its successful delegated
  lifetime controls. The nested shared_ptr<Tiles> implements IDisposable and initially contains
  the correct tile bytes; holding it while calling Unload shows that Dispose was not invoked.
  The public ReadAsset header explicitly promises manager recording when the callback is empty.
  ContentReader::RecordDisposable's supplied-callback branch works, but its manager fallback is
  still only a deferred TODO. This is a different route to the same missing disposal behavior,
  not a failure of the caller-owned callback workflow or a new issue identifier.
- **Extension evidence:** `/tmp/cna-438-ownership-{SOFTWARE,OPENGLES3}.{json,log}` in fresh
  Linux GCC 14.2 processes without DISPLAY/WAYLAND_DISPLAY: 16 checks, fifteen passing, one
  initially unlabeled default-recorder failure. All original fourteen checks remain. Explicit
  cleanup follows the assertion and cannot make it pass. No CNA source was edited. Final labeled evidence
  `/tmp/cna-438-ownership-final-{SOFTWARE,OPENGLES3}.{json,log}` retains the same one failure
  with ISSUE-011. Its WILL_FAIL CTest and the other 20 focused tests pass. CNA observed at
  ea41d758d9689cda348887a92a93bbe188864e8b; inspected content modules have no uncommitted
  edits. Full 438 at 973a63a now retains exactly this one new failure on both selections;
  original checkpoint-437 comparisons retain every older failed assertion (29/30 -> 30/31).
  All 451 enabled CTests pass, and the labeled complete evidence/reviews are archived under
  reports/checkpoint-438-*. No graphics/genuine-XNA execution claim.

### CNA-DEMOS-ISSUE-012 -- `ContentManager::Load<T>` never calls an `OpenStream` override

- **Area:** content -- `ContentManager::Load<T>` (header template) resolves `.xnb`, `.cnb` and
  registered loose-file readers directly on the filesystem (`TryReadAssetBytes`). Only the protected
  `ReadAsset<T>` goes through the virtual `OpenStream`, and `Load<T>` (not virtual in C++) never
  calls it. `ResourceContentManager` overrides only `OpenStream`, so its `Load<T>` does not read
  resources either.
- **Demos:** `content.content_manager.open_stream_override` (knownIssue; expected failure): a
  derived manager serving an XNB texture from memory; `ReadAsset` works, `Load` does not.
  `content.content_manager.resource_content_manager` (knownIssue; expected failure): a
  `ResourceContentManager` over a resource family holding a Curve `.xnb`; `Load<Curve>("Bounce")`
  throws "Cannot open file: Bounce" -- it looks for a file, not the resource.
- **Reproduction:**
  ```cpp
  class PackContentManager : public ContentManager {
      std::unique_ptr<System::IO::Stream> OpenStream(const std::string& name) override; // memory
  };
  pack.Load<Texture2D>("wheel");   // ContentLoadException: Failed to load image: pack/wheel
                                   // (OpenStream is not called)
  ```
- **Expected (XNA 4.0):** `Load<T>` -> `ReadAsset<T>` -> `OpenStream`, so overriding `OpenStream`
  is the documented way to load from pack files, archives, encrypted or network content, and
  `ResourceContentManager.Load` reads embedded resources.
- **Suggestion:** let `Load<T>` fall back to (or, for derived managers, prefer) `ReadAsset<T>`
  through `OpenStream` when the filesystem tiers find nothing; the tier ladder could stay as the
  base class's `OpenStream` behaviour.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open; not reported upstream yet.

### CNA-DEMOS-ISSUE-013 -- The content manifest lists the content pipeline's bookkeeping files

- **Area:** content -- `ContentManager::GetContentManifest()` (CNAEXT) reports
  `.cna-content` (`.lock`) and `.cna-content-manifest` (`.json`), the files `cna-content` writes
  into every output directory, as if they were assets.
- **Demo:** `content.content_manager.manifest` prints them (and says so in a comment).
- **Expected:** only loadable assets; tools that walk the manifest (mod loaders, content checks)
  otherwise have to know and filter CNA's internal file names.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-014 -- Before the platform exists, the content root is the working directory

- **Area:** content -- `ContentManager::TitleRootedDirectory()` resolves a relative
  `RootDirectory` against the title path, else against the current platform's base path -- but
  only if a platform already exists (`HasCurrentPlatform()`); otherwise against the working
  directory. Nothing creates the platform until some call needs it (the first `Load` does, as a
  side effect).
- **Found by:** `content.content_manager.manifest`, first written as a console demo, which passed
  when started from `bin/` and failed under CTest (another working directory). It is now a Game
  demo, the context where a platform always exists.
- **Reproduction (console program started from another directory):**
  ```cpp
  ContentManager content(nullptr, "assets");
  content.GetContentManifest().size();      // 0 -- scanned <cwd>/assets
  (void)content.Load<Curve>("curves/bounce"); // works; creates the platform
  content.RefreshContentManifest();
  content.GetContentManifest().size();      // 21 -- now <executable dir>/assets
  ```
- **Expected:** the same root for every call, independent of call order -- XNA resolves content
  under `TitleContainer`/`TitleLocation.Path` (CNA's own comment says so) regardless of what ran
  before. Tools and tests that use a `ContentManager` without a `Game` are affected.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-015 -- `Song::FromUri` resolves a relative path against the working directory

- **Area:** media -- `Song::FromUri` (`Song.cpp`) uses a scheme-less path as it is, so a relative
  one is opened relative to the process's working directory. Its comment notes that FNA combines
  a relative URI with `TitleLocation.Path` but says CNA "has no such content-root concept for
  Song" -- yet CNA has `TitleLocation::Path()` (and resolves `ContentManager` and `TitleContainer`
  paths against the title directory).
- **Demos:** every `media.*` demo builds an absolute path from `TitleLocation::Path()` and says why.
- **Reproduction (program started from another directory than its executable's):**
  ```cpp
  Song::FromUri("Rising", "assets/music/rising.wav");
  // throws: Could not find file 'assets/music/rising.wav'.
  ```
- **Expected:** the same resolution as FNA (and as CNA's own content paths): relative to the title
  location, so a game finds its music however it was started (desktop shortcut, launcher, IDE).
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-016 -- Linking `CNA_GamerServices` after another library duplicates CNA in the executable

- **Area:** build system -- `CNA_GamerServices` and `CNA_Net` with a shared CNA
  (`CNA_SHARED_LIBRARY`, `cmake/SharedRuntimeLibrary.cmake`).
- **Demos:** every `gamer_services.*` demo (the first one to exist exposed it); the fix is in
  cna-demos' top-level `CMakeLists.txt`, with a comment pointing here.
- **Environment:** CMake 4.4 (pip), GCC 14, lld, Linux, `CNA_SHARED_LIBRARY=ON` (CNA's default here).
- **Reproduction:** an executable links a static library `framework` that links `CNA`, plus an
  OBJECT library that links `framework` and `CNA::GamerServices` (the usual shape of a project
  with more than one target). Run any program that adds a `GamerServicesComponent` and exits.
- **Observed:** `double free or corruption (!prev)` at exit, in the destructor of
  `SamplerState::PointWrap` (`GraphicsResource::~GraphicsResource`), run once by the executable
  and once by `libcna.so`. `CNA_GamerServices` is not inside `libcna.so` and links CNA's module
  archives (`cna_runtime`, `cna_storage`, ...) directly; CMake places it ahead of `libcna.so` on the
  link line, so the linker takes CNA's graphics, runtime and storage objects from those archives:
  the executable carries a second CNA (3,225 `Graphics` symbols in `cna-demos`) whose static objects
  libcna.so's own references then bind to. Naming the `CNA` umbrella first does not help -- it is
  an INTERFACE library and CMake orders its `libcna.so` after the archives' dependency closure.
  CNA's examples avoid it only because each links exactly `CNA CNA_GamerServices` (their CMake
  comment says the duplicate runtime "aborted every one of these demos at device creation").
- **Workaround in cna-demos:** the executable names the `libcna.so` file itself first on its link
  line (`$<TARGET_FILE:cna_shared>`), so the archives behind it contribute only what libcna.so
  lacks (0 duplicated code symbols afterwards).
- **Expected:** a consumer cannot get two copies of CNA by linking CNA's own targets in any order.
  Possible fixes: link the `CNA` umbrella (not the module archives) from `CNA_GamerServices` and
  `CNA_Net` when CNA is shared, or build them into `libcna.so` too.
- **Additional reproduction (2026-10-11):** `content.custom.xna_pipeline_components` linked
  `CNA::ContentPipelineBuild` and `cna_demos_framework` through the standalone helper on
  Linux/GCC 14/CMake 3.31.6. Its seven pipeline checks passed, then exit aborted in
  TouchPanel::gestures_' deque destructor. GDB shows executable-owned destruction reached
  from libcna.so's global finalizers; nm confirmed TouchPanel::GetState and gestures_ were
  defined in the executable as well. Evidence: `/tmp/cna-xna-pipeline-{gdb,unbuffered}.log`
  and `/tmp/cna-xna-pipeline-duplicate-symbols-{before,after}.txt`.
  The standalone C++ helper now uses the same shared-library-first workaround as the browser.
  The duplicate definitions disappear and both renderer-selected tool runs exit successfully.
  This extends the existing issue to ContentPipelineBuild; no new issue number or expected
  failure hides it. The C API linking path is unchanged.
- **Status:** open (medium: a silent ODR violation that crashes at exit); not reported upstream yet.

### CNA-DEMOS-ISSUE-017 -- `Achievement::EarnedOnline` is always true

- **Area:** gamer services -- `Achievement` (`Achievement.cpp` sets `earnedOnline_(true)` in its
  only constructor; nothing changes it).
- **Demos:** `gamer_services.achievement.award` (expected failure).
- **Reproduction:** sign in a local offline profile (no CNA service), award an achievement from
  the title's offline catalog, read it back with `GetAchievements`:
  ```cpp
  player->AwardAchievement("first_steps");
  // Find(player->GetAchievements(), "first_steps")->getEarnedOnlineProperty() == true
  ```
  Unearned achievements report `true` as well.
- **Expected:** XNA documents `EarnedOnline` as whether the achievement was earned while signed
  in to the online service; an achievement earned by a local offline profile was not (and an
  unearned one was not earned at all). The offline store already records how it was earned
  (`SaveEarnedAchievementEXT`), so the value is available.
- **Environment:** CNA 0.1.0, Linux, offline gamer services.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-018 -- System Link socket failures are `std::runtime_error`, not `NetworkNotAvailableException`

- **Area:** net -- `NetworkSession::Create` / `BeginCreate` of a `SystemLink` session
  (`ENetDiscoveryService.cpp`, `ENetHostHandle.cpp` throw `std::runtime_error`; nothing in
  `NetworkSession.cpp` throws `NetworkNotAvailableException`).
- **Demos:** `net.packet.send_and_receive` catches `NetworkNotAvailableException` to report SKIPPED
  (its declared `Need::Network`); with CNA's current exception it reports FAILED instead.
- **Reproduction:** occupy UDP port 61190 (CNA's discovery port) with a socket bound without
  `SO_REUSEADDR`, then create a System Link session:
  ```python
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("0.0.0.0", 61190))
  ```
  ```cpp
  NetworkSession::Create(NetworkSessionType::SystemLink, 2, 4);
  // throws std::runtime_error: "Failed to bind discovery UDP socket."
  ```
- **Expected:** XNA reports a missing or unusable network with
  `NetworkNotAvailableException` (and other transport failures with `NetworkException`), which games
  catch to show "network not available"; a `std::runtime_error` escapes such handlers.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low impact; same family as ISSUE-010); not reported upstream yet.

### CNA-DEMOS-ISSUE-019 -- `Game::ResetElapsedTime` is ignored with a fixed time step

- **Area:** runtime -- `Game::ResetElapsedTime` (`Game.cpp`) sets its flag only when
  `IsFixedTimeStep` is false, and only the variable-step path reads it. The rule is FNA's (its
  `ResetElapsedTime` says the call is "totally ignored" for a fixed time step).
- **Demos:** `game.time_step.reset_elapsed_time` (expected failure).
- **Reproduction:** in a default (fixed 60 Hz) game, stall one `Update` for 200 ms and call
  `ResetElapsedTime()`; count the Updates before the next `Draw`.
- **Observed:** 12 catch-up Updates run before the next Draw (the stall is made up step by step).
- **Expected:** XNA documents the method for exactly this case (recovering from a slow frame
  whose elapsed time is too large to be useful), and Microsoft's XNA GameStateManagement sample
  calls it from its loading screen -- in a game with the default fixed time step -- so that the
  timing does not try to catch up after the long loading frame (paraphrased from the sample's
  comment). The next frame should run one ordinary step.
- **Environment:** CNA 0.1.0, Linux, SOFTWARE and OPENGLES3.
- **Status:** open (medium for games with loading screens: the world jumps ahead after a load);
  not reported upstream yet.

### CNA-DEMOS-ISSUE-020 -- A compiled Song/Video stream is looked up from the manager's root, not beside the asset

- **Area:** content -- `ResolveMediaStreamPathEXT` (`ContentManager.cpp`) resolves a compiled
  `.cnb` Song's or Video's stream reference with `BuildAssetPath(reference)`, i.e. relative to
  the ContentManager's RootDirectory. The reference itself is written relative to the root of
  the content build that compiled it (`music/theme.ogg` for the asset `music/theme`).
- **Demos:** `media.song.compiled_song`, `media.video_player.play_video` (both pass by giving the
  media content build, output to `assets/media`, a ContentManager rooted there).
- **Reproduction:** build a content project with a song `music/theme.ogg` into `assets/media`,
  then `ContentManager(nullptr, "assets").Load<Song>("media/music/theme")`.
- **Observed:** `ContentLoadException: 'media/music/theme' streams 'music/theme.ogg', which was not
  found beside it.` -- the file is beside it (`assets/media/music/theme.ogg`).
- **Expected:** XNA's compiled song/video stores the stream path relative to the `.xnb` and it is
  resolved from the asset's own directory, so a content build can sit anywhere below a manager's
  root (as the message's "beside it" says). Resolving from the asset's directory (or storing the
  reference relative to the asset) would make the load work from any root.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low: one ContentManager per content build avoids it); not reported upstream
  yet.

### CNA-DEMOS-ISSUE-021 -- EasyGL samples a rendered RenderTargetCube face upside down

- **Area:** graphics, EasyGL renderer (OPENGLES3, OPENGL33). EasyGL keeps a render target's GL
  texels bottom-up and corrects 2D sampling per texture unit, but `BindDrawParams` leaves cube maps
  out of that correction ("REMED-GFX-137 owns rendered cube faces", `EasyGLRenderer.cpp`), so a
  face drawn into with `SetRenderTarget(RenderTargetCube*, CubeMapFace)` is sampled flipped
  vertically by `EnvironmentMapEffect`.
- **Demos:** `graphics.render_target.render_target_cube` (expected failure on OPENGLES3 and
  OPENGL33 only).
- **Reproduction:** fill the +Z face of a `TextureCube` (SetData) and of a `RenderTargetCube`
  (SpriteBatch into the face) with the same image -- top half red, bottom half black; both read
  back identical with `GetData`. Draw a mirror sphere (EnvironmentMapEffect, FresnelFactor 0) with
  each as its EnvironmentMap, seen from +Z.
- **Observed:** on OPENGLES3 and OPENGL33 the `SetData` cube shows red above the sphere's centre and
  the rendered cube below it. In the demo, cubes orbiting above the sphere are reflected in its
  lower half (0-30 reflected pixels above the centre, ~360 below).
- **Expected:** both cubes sampled the same way, as SOFTWARE, VULKAN and SDL_GPU do (red above the
  centre; ~450 reflected cube pixels above the centre, 0 below): a dynamic environment map must
  reflect the scene upright.
- **Environment:** CNA 0.1.0, Linux, Mesa llvmpipe under Xvfb (GL ES 3.2 and GL 3.3 core).
- **Status:** open (medium for dynamic reflections on the GL renderers); not reported upstream yet.

### CNA-DEMOS-ISSUE-022 -- A compiled glTF with an embedded texture refers to a file the build never writes

- **Area:** content pipeline -- `CNA.GltfImporter` + `CNA.ModelProcessor` (CNB). A glTF whose
  base-colour image is embedded (a PNG in a `bufferView`) compiles into a model that refers to
  `<name>_tex0.png`; the asset's manifest entry lists it under `runtimeReferences`
  (`"textured_crate_tex0.png"`) with no `deploymentFiles`, and no such file is produced.
- **Demos:** `graphics.model.textured_gltf` (expected failure; it draws the run-time import of the
  same file, which works).
- **Reproduction:** compile `content/models/textured_crate.gltf` (tools/gltf_models.py
  `textured_crate`) with `cna_add_content`, then `Content.Load<Model>("models/textured_crate")`.
- **Observed:** `ContentManager: could not load asset 'textured_crate_tex0.png'. ---> Failed to
  load image: <bin>/assets/textured_crate_tex0.png`.
- **Expected:** the model loads with its texture, as the run-time import of the same `.gltf` does
  (`Load<Model>` of a loose `.gltf`): the image is either compiled into the model or written beside
  it where the reference points.
- **Environment:** CNA 0.1.0, Linux, `cna_add_content` (CNB).
- **Status:** open (high for textured glTF models through the pipeline); not reported upstream yet.

### CNA-DEMOS-ISSUE-023 -- A compiled glTF model loses its cameras

- **Area:** content pipeline -- CNB model encoding. `Model::getCamerasEXTProperty()` is filled by
  the run-time glTF import (two cameras for the demo's scene) but is empty for the same scene
  compiled by `cna_add_content`; `CnbModelData` has no camera data.
- **Demos:** `graphics.model.gltf_cameras` (expected failure; it uses the run-time import).
- **Reproduction:** compile `content/models/camera_rig.gltf` (a perspective and an orthographic
  camera) and compare `Load<Model>("models/camera_rig")` with the loose copy
  `Load<Model>("models/loose_camera_rig")`.
- **Observed:** 0 cameras from the compiled model, 2 from the loose file.
- **Expected:** the same cameras either way (CNA documents `ModelCameraEXT` as imported from the
  glTF scene, `GLTF-317`), or a build warning that the pipeline drops them.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (medium); not reported upstream yet.

### CNA-DEMOS-ISSUE-024 -- `BoundingBox::ToString` and `Ray::ToString` print doubled braces

- **Area:** math -- `BoundingBox.cpp` and `Ray.cpp` write the literal text `{{Min:` ... `}}` and
  `{{Position:` ... `}}`; CNA's own `BoundingBoxTests` pins FNA's string. XNA builds both with
  `string.Format(culture, "{{Min:{0} Max:{1}}}", ...)` / `"{{Position:{0} Direction:{1}}}"`, where
  `{{`/`}}` are format escapes that print one brace each. CNA's `BoundingSphere`, `Plane` and the
  other math types already print single braces.
- **Demos:** `math.formatting.to_string` (expected failure); `design.type_converter.string_round_trip`
  shows the same text through the converters.
- **Reproduction:** `BoundingBox(-Vector3::One, Vector3::One).ToString()`,
  `Ray(Vector3::Zero, Vector3::Forward).ToString()`.
- **Observed:** `{{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}}`,
  `{{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}}`.
- **Expected:** `{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}`,
  `{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}`.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low: text only); not reported upstream yet.

---

### CNA-DEMOS-ISSUE-025 -- Offline, leaderboard columns written after the rating are lost

- **Area:** gamer services -- `LeaderboardWriter::GetLeaderboard` (offline: no CNA service) saves
  an entry to the local store only from the hook `LeaderboardEntry::setRatingProperty` runs
  (`SetOnRatingChangedHookEXT`); writing `LeaderboardEntry::Columns` saves nothing. In XNA the
  row -- rating and columns -- is one write, whatever order the game sets them in.
- **Demos:** `gamer_services.leaderboard.paging` sets its columns before the rating, with a comment
  naming this issue.
- **Reproduction:** in a `LocalWithLeaderboards` game, `entry->setRatingProperty(4200)`, then
  `entry->getColumnsProperty().SetValue("Result", LeaderboardOutcome::Win)`; end the game and
  `LeaderboardReader::Read(board, 0, 10)`.
- **Observed:** the entry has rating 4200 and no columns: `GetValueOutcome("Result")` throws
  `std::out_of_range` (`map::at`).
- **Expected:** the entry reads back `Result = Win`.
- **Environment:** CNA 0.1.0, Linux, offline gamer services.
- **Status:** open; not reported upstream yet.

---

### CNA-DEMOS-ISSUE-026 -- On Linux, a process that searched before a host started does not find it

- **Area:** net -- `ENetDiscoveryService` (System Link discovery). Every process binds one UDP
  socket to port 61190 with `SO_REUSEADDR` (Linux; `SO_REUSEPORT` is set on Apple only) and keeps
  it for the life of the process; a search sends its query by broadcast and to 127.0.0.1, and the
  host answers by unicast to the querying address, port 61190. On Linux a unicast datagram to a
  port shared through `SO_REUSEADDR` reaches only the most recently bound socket, while broadcasts
  reach all of them. When the host's socket is newer than the searcher's, the host's own socket
  receives the answer, and the searcher never does.
- **Demos:** `net.network_session.host_leaves` starts a new player process for each round, with a
  comment naming this issue; `net.network_session.find_and_join` hosts before it starts the
  joiner, which is the order that works.
- **Reproduction:** process A calls `NetworkSession::Find(NetworkSessionType::SystemLink, 1, {})`
  (nothing is hosted yet); then process B on the same machine creates a SystemLink session; A
  calls `Find` again, repeatedly.
- **Observed:** A's searches return no sessions for as long as A runs (B receives its own
  answers). Started after B, a fresh process finds B's session at once. The same delivery rule is
  shown without CNA by two Python UDP sockets bound to one port with `SO_REUSEADDR`.
- **Expected:** A finds B's session -- a lobby screen that searched before a friend on the same PC
  started hosting must find the game (on separate machines the addresses differ and it works).
- **Environment:** CNA 0.1.0, Linux 6.18; affects processes on one machine only.
- **Status:** open; not reported upstream yet. A searcher could take its replies on a socket of its
  own (as the Apple code path's `SearchSocket` does) instead of the shared port.

---

### CNA-DEMOS-ISSUE-027 -- SOFTWARE does not clamp source values above 1 before blending into an 8-bit target

- **Area:** graphics, SOFTWARE renderer -- blending a texel sampled from a floating-point texture
  (`HdrBlendable`) into a `Color` target. Direct3D (XNA) and OpenGL clamp the source colour, the
  destination and the blend factors to [0, 1] before the blend equation when the target is a
  fixed-point format; SOFTWARE uses the raw value, so with `BlendState::AlphaBlend` a source alpha
  of 2 gives the destination a factor `1 - 2 = -1`.
- **Demos:** none fails; found while writing `graphics.render_target.hdr_blendable`, which shows its
  targets with `BlendState::Opaque` (their alpha is a sum of additive draws, not a coverage).
- **Reproduction:** an `HdrBlendable` target filled additively with three `(0.8, 0.5, 0.2, 1)`
  sprites over a cleared black (alpha 1) target, so that alpha reaches 2, 3 and 4 where one, two
  and three sprites overlap; draw it with `SpriteBatch` (default `AlphaBlend`, tint 0.4) over a
  back buffer cleared to `(24, 24, 32)`.
- **Observed (SOFTWARE):** red 58 / 115 / 173 where one / two / three sprites overlap -- 24 below
  OPENGLES3 per unit of alpha above 1 (the background subtracted); blue 0.
- **Expected (OPENGLES3, and XNA):** red 82 / 163 / 245, blue 20 / 41 / 61: the source clamped to 1
  (alpha 1, so the background is replaced).
- **Environment:** CNA 0.1.0, Linux, SOFTWARE renderer.
- **Status:** open (low: only float sources with values above 1 are affected); not reported
  upstream yet.

---

### CNA-DEMOS-ISSUE-028 -- A streaming WaveBank opens a relative path from the working directory

- **Area:** audio -- `WaveBank(AudioEngine*, streamingWaveBankFilename, offset, packetSize)` opens
  the file with a plain file stream (`ParseXwbStreamingHeader`), so a relative name is resolved
  against the working directory; the settings file (`AudioEngine`), the in-memory `WaveBank` and
  the `SoundBank` are read through the title container, relative to the title location. CNA
  follows FNA here (FNA's streaming constructor calls `FAudio_fopen` on the name), so the same
  relative name works for an in-memory bank and fails for a streaming one unless the game runs
  from its own directory.
- **Demos:** `audio.xact.streaming_wave_bank` builds the path from `TitleLocation::Path()`, with a
  comment naming this issue (as ISSUE-015 does for `Song::FromUri`).
- **Reproduction:** run from another directory than the executable's (CTest runs in the build
  tree): `WaveBank(&engine, "assets/xact/music.xwb", 0, 16)` beside
  `AudioEngine("assets/xact/demo.xgs")`.
- **Observed:** `[WaveBank] XWB streaming parse error (assets/xact/music.xwb): XWB: cannot open file
  for streaming`; the bank never becomes prepared and its cues find no wave.
- **Expected:** the streaming bank is found where the other XACT files of the title are.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low; follows FNA); not reported upstream yet.

---

### CNA-DEMOS-ISSUE-029 -- `SoundEffect::FromStream` takes a `std::istream`, not a `System::IO::Stream`

- **Area:** audio API -- XNA's `SoundEffect.FromStream(Stream)` takes the same `Stream` type as
  `Texture2D.FromStream` and `TitleContainer.OpenStream`; CNA's `Texture2D::FromStream` and
  `TitleContainer::OpenStream` use `System::IO::Stream`, but `SoundEffect::FromStream` takes a
  `std::istream&`, and sharp-runtime has no adaptor between the two. A game cannot hand the stream
  `TitleContainer::OpenStream` returns to `SoundEffect::FromStream` (XNA's usual pattern) without
  copying it into a `std::istringstream` first.
- **Demos:** `audio.sound_effect.from_stream` opens the WAV as a `std::ifstream` from
  `TitleLocation::Path()`, with a comment naming this issue.
- **Reproduction:** `SoundEffect::FromStream(*TitleContainer::OpenStream("assets/sounds/tone_440.wav"))`.
- **Observed:** does not compile (no conversion from `System::IO::Stream` to `std::istream&`).
- **Expected:** an overload taking `System::IO::Stream&` (the `std::istream` one can stay as an
  extension).
- **Environment:** CNA 0.1.0.
- **Status:** open (API shape); not reported upstream yet.

---

### CNA-DEMOS-ISSUE-030 -- `SimulatedPacketLoss` drops reliable packets for good

- **Area:** net -- `ENetBackend` applies `NetworkSession::SimulatedPacketLoss` where AppData is
  handed to a local gamer, after ENet's reliable transport has delivered it
  (`ShouldDropForSimulatedLoss`, whatever the packet's `SendDataOptions`). A packet sent with
  `SendDataOptions::Reliable` (or `ReliableInOrder`) is then lost for good. In XNA, reliable delivery
  is guaranteed whatever the connection does -- the simulation makes it late (resent), never lost --
  which is what a game tests with the simulation.
- **Demos:** `net.network_session.simulated_conditions` (expected failure).
- **Reproduction:** two processes in a System Link session; the receiver sets
  `SimulatedPacketLoss = 0.3`; the sender sends one `SendDataOptions::Reliable` packet per frame with a
  sequence number.
- **Observed:** about 70 % of the reliable sequence numbers arrive (68 % in one run), the same share
  as for unreliable packets.
- **Deterministic demo revision (2026-10-11):** separate 24-pair bursts now test baseline,
  200 ms latency, and `SimulatedPacketLoss = 1`. After the final burst settles, the receiver
  restores loss to zero and both peers pump for another second before disconnecting. SOFTWARE
  and OPENGLES3 each received all 24 reliable packets in the first two phases and zero from the
  outage, even after recovery. The source still drops AppData after transport acknowledgement,
  with no reliable retry queued there. Exact authored counts include missing endpoints.
  The earlier short-sample percentage gates for unreliable delivery were demo assumptions,
  not CNA guarantees; their observed fluctuations are not a new CNA issue. This is CNA runtime
  evidence plus source inspection, not a genuine-XNA runtime comparison.
- **Expected:** every reliable packet arrives; only unreliable ones are lost.
- **Environment:** CNA 0.1.0, Linux, System Link (ENet).
- **Status:** open; not reported upstream yet.

- **Demo watchdog correction (2026-10-12):** initial 466 SOFTWARE reached recovery but its
  aggregate 20-second orchestration deadline expired before packet evaluation. The original raw
  failure is archived and was not accepted as an ISSUE-030 reproduction. Unchanged isolated
  runs and the per-phase-watchdog repair both retain all five original child checks: four pass,
  and exactly the reliable-loss assertion fails on both renderers. The repair changes only
  bounded coordination timing, including sign-in waits; packet counts, pacing, latency and
  recovery assertions remain unchanged. This does not add a CNA issue or waive the earlier
  interrupted evaluation. Full repaired466-timing validation passes all479enabled CTests
  and both exact smoke comparisons. Controlled identical8-second phase delays also prove the
  repaired37.41-second exchange retains all five host checks while the original sender times
  out at22.88seconds. Complete466-transfer subsequently preserves the same network checks.

---

### CNA-DEMOS-ISSUE-031 -- glTF STEP and CUBICSPLINE animation samplers play as LINEAR

- **Area:** content, glTF import -- `GltfImportCore`'s `ExtractClips` evaluates each channel
  (STEP, LINEAR or CUBICSPLINE correctly, `EvaluateVec3Channel`/`EvaluateQuatChannel`) but only at
  the bone's own key times, and `KeyframeEXT` has no interpolation mode: `ApplyClipToBonesEXT`
  then interpolates linearly between those keys. A STEP channel slides instead of holding, a
  CUBICSPLINE channel loses its tangents. glTF 2.0 §3.11 defines all three samplers. Both the
  compiled model and the run-time import behave so (the extraction is shared; skinned clips go
  through the same code, not checked separately).
- **Demos:** `graphics.model.clip_interpolation` (expected failure).
- **Reproduction:** `content/models/slide_rig.gltf` (tools/gltf_models.py: three translation
  channels keyed `x = -2, 0, 2` at `0, 1, 2` s, samplers STEP, LINEAR, CUBICSPLINE with zero
  tangents); `ApplyClipToBonesEXT(model, clip, 0.25 s)`.
- **Observed:** x = -1.5 for all three.
- **Expected:** STEP -2 (held), LINEAR -1.5, CUBICSPLINE -1.6875 (Hermite, flat tangents).
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open; not reported upstream yet. Resampling STEP channels with a hold key just before
  each step, and CUBICSPLINE ones densely (or carrying the mode in the clip), would keep them.

---

### CNA-DEMOS-ISSUE-032 -- a System Link join refused for a game in progress reads `SessionNotFound`

- **Area:** net, System Link (`ENetBackend::HandleClientHello`, `NetworkSession::EndJoin`). A host
  whose session is Playing with `AllowJoinInProgress == false` refuses a joiner by disconnecting the
  peer without a reason; the joiner's `EndJoin` turns every failure into
  `NetworkSessionJoinException(SessionNotFound)`. XNA reports `NetworkSessionJoinError::SessionNotJoinable`
  for this case (the error exists for it), and a game shows "game in progress" rather than "not found".
  The online binding (`OnlineSessionBinding`) does distinguish SessionFull and SessionNotJoinable.
- **Demos:** `net.network_session.join_refused` (expected failure).
- **Reproduction:** two processes; the second finds the host's System Link session in its lobby, the
  host calls `StartGame` (AllowJoinInProgress false), the second calls `NetworkSession::Join`.
- **Observed:** `NetworkSessionJoinException` "The requested network session could not be found.",
  `JoinError` SessionNotFound.
- **Expected:** `JoinError` SessionNotJoinable.
- **Environment:** CNA 0.1.0, Linux, System Link (ENet).
- **Status:** open; not reported upstream yet. A refusal message (or an ENet disconnect data value)
  carrying the reason would let the joiner map it.

---

### CNA-DEMOS-ISSUE-033 -- a gamer who joins a System Link game in progress sees the session in Lobby

- **Area:** net, System Link (`ENetBackend`). The host's `ServerWelcomeMessage` carries the roster and
  the session properties but not the session state, and the state is broadcast only when it changes
  (StartGame/EndGame). A joiner admitted while the host is Playing (`AllowJoinInProgress == true`)
  therefore keeps `SessionState == Lobby` and never sees `GameStarted`, while the host is Playing.
- **Demos:** `net.network_session.join_refused` (expected failure; its known issue is ISSUE-032, this
  is the demo's second failing check).
- **Reproduction:** as ISSUE-032, then the host sets `AllowJoinInProgress = true` and the second
  process finds and joins again; it calls `NetworkSession::Update` for 0.5 s.
- **Observed:** the joiner's `SessionState` is Lobby; the host's is Playing.
- **Expected:** Playing (the joiner entered a game in progress).
- **Environment:** CNA 0.1.0, Linux, System Link (ENet).
- **Status:** open; not reported upstream yet. Adding the state to the welcome message would fix it.

---

### CNA-DEMOS-ISSUE-034 -- Stock effect parameters accept a scalar getter on a Vector4

- **Area:** graphics -- `EffectParameter::GetValueSingle` in `EffectParameter.cpp` calls
  `RequireCompiledScalarGetterShape` only for compiled storage. Stock effects use the separate
  float-vector storage path, which returns its first component regardless of the reflected shape.
- **Demo:** `graphics.effect.parameter_values` (expected failure; also exposes ISSUE-035).
- **Reproduction:** take `BasicEffect.Parameters["DiffuseColor"]`, set
  `Vector4(0.2f, 0.4f, 0.6f, 0.8f)`, then call `GetValueSingle()`.
- **Observed:** returns 0.2 on SOFTWARE and OPENGLES3 instead of rejecting the vector-to-scalar
  access. The parameter still reports Vector, Single, one row and four columns.
- **Expected:** `System::InvalidCastException` for this non-array Vector parameter. CNA's own
  `EffectTests.cpp` compiled-effect getter conformance checks require that exception, and
  `docs/software-easygl-parity-ledger.md` SOFTWARE-270 records the recovered Microsoft IL basis.
  The stock path should expose the same public shape contract. Genuine XNA was not run here.
- **Environment:** CNA `2b15506a4`, GCC 14.2, Linux, SOFTWARE and OPENGLES3/private Xvfb.
- **Status:** confirmed compatibility gap, open; not reported upstream. The stock/standalone
  storage leniency is deliberate for C API compatibility, so an upstream fix needs to distinguish
  reflected stock parameters from freely constructed standalone parameters.

---

### CNA-DEMOS-ISSUE-035 -- SkinnedEffect's Bones array has no reflected elements

- **Area:** graphics -- `SkinnedEffect::CacheEffectParameters` constructs `Bones` with
  `AddParam(params, "Bones", MaxBones, 4, Matrix, Single)` and never adds array elements.
- **Demo:** `graphics.effect.parameter_values` (its primary known-issue marker is ISSUE-034).
- **Reproduction:** construct a SkinnedEffect and inspect its `Bones` parameter's Elements,
  RowCount and ColumnCount; write two matrices and read them with GetValueMatrixArray.
- **Observed:** zero elements, 72 rows and four columns. The value-array round trip works, but a
  reflection-driven editor cannot discover the bone entries or their true matrix shape.
- **Expected:** 72 individual matrix elements. CNA's stock shader declares `float4x3 Bones[72]`;
  the stored FNA reflection fixture (`tests/fixtures/compiled-effects/fna-effect-reflection.json`)
  reports 72 elements with RowCount 3 and ColumnCount 4. The old handoff inferred reflected 4x3
  directly from HLSL, which the fixture contradicts. The demo therefore checks the unambiguous
  array cardinality, prints the dimensions, and does not impose an unmeasured genuine-XNA
  row/column convention. Genuine XNA reflection was not executed in this session.
- **Environment:** CNA `2b15506a4`, GCC 14.2, Linux, SOFTWARE and OPENGLES3/private Xvfb.
- **Status:** confirmed stock-effect reflection defect, open (value-array access still works);
  not reported upstream.

---

### CNA-DEMOS-ISSUE-036 -- Linear curve post-loop reads the first key's outgoing tangent

- **Area:** math, `Curve::Evaluate`, `modules/math/src/Curve.cpp` (PostLoop Linear branch).
- **Demo:** `math.curve.loop_types` (marked `knownIssue`).
- **Environment:** Linux, GCC 14.2, CNA `2b15506a48e1a6202c985823131daf5856826f71`.
- **Reproduction:** two keys `(0, 0)` and `(1, 1)`, ComputeTangents(Linear), then set the last
  key's TangentOut to 3 and PostLoop to Linear. Evaluate at position 2.
- **Observed:** 2, from `last.Value + first.TangentOut * (position - last.Position)`.
- **Expected:** 4, from the last key's value 1 plus its outgoing tangent 3 times distance 1.
  The locally available decompilation of genuine XNA 4.0 `Curve.Evaluate` explicitly reads
  `curveKey2.tangentOut`, where curveKey2 is the last key. This is source evidence, not a
  genuine-XNA runtime comparison. CNA's own CurveTest.EvaluatePostLoopLinear currently expects
  the first-key behavior, so passing that upstream test does not establish XNA compatibility.
- **Investigation:** the prior demo expected 2 after computing Linear tangents on a two-key
  ramp. The last outgoing tangent is then zero, so even that original expectation was wrong:
  XNA would hold at 1. The corrected demo sets distinct endpoint slopes explicitly and checks
  the intended contract; first-key incoming extrapolation and negative CycleOffset/Oscillate
  checks remain independent. No CNA source was modified.
- **Related upstream record:** [CNA-BUG-064](https://libcna.com/known-issues/bugs/cna-bug-064.html)
  also describes this post-loop error and the Step error already tracked here as ISSUE-002.
- **Validation:** corrected demo executed on SOFTWARE and OPENGLES3: 16 checks each, exactly
  one failure (actual 2, expected 4). All five curve demos and catalog validation passed their CTest expectations;
  metadata validation reports 404 demos and zero problems. Genuine XNA was not executed.

### CNA-DEMOS-ISSUE-037 -- DynamicSoundEffectInstance header incorrectly says IsLooped is ignored

- **Area:** documentation only, `DynamicSoundEffectInstance::setIsLoopedProperty` in
  `modules/audio/include/Microsoft/Xna/Framework/Audio/DynamicSoundEffectInstance.hpp`.
- **Related demo:** `audio.dynamic_sound_effect_instance.streaming` now checks that true is
  refused and false is accepted. Its failure marker is for the separate disposal issue below.
- **Evidence:** source review at CNA `2b15506a48e1a6202c985823131daf5856826f71`. Both setter
  declarations say the argument is ignored/no effect. The implementation in
  `modules/audio/src/Xna/DynamicSoundEffectInstance.cpp` first checks disposal, then throws
  `System::InvalidOperationException` when the value is true; false is accepted. The rvalue
  overload forwards to this same implementation.
- **Expected:** document that dynamic instances cannot loop: setting true throws, setting false
  is accepted, and using the setter after disposal throws ObjectDisposedException. This agrees
  with the locally available decompilation of genuine XNA 4.0 DynamicSoundEffectInstance.IsLooped.
- **Validation scope:** header, implementation and reference source inspected; the extended
  streaming demo executed both setter values successfully on SOFTWARE and OPENGLES3. Genuine XNA was not run.
  This is a confirmed comment contradiction, not a setter implementation defect. No CNA source
  was changed.
- **Suggested correction:** update both setter comments and their exception documentation.

### CNA-DEMOS-ISSUE-038 -- Dynamic audio getters ignore disposal

- **Area:** `DynamicSoundEffectInstance::getPendingBufferCountProperty` and
  `getIsLoopedProperty`, `modules/audio/src/Xna/DynamicSoundEffectInstance.cpp`.
- **Demo:** `audio.dynamic_sound_effect_instance.streaming` (marked `knownIssue`).
- **Reproduction:** construct the dynamic instance, submit and play PCM, stop it, call Dispose,
  verify IsDisposed, then read PendingBufferCount and IsLooped.
- **Observed:** PendingBufferCount returns 0 and IsLooped returns false; neither getter throws.
  The implementation returns queue size / false without
  checking IsDisposed. The streamed playback, duration conversion and unsupported-request
  checks pass independently; the failing checks concern only the two post-disposal getters.
- **Expected:** both getters throw System::ObjectDisposedException. This is stated in the local
  xna4-spec DynamicSoundEffectInstance.xml and appears explicitly in the genuine-XNA 4.0
  decompilation's two getters. That reference was read, not executed.
- **Investigation:** CNA's PendingBufferCountResetsToZeroAfterDisposeWhilePlaying unit test
  currently expects zero after disposal. Queue cleanup is appropriate, but exposing its count
  through a disposed public object conflicts with XNA; that upstream test does not establish
  compatibility. The same unguarded implementations exist at the CI-pinned CNA revision
  (source checked only). No upstream source was changed.
- **Environment/evidence:** CNA `2b15506a48e1a6202c985823131daf5856826f71`, Linux GCC 14.2,
  SOFTWARE and OPENGLES3/private Xvfb, SDL dummy audio. Both runs execute 11 checks with exactly
  these two failures. Direct reports/logs: `/tmp/cna-streaming-lifecycle-{SOFTWARE,OPENGLES3}.{json,log}`.
  Targeted CTest/catalog and metadata validation pass. No audible validation is claimed.
- **Status:** runtime-confirmed missing disposal guards; not reported upstream.

### CNA-DEMOS-ISSUE-039 -- ContentManager permits changing the root with loaded assets

- **Area:** both ContentManager::setRootDirectoryProperty overloads in
  `modules/content/src/Xna/ContentManager.cpp`.
- **Demo:** `content.content_manager.cache_and_unload` (marked `knownIssue`).
- **Reproduction:** create a manager rooted at assets, Load<Texture2D>("textures/checker"),
  then attempt to set RootDirectory to assets/xnb without first calling Unload.
- **Observed:** the setter accepts the new path. Both overloads directly assign rootDirectory_
  and never check the loaded-asset cache. The old cached asset and the new root can consequently
  describe different content trees under one manager.
- **Expected:** InvalidOperationException and an unchanged root while the manager has loaded
  assets. The local genuine-XNA 4.0 ContentManager decompilation checks loadedAssets.Count > 0
  before assigning the root. This is reference-source evidence; genuine XNA was not executed.
- **Independent behavior:** after Unload, switching to assets/textures and loading checker
  works, creating one new texture. Original first-load/cache/reload checks continue to pass;
  the extended demo has 5 checks with exactly the root-mutation guard failing.
- **Environment/evidence:** CNA `2b15506a48e1a6202c985823131daf5856826f71`, Linux GCC 14.2,
  SOFTWARE and OPENGLES3/private Xvfb. Reports: `/tmp/cna-content-root-{SOFTWARE,OPENGLES3}.json`;
  both execute 5 checks with exactly one failure, and both screenshots were inspected. Build,
  metadata (407/0), targeted CTest and catalog validation pass. The same setter implementation
  exists at the CI pin (source checked only).
- **Status:** runtime-confirmed XNA compatibility gap, not a demo workaround; no CNA source
  changed and no upstream report sent.

### CNA-DEMOS-ISSUE-040 -- Picture and Album thumbnail comments contradict generation paths

- **Area:** documentation only, Picture::GetThumbnail and Album::GetThumbnail in
  `modules/media/include/Microsoft/Xna/Framework/Media/{Picture,Album}.hpp`, plus the
  Album implementation comment described below.
- **Related demo:** `media.media_library.save_picture` (normal thumbnail checks pass; its expected-failure
  marker belongs to the separate short-read ISSUE-041).
- **Evidence:** the public comment says no separate thumbnail is generated and that GetThumbnail
  returns the same image as GetImage. The implementation in `modules/media/src/Xna/Picture.cpp`
  instead calls ThumbnailGenerator::CreatePngThumbnail and returns a caller-owned MemoryStream
  containing the generated image when possible; it otherwise falls back to the source FileStream.
- **Reproduction/observed:** save the original 640x320 PNG into a private library, read GetImage
  and verify its bytes equal the source, then save GetThumbnail's stream as a second picture.
  The second Picture reports 128x64, and both files are indexed by a fresh MediaLibrary. The
  demo checks smaller dimensions and preserved aspect ratio, not a fixed XNA thumbnail size.
- **Environment/evidence:** Linux GCC 14.2, current CNA ce74063e6483020835af07c4af1e3614ebc5d962
  (media implementation unchanged from e60fa35f7). The standalone ran under both SOFTWARE and
  OPENGLES3 selections and passed 11 child checks each, including private XDG root verification
  before library access. This is file/stream evidence, not GPU or visual thumbnail validation.
  Reports/logs: `/tmp/cna-save-picture-{SOFTWARE,OPENGLES3}.{json,log}`; parent JSON check count
  is zero for standalone tools, so the checks are in child stdout. No real user media was used.
- **Expected/suggested correction:** document the possible generated thumbnail and original-image
  fallback. The current 128-pixel maximum is an implementation choice, not a universal XNA
  contract. The implementation's useful resizing behavior should not be removed to match the
  stale comment.
- **Additional source-only observation (2026-10-13):** at clean CNA `eff31310179168fb4f47258e6226a5234c2140d0`,
  `Album.hpp::GetThumbnail` repeats the unconditional claim that it returns the same image as
  GetAlbumArt. `Album.cpp::GetThumbnail` instead calls CreatePngThumbnail for file-based art,
  returning generated PNG bytes on success and the original FileStream on failure. Embedded
  art follows a different branch and currently returns GetAlbumArt directly; its implementation
  comment about downscaling decoded bytes also overstates that branch. This is a source-level
  documentation contradiction, not a new executed Album test or a diagnosed image-output defect.
  The proposed music_index workflow does not call either art method. Future album-art coverage
  should distinguish file-based generation, embedded-art behavior and fallback. Source hashes:
  `checkpoint-472-cuelimits-next-album-art-source.json`. No runtime knownIssue marker is added.
- **Status:** confirmed header contradictions; Picture has the native evidence above, while the
  Album follow-up is source-only. No CNA source changed and no upstream report sent.

### CNA-DEMOS-ISSUE-041 -- SavePicture ignores short reads and saves a zero-padded image

- **Area:** MediaLibrary::SavePicture(std::string, System::IO::Stream*) in
  `modules/media/src/Xna/MediaLibrary.cpp`.
- **Demo:** `media.media_library.save_picture` (marked `knownIssue` for this one failing check).
- **Reproduction:** wrap the valid, original 1,666-byte 640x320 PNG in a public MemoryStream
  subclass whose Read caps each delivery at 13 bytes. Start at position zero. A control reader
  loops over the same fixture and verifies every byte matches the source, so the input is neither
  truncated nor malformed. Pass another such stream to SavePicture and read back its GetImage.
- **Observed:** only one Read occurs, consuming 13/1666 bytes. CNA nevertheless writes the full
  allocated 1,666-byte vector, whose unread tail remains zero, and returns a Picture with 0x0
  decoded dimensions. The saved image differs from the original. Both renderer selections
  reproduce this; ordinary byte-vector and full-memory-stream saves pass independently.
- **Expected:** consume repeated positive short reads until the intended image data is copied;
  detect premature EOF instead of silently writing unread zero-filled storage. The public
  SharpRuntime Stream.hpp contract explicitly permits Read to return fewer than count bytes
  before EOF. This is a stream-consumer defect, not a malformed-image fixture or an assertion
  about a particular genuine-XNA implementation.
- **Investigation:** the overload allocates source->getLengthProperty() bytes, calls Read once,
  ignores its returned count, and forwards the entire vector to the byte-buffer overload. That
  overload writes the bytes and catches the decoder failure, producing width/height zero.
  Partial-read handling is the confirmed defect; nonzero starting positions, unknown-length
  streams and premature EOF have not been exercised by this reproduction.
- **Environment/evidence:** Linux GCC 14.2, CNA ce74063e6483020835af07c4af1e3614ebc5d962
  (media implementation unchanged from e60fa35f7). Initial repro logs/reports:
  `/tmp/cna-save-picture-chunks-{SOFTWARE,OPENGLES3}.{json,log}` (13 checks before adding the
  returned-pointer guard and issue label; exactly one failure in each). Final labeled runs
  `/tmp/cna-save-picture-final-{SOFTWARE,OPENGLES3}.{json,log}` have 14 checks and the same
  single failure; the targeted WILL_FAIL CTest passes. All filesystem work
  used verified private XDG roots; no user pictures were read or written. The normal thumbnail
  path still passed and returned 128x64. No visual/GPU validation is claimed for this file tool.
- **Complete checkpoint:** all 423 demos validated at cna-demos 37ad437. Both original stdout
  logs add exactly this one assertion relative to 422; all older statuses and failures are
  unchanged. SOFTWARE 377/18/25/3, OPENGLES3 398/19/3/3; 436 enabled CTests pass including
  this WILL_FAIL test. Archives/reviews: `build/linux-full/reports/checkpoint-423-*`.
- **Status:** runtime-confirmed CNA bug. The demo keeps the failing byte-preservation check;
  no loop/workaround was added around CNA's SavePicture call, no CNA source changed and no
  upstream report was sent.

### CNA-DEMOS-ISSUE-042 -- Root-prefix comments describe an older mesh-cancellation representation

- **Area:** documentation only, SkinningData::SkeletonRootPrefix and
  ApplyBindPoseBoneTransformsEXT comments in `modules/graphics/include/Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp`.
- **Related demo:** `graphics.model.multiple_skins` (passes; no expected-failure marker).
- **Evidence:** the root-prefix comment describes non-joint ancestry and inverse mesh-node
  cancellation as context retained in SkeletonRootPrefix. The current loose glTF loader instead
  calls BuildSkeleton with scene ancestry alone and parents each skinned ModelMesh to its identity
  model root. The authored mesh node remains in Model.Bones but its placement is not applied to
  the skinned draw. The runtime result is correct; the public storage explanation is stale.
- **Reproduction/observed:** import two one-joint skins sharing a quad and material, with non-joint
  ancestors at X=-0.5/+0.5 and mesh nodes at X=+4/-4. The public prefixes are -0.5/+0.5, not
  -4.5/+4.5. Both mesh parents are the identity model root, while named scene nodes retain +4/-4.
  Animating the left root preserves its ancestor; restoring the right bind pose touches only its
  own effect. Actual red/green positions and the vacated left position pass pixel checks.
- **Environment/evidence:** Linux GCC 14.2, CNA 8b5564f1e81196f3aaee9e9a652b8030e21f4c6c;
  no runtime library change since the preceding checkpoint (external documentation/test cleanup).
  Both SOFTWARE and OPENGLES3 initially passed 17 checks; the final live-bounds extension
  passes 21. Captures were inspected and RGBA-identical; final captures match those inspected
  images: `/tmp/cna-427-multiple-skins-final-{SOFTWARE,OPENGLES3}.{json,log,png}`.
- **Expected/suggested correction:** describe the current loose loader's identity mesh parent and
  ancestor-only prefix, distinguishing the ignored mesh-node transform from a stored inverse
  matrix. Avoid implying that every legacy/custom SkinningData carrier has this same representation.
- **Status:** confirmed public-comment mismatch, not a rendering failure. No CNA source changed,
  no upstream report sent and no failing check suppressed. The final demo independently verifies
  live-bound updates with an intentionally distant palette and restoration; broader skinning
  bounds and multi-influence cases are not implied.

### CNA-DEMOS-ISSUE-043 -- GetTypeReader confuses the target type name with the reader factory key

- **Area:** ContentTypeReaderManager::GetTypeReader/CreateReaderForTargetTypeEXT and the
  constructor association in ContentTypeReader<T>.
- **Demo:** `content.xnb.existing_objects` (two explicit known-failing lookup checks; the other
  eleven checks exercise independent existing-object/raw-payload behavior).
- **Reproduction:** register a capture-free WeaponReader factory under
  `CnaDemos.LoadoutWeaponReader`, as named by the XNB reader table. Its base constructor correctly
  declares target `CnaDemos.LoadoutWeapon`. After the entire two-entry table has been constructed,
  the root reader's Initialize calls GetTypeReader(System::Type::From<WeaponPtr>()).
- **Observed:** GetTypeReader returns null although the weapon reader has already been constructed.
  Explicit CreateReader("CnaDemos.LoadoutWeaponReader") returns the correct reader, whose public
  target name remains `CnaDemos.LoadoutWeapon`. Indexed ReadObject, explicit-reader ReadRawObject,
  existing slot identity/runtime state, null fallback and type-erased tags all work.
- **Expected:** type lookup resolves the reader for that target type without requiring a data
  type's canonical name to equal its reader class/factory name. Those names are also distinct
  throughout CNA's built-in readers (for example System.String versus its StringReader).
- **Cause:** ContentTypeReader<T>'s constructor associates C++ type T with the target name.
  CreateReaderForTargetTypeEXT retrieves that string and passes it directly to CreateReader,
  which indexes factories by reader name. No target-to-reader-name mapping bridges them.
  ContentRuntimeContractTests uses the same string for target and factory key, so its passing
  lookup test does not cover this distinction. The demo now directly executes implicit
  ReadRawObject<T>(existing); it throws before consuming any payload bytes. The explicit-reader
  fallback is guarded by unchanged stream position and the unresolved lookup; partial decode
  failures are rethrown, not hidden.
- **Evidence/environment:** Linux GCC 14.2, both renderer selections WITHOUT DISPLAY or
  WAYLAND_DISPLAY: `/tmp/cna-430-existing-{SOFTWARE,OPENGLES3}.{json,log}` (12 checks, exactly
  one failure before adding the issue label). CNA HEAD 2fda5389d2990b40146d06de0ae0531d05055f24
  with an external C API refactor in progress; the inspected C++ content reader implementation
  is unchanged. No graphics/GPU or genuine-XNA runtime comparison is claimed.
- **Final labeled reproduction:** `/tmp/cna-430-existing-final-{SOFTWARE,OPENGLES3}.{json,log}`
  retains the same single failed check with the issue identifier. Its WILL_FAIL CTest passes;
  full 430 at b113bdc completed with 443 enabled CTests and both complete smoke runs. The only
  added failed assertion versus 428 is this one (25/26 -> 26/27); all older states/check counts
  remain unchanged. Original logs and exact review/hashes: `reports/checkpoint-430-*`.
  Both new external-reference renders pass independently; no failure was hidden by the marker.
- **Extended reproduction (433 batch):** `/tmp/cna-433-existing_objects-{SOFTWARE,OPENGLES3}.{json,log}`
  records 13 checks, exactly two failures, in fresh processes with DISPLAY/WAYLAND_DISPLAY unset.
  The original GetTypeReader assertion is retained verbatim; the new implicit raw-read assertion
  shares ISSUE-043. All eleven other checks still pass. CNA HEAD at build start was 4d774868e9
  with further external C API work; the content lookup source still has the same defect.
  Full 433 at a01ab68 passed all 446 enabled CTests and both complete smoke runs; ORIGINAL
  comparisons to 432 require review exactly for this additional assertion and 12 -> 13 checks.
  Raw totals 26/27 -> 27/28, no older failures removed. Exact evidence reviews/hashes are in
  `reports/checkpoint-433-{SOFTWARE,OPENGLES3}-review.json`.
- **Handling:** retain both failing lookup assertions. A clearly labeled public named-factory
  fallback lets the remaining schema example run; it does not change the target name or add
  a hidden alias registration. No global registry clearing, CNA source edits or upstream report.
  A library fix should preserve the distinction between target identity and reader factory name.

### CNA-DEMOS-ISSUE-045 -- Truncated XNB leaks EndOfStreamException through ContentManager::Load

- **Area:** ContentManager::Load<T>/LoadXnbAsset and the low-level XNB header parser.
- **Demo:** `content.content_manager.asset_precedence`, 14 console checks: thirteen pass and
  one preserves the public exception-type failure. The format-priority behavior itself passes.
- **Reproduction:** a private content directory contains valid same-name CNB, CNJ and native
  rules files. Write a three-byte `rules.xnb` containing `XNB`, then call Load<Rules>("rules")
  on a fresh manager. The valid compiled XNB control first loads the authored value 400.
- **Observed:** Load throws System::IO::EndOfStreamException. Catching only the publicly
  documented ContentLoadException would not recover. No lower-tier loader runs; repairing the
  XNB permits the same manager to load it successfully without Unload. A three-byte CNB is
  correctly refused as ContentLoadException, and its repair/retry also works.
- **Expected:** failure to load an invalid XNB reaches the caller through ContentLoadException,
  consistent with Load<T>'s public @throws contract and its wrapping of ordinary loose-reader
  exceptions. This is a CNA contract comparison, not a genuine-XNA runtime observation.
- **Cause:** Load dispatches directly to LoadXnbAsset, which calls ParseXnbHeader without a
  wrapping boundary. The internal parser intentionally documents EndOfStreamException for a
  truncated header and reads the fourth platform byte after the three magic bytes. That
  low-level behavior escapes through the public content-loading boundary unchanged.
- **Evidence/environment:** `/tmp/cna-434-asset_precedence-{SOFTWARE,OPENGLES3}.{json,log}`,
  fresh Linux GCC 14.2 processes WITHOUT DISPLAY/WAYLAND_DISPLAY, exactly one failure in each
  initial unlabeled 14-check run. CNA observed clean HEAD 9e340c876f6426218e2f82651fcb0b086de25e15
  after build start; external C API refactors are unrelated to the inspected unchanged XNB path.
- **Final labeled reproduction:** `/tmp/cna-434-asset-final-{SOFTWARE,OPENGLES3}.{json,log}`
  keeps all 14 checks and the same one failure with ISSUE-045. Both temporary directories were
  verified removed after exit. Its WILL_FAIL CTest passes along with the other 20 targeted tests.
  Full 434 at 652398e passed all 447 enabled CTests and both complete smoke runs. ORIGINAL
  comparisons to 433 require review only for this new failure and the two passing demo extensions;
  all older failed assertions are retained (27/28 -> 28/29). Original evidence and exact reviews:
  `reports/checkpoint-434-{SOFTWARE,OPENGLES3}*`.
- **Handling:** retain the failing expectation with its issue identifier. The negative probe
  explicitly catches EndOfStreamException solely so independent priority/cache/repair checks
  continue; it does not relabel the exception or fall back to a lower-priority asset. Temporary
  files belong to this demo and are removed by scoped cleanup. No CNA edits or upstream report.


### CNA-DEMOS-ISSUE-046 -- Intermediate XML cannot read its own aliased external TargetType

- **Area:** IntermediateSerializer external-reference writing/reading and namespace resolution.
- **Demo:** `content.custom.intermediate_external_references`; ten child checks, nine passing
  and one native emitted-XML round-trip failure. No graphics or referenced-file loading is needed.
- **Reproduction:** describe Scene and Profile in the same `CnaDemos.External` namespace. Scene
  has an inline Profile preview and two ExternalReference<Profile> wrappers naming one file,
  plus a second file and a null reference. Serialize with an absolute document relocation path,
  then feed that exact XML to Deserialize<Scene> with the same path.
- **Observed:** the writer emits `xmlns:External="CnaDemos.External"` and
  `TargetType="External:Profile"`. Deserialize throws InvalidContentException containing
  `Cannot find type "External:Profile".` The embedded preview loads via the same registered
  Profile serializer. A separately identified control using `CnaDemos.External.Profile` for the
  two TargetType attributes succeeds, including filename relocation and wrapper/null semantics.
- **Expected:** the serializer can deserialize its own valid namespace-qualified external
  references, resolving their target identities in the document's namespace context. This is a
  CNA round-trip contract check, not a genuine-XNA runtime comparison.
- **Cause:** IntermediateWriter::WriteExternalReferences calls SpellDeclaredTypeName, which
  uses an existing namespace alias. IntermediateReader::ReadExternalReferences instead sends
  the raw attribute to IsKnownTypeName and stores it unchanged; CanonicalTypeName does not
  resolve document namespace aliases. ApplyFixups also compares the stored spelling directly
  against the canonical target name. Both lookup and comparison need the resolved identity;
  registering a hidden global alias in the demo would obscure the defect.
- **Evidence/environment:** fresh Linux GCC 14.2 processes WITHOUT DISPLAY/WAYLAND_DISPLAY,
  `/tmp/cna-437-xml-external-{SOFTWARE,OPENGLES3}.{json,log}`. Each initial unlabeled run
  has exactly this failed check and nine passing checks. CNA observed at
  e5014ffaaec2b7f2b018b4dad1f5e299899315f7 after external C API changes; the inspected
  content-pipeline sources have no uncommitted edits. No atomic dependency snapshot is claimed.
- **Handling:** preserve the original XML and failing assertion with ISSUE-046. Catch only the
  diagnosed InvalidContentException so independent controls can run; other failures propagate.
  Canonical-name control XML is explicitly separate and does not replace the original output.
  Referenced filenames are recorded, not imported, compiled or published by this XML tool.
  No CNA code was changed and no upstream report was sent.
- **Final labeled evidence:** `/tmp/cna-437-xml-external-final-{SOFTWARE,OPENGLES3}.{json,log}`
  retains the same one failure with ISSUE-046 and nine passes. Its WILL_FAIL CTest passes with
  the other 20 focused tests. Full 437 at b697786 passed all 450 enabled CTests and both
  renderer smoke runs. Exact original reviews add only this failed assertion (28/29 -> 29/30),
  retaining all older states/check counts/failures. Archives: reports/checkpoint-437-*; original
  streams and final binary hashes are preserved. The first pre-CTest staging-warning attempt
  remains archived separately; it was not an XML demo failure.

### CNA-DEMOS-ISSUE-047 -- AddOutputFile refuses a generated file in the configured output directory

- **Area:** XNA pipeline adapter `ContentProcessorContext::AddOutputFile`, forwarded by
  `XnaBridgeProcessorContext` to canonical `ContentProcessorContext::AddDeploymentFile`.
- **Demo:** `content.custom.xna_pipeline_components` (knownIssue; expected failure).
- **Expected (public CNA contract):** the XNA-shaped header describes related files written by
  the processor beside the compiled asset, to be deployed and cleaned with it. The public build
  environment separately supplies outputDirectory. An existing processor-written file inside
  that configured output directory should be recordable when source/output directories differ.
- **Reproduction:** a real registered route processor reads its label with AddDependency, writes
  `output/patrol.bounds.txt`, closes the file successfully, then calls AddOutputFile with its
  absolute path. The source root is the separate sibling `source/` directory; both directories
  belong to this demo's private temporary tree. The real ContentPipeline::Build refuses the call
  in the Process stage with the correct processor name:
  `deployment source '.../output/patrol.bounds.txt' is outside the primary source root and was
  not explicitly resolved through a named external source capability.`
- **Cause:** the adapter maps the file to a basename deployment destination, but the canonical
  deployment path accepts only source-contained files or exact dependencies previously resolved
  through a named external source capability. It does not recognize the configured output root
  for generated processor files. This entry requests no relaxation for arbitrary outside paths.
- **Controls:** the report's actual bytes contain the expected label and computed bounds before
  registration fails. A separately identified source-contained output layout succeeds and records
  both the processor label dependency and generated deployment file. An explicit following build
  consumes a changed label while retaining identical compiled bounds. This control is not a
  correction of the failed separate-output case and does not prove CLI deployment/cleanup or
  incremental caching. All temporary files are explicitly removed by the demo.
- **Initial evidence:** `/tmp/cna-438-outputs-xna_pipeline_components-{SOFTWARE,OPENGLES3}.{json,log}`,
  fresh displayless Linux GCC 14.2 processes, with the same output-root refusal on both selections.
  These initial logs also retain a separate demo diagnostic mismatch: its own missing-label
  exception omitted the filename expected by the check. ResolveSourceDependency records a
  contained path and does not promise an existence check; this is not a second CNA defect.
  The demo diagnostic now names patrol.label and still verifies Source/LogicalName/Stage/Component.
  Final labeled evidence `/tmp/cna-438-outputs-final-xna_pipeline_components-{SOFTWARE,OPENGLES3}.{json,log}`
  has exactly 12 passing child checks and this one failure per fresh process. All seven original
  checks remain. Both private trees are verified gone. All 22 targeted CTests pass, including
  the expected-failure marker, both audio corrections, all 16 C ABI demos and catalog checks.
  The complete 438-outputs checkpoint at e6327d2 passes all 451 enabled CTests. Both original
  checkpoint-438 comparisons retain every older failed assertion and add exactly this failure
  (30/31 -> 31/32). Full child output has the same twelve passes/one failure as the focused runs;
  evidence and exact reviews are archived under reports/checkpoint-438-outputs-*.
- **Environment:** CNA checkout observed at d751cf46ae41ab49890176530dd564d05482ad32 with external
  C API work ongoing; no atomic source snapshot claimed and no CNA source edited by this task.
  These are CPU pipeline runs under renderer selections, not GPU or genuine-XNA execution.
- **Status:** open; not reported upstream.

### CNA-DEMOS-ISSUE-048 -- Lenient builds report dropped parameters as effective

- **Area:** native `ContentPipeline::Build`, `ContentBuildResult::parameters` and XNA processor
  parameter adapters under `ContentStrictness::XnaCompatible`.
- **Demo:** `content.custom.xna_pipeline_components`; existing ISSUE-047 remains the primary
  knownIssue marker, with this second assertion explicitly labeled and documented.
- **Public contract:** ContentPipeline.hpp documents the returned `parameters` as "Effective
  processor parameters". After warning/default handling removes rejected entries before Process,
  this result should describe what the processor actually received. The caller's original request
  should remain intact; keeping that request is a separate passing check.
- **Reproduction:** register the actual route importer, bounds processor and CNB writer. Request
  `Padding = string("wide")` and `UnknownPadding = double(1.5)` in XnaCompatible mode, then call
  `ContentPipeline::Build`. Compare the returned bytes to a default build, inspect both contextual
  warnings and inspect `result.parameters.Values()`.
- **Observed:** the build succeeds, warns once for each rejected parameter and emits bytes equal
  to the default bounds. Nevertheless, both rejected entries remain in `result.parameters`.
  The focused demo has 17 passing checks and two failures (this discrepancy plus older ISSUE-047)
  under both displayless SOFTWARE and OPENGLES3 selections. A valid Padding override survives
  removal of the unknown parameter; a missing processor input still refuses the build.
- **Cause verified in source:** RunImportAndProcess computes a local `effective` set and supplies
  it to ContentProcessorContext, but Build later assigns `result.parameters = request.parameters`.
  MakeContentBuildManifestEntry also copies result.parameters. That latter observation does not
  establish a manifest/cache failure: requested configuration may need its own fingerprint identity.
  The fix should distinguish requested versus effective metadata without silently changing cache
  semantics, or explicitly correct the public contract if requested parameters are intended.
- **Evidence:** `/tmp/cna-441-strictness-first-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` retains
  the initial unlabeled failure. Child stdout is authoritative (standalone JSON has zero checks);
  stderr is empty and private directories were independently confirmed absent. Build log:
  `/tmp/cna-441-contracts-build.log` (clean); metadata 441/0. Final labeled evidence is
  `/tmp/cna-441-strictness-final-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`: exactly the same two
  failures, 17 passes, and both returned key names printed. All 13 original checks are retained.
  Original focused streams and inspected source hashes are archived as
  `reports/checkpoint-441-contracts-focused-*`. No incorrect compiled output, GPU behavior or genuine-XNA comparison is claimed.
- **Environment:** Linux, GCC 14.2; observed CNA HEAD
  `69acd67e51b6ec542d8ad429825da3e5bfce89db`. The inspected pipeline header/implementation and
  XnaPipelineBridge.hpp had no uncommitted changes; other external C API work continues. This is
  not an atomic source/dependency snapshot. No CNA source was changed or upstream message sent.
- **Complete checkpoint:** 441-contracts at 2d28a13 passes all 454 enabled CTests and both full
  smoke runs. Exact comparisons with ORIGINAL 441-config add only this assertion (31/32 -> 32/33),
  retain every prior failure and match focused adapter checks. Normalized stderr is unchanged;
  all 30 selected runtime hashes match. Archives: `reports/checkpoint-441-contracts-*`.
- **Status:** open; an observable public metadata/contract discrepancy, not a demo failure.

### CNA-DEMOS-ISSUE-049 -- Build-environment overview incorrectly says built-ins ignore its fields

- **Area:** documentation only, the host-level build-facts overview above ContentBuildEnvironment
  in `modules/content/include/CNA/Content/Pipeline/ContentPipeline.hpp` (around lines 169-176).
- **Related demo:** `content.texture_processor.resize_to_power_of_two` (passes; no expected-failure marker).
- **Mismatch:** the overview says "None of them changes what a built-in CNA component does today."
  Actual TextureProcessor behavior consults targetProfile when processing an XNB-targeted texture.
- **Reproduction/observed:** register the native texture pipeline and ImportAndProcess the original
  100x60 PNG with Xnb output intent, Color format and generateMipmaps=true. Reach refuses it at
  Process/CNA.TextureProcessor with the power-of-two requirement. Changing only targetProfile to
  HiDef produces seven mip payloads at the original size. Explicit resizing instead permits Reach
  with 128x64 and eight levels; disabling mipmaps permits the unscaled Reach case. CNB output
  intentionally has no XNB profile gate and writes/decodes the original seven mip levels.
- **Evidence:** seven checks pass on SOFTWARE and OPENGLES3; the old rendered CNB scene remains
  RGBA-identical before/after and across renderers, and fresh resized base pixels match its loaded
  texture. Original streams/captures/source hashes are `reports/checkpoint-445-profile-focused-*`,
  with baseline captures `checkpoint-445-profile-before-*`; all 33 targeted CTests pass.
  The XNB cases inspect processed values, not written XNB bytes or XNA/Reach hardware execution.
- **Environment:** Linux/GCC 14.2; observed CNA HEAD `30831ba6ca01a8820e84720c12ddd10a04d52051`.
  The inspected pipeline header and texture processor source have no uncommitted changes; other
  C API work continues. This is not an atomic source/dependency snapshot.
- **Expected/suggested correction:** describe the environment as host facts that built-in and
  custom components can consult; remove the obsolete blanket claim. For example, distinguish
  XNB texture-profile policy from CNB processing without implying every field affects every route.
- **Status:** confirmed public-comment mismatch; runtime behavior is correct. No CNA source
  changed, upstream message sent, failing assertion suppressed or new known-failure demo added.

### CNA-DEMOS-ISSUE-050 -- Owner-only C game operations enter the foreign-thread queue

- **Area:** C ABI game-thread affinity; generated `Runtime.inc`, `CnaCApiRuntime.cpp` and
  `CnaCApiDetail.hpp/.cpp` in `modules/c-api/src`.
- **Contract:** public `CNA/C/runtime.h` says driving or destroying the game still refuses a
  foreign caller when foreign-thread dispatch is enabled. Ordinary resource/property calls may
  be queued; this exception protects the owner-driven game lifecycle.
- **Observed:** with dispatch enabled and the owner never pumping, a worker's
  `cna_game_run_frame_ext` or `cna_game_destroy` blocks. Disabling dispatch releases it with
  `CNA_RESULT_INVALID_STATE` and category STATE, rather than its required direct THREAD refusal.
  Both operations reproduce in separate fresh processes under SOFTWARE and OPENGLES3 selections.
  No Game loop, rendering or queued destruction is executed by this probe.
- **Why this proves queuing:** a readiness flag or a delayed thread alone would be inconclusive.
  Here cancellation returns the queue's INVALID_STATE result. Each probe additionally verifies
  that the owner's target step is still 166667 and owner-thread game destruction succeeds.
  A thread that has not entered its call before dispatch is disabled returns a distinct
  inconclusive exit, not this defect result.
- **Reproducer:** `tools/repro_c_api_owner_thread.c` uses only public C APIs and bounded C11
  waits. It never pumps the forbidden call, cancels before joining and terminates the isolated
  process if cancellation cannot release the worker. Exit 0 means direct THREAD refusal;
  exit 1 confirms this defect; exit 2 requires investigation. Build in the existing tree:

  ```bash
  CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv ccache gcc-14 -std=c11 -Wall -Wextra -Werror -pthread \
    -I ../cna/modules/c-api/include tools/repro_c_api_owner_thread.c \
    -L build/linux-full/CNA/modules/c-api -Wl,-rpath,'$ORIGIN/../CNA/modules/c-api' \
    -lcna_c_api -o build/linux-full/bin/cna-repro-c-api-owner-thread
  export CNA_GRAPHICS_RENDERER=SOFTWARE SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11
  env -u WAYLAND_DISPLAY xvfb-run -a build/linux-full/bin/cna-repro-c-api-owner-thread
  env -u WAYLAND_DISPLAY xvfb-run -a build/linux-full/bin/cna-repro-c-api-owner-thread destroy
  ```

- **Evidence:** both initial and final-source probes exit 1 for both operations on both renderer
  selections. Native renderer initialization/selection INFO diagnostics on stderr are preserved. Every final line reports `cancelled=1, result=3, diagnostic=0,
  category=3, disable=0, owner_read=0, ticks=166667, cleanup=0`. Original logs/exits, the initial
  source, selected source/binary hashes and clean-state observations are archived under
  `reports/cna-issue-050-*`. The seven-target incremental rebuild completed without diagnostics.
- **Source analysis:** these generated routes use `CallWithExceptionBarrier`. Their handle
  lookup records a foreign-thread refusal, so that barrier queues the body when dispatch is on.
  `GetCallableGame` adds a callback-reentrancy guard but does not prevent this retry. The same
  generated shape appears on other driving routes; only RunFrameEXT and Destroy were exercised.
  Executing a forbidden queued body through a pump was deliberately not tested.
- **Suggested correction:** encode owner-only routing in the generator/specification and use
  exception translation without foreign-thread retry for these routes. Preserve normal queued
  calls and their error forwarding; do not weaken the declared owner-thread contract.
- **Environment/status:** confirmed on Linux/GCC 14.2. CNA was clean at
  `4ee1e0b553667b6e999248fcefdea729654ee497` before rebuild and after reproduction. This records
  the observed clean source and selected artifacts, not an atomic full dependency snapshot.
  Open; no CNA files changed or upstream message sent.
- **Catalog integration:** `c_api.game.foreign_thread_dispatch` now carries ISSUE-050 and checks
  both owner-only operations without pumping. Focused SOFTWARE/OPENGLES3 runs retain all eighteen
  original passing assertions and produce exactly 28 passes/two labeled failures out of 30.
  Both cancellation results are INVALID_STATE/STATE; the owner remains usable and destroys the
  game. Delayed pre-call scheduling is separately inconclusive. Both stderr streams are empty.
  All 31 focused C/CLI CTests pass, including this expected-failure test (7.31 s). Source/logs:
  reports/checkpoint-446-owner-focused-*. Complete 446-owner at 9e7a27b passes all 459 enabled
  CTests and both full smoke runs. Exact ORIGINAL 446-reentry comparisons add only these two
  failures (32/33 -> 34/35), retain all prior failures and preserve every other demo status/check
  count and C/custom-content child list. Selected hashes and normalized stderr are unchanged.
  Original evidence and strict reviews: reports/checkpoint-446-owner-*.

### CNA-DEMOS-ISSUE-051 -- Host-frame failure contract promises suppressed C exit hooks

- **Area:** public `CNA/C/runtime.h` host-frame contract and the C lifecycle adapter in
  `CnaCApiRuntime.cpp`. This concerns C callback tables, not whether native Game::EndRun executes.
- **Contract:** cna_game_run_frame_ext explicitly says an exit through a failed callback completes
  the run with `exiting`, then `end_run`, returns CALLBACK and reports out_running=false.
- **Observed:** after an ordinary successful variable-time frame, the second Update returns IO
  with a supplied message. RunFrameEXT returns CALLBACK (9) and false, but neither C exit hook
  runs. A later host call returns CALLBACK/false without replay, and destruction returns CALLBACK
  and invokes UnloadContent exactly once. Terminal state and cleanup work; the promised C hook
  sequence is the discrepancy. No rendered-content or error-message preservation claim is made.
- **Reproducer:** tools/repro_c_api_host_failure.c records independent counters and an ordered
  trace. Exit 0 means the promised ILBUDUXEZ sequence; exit 1 confirms ILBUDUZ with both exit
  counters zero; exit 2 is a different/inconclusive result. Build in the existing tree:

  ```bash
  CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv ccache gcc-14 -std=c11 -Wall -Wextra -Werror \
    -I ../cna/modules/c-api/include tools/repro_c_api_host_failure.c \
    -L build/linux-full/CNA/modules/c-api -Wl,-rpath,'$ORIGIN/../CNA/modules/c-api' \
    -lcna_c_api -o build/linux-full/bin/cna-repro-c-api-host-failure
  export CNA_GRAPHICS_RENDERER=SOFTWARE SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11
  env -u WAYLAND_DISPLAY xvfb-run -a build/linux-full/bin/cna-repro-c-api-host-failure
  ```

- **Evidence:** initial and final persistent-source builds reproduce exit 1 on SOFTWARE and
  OPENGLES3. Both produce first `ILBUD`, failed/terminal `ILBUDU`, final `ILBUDUZ`, two updates,
  zero exiting/end_run callbacks and one unload. Renderer INFO stderr is preserved (89/782 bytes)
  and identical between initial/final runs. Original streams, exits, initial source and selected
  source/binary hashes: reports/cna-issue-051-*. GCC 14/C11/-Wall/-Wextra/-Werror builds pass.
- **Cause:** native RunFrameEXT calls OnExiting and EndRun when the run ends. The C adapter routes
  both tables through Invoke, whose existing callbackFailure_ guard suppresses them. UnloadContent
  explicitly opts into runAfterFailure and therefore still runs. This is distinct from ISSUE-050
  and the passing owner-thread reentry guards.
- **Suggested resolution:** reconcile the public promise with the intended failure policy. If
  suppressing ordinary callbacks after failure is intended, document that exception explicitly;
  otherwise provide the promised completion hooks while retaining the original failure and
  exactly-once cleanup. This probe does not establish genuine XNA behavior or recommend changing
  the native Game lifecycle. Other failing callback locations and browser scheduling are untested.
- **Environment/status:** confirmed contract/implementation mismatch on Linux, CNA observed clean
  at 4ee1e0b553667b6e999248fcefdea729654ee497 before/after reproduction. Selected evidence is not an
  atomic dependency snapshot. Open; no CNA edits or upstream message.
- **Catalog integration:** `c_api.game.host_failure` now gives this terminal-failure contract its
  own fresh process, preserving the existing normal host and RunOneFrame scenarios. Focused
  SOFTWARE/OPENGLES3 runs pass nine checks and fail exactly the combined promised C exit-hook
  assertion. The supplied callback diagnostic survives, later host calls remain terminal, unload
  runs once and the released handle is invalid despite destruction returning CALLBACK. Both
  traces are ILBUDUZ and stderr is empty under the demo logger. All32 focused C/CLI CTests pass
  (7.36 s), including the expected-failure marker. Evidence: reports/checkpoint-447-focused-*.
  Complete 447 at 4272ce1 passes all 460 enabled CTests and both full smoke runs. Exact original
  comparisons add only this ID/assertion (34/35 ->35/36), retaining all older statuses/counts/
  failures and C/custom-content child lists. Selected 33 binary/four asset hashes and normalized
  stderr match. Full archives/reviews: reports/checkpoint-447-*. The minimal reproducer remains available.




### CNA-DEMOS-ISSUE-054 -- Cube DDS loading ignores short reads and masks offset truncation

- **Area/API:** `TextureCube::DDSFromStreamEXT`, `modules/graphics/src/Xna/TextureCube.cpp`.
- **Demo:** `graphics.texture_cube.dds_streams`; five controls, three pass and two fail on both
  SOFTWARE and OPENGLES3. The demo is marked with this issue and retains both failing assertions.
- **Fixture and reproduction:** an original 506-byte RGB24 DDS contains six 4x4 faces and authored
  2x2/1x1 levels, with colors varying by face and level and a final nonzero blue byte (255).
  Load it from MemoryStream and verify all eighteen face/level images using public GetData.
  Repeat from position 16 of a stream containing a sixteen-byte prefix plus the same payload.
  Both complete controls pass and return size 4, three levels and opaque Color data.
  Then use a MemoryStream subclass whose Read returns at most seven bytes. Finally remove the
  same last payload byte and try it at positions zero and sixteen.
- **Observed:** the legal short-read stream is read once (seven bytes) and rejected with
  `TextureCube::DDSFromStreamEXT: invalid DDS flags`. Ordinary 505-byte truncation is correctly
  refused with System::FormatException (`truncated DDS stream`). With the sixteen-byte prefix,
  the same truncated payload is accepted and the decoded data no longer matches the full fixture.
  Both renderers produce exactly the same two failed checks and empty stderr after the demo's
  exception catch was corrected.
- **Expected:** honor Stream::Read's documented returned byte count, accumulate positive short
  reads until the intended payload is complete, and reject the same truncated encoded data
  regardless of its starting position. Prefix bytes before Position are not unread DDS bytes.
- **Source investigation:** the method allocates a zero-initialized buffer of total stream Length,
  calls Read once and ignores its return value, then passes the total Length to DecodeDdsCube.
  The prefixed truncated stream has total Length 521 but only 505 unread bytes. The extra zero
  bytes make the decoder's 506-byte bounds check pass and replace the missing nonzero component.
  The decoder itself correctly refuses the unprefixed truncation. This is a stream-consumption
  defect in the cube loader, distinct from SavePicture's analogous ISSUE-041.
- **Demo correction, not a CNA defect:** the first draft caught std::runtime_error for truncation;
  CNA actually throws System::FormatException. Those initial runs stopped after three checks.
  Their source/logs are retained as `checkpoint-459-initial-*`; the catch was corrected explicitly
  before the five-check reproduction, archived as `checkpoint-459-dds-fixed-*` and
  `checkpoint-459-dds-unmarked-source.cpp`. No reader workaround or failure suppression was added.
- **Suggested repair:** read the actual remaining payload to completion (respecting each Read
  result), and give the decoder only initialized bytes actually read; preserve truncation errors.
  No CNA file was modified by this task.
- **Complete checkpoint:** 459 at 5ed8055 passes all 472 enabled CTests and both full smoke runs.
  Original457 comparison adds exactly these two failures (35/36 -> 37/38), retains every older
  failure/status and C child list after six unrelated passing destruction controls. Runtime hashes
  are stable; stderr adds only the separately declared SOFTWARE package skip diagnostic.
- **Environment/status:** Linux, GCC 14.2/CMake 3.31.6 linux-full, explicit X11/Xvfb and dummy audio.
  CNA 2202cb4f1483f7bb54c12c9f9a88c0bf374fe0a6 observed after reproduction, with unrelated
  documentation/content-test changes present. Source observations/hashes are archived separately;
  no atomic build-input snapshot is claimed. Open; no upstream message or genuine-XNA execution.

### CNA-DEMOS-ISSUE-055 -- ShaderEffect clone loses matrices and accepts a disposed original

- **Area/reproduction:** `graphics.shader_effect.clone_state`, OPENGLES3 under isolated X11/Xvfb,
  at least two frames. An explicitly labeled GLSL pair compiles, receives nonidentity World,
  View and Projection, and is cloned through the public owning `Effect*` return. Both programs
  draw one triangle in equal viewports; the original is then disposed while its device stays alive.
- **Observed:** nine checks execute, six pass and exactly three fail. World translation changes
  -0.75 -> 0, View translation 0.25 -> 0 and projection X scale 0.5 -> 1. The clone's triangle is
  larger and differently positioned. Calling Clone on the disposed original returns another
  compiled effect instead of throwing System::ObjectDisposedException. Both GL stderr streams
  are empty. SOFTWARE declares its CustomEffects capability skip; no SOFTWARE clone execution.
- **Expected/contract scope:** Effect::Clone promises an independent clone with current values
  and explicitly throws ObjectDisposedException after disposal. ShaderEffect documents a
  deliberate fresh-program recompilation instead of program sharing; it does not document
  resetting public IEffectMatrices state. Matrix retention is the state-copy interpretation of
  those clone contracts; the disposed-object refusal is explicit. This report makes no claim
  that raw SetUniform values or texture bindings must be copied.
- **Cause observed in source:** ShaderEffect::Clone constructs from the two source strings and
  copies language/labels only. Its three matrix fields retain identity defaults, and it does not
  guard IsDisposed before dereferencing the still-live device and constructing a new effect.
  The demo owns every returned pointer and restores the clone's observed World after its
  independent-mutation check, so it does not silently repair the omitted values before drawing.
- **Positive controls:** original and clone compile in the same explicit language with identical
  sources; changing the clone's World leaves the original unchanged; both draw visible output.
  Public GraphicsResource::Dispose releases the original program while the clone remains valid,
  and the surviving clone's checked pixels stay identical in the next frame.
- **Evidence:** reports/checkpoint-460-initial-* preserves the unmarked runs and both inspected
  800x480 images. Frame one has eight checks; frame two adds the passing survival assertion.
  checkpoint-460-unmarked-clone_state.cpp, -observed-ShaderEffect.cpp/.hpp, -observed-Effect.hpp
  and -focused-observation.json retain source observations. CNA f130abff4 was observed with
  external content-registration changes; these are not atomic build-input snapshots.
- **Suggested repair/status:** guard disposed state and copy World/View/Projection when creating
  the independent program. Open; no CNA source edited, upstream message, physical-GPU or
  desktop-GL runtime claim. Marked focused461 repeats all nine checks and the same three failures;
  its frame-two capture is RGBA-identical to the inspected initial image. Full 461 at a447ad2
  passes 474 enabled CTests and both smokes, retaining every original 459 failure and adding
  exactly these three GL assertions (38 ->41; SOFTWARE37 unchanged). Runtime hashes stay stable;
  stderr adds only separate declared skips and the intentional compiler-diagnostics fixture.

## Resolved

### CNA-DEMOS-ISSUE-053 -- In-flight framework update registration uses an out-of-scope array

- **Area:** external uncommitted `modules/audio/src/Xna/FrameworkDispatcher.cpp` and new
  `CNA/Internal/Audio/FrameworkUpdates.hpp`, during the link-boundary refactor.
- **Reproduction:** documented GCC 14.2/CMake 3.31.6 linux-full incremental build with the shared
  compiler cache and `-j4`, after the first 456 batch build succeeded.
- **Observed:** `FrameworkUpdates::Install` is defined in `CNA::Internal::FrameworkUpdates`,
  but names `frameworkUpdates` without qualification. The array lives in an anonymous namespace
  nested inside `Microsoft::Xna::Framework`; it is not found in the Install function's scope.
  GCC rejects the translation unit, and Ninja exits 1 before the current sentinel-enhanced
  launch-parameter demo can be linked or run. This is a CNA build defect, not a C demo failure.
- **Expected:** both registration and dispatch can access the intended shared update array
  through valid C++ name lookup.
- **Evidence:** `reports/checkpoint-456-failed-build.log`, `-failed-state.json`, archived
  `-failed-FrameworkDispatcher.cpp` and `-failed-FrameworkUpdates.hpp`. HEAD was ced719909 with
  ongoing external audio/media/content/runtime edits; this is not a clean-HEAD reproduction or
  an atomic source snapshot. The successful first build is separately retained as
  `checkpoint-456-initial-build.log`; no new focused runtime or complete 456 checkpoint yet.
- **Capture correction:** direct source review on continuation found that the archived
  failed-FrameworkDispatcher.cpp and failed-state diff already contain the external repair:
  the array is in a translation-unit-global anonymous namespace. The source changed between
  the genuine failed compilation and capture. Comparing current hashes to that repaired capture
  incorrectly prolonged the reported blocker; those prior interpretations are superseded by
  reports/checkpoint-456-evidence-correction.json. The failure log is retained unchanged.
- **Repair observed:** external commit 7d8eba687 contains the global-namespace array.
  HEAD 18d8398af was observed during repair review, with later build-system edits. The existing linux-full retry passed (200 scheduled targets; no compiler diagnostics). Both
  focused launch-parameter runs pass all fifteen child checks with empty stderr. Resolved for
  the exercised build; subsequent full456 validation passed all469enabled CTests and both
  smoke suites with exact prior35/36failed assertions retained. No CNA source edited/reverted, warning suppressed, runtime
  knownIssue marker or upstream message.



### CNA-DEMOS-ISSUE-052 -- In-flight cgltf implementation split breaks the importer build

- **Area:** external uncommitted changes to `modules/content/src/GltfImport/GltfImportCore.cpp`
  and new `CgltfImplementation.cpp`, while refactoring CNA link boundaries/GamerServices.
- **Reproduction:** reuse the documented GCC 14.2/CMake 3.31.6 linux-full tree and run
  `CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv cmake --build --preset linux-full -j4`.
- **Observed:** the first 455-batch build exits 1 in GltfImportCore.cpp. Removing
  CGLTF_IMPLEMENTATION from that translation unit makes its existing calls to
  cgltf_component_read_index and cgltf_element_read_float undeclared. Both helpers are static
  inside cgltf.h's implementation section. The same removal loses transitive C memory/string
  declarations used by the local stb implementations (memcpy, memset and related errors).
  The two new material/specular demo objects compile, but the new executable is not linked.
- **Expected:** separating cgltf's public implementation into one translation unit should leave
  the importer self-contained and preserve its sparse-accessor decoding through available APIs
  or deliberately retained local helpers, with explicit standard-header dependencies.
- **Evidence:** `reports/checkpoint-455-initial-build.log`, `-cna-state.json`, archived initial
  GltfImportCore.cpp/CgltfImplementation.cpp/cgltf.h and `-runtime-integrity.json`. HEAD was
  3ee6ae3c83d0eb59551f496d6c85109e377edc81 with ongoing external edits; this is **not** a clean-HEAD
  reproduction or an atomic source snapshot. All 34 selected binaries from complete checkpoint
  453 remained unchanged after the failed build; the new demos have no runtime evidence yet.
- **Resolution:** the external author added explicit string.h and a CgltfInternals.hpp interface;
  CgltfImplementation.cpp supplies wrappers reaching the two implementation-local helpers.
  The same linux-full build then succeeds (111 scheduled targets, known Draco CMP0148 developer
  warning on regeneration, no compiler diagnostics). All 47 relevant C/graphics/CLI CTests pass
  (18.80 s; one capability skip), and both new four-check scenes pass under SOFTWARE/OPENGLES3.
  Original build evidence remains above; repaired-source hashes, retry log and focused results
  are in `reports/checkpoint-455-focused-*`. External uncommitted work is still ongoing, so this
  is not a clean-commit or atomic-input claim. No CNA source edited/reverted or warning suppressed
  by cna-demos. The external cgltf split/fix was subsequently committed as bf9089edc; later clean
  ced719909 was observed during checkpoint 455 runtime. Resolved for the exercised build; no
  runtime knownIssue marker or upstream message.


### CNA-DEMOS-ISSUE-044 -- In-flight C API generation shadows a result-handle parameter

- **Area:** external work in `modules/c-api/src/generated/Cnb.inc`, generated by
  `tools/c-api/generate_routes.py`; found by the cna-demos 432-batch build.
- **Observed:** cna_cnb_cnj_result_copy_asset_type_name receives a handle parameter named result,
  but the generated lookup declared `const CNA_Result result = GetCompiledCnj(result, &object)`.
  The local shadows the parameter even within its initializer, reading an uninitialized local.
  GCC 14.2 with CNA's -Werror rejects both that use and the now-unused handle parameter.
- **Evidence:** `/tmp/cna-432-build.log` records the exact generated source line and diagnostics.
  This was an uncommitted, in-flight extension of CNA's route generation over HEAD 4eed3baa7;
  it is not evidence that clean 4eed3baa7 itself failed this build.
- **Resolution:** the external author changed the generated temporary to routeResult while
  this task was investigating. No CNA source was edited by cna-demos. Resuming the same build
  succeeds without warning/error diagnostics: `/tmp/cna-432-retry-build.log`.
  The new C++ graphics demos subsequently pass on SOFTWARE and OPENGLES3, and all 16 C ABI
  demos pass the focused CTest run (`/tmp/cna-432-targeted-ctest.log`).
- **Scope:** a confirmed transient generator/build defect, not a demo defect or a new expected
  runtime-failure marker. This entry does not claim that every possible generated-name collision
  is covered or that the evolving C API has comprehensive behavioral coverage.
