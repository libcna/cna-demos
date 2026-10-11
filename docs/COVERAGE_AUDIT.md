# Coverage audit

Phase 15 of [ROADMAP.md](ROADMAP.md): for every XNA 4.0 type that
[COVERAGE.md](COVERAGE.md) lists under "Types without a demo yet", a decision -- a demo, part of
another demo, or not independently meaningful -- with the reason. COVERAGE.md is generated; this
file is maintained by hand and should be revisited whenever that list changes. Later sections
review public member behavior and selected CNA-specific APIs separately from type counts.

Decisions:

* **demo** -- a demo exists or is planned (ROADMAP names it).
* **shown in** -- an existing demo already uses the type; its `.apis` names it (audit pass 1 added
  the names that were missing), so it drops off the list at the next coverage run.
* **environment** -- the type only appears when the environment lacks something (no audio
  device, no graphics adapter, a removed storage device, a lost device): it cannot be shown on
  demand, and cna-demos reports the situation as SKIPPED where it applies.
* **service** -- needs the CNA online service or another machine; a manual demo at most.
* **not meaningful** -- C#-only machinery with no C++ use of its own.

## Microsoft.Xna.Framework

| Type | Decision |
|---|---|
| DisplayOrientation | shown in `input.touch.capabilities` |
| IGameComponent | shown in `game.components.collection_events` |
| IGraphicsDeviceManager | demo: `game.services.graphics_device_service` |
| IDrawable, IUpdateable | not meaningful on their own: the interfaces of `DrawableGameComponent`/`GameComponent`, whose members the `game.components.*` demos show through the classes |

## Microsoft.Xna.Framework.Audio

| Type | Decision |
|---|---|
| MicrophoneState | shown in `audio.microphone.input_level` |
| NoAudioHardwareException | environment: thrown without an audio device; the runner turns it into SKIPPED for demos that declare `Need::Audio` |
| NoMicrophoneConnectedException | environment (no microphone); `audio.microphone.input_level` is manual |
| InstancePlayLimitException | not meaningful: CNA declares it but never throws it (no cap on playing instances) |
| RendererDetail | demo: `audio.xact.renderer_details` |

## Microsoft.Xna.Framework.Content

