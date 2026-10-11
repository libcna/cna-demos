# Changelog

All notable changes to cna-demos. The format follows [Keep a Changelog](https://keepachangelog.com/);
versions follow Semantic Versioning (pre-1.0: a minor version may change the CLI or metadata).

## [Unreleased]

### Added
- Project foundation: CMake integration with a CNA source checkout (`CNA_ROOT_DIR`, presets
  `default`, `windowless`, `linux-multi`, `linux-full`), Ms-PL licence, documentation set.
- Demo framework: self-registering registry, `DemoInfo` metadata, console/Game/standalone demo
  kinds, `DemoGame` base class (need checks, automated frame limit, Escape to exit, screenshots),
  `DemoContext` (output, checks, seed), runner with PASSED/FAILED/SKIPPED/NOT RUN results.
- `cna-demos` executable: interactive terminal browser and CLI (`--list`, `--categories`,
  `--search`, `--info`, `--run-category`, `--run-tag`, `--run-all`, `--smoke`, `--catalog-json`,
  `--list-ids`, `--validate`, `--renderers`, `--renderer`, `--frames`, `--seed`, `--screenshot`,
  `--report`).
- CTest integration: CLI tests and per-demo tests discovered at ctest time.
- Coverage report generator (`tools/coverage_report.py`) and deterministic asset generator
  (`tools/generate_assets.py`).
- Pilot demos in seven categories (math, core, graphics, content, audio, input, platform),
  including one standalone demo.
- Math demos: Vector2/Vector3/Vector4 (constants, arithmetic, dot, cross, normalize, distance,
  lerp, smooth step, Catmull-Rom, Hermite, barycentric, clamp/min/max, reflect, transforms,
  by-reference overloads, equality/ToString, homogeneous coordinates), Matrix (identity, SRT,
  axis rotations, axis-angle, yaw/pitch/roll, look-at, perspective, orthographic, invert,
  transpose, determinant, decompose, world/direction properties, billboards, reflection/shadow,
  element-wise operations, quaternion transform), Quaternion (axis-angle, matrix conversion,
  concatenate, slerp/lerp, inverse/conjugate, normalize), MathHelper (angles, scalar helpers,
  curves).
- `--log-level` option (CNA's logger defaults to `warn` in cna-demos).
- `tools/validate.sh`: build, validate, ctest, smoke reports and coverage in one command.
- `CnaDemos/MathChecks.hpp`: tolerance helpers for matrix/quaternion checks.
- Math demos: Point, Rectangle (edges, contains, intersect/union, inflate/offset), BoundingBox,
  BoundingSphere, BoundingFrustum culling, Plane, Ray, Curve (evaluate, tangents, loop types, step
  continuity, key collection), Color (construction, premultiplied alpha, lerp/vectors, packed
  value); packed vector formats (compact colour, half precision, signed normalized, integer,
  wide unorm, IPackedVector).
- `knownIssue` metadata: demos that fail because of a recorded CNA defect are expected failures in
  batch runs and CTest (`WILL_FAIL`), and an unexpected pass is reported.
- Game lifecycle demos (25): method order, Exit/Exiting, RunOneFrame, host-driven RunFrameEXT,
  fixed/variable time step, IsRunningSlowly, SuppressDraw, GameComponent, DrawableGameComponent,
  UpdateOrder/DrawOrder, component collection events, GameServiceContainer, back-buffer size,
  PreparingDeviceSettings, device events, Reach/HiDef profile, vertical sync, full screen (manual),
  window title/client bounds/resizing, IsMouseVisible/InactiveSleepTime, LaunchParameters,
  TitleContainer.
- SpriteBatch demos (14): destination/source rectangles, sprite-sheet animation, tint and fade,
  rotation/origin, scale, SpriteEffects flips, layer depth and sort modes, blend states, straight-alpha
  loose PNGs (NonPremultiplied), point/linear sampling, wrap tiling, transform-matrix camera.
- Graphics device demos (12): ClearOptions (and the error for a missing plane), split screen with
  viewports (XNA 4.0's Clear ignores the viewport), Viewport properties and Project/Unproject
  picking, scissor rectangle, PresentationParameters, GetBackBufferData (whole/region),
  SupportsCapability and the capability report (CNAEXT), GraphicsAdapter display modes and
  format queries, GraphicsResource lifetime events (ResourceCreated/Destroyed, Name, Tag).
- `CnaDemos/GraphicsNames.hpp`: names of graphics enums for demo output.
- Texture2D demos (7): procedural SetData, rectangle SetData on a canvas (and XNA's rule that a
  texture set on the device cannot be written), per-pixel collision with GetData, mipmap levels,
  16-bit surface formats, FromStream (as stored, fitted, zoomed), SaveAsPng/SaveAsJpeg round trips.
- SpriteBatch demos (5): SpriteSortMode::Texture, Immediate mode with device states changed between
  Draws (blend state; sampler state is CNA-DEMOS-ISSUE-004), several batches per frame, Begin/End
  rules.
- Compiled content: `content/` is built at build time by CNA's `cna-content` (`cna_add_content`)
  into `bin/assets`; `CNA_DEMOS_CONTENT_EXECUTABLE` names a host tool for cross builds.
- Original pixel font (`tools/pixel_font.py`), generated as an XNA font-texture sheet and compiled
  with `CNA.FontTextureProcessor` into `fonts/pixel_font`.
- SpriteFont demos (7): DrawString, MeasureString (centring, alignment, line rules), rotated/scaled/
  mirrored text, Spacing/LineSpacing, Characters/DefaultCharacter, StringBuilder text, and a font
  built in code with the CNAEXT SpriteFont constructor.
- Render-state demos (9): custom blend functions (multiply, screen, subtract, min, max), colour
  write channels, blend factor, bound states are immutable, texture address modes, wireframe
  sprites, back-face culling, depth testing, 2D stencil masking.
- 3D primitive and buffer demos (8): hello triangle with BasicEffect, primitive types, indexed
  triangles, VertexBuffer + DrawPrimitives, DynamicVertexBuffer with Discard, indexed cube,
  32-bit indices (90,000-vertex grid), custom vertex declaration.
- `CnaDemos/PixelChecks.hpp`: back-buffer pixel checks for Game demos.
- Render-target demos (5): Discard/Preserve usage, reading a target back, a target with its own
  depth buffer, mipmapped target, multisampled target.
- More render-state demos (7): DepthRead for transparency, depth-function X-ray silhouette, stencil
  overlap counting, depth bias for decals, MaxMipLevel, LOD bias (skipped on GL ES), anisotropic
  filtering.
- Effect demos (14): BasicEffect texture, default lighting, three coloured directional lights,
  per-pixel lighting, material colours, fog, alpha; AlphaTestEffect cut-outs; DualTextureEffect
  light map (two texture-coordinate sets); EnvironmentMapEffect with a TextureCube and Fresnel;
  SkinnedEffect with two bones; Effect parameters/techniques and Clone; CNA's PbrEffect.
- Audio demos (7): SoundEffect from PCM, Play with volume/pitch/pan and MasterVolume, instance
  playback state, looping, volume/pitch/pan ranges, Apply3D panning (smoke-only),
  DynamicSoundEffectInstance streaming, microphone input level (manual).
- Input demos (14): key pressed/released by state comparison, KeyboardState values and replay,
  key names and layouts (CNAEXT), modifier and lock keys (CNAEXT); mouse state, clicks and
  dragging, SetPosition and mouse look, relative mode, cursor shapes and a cursor from a texture,
  the ClickedEXT event; typing with TextInputEXT, input-method composition; clipboard text and
  MIME data formats (CNAEXT).
- Gamepad and touch demos (12): gamepad state, capabilities, GamePadState values and virtual
  buttons, dead-zone modes, vibration, keyboard emulation and device information (CNAEXT); touch
  state with mouse emulation, capabilities and coordinate space, gestures, TouchLocation and
  TouchCollection values, the gesture queue with the CNAEXT EnqueueGesture.
- CNAEXT device demos (12): raw joystick state, hot-plug events, device lists, force-feedback
  rumble and effects, device motion sensors, battery state; controller light bar, motion sensors,
  touchpad, trigger vibration, paddles and the share button.
- CNA-DEMOS-ISSUE-009 recorded: `HapticDevice`'s inline default constructor needs a complete
  platform type, so user code cannot create a closed device.
- Storage demos (7): StorageDevice selector (async pattern, free/total space), save and load a game
  with BinaryWriter/BinaryReader, FileMode/FileAccess, directories and search patterns, container
  lifetime (Disposing), where saves are stored and the per-player layout (CNAEXT), and CNA's rule
  that container paths stay inside the container.
- CNA-DEMOS-ISSUE-010 recorded: storage reports argument and filesystem errors with
  `std::invalid_argument`/`std::filesystem::filesystem_error` instead of XNA's exception types.
- Content demos (15): TextureProcessor parameters (colour key, premultiplied alpha against a loose
  PNG, mipmaps, resize to a power of two), XNB textures with DXT compression, a Curve from CNJ, a
  compiled WAV, ContentManager cache and Unload, one manager per level, missing assets, the CNAEXT
  manifest, a custom LooseFileContentTypeReader, an OpenStream override and Unload disposal (the last
  two expected failures).
- `content_xnb/` and the `cna_demos_content_xnb` target: content compiled into XNA's XNB format.
- CNA-DEMOS-ISSUE-011 (Unload does not dispose loaded assets), CNA-DEMOS-ISSUE-012 (Load<T> never
  calls an OpenStream override) and CNA-DEMOS-ISSUE-013 (the manifest lists the pipeline's
  bookkeeping files) and CNA-DEMOS-ISSUE-014 (before CNA's platform exists, the content root is the
  working directory) recorded; ISSUE-010 extended to ContentManager::Load after Dispose.
- Media demos (7): play a song and follow PlayPosition to its end, MediaPlayer events raised by
  FrameworkDispatcher::Update without a Game, playlists (Queue, MoveNext/MovePrevious, IsRepeating,
  IsShuffled), volume and mute, a spectrum/waveform visualizer, Song properties, MediaLibrary
  collections; two short generated tunes in `assets/music/`.
- `--screenshot` captures the last frame drawn even when a demo ends itself with `Exit()` before
  its frame limit (the frame is kept each frame and saved in `EndRun`).
- CNA-DEMOS-ISSUE-015 recorded: `Song::FromUri` resolves relative paths against the working
  directory, not the title location (FNA's behaviour).
- Model demos (7): Model::Draw, bones and their transforms, bone animation with the per-mesh drawing
  loop, frustum culling with mesh bounding spheres, per-mesh effect settings, skinned animation with
  AnimationPlayer and SkinnedEffect, and the CNAEXT glTF import report of a glTF loaded at run time.
- `tools/gltf_models.py`: original glTF scenes (a robot arm with baked vertex colours, a skinned
  strip with a bend clip) compiled by the content pipeline; one loose copy for run-time loading.
- XACT demos (5): AudioEngine/WaveBank/SoundBank with a cue played by name, Cue states, categories
  (volume, pause, stop), global variables, positioned cues; `tools/xact_files.py` writes the
  `.xgs`/`.xwb`/`.xsb` files (original tones) in the layout of CNA's own XACT example.
- Gamer services demos (10): signed-in gamers and the SignedIn event, Guide message box and keyboard
  input (password mode, cancel), trial mode with SimulateTrialMode, the Guide's sign-in screen for a
  second player, achievements from an offline title catalog, GamerProfile, rich presence,
  GameDefaults and GamerPrivileges -- offline, with local profiles.
- Graphics, model and content demos (7): occlusion queries, VertexBuffer::GetData,
  DynamicIndexBuffer with Discard, Texture3D sub-box data, glTF node-animation clips
  (ModelAnimationsEXT, ApplyClipToBonesEXT) on a generated turntable model, and the same font, sound
  and curve compiled to CNB and to XNB.
- Media demos (2): `media.video_player.play_video` plays a generated WMV and
  `media.song.compiled_song` a generated Ogg Vorbis tune, both compiled by the content pipeline
  (`content_media/` into `assets/media`, compiled only when CNA has its FFmpeg backend;
  `tools/media_files.py` draws the frames and computes the audio, ffmpeg encodes them).
  `Need::Video` now skips without CNA's video backend.
- CNA-DEMOS-ISSUE-020 recorded: a compiled Song/Video stream is resolved from the ContentManager's
  root, not beside the asset.
- Coverage report: a bare XNA type name in `.apis` (an enumeration, an exception type) now credits
  that type.
- Phase 13 demos (17): devices (sensor availability, keyboard-emulated accelerometer,
  VibrateController, CNA::Devices machine information), phone (PhoneApplicationService lifecycle),
  design (TypeConverter text, cultures, property descriptors, InstanceDescriptor), diagnostics
  (modes, counters and gauges), core logging (sink, levels), cnaext (ASCII post-process, CRT effect,
  colour depth reduction, DebugDraw).
- Game demos (4): ResetElapsedTime after a load, GameTime constructors, CNAEXT file/text drop, focus
  and pausing (IsActive, OnActivated/OnDeactivated).
- CNA-DEMOS-ISSUE-019 recorded: `Game::ResetElapsedTime` is ignored with a fixed time step.
- Net demos (6): a Local session of two players (GamerJoined replayed on subscription, host, ids,
  machine), the lobby/ready/StartGame/EndGame flow, NetworkSessionProperties, BeginCreate/EndCreate,
  game objects in Gamer::Tag, and a PacketWriter -> SendData -> ReceiveData -> PacketReader round
  trip in a System Link session (`Need::Network`, the only demo that opens sockets).
- CNA-DEMOS-ISSUE-018 recorded: System Link socket failures surface as `std::runtime_error`
  instead of `NetworkNotAvailableException`.
- Avatar demos (4): AvatarRenderer with the Wave preset and its loading state, a custom pose from
  AvatarBone transforms with an AvatarExpression and the skeleton (ParentBones, BindPose),
  AvatarDescription bytes round trip and invalid descriptions, six animation presets side by side.
- Leaderboard demo: two local players write scores during a LocalWithLeaderboards session and
  LeaderboardReader reads the board and a page around one player; the `gamer_services` category
  links CNA::Net for it. `GamerServicesDemo` takes the names of several local players.
- `ownProcess` metadata and `--batch-child`: demos that set up once-per-process CNA state (gamer
  services) run as `cna-demos <id>` in a process of their own from batch runs and the browser.
- `CnaDemos/GamerServicesDemo.hpp` (offline configuration, the demos' own profile store, "Demo
  Player" signed in, the `GamerServicesComponent`) and `CnaDemos/GamerServicesNames.hpp`.
- `title/` and the `cna_demos_title_files` target: files CNA reads by a fixed title-relative path,
  starting with the offline achievement catalog `GamerServices/Achievements.json`.
- `cna-demos` names `libcna.so` first on its link line, so CNA's static gamer services library
  cannot pull a second copy of CNA into the executable (CNA-DEMOS-ISSUE-016, double free at exit).
- CNA-DEMOS-ISSUE-016 (linking `CNA_GamerServices` after another library duplicates CNA) and
  CNA-DEMOS-ISSUE-017 (`Achievement::EarnedOnline` is always true) recorded.
- `CnaDemos/InputNames.hpp`: names of input enums (keys, buttons, gestures) for demo output.
- CNA-DEMOS-ISSUE-008 recorded: sharp-runtime's `Prop.hpp`, reached from CNA's public headers,
  defines macros named `YES`, `NO`, `nothing` and similar.
- Custom shader demos (2): toon shading with a GLSL `ShaderEffect`, a grayscale sprite shader
  through `SpriteBatch::Begin`; `ColorMatrixEffect` sepia/negative on SOFTWARE.
- CNA-DEMOS-ISSUE-007 recorded: GPU renderers silently ignore `ColorMatrixEffect`.
- `CnaDemos/Geometry.hpp`: sphere and cube meshes with normals and UVs for 3D demos.
- Screenshots are saved opaque (what the window shows): XNA's SaveAsPng, which CNA matches,
  stores fully transparent pixels as (0, 0, 0, 0).
- CNA-DEMOS-ISSUE-006 recorded: no capability query tells a program that GL ES renderers ignore
  `MipMapLevelOfDetailBias` (documented CNA/FNA3D limitation).
- CNA-DEMOS-ISSUE-005 recorded: CNA's `Color` is polymorphic (16 bytes), so XNA-style custom
  vertex structs containing a `Color` silently draw garbage.
- CNA-DEMOS-ISSUE-003 (SpriteBatch leaves Textures[0] unset) and CNA-DEMOS-ISSUE-004 (Immediate
  SpriteBatch ignores sampler states set between Draws) recorded.
- Screenshots of a demo running under Reach are reported as unavailable instead of failing the run.
- `tools/contact_sheet.py`: captures Game demos on a renderer into one labelled image for review.
- `deterministic` now means "identical capture on every machine"; GameTime-driven animation is
  excluded (catch-up Updates make game time at frame N machine-dependent).
- The runner calls `Dispose()` after `Run()` (XNA's `using` pattern), so demos' `UnloadContent`
  runs while the demo object is alive.
- CNA-DEMOS-ISSUE-002 recorded: `Curve::Evaluate` with Step keys compares the absolute position
  with 1.0.
