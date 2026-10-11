# Coverage audit

Phase 15 of [ROADMAP.md](ROADMAP.md): for every XNA 4.0 type that
[COVERAGE.md](COVERAGE.md) lists under "Types without a demo yet", a decision -- a demo, part of
another demo, or not independently meaningful -- with the reason. COVERAGE.md is generated; this
file is maintained by hand and should be revisited whenever that list changes.

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
| ContentSerializerAttribute, ContentSerializerCollectionItemNameAttribute, ContentSerializerIgnoreAttribute, ContentSerializerRuntimeTypeAttribute, ContentSerializerTypeVersionAttribute | not meaningful: .NET attributes that steer the XNA IntermediateSerializer at build time; C++ has no attributes and CNA's pipeline does not read them |

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
| Constructors, RootDirectory | `content.content_manager.several_managers` shows separate roots and caches, and `missing_assets` shows slash normalization. The CNA default constructor and nullable service provider support console data loading; they do not need independent demos. Changing RootDirectory after loading deserves an explicit cache-lifetime example before claiming that behavior covered. |
| ServiceProvider | Several managers receive the game's services; `game.services.graphics_device_service` explains device service lookup. The getter is ordinary identity plumbing, intentionally without an independent demo; it is not currently named as a demonstrated member. |
| Load (cache, type and name resolution) | `cache_and_unload` counts created textures, `custom.cnb_loader` counts factories, and `several_managers` distinguishes per-manager caches. `missing_assets` checks failures and backslash names. Same-name files in competing XNB/CNB/CNJ/native tiers and the cache's type key remain meaningful gaps; loading different names in `xnb.same_assets` does not establish precedence. |
| Unload, Dispose() | `cache_and_unload` shows rereading after Unload; `unload_disposes` explicitly fails on undisposed textures (ISSUE-011). `several_managers` calls Dispose on level exit. These uses do not demonstrate the protected Dispose(bool) override hook or post-disposal errors; add those to the custom-manager example when extending it. |
| OpenStream, ReadAsset | `open_stream_override` serves an in-memory XNB via ReadAsset, then demonstrates that Load bypasses the override (ISSUE-012). `resource_content_manager` exposes the same defect for embedded resources. The ReadAsset disposable-recording callback and uncached repeat reads remain useful extensions, not implicitly covered by one successful read. |
| RegisterTypeReader, LooseFileContentTypeReader::Read / GetExtensions | `custom.loose_file_reader` teaches a tile-map format and checks caching. Ordinary adapter methods need no independent demos. |
| RegisterCnbLoaderEXT | `custom.cnb_loader` shows a game-defined binary schema, process-wide registration, separate caches, mandatory chunk validation and unsupported-schema rejection. Malformed chunks and optional unknown chunks are not yet exercised; the successful call alone does not cover those failure behaviors. |
| RegisterCnjLoader, CnjLoaderFn | Pending: unlike CNB registration, these factories belong to one manager and dispatch on both C++ type and document type name. They receive raw JSON. A meaningful payload example needs an explicit parser choice; the project currently has no public JSON-parser dependency. Do not borrow CNA's Internal parser or imply that constant-return factories demonstrate data deserialization. |
| GetContentManifest, GetXnbReaderUsageSummary | Existing `content_manager.manifest` enumerates logical asset names, format flags and XNB reader availability. A count alone would not show these distinct fields. Pipeline bookkeeping files remain visible (ISSUE-013). |
| RefreshContentManifest | `manifest` now adds/removes a private CNJ file between scans, loads the new file before refreshing, and confirms refresh does not discard the loaded asset. All seven new snapshot/cache checks pass on SOFTWARE and OPENGLES3. |
| setGraphicsDevice, getGraphicsDeviceInternal | CNA integration helpers; normal game demos use IGraphicsDeviceService through the supplied provider. No separate demonstration unless a standalone graphics-content scenario needs explicit device injection. |

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
| GetSampleSizeInBytes, GetSampleDuration | Streaming uses the size helper to allocate 50 ms buffers. The inverse dynamic-instance duration helper is not shown; add a round trip there. SoundEffect's static helpers are shown in `from_pcm_buffer` but that does not name the dynamic-instance member. |
| SubmitBuffer offset/count; SubmitFloatBufferEXT | Pending coherent buffer-format extension: sliced submissions, frame alignment and float PCM mode changes. These behaviors are not inferred from one whole-vector 16-bit submission. Avoid one demo per overload. |
| Dynamic IsLooped / Stop(false) | Pending targeted guards in streaming: the current implementation rejects IsLooped=true, despite the public header's stale claim that the setter has no effect. Read the implementation and reference contract before adding expectations; no new runtime claim is made by this audit. Stop(false) after playback is a separate unsupported request to demonstrate. |
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
| Disposing events, IsDisposed, explicit bank/engine disposal | Cue disposal is checked and renderer selection disposes a probe engine. Events and cascading disposal deserve a coherent ownership example; ordinary destructor use does not prove event ordering or idempotence. |
| RPC curves, complex variations, instance limits | `xact.instance_limits` now uses authored category limit 1 with Fail and ReplaceOldest, checking paused slots, category isolation and reuse after Stop through public cue states (7 checks pass on both renderer selections). RPC/variation tables, cue-level limits, priorities and fades remain pending; volume/pitch and variation choice still need listening or a supported observation route. |

Remaining audit scope: other XNA public members, overload families and behaviors, CNA-specific
headers, renderer-specific execution, and manual/service scenarios. Neither these tables
nor the existing type decisions constitute a complete public API audit.