| Type | Decision |
|---|---|
| ContentReader, ContentTypeReader, ContentTypeReaderManager | demo: `content.xnb.custom_type_reader` (the members it uses come from BinaryReader or are CNA's, so its `.apis` names the types) |
| ResourceContentManager | demo: `content.content_manager.resource_content_manager` (expected failure, CNA-DEMOS-ISSUE-012) |
| ContentSerializerAttribute | Meaningful build-time gap: current IntermediateReader/Writer consume this public format object. A focused IntermediateSerializer example should show named/optional fields and shared data. The earlier claim that CNA has no consumer was wrong for the inspected source. |
| ContentSerializerCollectionItemNameAttribute, ContentSerializerIgnoreAttribute, ContentSerializerRuntimeTypeAttribute, ContentSerializerTypeVersionAttribute | No independent wrapper-object demos. Current ContentTypeDescriptor/ContentMemberDescriptor APIs implement useful collection naming and ignored-field semantics explicitly in C++; investigate and demonstrate those with IntermediateSerializer. The nominal .NET attribute wrappers are not automatically credited, and their absence from `.apis` must not erase this real behavior gap. |

## Microsoft.Xna.Framework.Design

| Type | Decision |
|---|---|
| RectangleConverter, PlaneConverter, RayConverter, Vector4Converter | shown in `design.type_converter.string_round_trip` |

## Microsoft.Xna.Framework.GamerServices

| Type | Decision |
|---|---|
| AvatarBodyType | shown in `gamer_services.avatar.animation_presets` |
| ControllerSensitivity, GameDifficulty, RacingCameraAngle | shown in `gamer_services.signed_in_gamer.game_defaults` |
| GamerZone | shown in `gamer_services.gamer_profile.get_profile` |
| GamerPrivilegeException | candidate: needs CNA's online service configured (offline, `net.network_session.online_session_types` shows GamerServicesNotAvailableException first) |
| GamerCollection, GamerCollection.GamerCollectionEnumerator | shown by the `net.*` demos (AllGamers, LocalGamers); the enumerator is C#'s `foreach` machinery (C++ range-for) |
| IAvatarAnimation | demo: `gamer_services.avatar.custom_animation` |
| NotificationPosition | demo: `gamer_services.guide.notification_position` |
| LeaderboardOutcome | demo: `gamer_services.leaderboard.paging` (a column type) |
| SignedOutEventArgs | demo (manual): `gamer_services.signed_in_gamer.signed_out` -- offline, a profile signs out only through the Guide's own menu, so a person does it |
| FriendCollection, FriendGamer, InviteAcceptedEventArgs, GameUpdateRequiredException | service |
| NetworkException, NetworkNotAvailableException | environment / CNA-DEMOS-ISSUE-018 (System Link failures surface as `std::runtime_error`) |

## Microsoft.Xna.Framework.Graphics

| Type | Decision |
|---|---|
| IEffectFog, IEffectLights, IEffectMatrices | demo: `graphics.effect.effect_interfaces` |
| IGraphicsDeviceService | demo: `game.services.graphics_device_service` |
| EffectParameterClass, EffectParameterType, EffectPassCollection | shown in `graphics.effect.parameters_and_techniques` |
| ModelMeshCollection, ModelMeshPartCollection, ModelEffectCollection | shown in `graphics.model.bones` (the indexer), `graphics.model.morph_targets`, `graphics.model.mesh_effects`; their enumerators are C#'s `foreach` machinery |
| RenderTargetBinding | demo: `graphics.render_target.multiple_render_targets` |
| VertexPositionColorTexture, IVertexType | demo: `graphics.vertex_declaration.stock_vertex_types` |
| EffectAnnotation, EffectAnnotationCollection, EffectMaterial | environment: compiled XNA effects need Microsoft's `fxc` (`fx_2_0`, a Windows tool run through wine) to compile `.fx`, and a CNA build with `CNA_<RENDERER>_COMPILED_EFFECTS` (fetches FNA3D/MojoShader); neither is part of this repository's builds |
| DeviceLostException, DeviceNotResetException, GraphicsDeviceStatus, NoSuitableGraphicsDeviceException | environment: device loss does not happen on CNA's desktop renderers; no adapter means no demos at all |

## Microsoft.Xna.Framework.Input

| Type | Decision |
|---|---|
| Keys, KeyState | shown in `input.keyboard.state_values` |
| TouchCollection.Enumerator | not meaningful: C#'s `foreach` machinery (C++ range-for) |

## Microsoft.Xna.Framework.Media

| Type | Decision |
|---|---|
| MediaSourceType | shown in `media.media_library.collections` |
| VideoSoundtrackType | shown in `media.video_player.play_video` |
| AlbumCollection, ArtistCollection, GenreCollection, PlaylistCollection, PictureCollection | shown (counts) in `media.media_library.collections`; their contents depend on the machine's library |
| Album, Artist, Genre, Playlist, Picture, PictureAlbum, PictureAlbumCollection | environment: the user's own library (empty on CI); a manual demo at most |

## Microsoft.Xna.Framework.Net

| Type | Decision |
|---|---|
| GameStartedEventArgs, GameEndedEventArgs | shown in `net.network_session.game_flow` |
| AvailableNetworkSession, AvailableNetworkSessionCollection, QualityOfService | demo: `net.network_session.find_and_join` (two processes) |
| NetworkSessionJoinError, NetworkSessionJoinException | demo: `net.network_session.join_refused` (a game in progress; expected failure, CNA-DEMOS-ISSUE-032). A full session cannot be shown: `Find` lists only sessions with room for the searcher's gamers |
| GamerLeftEventArgs | demo: `net.network_session.find_and_join`, `net.network_session.host_leaves` |
| HostChangedEventArgs, NetworkSessionEndReason, NetworkSessionEndedEventArgs | demo: `net.network_session.host_leaves` (two processes) |
| WriteLeaderboardsEventArgs | service: CNA raises the `Write*Leaderboard`/`WriteTrueSkill` events only for leaderboard gameplay on the CNA service |

## Microsoft.Xna.Framework.Storage

| Type | Decision |
|---|---|
| StorageDeviceNotConnectedException | environment: a removed storage device |

## Public member and behavior audit (started 2026-10-11)

Type coverage is not comprehensive coverage. The generated member count collapses overloads
and records metadata claims; it does not prove that overloads, error paths, or behavior are
demonstrated. This pass examines those separately. The tables below are source reviews against
CNA `2b15506a4`; planned work is not counted as completed or runtime-validated.

### Quaternion

| Members / behavior | Decision and evidence |
|---|---|
| Constructor, Identity, CreateFromAxisAngle, CreateFromRotationMatrix, CreateFromYawPitchRoll | Existing `from_axis_angle` and `from_yaw_pitch_roll` demonstrate orientation construction and matrix conversion; no separate constructor demo needed. |
| Concatenate, Multiply, operator multiply | Existing `concatenate` checks sequential vector transforms and noncommuting rotations. Scalar multiplication is component scaling, not a rotation composition; explain it when discussing normalization rather than creating another demo. |
| Inverse, Conjugate | Existing `inverse_and_conjugate` checks undoing a rotation and the unit-quaternion precondition for replacing inverse by conjugate. |
| Normalize, Length, LengthSquared | Existing `normalize` demonstrates accumulated drift and both normalization forms. LengthSquared is a trivial squared norm; can be named and checked there if extended. |
| Lerp, Slerp | Existing `slerp` checks constant versus uneven angular speed. Negative-dot inputs and antipodal representations deserve the `double_cover` demo. |
| Negate, unary negation, Dot, Add, operator addition, equality / inequality | Demonstrated in `double_cover`: distinguish component equality from orientation equality; negate all four components, choose a common hemisphere before normalized blending, and show that adding antipodes gives zero and cannot be normalized meaningfully. |
| Divide, operator division | Demonstrated in `double_cover`: check `Divide(target, start)` as the additional rotation after start, including different axes so an order error cannot pass accidentally. |
| Subtract, operator subtraction | Trivial component arithmetic; does not compute the relative orientation. Explain that distinction with Divide; no independent demo. |
| Equals, GetHashCode, ToString | Exact value equality and formatting follow the existing math value-type demos; hashing needs no independent demo. Quaternion text is already printed in `inverse_and_conjugate`. |
| Output-reference overloads | Existing `math.vector3.by_reference_overloads` explains the C++ calling convention. Audit quaternion aliasing only if an actual defect or distinct behavior emerges; do not duplicate every return-value demo. |

### EffectParameter

| Members / behavior | Decision and evidence |
|---|---|
| Name, ParameterClass, ParameterType, RowCount, ColumnCount | `parameters_and_techniques` already lists names/classes; `parameter_values` relates shape/type to the accessor used and inspect Bones array shape. |
| SetValue; GetValueSingle, GetValueInt32, GetValueVector4, GetValueTexture2D | Demonstrated in `parameter_values`: typed round trips through named stock parameters; do not imply that setting a parameter updates the stock effect's cached public properties. |
| GetValueMatrix, GetValueMatrixTranspose, SetValueTranspose | Demonstrated in `parameter_values`: use a non-symmetric translation/rotation matrix to make transpose semantics observable. Identity alone would not test it. |
| Elements, GetValueMatrixArray, GetValueMatrixTransposeArray | Demonstrated in `parameter_values`: two distinct bone transforms, round trips and reflected array shape. Source currently constructs Bones as a 72-by-4 matrix with no Elements; confirmed on SOFTWARE and OPENGLES3 (ISSUE-035). |
| GetValueBoolean, GetValueBooleanArray, GetValueInt32Array, GetValueSingleArray | Add to a compiled-effect demo when that workstream is available: stock parameters do not supply meaningful examples of every array type. No synthetic standalone parameter for each getter merely to raise counts. |
| GetValueVector2, GetValueVector3, GetValueQuaternion and their Array forms; GetValueVector4Array | Same typed data concept, but reflected vector widths, array bounds and quaternion-as-float4 deserve compiled-effect checks. Defer to that coherent demo rather than claiming the Vector4 round trip covers all behavior. |
| GetValueTexture3D, GetValueTextureCube | Renderer/effect-specific shader inputs: incorporate round trips into effects using these samplers; existing environment mapping demonstrates cube sampling, not these parameter getters. |
| GetValueString, Semantic, Annotations, StructureMembers | Compiled effect reflection workstream (current build lacks compiled-effect support). Stock effects' empty metadata does not demonstrate annotated strings or nested structs. |
| Invalid accessor / parameter-shape combinations | Demonstrated in `parameter_values`: Vector4 read as Single should be checked explicitly. CNA validates compiled storage shapes but the stock storage getter returns its first component; confirmed on SOFTWARE and OPENGLES3 (ISSUE-034). |

### Curve and CurveKeyCollection

| Members / behavior | Decision and evidence |
|---|---|
| Curve constructor, Keys, Evaluate | Existing `math.curve.evaluate` constructs a brightness timeline and checks keys and interpolation. `key_collection` now checks zero for empty curves and a nonzero single-key value on both sides of its position. |
| ComputeTangents | Existing `tangents` compares Flat/Linear/Smooth. `tangents` also computes different incoming/outgoing modes and checks the resulting midpoint (3.75). |
| ComputeTangent | `tangents` now changes one key with separate modes, checks the untouched neighbor and midpoint 6.25, then restores Flat with the single-mode overload. |
| PreLoop, PostLoop | `loop_types` checks all pre/post modes, explicit endpoint slopes and negative CycleOffset/Oscillate boundaries. Linear post-loop has one confirmed failure (ISSUE-036); the original demo expected the wrong first-key behavior. |
| Curve Clone, IsConstant | Existing `key_collection` checks independence after removals and the transition to a single key. No separate clone demo. |
| Collection constructor, Add, Item, Count, IndexOf, Remove, RemoveAt, Clear | Existing `key_collection` demonstrates sorted insertion and editing. Duplicate positions and Item replacement order are distinct edge cases to investigate before claiming complete collection behavior. |
| Collection Clone, Contains, CopyTo, IsReadOnly | Clone fits existing copy/edit checks if extended; Contains is the boolean counterpart of demonstrated IndexOf. CopyTo and IsReadOnly are ordinary collection mechanics, intentionally without independent demos. They remain uncredited until actually shown. |
| GetEnumerator, explicit IEnumerable.GetEnumerator | C# iteration machinery; C++ uses range iteration/indexing. No independent demo. |
| Step continuity | Existing `step_continuity` is a failing compatibility example (ISSUE-002), not a successful implementation of Step. |

### ContentManager (including CNA extensions)

| Members / behavior | Decision and evidence |
|---|---|
| Constructors, RootDirectory | `content.content_manager.several_managers` shows separate roots and caches, and `missing_assets` shows slash normalization. The CNA default constructor and nullable service provider support console data loading; they do not need independent demos. `cache_and_unload` now tries to change the root while assets are loaded (CNA wrongly accepts it, ISSUE-039), then switches successfully after Unload and loads a new relative name. All 5 checks run on both renderer selections with just that one expected failure; screenshots inspected. |
| ServiceProvider | Several managers receive the game's services; `game.services.graphics_device_service` explains device service lookup. The getter is ordinary identity plumbing, intentionally without an independent demo; it is not currently named as a demonstrated member. |
| Load (cache, type and name resolution) | `cache_and_unload` counts created textures, `custom.cnb_loader` counts factories, and `several_managers` distinguishes per-manager caches. `missing_assets` checks failures and backslash names. Same-name files in competing XNB/CNB/CNJ/native tiers and the cache's type key remain meaningful gaps; loading different names in `xnb.same_assets` does not establish precedence. |
| Unload, Dispose() | `cache_and_unload` shows rereading after Unload; `unload_disposes` explicitly fails on undisposed textures (ISSUE-011). `several_managers` calls Dispose on level exit. These uses do not demonstrate the protected Dispose(bool) override hook or post-disposal errors; add those to the custom-manager example when extending it. |
| OpenStream, ReadAsset | `open_stream_override` serves an in-memory XNB via ReadAsset, then demonstrates that Load bypasses the override (ISSUE-012). `resource_content_manager` exposes the same defect for embedded resources. The ReadAsset disposable-recording callback and uncached repeat reads remain useful extensions, not implicitly covered by one successful read. |
| RegisterTypeReader, LooseFileContentTypeReader::Read / GetExtensions | `custom.loose_file_reader` teaches a tile-map format and checks caching. Ordinary adapter methods need no independent demos. |
| RegisterCnbLoaderEXT | `custom.cnb_loader` shows a game-defined binary schema, process-wide registration, separate caches, mandatory chunk validation and unsupported-schema rejection. Unknown optional/mandatory chunks and retry after repair are now exercised (8 checks pass on both renderer selections). Malformed chunk structure remains a separate gap. |
| RegisterCnjLoader, CnjLoaderFn | `content.custom.cnj_loader` uses two named factories for the same Enemy type, parses authored fields with nlohmann-json (an existing README/CI prerequisite, now explicitly linked for content), rejects duplicate registration and a missing payload field, and shows manager-local registration surviving Unload. All 7 checks pass on both renderer selections. No CNA Internal parser is used; unknown type names and schema-envelope version rejection remain separate gaps. |
| GetContentManifest, GetXnbReaderUsageSummary | Existing `content_manager.manifest` enumerates logical asset names, format flags and XNB reader availability. A count alone would not show these distinct fields. Pipeline bookkeeping files remain visible (ISSUE-013). |
| RefreshContentManifest | `manifest` now adds/removes a private CNJ file between scans, loads the new file before refreshing, and confirms refresh does not discard the loaded asset. All seven new snapshot/cache checks pass on SOFTWARE and OPENGLES3. |
| setGraphicsDevice, getGraphicsDeviceInternal | CNA integration helpers; normal game demos use IGraphicsDeviceService through the supplied provider. No separate demonstration unless a standalone graphics-content scenario needs explicit device injection. |

### CNA binary content: writer, document and primitive cursors

This initial CNA-specific review covers `CnbWriter`, `CnbDocument`, `CnbByteWriter`,
`CnbByteReader` and their format/limit helpers. These extensions are outside the XNA member
percentage; their presence in the unmatched-name list is not evidence of an API spelling error.

| Members / behavior | Decision and evidence |
|---|---|
| CnbWriter constructor, SetMetadata, AddChunk, WriteToFile; CnbAssetTypeIdFromName, MakeChunkId | `content.custom.cnb_loader` writes a game-owned rules schema and loads it through the typed registration. The canonical name and hashed ID are visible. Hash-collision/name-mismatch rejection is not exercised; the successful identity round trip alone does not establish it. |
| Writer Build, SchemaChunkCount, deterministic output | WriteToFile uses Build internally, but the demo does not directly inspect an in-memory image or compare two generated images. `pipeline_components` now compares two full builds' resulting CNB bytes; it does not claim every compression/platform setting is deterministic. SchemaChunkCount is ordinary inspection and needs no independent demo. |
| CnbDocument::RequireAsset | The loader checks type/schema and rejects a future schema version. Type mismatch, missing metadata and malformed container bytes remain unexercised error paths. |
| RequireMandatoryChunksUnderstood | `custom.cnb_loader` now loads the same schema with an unknown optional chunk and verifies intact data, refuses the same extension marked mandatory before decoding, then repairs that filename and successfully retries through the same manager without Unload. All 8 checks pass on SOFTWARE and OPENGLES3; failed loads do not poison this cache path. |
| RequireSingle, OpenChunk; primitive WriteString/WriteF32/WriteU32/Take and ReadString/ReadF32/ReadU32/RequireExhausted | The rules fields round trip through a bounded chunk cursor. Missing/duplicate payload chunks, trailing bytes and truncated fields can be demonstrated together in a focused format-diagnostics extension. The current success path does not test them. |
| Other numeric primitive reads/writes, WriteBytes/WriteZeros, ReadBytes/Skip, cursor Size/Position/Remaining/Context, writer Size/View | Ordinary encoding/cursor mechanics; no independent example for every width or accessor. Add them only when a real asset schema needs those representations. They remain uncredited until actually used. The View/ReadBytes spans borrow storage and must not outlive their owner. |
| ReadCount, Fail, IsWellFormedUtf8 | Schema validation helpers. The patrol loader uses ReadCount(8, "waypoints") before allocating/reading coordinate pairs. Malformed counts are still a separate error path; a stand-alone UTF-8 predicate would duplicate parser tests rather than teach a CNA game concept. |
| Parse/ParseFile, HasMagic, Origin, container versions, asset identity, ChunkCount/ChunkAt/ChunkData, FindAll/FindSingle, Metadata | A focused container-inspection/diagnostics demo could inspect a written asset and reject malformed data. Current ContentManager loading invokes parsing internally but does not meaningfully show these public inspection members. HasMagic alone is only a prefix check, not complete validation. |
| SetExternalReferences, ExternalReferences, ExternalReferenceAt, CnbLogicalNameProblem | Pending a composite custom asset that loads another named asset through its owning manager. This deserves more than a synthetic getter round trip. Path and reference-index failures belong with that scenario. |
| SetCompression, IsCnbCompressionSupported, CompressCnbChunk/DecompressCnbChunk | Build-dependent optional codecs. A compression example should query support and compare logical payloads, document the stored-size versus load-time tradeoff, and avoid claiming a tiny chunk must compress. Raw codec helper calls need no separate example if the writer/document path teaches the behavior. No compressed custom asset is currently demonstrated. |
| CnbReadLimits / DefaultCnbReadLimits, writer SetLimits/Limits, document Limits | Pending a small bounded-data example with deliberately low limits, covering both writer refusal and reader refusal without large allocations. File size, per-chunk size and aggregate uncompressed size are different constraints; one rejected file does not demonstrate every limit. |
| ChunkIdToString, IsWellFormedChunkId, IsCustomAssetTypeId, AssetTypeIdToString and format constants | Diagnostic/format plumbing; use within inspection or schema authoring when it clarifies the example. No separate enum/constant demos. |

### Custom content pipeline and intermediate serialization

The public experimental `CNA::Content::Pipeline` contracts live in the runtime content module;
the XNA-shaped XML serialization/importer/processor adapters live in the separate build-time
`cna_content_pipeline` target. `content.custom.pipeline_components` now executes the native
three-stage route (6 checks pass on SOFTWARE and OPENGLES3); the XNA/XML adapter gaps below
remain source-audited, not runtime-validated.

| Members / behavior | Decision and evidence |
|---|---|
| ContentPipelineRegistry RegisterImporter/RegisterProcessor/RegisterWriter; ContentPipeline::Build | `pipeline_components` registers all three components, lets Build select them by declared types, precomputes an original patrol's open/closed lengths and loads the writer's CNB output. Selection identities are checked; no direct writer invocation is substituted for the coordinator. |
| ContentValue Create/Get/StableType/CppType/RawData/Empty | Create/Get belong naturally in typed component boundaries. Stable/C++ identity differences matter for adapters, but RawData/Empty inspection needs no independent demo. Wrong stored type and declared-versus-returned type mismatches remain distinct diagnostic gaps. |
| ContentProcessorParameters Set/Find/Values; ValidateParameters | The patrol checks a boolean loop parameter changing length 12 to 7 and rejects a string value through ContentPipelineError. Missing/default parameters, unknown names and component versions affecting incremental identity remain unvalidated. |
| ContentBuildResult output/schema/component identities and deterministic bytes | The patrol example checks selected components, compares identical Build outputs and loads the produced bytes. Build returning equal bytes does not establish the separate command-line compiler's incremental manifest/cache behavior. |
| ContentPipelineError Stage/Component/Source/LogicalName | The patrol checks Stage=Process and its processor name when typed parameter validation rejects a string. One rejected processor parameter does not cover import, selection, write, graph or publish failures. |
| Source dependency capabilities, runtime references, nested builds, deployment/additional outputs | Meaningful composite-asset work beyond a single schema. These need original authored dependencies and observable dependency/reference separation. Do not infer support from the compiler's use by cna_add_content alone. |
| Registry freezing/concurrent builds, RunContentCompiler embedding, XNA component adapters | Separate tool-integration and concurrency scenarios; the current demo executable need not grow an imitation build system. A standalone build tool is appropriate where the real build-time module must be linked. No threading or external-plugin coverage is claimed. |
| ContentSerializerAttribute, IntermediateReader/Writer, IntermediateSerializer, ContentTypeDescriptor | Real public APIs, not inert metadata. Current sources use format attributes and fluent field descriptors for element names, optional/null/ignored/flattened content and collection item names. A focused custom XML asset belongs in a build-time example; it requires the build-time target rather than borrowing an Internal XML parser. Shared-resource cycles, polymorphism and version/runtime-type metadata need their own scope decisions. |

### Texture2D (overload families and ownership)

| Members / behavior | Decision and evidence |
|---|---|
| Constructors, Width, Height, Bounds, inherited Format / LevelCount | `texture2d.set_data`, `surface_formats`, `mipmaps` and `from_stream` show allocated images, packed formats, mip levels and decoded bounds. Empty C++ handles are normal members in many demos; no separate constructor-per-overload demos. |
| SetData / GetData, whole image | `set_data` round trips procedurally generated pixels; `get_data_collision` puts readback to use for collision. These show the Color path, not arbitrary element layouts. |
| Rectangle, mip level, CPU startIndex / elementCount | `set_data_rectangle` checks a stroke and an untouched adjacent pixel; `mipmaps` writes every level and reads level 3. `set_data_rectangle` now uploads from CPU offset 1 and reads into offset 2 with untouched prefix/suffix checks; all four checks pass on SOFTWARE and OPENGLES3. |
| Packed and floating-point data overloads | `surface_formats` uploads Bgr565/Bgra4444/Bgra5551 and reads Bgr565 back. `render_target.hdr_blendable` reads HalfVector4 values past one. Other integer/normalized/float/vector layouts and mismatched element sizes remain unexercised; do not infer them from the collapsed SetData/GetData member count. Extend by useful format family, not one demo per overload. |
| Bound-texture update restriction | `set_data_rectangle` explicitly binds the texture and checks InvalidOperationException before unbinding for updates. Invalid rectangles, short buffers and disposed handles remain error-path gaps; add only coherent diagnostics to the relevant example. |
| FromStream, resize/crop overload | `from_stream` compares original dimensions, aspect-preserving fit and zoom/crop. `save_as_image` also decodes PNG and JPEG round trips. GIF decoding and malformed input are not thereby demonstrated. |
| SaveAsPng / SaveAsJpeg | `save_as_image` compares lossless and lossy in-memory round trips. Filename convenience overloads need no independent example. Requested dimensions differ from source dimensions only in a future resize-contract check; the current demo passes matching sizes. |
| DDSFromStreamEXT | Pending focused compressed-stream example. `content.xnb.dxt_texture` establishes XNB DXT1 loading and display, not DDS stream parsing or compressed block uploads. DDS resize/crop decodes to Color and deserves separate attention within that example. |
| C++ copy / move / assignment and shared resource ownership | Existing demos routinely copy handles and the handoff describes sharing, but there is no explicit copy-and-edit/lifetime check. Add a focused ownership example that distinguishes shared pixels from per-wrapper Name/Tag before calling this behavior covered. Move syntax alone needs no demo. |
| Dispose, inherited GraphicsResource properties/events | `graphics_resource.lifetime_events` checks explicit disposal, resource events, Name/Tag and IsDisposed. Alias disposal semantics are part of the pending ownership example. |
| SetDataRGBA, CreateFromPixels | CNA convenience paths; ordinary uploads already show the concept. Byte layout may fit a future C/C++ interop example, but no independent demo merely for the helper name. |
| GetRenderer / GetRendererWeak / GetCpuPixelsWeak / HasRenderer, ReconstructFromCache | Publicly exposed CNA integration and cache hooks, not ordinary game texture operations. Intentionally no independent demo unless a renderer/cache integration scenario requires them. |
| CreateCpuOnlyForTests, CreateWithRendererForTests | Explicit test seams; intentionally not game demonstrations. GetTypeName is routine runtime type identification, not an independent texture behavior. |

### SpriteBatch and SpriteFont

Reviewed against current public headers and the 19 sprite-batch / 7 sprite-font demos. This is
an overload-family and behavior audit, not a claim that every picture has a pixel assertion.
In particular, a successful bounded Game run does not itself verify the rendered layout.

| Members / behavior | Decision and evidence |
|---|---|
| SpriteBatch(GraphicsDevice), empty constructor, renderer-injection constructor | Device construction is shown in `sprite_batch.draw_texture` and throughout the graphics demos. Empty handles do not need their own example. The header explicitly labels renderer injection test-only; intentionally leave mock renderer construction to CNA's tests. |
| Begin/End sequencing | `begin_end_rules` checks Draw before Begin, End without Begin, duplicate Begin and concurrent batch rules. It exercises CNA's cross-batch Immediate guard; this audit does not establish that guard's genuine-XNA compatibility. It does not yet check DrawString outside a batch or post-Dispose calls. |
| Begin overloads, blend/sampler/depth/rasterizer states and transform matrix | `blend_states`, `sampler_states`, `multiple_batches` and `transform_matrix` demonstrate the coherent settings, not every progressively longer overload. Null/default forwarding and pointer-versus-value syntax do not warrant independent demos. Explicit state restoration/retention after End is a meaningful gap; do not infer it from the next Begin overwriting state. |
| Deferred, BackToFront and FrontToBack sorting; layerDepth | `layer_depth_sorting` now checks each mode's overlap color plus all three unobscured sprite colors (4 checks pass on SOFTWARE/OPENGLES3; screenshots inspected). Equal-depth stability and interaction with an enabled depth buffer are distinct pending behaviors; a sort mode is not a substitute for depth testing. |
| Texture sorting | `sort_mode_texture` reads overlaps and checks that Deferred alternates while Texture groups them, deliberately leaving the inter-texture group order unspecified. It does not measure draw-call savings or prove a particular texture ordering. The grouped result check now requires an authored red/blue color as well as uniform overlaps, so a blank row cannot pass. Both checks pass on SOFTWARE/OPENGLES3; screenshots inspected. |
| Immediate state changes | `immediate_mode.blend_state` verifies Opaque then Additive with pixels. The parallel sampler example fails visibly with ISSUE-004. A failed sampler path must not be credited as working renderer support. |
| Draw positions, destination/source rectangles, tint and alpha | Existing `draw_texture`, `destination_rectangle`, `source_rectangle`, `tint_and_transparency`, `blend_states` and `loose_png_alpha` expose these inputs directly. The rectangle/tint pictures mostly lack pixel assertions; passing smoke proves execution, not exact edge or blend values. Degenerate/out-of-range rectangles and disposed textures remain diagnostic gaps. |
| Rotation, origin, scalar/Vector2 scale, SpriteEffects | `rotation`, `scale` and `sprite_effects` are focused visual demonstrations. Cropped-source origins, negative scale combined with flips and transformed clipping deserve extensions only when they teach a distinct interaction, not an overload-per-demo expansion. New visual validation was not performed as part of this table. |
| Wrapping and animated source rectangles | `tiling_with_wrap` demonstrates oversized source rectangles with wrapping; `sprite_sheet_animation` derives frames from game time. These are coherent uses of Draw rather than new API types. The latter is intentionally unsuitable as a fixed-frame pixel baseline because Update catch-up changes elapsed game time. |
| Begin custom effect | `shader_effect.sprite_shader` checks grayscale luma via a GLSL ShaderEffect and skips other dialects. Compiled XNA Effect bytecode/multiple passes and effect changes during a queued batch remain renderer/toolchain-dependent gaps. |
| Dispose, inherited IsDisposed/Disposing; queued resource lifetime | Demos destroy SpriteBatch through RAII. Explicit disposal, event ordering, disposing an open batch and releasing a queued texture before End are meaningful ownership scenarios still to investigate. GetTypeName and routine C++ destruction syntax need no separate demo. |
| SpriteFont constructor, atlas, glyph bounds/cropping/kerning | `sprite_font.glyph_constructor` authors a digit atlas with layout tables and checks advances/character count; `content.cnj.sprite_font` shows file-authored glyphs. Nonzero/negative bearings, asymmetric cropping and mismatched table lengths remain distinct gaps. The constructor stores a Texture2D handle by value; its atlas is not a borrowed raw pointer. |
| Characters, DefaultCharacter | `default_character` checks missing glyph measurement and fallback substitution. Choosing a fallback absent from Characters, resetting the optional fallback, supplementary Unicode and newline/fallback interaction are pending extensions; the current accented text demonstrates missing glyphs, not general localization support. |
| MeasureString(string), MeasureString(StringBuilder) | `measure_string` checks fixed-width advances, multiple lines, widest line and trailing newline and uses the result for alignment. `string_builder` checks equivalence with ToString. Empty text, carriage returns, unusual bearings and measured versus actually drawn extents remain gaps. These should extend the existing demos. |
| Spacing, LineSpacing, copy independence | `spacing` changes a copied font, checks inter-character spacing and line height and displays both versions. Source uses value copies; explicit copy-independence checks and negative spacing behavior are not currently established. |
| DrawString overloads and transforms | `draw_string`, `string_builder` and `transformed_text` show ordinary/mutable text, scalar/per-axis scale, centered rotation and whole-string mirroring. Nonzero text layer depth with sprite/text overlap and pixel-checked transformed bounds remain pending; redundant overload syntax needs no separate demo. |
| getTextureEXT, getGlyphBoundsEXT, getCroppingEXT, getKerningEXT | Public read-only inspection extensions primarily useful to custom layout/bindings. They are not demonstrated merely because CNA internally reads the same tables. A small glyph-layout inspector could extend `glyph_constructor`; no getter-per-demo expansion. Returned references must not outlive the font. |

### SoundEffectInstance and DynamicSoundEffectInstance

These are source/behavior reviews. The automated checkpoint uses SDL's dummy audio driver;
state and buffer checks can pass there, but audible quality, panning and Doppler are not validated.

| Members / behavior | Decision and evidence |
|---|---|
| SoundEffect::CreateInstance; Play, Pause, Resume, Stop(), State | `sound_effect_instance.playback_state` checks the normal state sequence, replay after stopping and natural completion. Repeated Play while already playing, Pause/Resume from stopped and disposed-object errors remain lifecycle gaps; extend that example when useful rather than creating one per state. |
| IsLooped | `sound_effect_instance.looping` sets it before Play, verifies continued playback and rejects mutation after first Play. `sound_effect.loop_region` supplies authored loop points. Stop(false) and the loop's release behavior remain distinct from immediate Stop and need their own explicit checks. |
| Volume, Pitch, Pan | `sound_effect_instance.volume_pitch_pan` changes the properties while playing. This is an audible example, not proof of mixer output under the dummy driver. Setter range errors and the interaction of Pan with a started 3D voice are not demonstrated. |
| Apply3D overloads, listener/emitter state | `sound_effect_instance.apply_3d` moves one emitter around one listener and calls Apply3D before and during playback. Multiple-listener forms, emitter/listener Velocity and actual Doppler effects remain gaps; a printed SpeedOfSound default does not cover them. |
| Dispose, IsDisposed, protected disposal hook; C++ moves | Scope destruction releases the ordinary demo voices, but explicit disposal/idempotence and a surviving instance after its source SoundEffect is destroyed are meaningful lifetime gaps. The move syntax and GetTypeName need no independent demonstrations. |
| Dynamic constructor, Play, SubmitBuffer, BufferNeeded, PendingBufferCount | `dynamic_sound_effect_instance.streaming` generates mono 16-bit PCM, prequeues buffers and refills from the game-thread event. It checks playback and event requests and reports queue counts; it does not establish exact callback counts or underrun recovery. |
| GetSampleSizeInBytes, GetSampleDuration | Streaming now checks that 50 ms of 24 kHz mono PCM16 needs 2400 bytes and converts that count back to 50 ms through the instance duration helper. SoundEffect's static helpers are separately shown in `from_pcm_buffer`. |
| SubmitBuffer offset/count; SubmitFloatBufferEXT | Pending coherent buffer-format extension: sliced submissions, frame alignment and float PCM mode changes. These behaviors are not inferred from one whole-vector 16-bit submission. Avoid one demo per overload. |
| Dynamic IsLooped / Stop(false) | Streaming now demonstrates accepting IsLooped=false, rejecting true (the header contradiction remains ISSUE-037), refusing Stop(false) while continuing playback, and immediate Stop clearing pending buffers. Post-Dispose PendingBufferCount and IsLooped must throw by both the XNA spec and local genuine-XNA source; CNA instead returns 0/false (ISSUE-038). Both renderer selections execute 11 checks with only these two expected failures. No listening or genuine-XNA runtime execution is claimed. |
| QueueInitialBuffers, ClearBuffers, Update (CNA extensions) | Integration helpers. Ordinary games demonstrate the managed refill path through Game/FrameworkDispatcher. Explicit queue clearing or manual pumping needs a streaming lifecycle scenario before being credited; no isolated helper-name demos. |

### XACT engine, banks, cues and categories

| Members / behavior | Decision and evidence |
|---|---|
| AudioEngine constructors, RendererDetails, ContentVersion | `xact.play_cue` creates the settings/banks and pumps Update; `renderer_details` lists output IDs/names and uses the look-ahead/output overload. The demo explicitly notes CNA's default-output limitation. ContentVersion is a format constant and needs no independent demo. |
| AudioEngine::GetCategory, AudioCategory::Name / equality / Pause / Resume / Stop / SetVolume | `xact.categories` distinguishes Music/SFX identities, controls their cues independently and rejects an unknown name. SetVolume is called, but audible attenuation is not measured by the dummy-driver checkpoint. Equals/GetHashCode/ToString add no separate category concept. |
| GetGlobalVariable / SetGlobalVariable; Cue::GetVariable / SetVariable | `global_variables` checks values and clamping; `cue_variables` checks independent per-cue storage, scope separation and read-only variables. These do not demonstrate an RPC curve changing audible volume/pitch or selecting an interactive variation. |
| SoundBank constructors, PlayCue overloads, GetCue, IsInUse | `play_cue`, `positional_cue` and `cue_states` distinguish fire-and-forget playback, positional playback and controlled cue instances. GetCue returns caller-owned C++ objects; the four demos now use unique_ptr so Dispose releases playback and scope exit deletes the object. All eight existing XACT demos pass after this ownership correction. Unknown cue lookup and disposing an engine/bank with live cues remain lifecycle gaps. |
| Cue::Name, state flags, Play/Pause/Resume/Stop/Dispose | `cue_states` exercises prepared/playing/paused/stopped/disposed states and obtains a fresh cue to replay. Printing IsStopping does not exercise an authored release, and IsCreated/IsPreparing are not demonstrated. Add authored fade/stop behavior with the necessary asset data. |
| Cue::Apply3D | `positional_cue` uses SoundBank's listener/emitter PlayCue overload, not an explicit moving Cue::Apply3D call. Extend that example for a controlled moving cue; do not credit the member merely because the internal implementation calls it. |
| WaveBank constructors, IsPrepared, IsInUse | `play_cue` loads an in-memory bank; `streaming_wave_bank` uses offset/packet-size construction, plays it and checks release. Its relative-path restriction is recorded as ISSUE-028. Other packet sizes are not independent concepts. |
| Disposing events, IsDisposed, explicit bank/engine disposal | `cue_states` now checks the Disposing sender, one notification across repeated Dispose, and no extra notification when the C++ object is deleted (9 lifecycle checks pass). Renderer selection disposes a probe engine. Bank/engine events and cascading disposal remain separate ownership gaps; the cue example does not establish their ordering. |
| RPC curves, complex variations, instance limits | `xact.instance_limits` now uses authored category limit 1 with Fail and ReplaceOldest, checking paused slots, category isolation and reuse after Stop through public cue states (7 checks pass on both renderer selections). RPC/variation tables, cue-level limits, priorities and fades remain pending; volume/pitch and variation choice still need listening or a supported observation route. |

Remaining audit scope: other XNA public members, overload families and behaviors, CNA-specific
headers, renderer-specific execution, and manual/service scenarios. Neither these tables
nor the existing type decisions constitute a complete public API audit.
