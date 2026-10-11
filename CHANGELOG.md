# Changelog

All notable changes to cna-demos. The format follows [Keep a Changelog](https://keepachangelog.com/);
versions follow Semantic Versioning (pre-1.0: a minor version may change the CLI or metadata).

## [Unreleased]

### Fixed
- Full validation removes stale current smoke JSON outputs before starting, while preserving
  named baseline archives; an interrupted process cannot leave an old report looking current.
- Four XACT demos now own GetCue results with unique_ptr: Dispose releases playback but does not
  delete the returned C++ object. Borrowed video/leaderboard pointers retain their existing ownership.
- Manifest paths are shortened to fit their on-screen columns; full names remain in console output.
- Curve loop demo now checks the last outgoing tangent for Linear extrapolation; the former
  expectation matched a CNA defect, now recorded as CNA-DEMOS-ISSUE-036.
- Catalog validation extracts each demo object once before checking its fields and known issue;
  this avoids repeatedly parsing the complete catalog and timing out at 401 demos.
- `tools/validate.sh` now stops on build and CTest failures even when their output contains no
  recognized diagnostic; complete logs are kept in the reports directory. `JOBS` controls
  parallelism. `tools/test_validate.py` checks failure propagation without building CNA.
- Coverage tables distinguish explicit NOT RUN results from demos absent in an older report,
  listing missing IDs so stale smoke evidence cannot silently appear current.

### Added
- Existing custom XNB reader now counts actual Read calls to establish cache reuse and verifies
  ContentReader's logical asset name and owning ContentManager.
- Intermediate XML format example now distinguishes absent/present/explicit-null optional fields
  and checks that a missing required field is rejected even when its constructor has a default.
- C content example rejects destroying an atlas retained by a live font and checks that text
  still renders after the refused release and manager destruction.
- Shared intermediate XML resources: forward fixups, identity-preserving aliases, equal but
  independent loot tables and inline copies, with mutation showing the actual ownership graph.
- Existing C error demo verifies failed diagnostic queries preserve the original error and
  exact-sized string copies honor the payload boundary without adding a terminator.
- Intermediate XML tool demonstrates public field descriptors and ContentSerializerAttribute:
  renamed/optional/ignored fields, collection item names, round trip and ignored-field rejection.
- Existing C content demo checks texture pixels and font metrics after manager Unload and
  destruction, then checks the rendered checker in the combined sprite/text batch.
- Existing SpriteBatch layer-depth example checks overlap and unobscured colors; texture-group
  sorting now rejects a uniformly blank row as well as incorrectly grouped sprites.
- Existing content cache demo now switches roots after Unload and exposes CNA accepting a root
  change while assets remain loaded (ISSUE-039); the visual shows the observed result.
- Custom content pipeline demo: registered importer/processor/writer, build-time patrol-length
  computation, deterministic CNB output, runtime loading and contextual parameter diagnostics.
- Custom CNJ enemy demo with two named factories, actual JSON payload parsing, manager-local
  registrations, Unload/cache behavior and malformed-payload diagnostics; original fixtures
  are reproducible through the asset generator.
- Custom CNB loader now distinguishes optional and mandatory unknown chunks and retries an
  asset after its failed file is repaired.
- Dynamic streaming now checks PCM size/duration, unsupported loop/release operations and
  stop/disposal behavior. Two missing CNA disposal guards remain visible as ISSUE-038.
- Semantic coverage audits now include content loading/binary formats, texture ownership,
  audio/XACT, sprite/text behavior and C ABI diagnostics/ownership, separating exercised checks
  from visual-only evidence and preserving the differences between C and C++ contracts.
- Existing XACT cue-state demo observes Disposing, repeated Dispose and deleting a disposed
  object, with callback state kept alive through scope unwinding.
- XACT category instance-limit demo: Fail and ReplaceOldest policies, paused voices retaining
  slots, category isolation and slot reuse, with generated settings and public state checks.
- Texture rectangle demo now transfers between nonzero CPU array offsets and verifies untouched
  destination padding alongside the adjacent texture pixel.
- Manifest demo shows files arriving/disappearing between explicit refreshes and the independent
  asset cache; private temporary content is removed after the checks.
- Existing curve demos now show selective/asymmetric tangent computation, negative-time loop
  boundaries, and empty/single-key evaluation without adding duplicate demos.
- Custom CNB loader demo: a game-defined rules schema, public writer, shared loader registration,
  per-manager cache behavior and unsupported-schema rejection.
- Quaternion double-cover/relative-turn demo and typed effect parameter round trips; confirmed
  stock-parameter getter and Bones reflection gaps recorded as CNA-DEMOS-ISSUE-034/035.
- Started semantic member/behavior audit tables for Quaternion, EffectParameter and Curve/CurveKeyCollection in
  `docs/COVERAGE_AUDIT.md`, with explicit outstanding work and environment limitations.
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
- Audit demos (3): `graphics.vertex_declaration.stock_vertex_types` (IVertexType, the four stock
  vertex structs, a VertexPositionColorTexture quad), `audio.xact.renderer_details`
  (AudioEngine::RendererDetails), `math.formatting.to_string` (ToString of the math types; expected
  failure, CNA-DEMOS-ISSUE-024: BoundingBox and Ray print doubled braces); the type-converter demo
  also lists Vector4, Plane and Ray.
- C API demos (3): `c_api.graphics.basic_effect_3d` (a lit cube from a vertex and an index
  buffer and a BasicEffect), `c_api.graphics.render_target` (render to texture, read back, drawn
  as a sprite) and `c_api.storage.save_game` (device, container, write/read/delete a save file).
- C API demos (3 more): `c_api.input.gamepad` (one button mask with the derived stick and trigger
  bits, the three dead-zone modes, live polling of player one), `c_api.game.components` (callback
  table components: draw order, visibility, enabled, collection events) and
  `c_api.media.media_player` (a song collection played as a queue: next, pause, resume, stop).
- `graphics.depth_stencil_state.two_sided_stencil` (a shadow volume counted in one draw with
  TwoSidedStencilMode), `net.network_session.private_slots` (PrivateGamerSlots) and
  `net.network_session.join_refused` (standalone, two processes: NetworkSessionJoinException for a
  game in progress, then AllowJoinInProgress; expected failure, CNA-DEMOS-ISSUE-032: the refusal
  reads SessionNotFound, and ISSUE-033: the late joiner's session stays in Lobby).
- `graphics.pbr_effect.texture_maps` (NormalMap with a Vector4 vertex tangent, EmissiveMap,
  OcclusionMap darkening ambient light only; GL renderers), `graphics.model.skinned_pbr` (a skinned
  glTF with a metallic-roughness material imports as SkinnedPbrEffect; tools/gltf_models.py writes
  painted_column.gltf), `content.cnj.sprite_font` (a seven-segment bitmap font authored as CNJ: atlas
  plus per-glyph rectangles, cropping and kerning; tools/generate_assets.py writes both),
  `game.window.screen_device_name` (ScreenDeviceName, ScreenDeviceNameChanged, the adapter's
  DeviceName), `platform.window.native_handle` (GameWindow::Handle and the typed
  NativeWindowHandle accessors).
- `c_api.game.components` takes one step a second when a person watches.
- `content.content_project.xna_content_project`: an XNA Game Studio `.contentproj`
  (`content_project/`) built as it is by `cna_add_content(... CONTENT_PROJECT ...)` into XNB:
  item names, the default colour key, a per-item processor parameter, a copied None item.
- Coverage: `Texture::Format` and `Texture::LevelCount` are named on XNA's base class `Texture`
  (they were credited to nothing as `Texture2D::...`).
- `math.color.named_colors` (chart of XNA's 141 named colours), `math.vector2.toolbox_2d`
  (Vector2's static helpers on 2D game tasks) and `math.vector4.color_math` (Vector4 helpers as
  RGBA colour maths).
- Coverage audit pass 2: 146 XNA member names that 66 existing demos call and teach were added to
  their `.apis` (effect World/View/Projection, Cue::Play, AvatarRenderer transforms, ...).
- `graphics.model.gltf_lights` (two KHR_lights_punctual lights become the PbrEffect's
  DirectionalLight0/1) and `graphics.model.clip_interpolation` (STEP, LINEAR and CUBICSPLINE
  samplers; expected failure CNA-DEMOS-ISSUE-031: CNA plays all three as LINEAR) and
  `graphics.model.material_variants` (KHR_materials_variants: Model::MaterialVariantEXT repaints a
  toy car); `tools/gltf_models.py` writes `lit_sphere.gltf`, `slide_rig.gltf` and
  `loose_paint_toy.gltf`.
- `net.network_session.simulated_conditions` (standalone: SimulatedLatency and SimulatedPacketLoss
  measured on packets between two processes; expected failure CNA-DEMOS-ISSUE-030: the simulated
  loss drops reliable packets too).
- `net.network_session.add_local_gamer` (AddLocalGamer in the lobby; AllowJoinInProgress during
  play), `net.network_machine.remove_from_session` (standalone: the host removes a player's machine,
  RemovedByHost) and the manual `gamer_services.signed_in_gamer.signed_out` (SignedOut through the
  Guide). `CnaDemos/StandaloneProgram.hpp` holds what the C++ standalone programs share; the two
  earlier net programs use it.
- `audio.sound_effect.from_stream` (a loose WAV loaded at run time; CNA-DEMOS-ISSUE-029:
  `SoundEffect::FromStream` takes a `std::istream`, not a `System::IO::Stream`),
  `game.components.change_events` (EnabledChanged, UpdateOrderChanged, VisibleChanged,
  DrawOrderChanged, the OnEnabledChanged hook) and `math.bounding_box.intersects` (box against
  boxes, spheres, rays and planes); `media.media_player.play_song` checks
  `MediaPlayer::GameHasControl` before starting its music.
- `game.window.orientation` (SupportedOrientations, CurrentOrientation, OrientationChanged, CNA's
  keyboard orientation emulation enabled before the device exists) and
  `graphics.shader_effect.vertex_displacement` (a heightmap sampled in a GLSL vertex shader through
  ShaderEffect::SetTexture); `game.graphics_device_manager.device_events` also follows the
  GraphicsDevice's own DeviceResetting/DeviceReset.
- `graphics.render_target.hdr_blendable` (additive light past 1.0 in an HdrBlendable target, read
  back as HalfVector4, shown at an exposure; new `Need::HalfFloatRenderTargets`),
  `audio.xact.streaming_wave_bank` (streaming WaveBank constructor, IsPrepared, IsInUse;
  `tools/xact_files.py` writes a streaming `music.xwb`) and
  `devices.display_info.scale_and_safe_area` (CNA::Devices::DisplayInfo), `audio.xact.cue_variables`
  (per-cue values, clamping, cue vs global variables; `demo.xgs` gains the cue variable
  Intensity). Recorded
  CNA-DEMOS-ISSUE-027 (SOFTWARE does not clamp source values above 1 before blending into an 8-bit
  target) and CNA-DEMOS-ISSUE-028 (a streaming WaveBank opens a relative path from the working
  directory).
- Two-process net demos (standalone): `net.network_session.find_and_join` (a second process finds
  the System Link session -- host gamertag, open slots, QualityOfService -- joins and leaves) and
  `net.network_session.host_leaves` (the host leaves twice: SessionEnded with HostEndedSession,
  then HostChanged with AllowHostMigration); CNA-DEMOS-ISSUE-026 (on Linux, a process that
  searched before a host started never receives its answers). CTest runs demos that need the
  network one at a time (`RESOURCE_LOCK`). `CnaDemos::UseOfflineDemoProfiles` shares the gamer
  services set-up with the standalone programs.
- Audit demos (4): `gamer_services.avatar.custom_animation` (a game's own IAvatarAnimation drawn
  by AvatarRenderer), `gamer_services.guide.notification_position` (Guide notifications at
  TopRight), `gamer_services.leaderboard.paging` (PageDown/PageUp, LeaderboardOutcome and TimeSpan
  columns; CNA-DEMOS-ISSUE-025: offline, columns written after the rating are lost) and
  `graphics.render_target.multiple_render_targets` (SetRenderTargets with two
  RenderTargetBindings and a two-output GLSL ShaderEffect).
- `game.services.graphics_device_service` (a component reaches the device through
  IGraphicsDeviceService) and `net.network_session.online_session_types` (Local vs PlayerMatch and
  Ranked without the online service).
- docs/COVERAGE_AUDIT.md: a decision for every XNA type without a demo (demo, shown in, environment,
  service, not meaningful); audit pass 2 names more types the demos already used (collections,
  converters, event arguments, MediaSourceType, VideoSoundtrackType).
- `graphics.effect.effect_interfaces`: IEffectMatrices, IEffectLights and IEffectFog on five stock
  effects; scene settings applied through the interfaces alone.
- Coverage audit, first pass: demos that already show an enumeration or type now name it in
  `.apis` (Keys, KeyState, EffectParameterClass/Type, DisplayOrientation, GamerZone, GameDefaults'
  enumerations, MicrophoneState, AvatarBodyType).
- `content.content_manager.manifest` requires XNA's own XNB readers only (a game's own reader is
  available once the game registers it).
- Content, audio and media demos (4): a custom ContentTypeReader<T> reading the game's own type
  from an .xnb (`tools/generate_assets.py` writes it), a ResourceContentManager over content
  embedded in the program (expected failure, ISSUE-012), a SoundEffect loop region, VideoPlayer
  Pause/Resume/IsLooped/Volume/IsMuted.
- Model demos (3): glTF morph targets driven by the file's weights animation, the cameras of a
  glTF scene in split screen, a glTF with an embedded texture. CNA-DEMOS-ISSUE-022 (a compiled glTF
  refers to an embedded texture file the build never writes) and ISSUE-023 (compiled models lose
  their cameras) recorded; both demos check the compiled model and then use the run-time import.
- Graphics demos (4): hardware instancing (DrawInstancedPrimitives with a per-instance world-matrix
  stream), several vertex streams (SetVertexBuffers), rendering into a RenderTargetCube (dynamic
  environment map), BasicEffect vertex colours with lighting.
- `knownIssueRenderers` metadata: a recorded defect can be limited to some renderers (expected
  failure there, a normal pass elsewhere; `--list-ids` prints `<issue>@R1,R2`, CTest follows it).
- CNA-DEMOS-ISSUE-021 recorded: the EasyGL renderers (OPENGLES3, OPENGL33) sample a rendered
  RenderTargetCube face upside down.
- C API demos (7, Phase 14): standalone C99 programs against CNA's C ABI (`CNA::CApi`) --
  lifecycle callbacks and the loop, error results and diagnostics, sprites from computed pixels,
  a ContentManager with texture and SpriteFont, keyboard/mouse state, a SoundEffect from PCM,
  vectors and matrices. `cna_demos_add_standalone(... C_API)`, `Need::CApi` (SKIPPED without the C
  API), the `CnaDemos/CApiDemo.h` scaffolding (checks, diagnostics, PNG capture), and the
  `linux-full` preset now builds CNA with `CNA_BUILD_C_API=ON`. Standalone demos receive
  `--screenshot`. docs/COVERAGE.md counts C ABI routes against CNA's C headers.
- `gamer_services.avatar.animation_presets`: 12 frames starting mid-clip (CI timed it out at 90
  frames on SOFTWARE under parallel load) and the avatars now face the camera.
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
