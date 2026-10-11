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
| Curve constructor, Keys, Evaluate | Existing `math.curve.evaluate` constructs a brightness timeline and checks keys and interpolation. Empty/single-key evaluation can be added to `key_collection` with its IsConstant checks. |
| ComputeTangents | Existing `tangents` compares Flat/Linear/Smooth. The separate incoming/outgoing tangent overload remains a meaningful gap; extend that demo rather than adding an enum-variant demo. |
| ComputeTangent | Extend `tangents` to edit one key's incoming/outgoing slopes and check the interpolated result; whole-curve computation alone does not show selective editing. |
| PreLoop, PostLoop | Existing `loop_types` checks every post-loop mode and pre-loop Cycle. Negative-time CycleOffset/Oscillate boundaries remain useful checks within the existing demo. |
| Curve Clone, IsConstant | Existing `key_collection` checks independence after removals and the transition to a single key. No separate clone demo. |
| Collection constructor, Add, Item, Count, IndexOf, Remove, RemoveAt, Clear | Existing `key_collection` demonstrates sorted insertion and editing. Duplicate positions and Item replacement order are distinct edge cases to investigate before claiming complete collection behavior. |
| Collection Clone, Contains, CopyTo, IsReadOnly | Clone fits existing copy/edit checks if extended; Contains is the boolean counterpart of demonstrated IndexOf. CopyTo and IsReadOnly are ordinary collection mechanics, intentionally without independent demos. They remain uncredited until actually shown. |
| GetEnumerator, explicit IEnumerable.GetEnumerator | C# iteration machinery; C++ uses range iteration/indexing. No independent demo. |
| Step continuity | Existing `step_continuity` is a failing compatibility example (ISSUE-002), not a successful implementation of Step. |

Remaining audit scope: other XNA public members, overload families and behaviors, CNA-specific
headers, renderer-specific execution, and manual/service scenarios. Neither these tables
nor the existing type decisions constitute a complete public API audit.
