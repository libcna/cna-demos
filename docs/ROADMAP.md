# Roadmap

Coverage work is organised in **phases** of concrete, CNA-named areas. Take one area at a time,
finish it (demos built, run, validated, coverage regenerated, HANDOFF updated), then move on.
The machine-derived backlog of XNA types without a demo is in [COVERAGE.md](COVERAGE.md)
("Types without a demo yet"); this file adds order, CNA-specific areas and judgement.

Status: ✅ done · 🟨 started · ⬜ not started. Each area lists the demos it should contain; "≈"
means the number follows the API, not a quota.

## Phase 0 -- research and architecture ✅
CNA inspected (modules, build, renderers, content formats, bindings, samples); architecture and
plan written.

## Phase 1 -- infrastructure ✅
CMake integration, registry, metadata, runner, DemoGame, browser, CLI, catalog JSON, validation,
CTest discovery, screenshots, coverage report, CI, pilot demos in 7 categories.

## Phase 2 -- math (`math.*`) ✅ (72 math demos + 6 packed-vector demos)
Console demos with checks; each shows one operation family.
- `math.vector2.*`, `math.vector3.*`, `math.vector4.*`: construction and constants, arithmetic
  operators, `Dot`, `Cross` (Vector3), `Normalize`, `Distance`/`DistanceSquared`, `Lerp`,
  `SmoothStep`, `Hermite`, `CatmullRom`, `Barycentric`, `Clamp`, `Min`/`Max`, `Reflect`,
  `Transform` (matrix, quaternion), `TransformNormal`. ≈ 30
- `math.matrix.*`: `CreateTranslation`, `CreateScale`, `CreateRotationX/Y/Z`, `CreateFromAxisAngle`,
  `CreateFromYawPitchRoll`, `CreateFromQuaternion`, `CreateLookAt`, `CreatePerspectiveFieldOfView`,
  `CreateOrthographic(OffCenter)`, `CreateWorld`, `CreateBillboard`, `CreateReflection`,
  `CreateShadow`, `Invert`, `Transpose`, `Decompose`, multiplication order, `Lerp`. ≈ 18
- `math.quaternion.*`: axis-angle, yaw/pitch/roll, `Slerp`/`Lerp`, `Concatenate`, `Conjugate`,
  `Inverse`, `Normalize`, rotating a vector. ≈ 8
- `math.math_helper.*`: `ToRadians`/`ToDegrees`, `Clamp`, `Lerp`, `SmoothStep`, `WrapAngle`,
  `Hermite`, `CatmullRom`, `Barycentric`, `Distance`, constants. ≈ 6
- `math.point.*`, `math.rectangle.*`: `Contains`, `Intersects`, `Intersect`, `Union`, `Inflate`,
  `Offset`, `Center`, `IsEmpty`. ≈ 8
- `math.bounding_box.*`, `math.bounding_sphere.*`, `math.bounding_frustum.*`, `math.plane.*`,
  `math.ray.*`: creation (`CreateFromPoints`, `CreateMerged`, `CreateFromSphere`), `Contains` →
  `ContainmentType`, `Intersects` (each pair that differs in meaning), `Plane.DotCoordinate`,
  `Ray.Intersects` distance, frustum culling. ≈ 18
- `math.curve.*`: keys, `Evaluate`, `CurveLoopType` (constant/cycle/oscillate/linear),
  `ComputeTangents`, `CurveContinuity`. ≈ 5
- `math.color.*`: constructors (bytes, floats, Vector3/4), `FromNonPremultiplied`, `Lerp`, `* float`,
  `PackedValue` layout, `ToVector3/4`, named colours. ≈ 6
- `graphics.packed_vector.*`: pack/unpack round trip per format (Alpha8, Bgr565, Bgra4444,
  Bgra5551, Byte4, HalfSingle/Vector2/Vector4, NormalizedByte2/4, NormalizedShort2/4, Rg32,
  Rgba1010102, Rgba64, Short2/4) -- one demo per format family, not per method. ≈ 8

