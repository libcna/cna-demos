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

### CNA-DEMOS-ISSUE-012 -- `ContentManager::Load<T>` never calls an `OpenStream` override

- **Area:** content -- `ContentManager::Load<T>` (header template) resolves `.xnb`, `.cnb` and
  registered loose-file readers directly on the filesystem (`TryReadAssetBytes`). Only the protected
  `ReadAsset<T>` goes through the virtual `OpenStream`, and `Load<T>` (not virtual in C++) never
  calls it. `ResourceContentManager` overrides only `OpenStream`, so its `Load<T>` does not read
  resources either.
- **Demo:** `content.content_manager.open_stream_override` (knownIssue; expected failure): a
  derived manager serving an XNB texture from memory; `ReadAsset` works, `Load` does not.
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

---

## Resolved

(none yet)