## Phase 3 -- game lifecycle (`game.*`) 🟨 (25 demos; remaining: Activated/Deactivated events,
ResetElapsedTime, ScreenDeviceName, orientation, CNAEXT file/text drop, GameTime constructors)
- Game: method order (`Initialize`→`LoadContent`→`BeginRun`→Update/Draw→`EndRun`→`UnloadContent`),
  `IsFixedTimeStep` and `TargetElapsedTime`, variable time step, `IsRunningSlowly`, `ResetElapsedTime`,
  `SuppressDraw`, `Exit`, `Exiting`/`Activated`/`Deactivated` events, `IsMouseVisible`,
  `InactiveSleepTime`, `RunOneFrame`/`Tick` (standalone where process-level), `RunFrameEXT`. ≈ 12
- GameTime: `ElapsedGameTime`, `TotalGameTime`, first-update zero elapsed. ≈ 2
- Components: `GameComponent`, `DrawableGameComponent`, `UpdateOrder`/`DrawOrder`,
  `Enabled`/`Visible`, `ComponentAdded`/`Removed` events. ≈ 5
- Services: `GameServiceContainer` `AddService`/`GetService`/`RemoveService`. ≈ 2
- GameWindow: `Title`, `ClientBounds`, `AllowUserResizing`, `ClientSizeChanged`,
  `ScreenDeviceName`, native handle (CNAEXT), file/text drop events (CNAEXT). ≈ 6
- GraphicsDeviceManager: preferred back-buffer size + `ApplyChanges`, `SynchronizeWithVerticalRetrace`,
  `PreferMultiSampling`, `PreparingDeviceSettings`, device events, `ToggleFullScreen` (manual),
  `GraphicsProfile` Reach vs HiDef. ≈ 8
- `LaunchParameters`, `TitleContainer.OpenStream`, `TitleLocation`. ≈ 3

## Phase 4 -- 2D graphics ⬜ (3 pilot demos)
- `graphics.device.*`: `Clear` overloads (colour, `ClearOptions`, depth/stencil), `Viewport`,
  `ScissorRectangle`, `PresentationParameters`, `GraphicsAdapter`/`DisplayMode` queries,
  `SupportsCapability` (CNAEXT), `GetBackBufferData`. ≈ 10
- `graphics.sprite_batch.*`: every `Draw` overload family (position, destination rectangle,
  source rectangle, rotation+origin, scale float/vector, `SpriteEffects` flips, layer depth),
  tinting, `Begin` overloads: `SpriteSortMode` (Deferred, Immediate, Texture, BackToFront,
  FrontToBack), `BlendState` (AlphaBlend, Additive, NonPremultiplied, Opaque), `SamplerState`
  (PointClamp, LinearWrap...), `DepthStencilState`, `RasterizerState`, transform matrix, custom
  `Effect`; sprite sheets; many Begin/End pairs per frame. ≈ 20
- `graphics.sprite_font.*`: loading a SpriteFont (XNB / CNJ / build-time CNB), `DrawString`,
  `MeasureString`, `LineSpacing`, `Spacing`, `DefaultCharacter`, `Characters`, scaled/rotated text,
  the CNAEXT glyph constructor. ≈ 8 (needs a font asset decision -- see PLAN.md)
- `graphics.texture2d.*`: `SetData`/`GetData` (whole, rectangle, mip level), mipmaps,
  `SurfaceFormat` variants, `FromStream`, `SaveAsPng`/`SaveAsJpeg`, `LevelCount`, premultiplied
  alpha. ≈ 10

## Phase 5 -- render states, buffers, primitives, render targets ⬜ (1 pilot demo)
- `graphics.blend_state.*`, `graphics.depth_stencil_state.*`, `graphics.rasterizer_state.*`,
  `graphics.sampler_state.*`: built-in objects and custom states, one concept each
  (blend functions, colour write channels, depth compare, stencil masking, cull modes, wireframe,
  depth bias, scissor test, filter and address modes, anisotropy, mip LOD bias). ≈ 25
- `graphics.vertex_buffer.*`, `graphics.index_buffer.*`, `graphics.dynamic_vertex_buffer.*`:
  creation, `SetData` options, `VertexDeclaration` (built-in vertex types and a custom one),
  16/32-bit indices, `SetDataOptions.Discard/NoOverwrite`. ≈ 8
- `graphics.primitives.*`: `DrawPrimitives`, `DrawIndexedPrimitives`, `DrawUserPrimitives`,
  `DrawUserIndexedPrimitives`, each `PrimitiveType`, instancing (`DrawInstancedPrimitives`). ≈ 8
- `graphics.render_target.*`: `RenderTarget2D` formats, depth formats, `RenderTargetUsage`
  (Discard/Preserve), multisampling, `RenderTargetCube`, multiple render targets, mipmapped
  targets, `GetData` readback. ≈ 8
- `graphics.texture_cube.*`, `graphics.texture3d.*`, `graphics.occlusion_query.*`. ≈ 5

## Phase 6 -- effects and shaders ⬜
- Stock effects, one concept per demo: `BasicEffect` (vertex colour, texture, default lighting,
  directional lights, per-pixel lighting, fog, alpha), `AlphaTestEffect`, `DualTextureEffect`,
  `EnvironmentMapEffect`, `SkinnedEffect`. ≈ 18
- CNA effects: `PbrEffect`, `SkinnedPbrEffect`, `ColorMatrixEffect`, `SpriteEffect`. ≈ 5
- Custom shaders: `ShaderEffect` per shader language (ask `RendererCapabilityProfile`), compiled
  XNA effect bytecode (`Effect(GraphicsDevice&, bytes)`), `EffectParameter`/`EffectTechnique`/
  `EffectPass` reflection and parameter setting. ≈ 8

## Phase 7 -- models, animation, glTF ⬜
- `content.model.*`/`graphics.model.*`: load XNB model, load glTF/GLB, draw with `Model::Draw`,
  iterate `ModelMesh`/`ModelMeshPart`, bones and `CopyAbsoluteBoneTransformsTo`, per-mesh effects.
- Animation: `AnimationPlayer`, `SkinnedModelEXT`, glTF clips (LINEAR/STEP/CUBICSPLINE), morph
  targets (`MorphTargetEXT`), glTF cameras and punctual lights, `GltfImportReportEXT`. ≈ 15
  Uses CNA's own Ms-PL glTF corpus files (decide asset policy first, see PLAN.md).

## Phase 8 -- content ⬜ (1 pilot demo)
- `content.content_manager.*`: `Load` caching semantics, `Unload`, `RootDirectory`, separate
  managers, `ContentLoadException`, loose files vs XNB vs CNB vs CNJ lookup order.
- `content.xnb.*`: Texture2D, SoundEffect, SpriteFont, Model, Effect, TextureCube, Curve XNBs.
- `content.cnb.*`, `content.cnj.*`: CNA's binary/JSON formats, `cna_add_content()` build step.
- `content.content_reader.*`: custom `ContentTypeReader`, `ResourceContentManager`,
  `ContentManifestEntry`. ≈ 20

## Phase 9 -- audio and media ⬜ (1 pilot demo)
- `audio.sound_effect.*`: `Play` overloads (volume, pitch, pan), `MasterVolume`, `Duration`,
  PCM buffer constructor, loop points, `GetSampleDuration`/`GetSampleSizeInBytes`.
- `audio.sound_effect_instance.*`: volume, pitch, pan, `IsLooped`, pause/resume/stop, `State`.
- `audio.positional.*`: `AudioListener`/`AudioEmitter`, `Apply3D`, `DistanceScale`, `DopplerScale`.
- `audio.dynamic_sound_effect_instance.*`: generated sine, `BufferNeeded`, pending buffers.
- `audio.microphone.*` (manual), `audio.xact.*` (AudioEngine/WaveBank/SoundBank/Cue; CNA's own
  example generates the XACT files at runtime).
- `media.*`: `Song` + `MediaPlayer` (play, pause, volume, repeat, state events), `MediaLibrary`,
  `Video`/`VideoPlayer` (needs `CNA_ENABLE_VIDEO`). ≈ 25

## Phase 10 -- input ⬜ (1 pilot demo)
- Keyboard: state, edge detection (pressed this frame), modifiers, `GetPressedKeys`.
- Mouse: position, buttons, scroll wheel (vertical/horizontal CNAEXT), `SetPosition`,
  `MouseCursor`, relative mode (CNAEXT).
- GamePad: state, capabilities, `GamePadDeadZone`, thumbsticks/triggers/buttons/DPad, vibration,
  CNA extensions (rumble, light bar, gyro).
- Touch: `TouchPanel.GetState`, `TouchLocation`, gestures (`EnabledGestures`, `ReadGesture`).
- `TextInputEXT`, `CNA::Input::Joysticks`, `Haptics`, `Sensors`, `Power`, `Clipboard`. ≈ 25
  Mostly smoke-only; logic-only parts (dead-zone maths, state comparison) are automated.

## Phase 11 -- storage ⬜
`StorageDevice` selector and containers, create/open/delete files and directories, `FileExists`,
`GetFileNames`, `SetAppNameEXT`/`GetStorageRootEXT`, `TitleContainer.OpenStream`. ≈ 8
(Demos write under a temporary storage root and clean up.)

## Phase 12 -- networking, gamer services, avatars ⬜
- `net.*`: local and SystemLink sessions over loopback, `PacketWriter`/`PacketReader`, session
  events, `NetworkSessionProperties`, simulated latency/packet loss. ≈ 10
- `gamer_services.*`: offline `SignedInGamer`, `GamerProfile`, `Guide` (message box/keyboard input,
  manual), achievements/leaderboards (manual: needs the CNA service). ≈ 10
- `gamer_services.avatar.*`: `AvatarDescription`, `AvatarRenderer` with built-in catalogs,
  `AvatarAnimation` presets, expressions. ≈ 6

## Phase 13 -- devices, phone, design, diagnostics, platform, CNA extensions ⬜
- `devices.*`: `Microsoft.Devices.Sensors` (accelerometer, compass, gyroscope, motion -- manual
  unless emulated), `VibrateController`; CNA devices-ext: clipboard, locale, power, system/display
  info (automated), message box, file dialog, URL launcher, camera, system tray (manual).
- `phone.*`: `PhoneApplicationService` lifecycle events, notification channel types.
- `design.*`: each `TypeConverter` (string ↔ value round trip).
- `diagnostics.*`: `CNA::Logger` levels and sinks, `CNA_DIAGNOSTICS` instrumentation.
- `platform.*`: platform capabilities, native window handle, renderer selection with fallback
  chain (standalone), renderer capability profiles.
- `cnaext.*`: `CRTEffect`, `AsciiPostProcessEffect`, `DepthEffect`, `DebugDraw`. ≈ 30

## Phase 14 -- bindings ⬜
C API demos as standalone C programs (one C game per process): window/loop, sprite drawing,
input, audio, content. Built only with `CNA_BUILD_C_API=ON` (experimental in CNA). C#/.NET demos
belong with the separate `cna-dotnet` binding; decide later whether to reference or host them.

## Phase 15 -- coverage audit ⬜
Walk COVERAGE.md's backlog and the CNA-specific headers; for each remaining type decide: demo,
part of another demo, or not independently meaningful (record why). Add screenshot references for
deterministic demos and run the visual-regression comparison across renderers.

## Cross-cutting work (any time)
- Visual regression: reference images + comparison step (AUTOMATION.md section 5).
- CI: an Xvfb job for OPENGLES3; more renderers as CI capacity allows.
- A text font for on-screen labels (SpriteFont asset policy; see PLAN.md).
