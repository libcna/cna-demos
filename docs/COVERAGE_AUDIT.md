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
| ContentSerializerAttribute | `content.custom.intermediate_serializer` consumes the public format object and now covers renamed and implicit fields, optional/null/ignored/flattened content, collection item names and required-value refusal through emitted XML and round trips. Shared resources, polymorphism and descriptor version/runtime metadata are covered by the related intermediate workflows. The nominal .NET-style wrapper objects are intentionally dispositioned in the next row because CNA's C++ serializer consumes descriptors instead. |
| ContentSerializerCollectionItemNameAttribute, ContentSerializerIgnoreAttribute, ContentSerializerRuntimeTypeAttribute, ContentSerializerTypeVersionAttribute | Intentionally no independent wrapper-object demos. The intermediate serializer workflows demonstrate collection item naming, ignored fields, polymorphic runtime types and versioned type serializers through the actual public C++ ContentTypeDescriptor/ContentMemberDescriptor registration API. CNA's C++ consumers read those descriptors, not instances of the nominal .NET attribute wrappers. Constructing and reading wrapper values would add seven inventory credits without exercising serialization behavior. |

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
| GamerPrivilegeException | shown in `c_api.gamer.guide_social_offline`: the offline Marketplace call now checks the exact canonical exception identity together with its INVALID_STATE/category STATE C result. |
| GamerCollection, GamerCollection.GamerCollectionEnumerator | shown by the `net.*` demos (AllGamers, LocalGamers); the enumerator is C#'s `foreach` machinery (C++ range-for) |
| IAvatarAnimation | demo: `gamer_services.avatar.custom_animation` |
| NotificationPosition | `gamer_services.guide.notification_position` checks the default/changed property and actual overlay pixels on both renderers. The corrected observer starts with early frames and terminates on either visible placement or a finite deadline; all four original checks remain. The old eleven-frame-plus-sleep assumption reproduced an empty-frame failure under load. Fresh runs capture the toast on observation two, and frame-eight screenshots were inspected on both renderers. This does not establish every enum position or notification queue behavior. |
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
| EffectAnnotation, EffectAnnotationCollection | environment: compiled XNA effects need Microsoft's `fxc` (`fx_2_0`, a Windows tool run through wine) to compile `.fx`, and a CNA build with `CNA_<RENDERER>_COMPILED_EFFECTS` (fetches FNA3D/MojoShader); neither is part of this repository's builds |
| EffectMaterial | shown in `graphics.effect.custom_apply_lifecycle`: construction clones the source Effect's fallback reflection graph, and the material remains usable after the source is disposed. This covers the public constructor and independent lifetime without claiming a compiled-FX material asset. |
| GraphicsDeviceStatus | `owned_lifecycle` observes Normal through construction, Present and four real Reset cycles. Lost and NotReset remain environment values: CNA changes them only after a renderer reports real device loss/resetting, and the current desktop renderers provide no public deterministic trigger. |
| DeviceLostException, DeviceNotResetException | environment: device loss does not happen on CNA's current desktop test renderers |
| NoSuitableGraphicsDeviceException | shown in `graphics.graphics_adapter.display_modes`: after a SOFTWARE device latches the renderer, changing the global request to HEADLESS makes a second real GraphicsDevice construction take CNA's production incompatibility refusal. |

## Microsoft.Xna.Framework.Input

| Type | Decision |
|---|---|
| Keys, KeyState | shown in `input.keyboard.state_values` |
| TouchCollection.Enumerator | shown in `input.touch.touch_values`: the explicit public value enumerator now proves invalid pre-start state, ordered movement, independent cursor copies, reset, typed and erased Current, harmless Dispose and snapshot independence after source mutation. |

## Microsoft.Xna.Framework.Media

| Type | Decision |
|---|---|
| MediaSourceType | shown in `media.media_library.collections` |
| VideoSoundtrackType | shown in `media.video_player.play_video` |
| Album, Artist, Genre, Playlist and their collections; SongCollection; MediaLibrary lifetime | `music_index` imports three original tagged FLAC tracks and an M3U through a private Linux/XDG MediaLibrary. Its 29 established controls retain tag, grouping, equality and duration coverage. Focused884-886 then separate successful repeated-disposal flags and non-cascading library ownership from XNA's required post-disposal refusals: pinned CNA leaves every tested entity/library operation callable and clears collection views instead of throwing, recorded as ISSUE-098. Evidence: `/tmp/checkpoint-884-*` through `/tmp/checkpoint-886-*`. CLR `GetEnumerator` entries remain separately dispositioned because actual C++ range iteration is already used. No playback, listening or other-platform credit. |
| Picture, PictureAlbum and their collections | `save_picture` uses verified private Linux/XDG roots, generated files, album membership and owned streams. Repeated disposal marks selected objects and collection views without destroying retained siblings or entries, but Focused881/886 show that none of the tested ordinary entity operations and neither collection's Count/Item pair produces XNA's required ObjectDisposedException (ISSUE-098). The workflow otherwise retains its picture-tree, image-stream and ISSUE-041 short-read evidence. Evidence: `/tmp/checkpoint-881-*` and `/tmp/checkpoint-886-*`. CLR `GetEnumerator` entries remain separately dispositioned because actual C++ range iteration is already represented. Other platforms remain untested. |
| VisualizationData constructor | `media.media_player.visualization` first requires both newly constructed 256-value buffers to be entirely zero, then MediaPlayer fills that same object and produces positive aggregate spectrum energy. Both renderers pass2/2 with identical stdout and empty stderr; focused SOFTWARE CTest passes. Evidence: `/tmp/checkpoint-692-visualization-*`. This is programmatic buffer-state coverage, not acoustic accuracy or visual-quality evidence. |
| Seven Media collection `GetEnumerator` pairs | These are generic/non-generic CLR inventory artifacts. Existing workflows already use CNA's public C++ `begin`/`end` ranges; naming CLR methods that CNA does not expose would falsely inflate coverage. All14 entries are intentionally uncredited. |

## Microsoft.Xna.Framework.Net

| Type | Decision |
|---|---|
| GameStartedEventArgs, GameEndedEventArgs | shown in `net.network_session.game_flow` |
| AvailableNetworkSession, AvailableNetworkSessionCollection, QualityOfService | demo: `net.network_session.find_and_join` (two processes). The listing exposes exact public/private capacity and sparse advertised properties; disposal is idempotent, retains readable listings and prevents Join through both original and copied entries. |
| NetworkSessionJoinError, NetworkSessionJoinException | demo: `net.network_session.join_refused` (a game in progress; expected failure, CNA-DEMOS-ISSUE-032). A full session cannot be shown: `Find` lists only sessions with room for the searcher's gamers |
| GamerLeftEventArgs | demo: `net.network_session.find_and_join`, `net.network_session.host_leaves` |
| HostChangedEventArgs, NetworkSessionEndReason, NetworkSessionEndedEventArgs | demo: `net.network_session.host_leaves` (two processes) |
| WriteLeaderboardsEventArgs | service: CNA raises the `Write*Leaderboard`/`WriteTrueSkill` events only for leaderboard gameplay on the CNA service |

## Microsoft.Xna.Framework.Storage

| Type | Decision |
|---|---|
| StorageDeviceNotConnectedException | shown in `storage.storage_device.show_selector`: on Linux a controlled file in place of `XDG_DATA_HOME` drives CNA's real root-creation failure, preserving the exact exception type and filesystem cause before the demo restores the environment and opens the normal device. |
| StorageDevice.DeviceChanged | CNA exposes subscription state but has no production path that raises the event. Invoking the public event from a demo would manufacture behavior rather than observe a device change; leave it uncredited until a platform removal path exists. |
| StorageContainer.Finalize | CLR finalizer inventory entry. Existing workflows explicitly dispose owned containers; no C++ finalizer demo. |

## Public member and behavior audit (started 2026-10-11)

Type coverage is not comprehensive coverage. The generated member count collapses overloads
and records metadata claims; it does not prove that overloads, error paths, or behavior are
demonstrated. This pass examines those separately. The initial source review used CNA `2b15506a4`; later
rows record focused follow-ups against the then-current checkout. Consult HANDOFF.md and each
checkpoint's evidence for the actually tested artifacts, rather than treating this whole audit
as one atomic API snapshot. Planned work is not counted as completed or runtime-validated.

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

### BoundingBox, BoundingSphere and Ray value identity

| Members / behavior | Decision and evidence |
|---|---|
| BoundingBox equality, operators and hash | `math.bounding_box.create_from_points` already compares the generated bounds with its exact manual min/max constructor. It now also uses typed/boxed equality, rejects wrong and empty boxes, checks equal hashes and makes a one-coordinate maximum change observably unequal. The workflow passes6 controls on both renderer selections. |
| BoundingSphere equality, operators and hash | `math.bounding_sphere.create` compares the actual CreateFromBoundingBox result with an independently constructed copy through typed/boxed equality, both operators and equal hashes, rejects wrong/empty boxes and distinguishes an expanded radius. Its existing point/box/merge construction remains intact; totals are9 controls on both selections. |
| Ray equality, operators and hash | `math.ray.intersects` compares the real +X casting ray with an independent copy through typed/boxed equality, both operators and equal hashes, rejects wrong/empty boxes and distinguishes a +Y direction. Its box/sphere/plane hit distances remain intact; totals are8 controls on both selections. |
| BoundingSphere.Contains and CreateFromFrustum | `math.bounding_sphere.create` now distinguishes Contains/Intersects/Disjoint for points and spheres, and verifies unambiguous box/frustum containment. CreateFromFrustum builds from an identity frustum and numerically encloses all eight actual corners within 1e-5 without asserting an unspecified minimum sphere. The first focused check incorrectly required strict `Contains` for each boundary corner; Ritter construction plus float rounding can put a corner just outside strict classification, so that was a demo assumption and was replaced by the geometric tolerance check already used by the C audit. |

Focused651 adds twelve member credits without new IDs, moving temporary coverage1545->1557/2326.
All three IDs have byte-identical renderer outputs and empty stderr; all focused CTests pass.
Evidence:`/tmp/checkpoint-651-*` and `/tmp/checkpoint-651-coverage.md`. No rendering, screenshot or
visual inspection was used.

Focused652 extends the sphere workflow from9 to12 controls and adds the two remaining member
credits, moving temporary coverage1557->1559/2326. SOFTWARE and OPENGLES3 are byte-identical passes
with empty stderr and the focused CTest passes. Evidence:`/tmp/checkpoint-652-*` and
`/tmp/checkpoint-652-coverage.md`. No rendering or visual inspection was used.

### EffectParameter

| Members / behavior | Decision and evidence |
|---|---|
| Name, ParameterClass, ParameterType, RowCount, ColumnCount | `parameters_and_techniques` already lists names/classes; `parameter_values` relates shape/type to the accessor used and inspect Bones array shape. |
| SetValue; GetValueSingle, GetValueInt32, GetValueVector4, GetValueTexture2D | Demonstrated in `parameter_values`: typed round trips through named stock parameters; do not imply that setting a parameter updates the stock effect's cached public properties. |
| GetValueMatrix, GetValueMatrixTranspose, SetValueTranspose | Demonstrated in `parameter_values`: use a non-symmetric translation/rotation matrix to make transpose semantics observable. Identity alone would not test it. |
| Elements, GetValueMatrixArray, GetValueMatrixTransposeArray | Demonstrated in `parameter_values`: two distinct bone transforms, round trips and reflected array shape. Source currently constructs Bones as a 72-by-4 matrix with no Elements; confirmed on SOFTWARE and OPENGLES3 (ISSUE-035). |
| GetValueBoolean and Boolean/Int32/Single arrays | `parameter_values` now groups independent CNA-constructed parameter storage by value family. A scalar true and ordered true/false/true values round trip; signed integers and three exact floats retain order. These checks cover public CPU storage and do not claim compiled FX packing. |
| GetValueVector2, GetValueVector3, GetValueQuaternion and their array forms; GetValueVector4Array | Distinct scalar and two-entry arrays round trip exact signed components; quaternion values are observed as float4 values. One coherent workflow covers widths/order without adding getter-list IDs. Compiled register layout remains a separate boundary. |
| GetValueTexture3D, GetValueTextureCube | Real 2x2x2 and 2x2 resource objects round trip by exact identity through correctly typed parameters. Their shader sampling behavior remains covered by the separate ShaderEffect volume/cube demos, not claimed here. |
| GetValueString, Semantic, Annotations, StructureMembers | An authored `Material` struct parameter carries semantic `MATERIAL0`, a `Tint` member found case-insensitively by `COLOR0`, and an attached `UiName` string annotation read back as `Surface tint`. This demonstrates the public CNA-constructed reflection graph and brings EffectAnnotation/EffectAnnotationCollection into coverage; it does not claim compiler-populated annotations or structs. |
| Invalid accessor / parameter-shape combinations | Demonstrated in `parameter_values`: Vector4 read as Single should be checked explicitly. CNA validates compiled storage shapes but the stock storage getter returns its first component; confirmed on SOFTWARE and OPENGLES3 (ISSUE-034). |

Focused668 extends the existing ID from16 to25 controls. Twenty-three pass on both renderers; the
same two documented stock-effect assertions expose ISSUE-034/035, yielding the expected known-
issue failure with byte-identical stdout and empty stderr. The known-issue SOFTWARE CTest passes.
Temporary coverage moves1738->1760/2326 and299->301/329 types without a new ID; EffectParameter has
no generated member gap, and EffectParameterCollection retains only CLR enumerators. Evidence:
`/tmp/checkpoint-668-*` and `/tmp/checkpoint-668-coverage.md`. No drawing, screenshot or visual
inspection was used.

### EffectAnnotation typed metadata

| Members / behavior | Decision and evidence |
|---|---|
| GetValueBoolean, GetValueInt32, GetValueSingle; ParameterClass, ParameterType | The authored `Material` annotations retain exact true, 42 and 0.35 scalar values and report Scalar with their exact public parameter types. |
| GetValueVector2, GetValueVector3, GetValueVector4 | Distinct exact component sequences establish each reflected width without relying on implicit truncation. |
| GetValueMatrix; Semantic, RowCount, ColumnCount | A non-symmetric 4x4 annotation round trips the exact public Matrix value and reports `LOCAL_TO_WORLD`, four rows and four columns. Its column-major storage fixture prevents a transposed implementation from passing. |

Focused669 extends the same ID from25 to28 controls and credits all12 remaining EffectAnnotation
members, moving temporary coverage1760->1772/2326. Both renderers retain26 passing controls plus
the two existing ISSUE-034/035 failures, with byte-identical stdout and empty stderr; the known-
issue SOFTWARE CTest passes. EffectAnnotation has no generated gap; EffectAnnotationCollection
retains only CLR enumerators. This establishes CNA-constructed annotation storage, not compiler-
populated FX reflection. Evidence:`/tmp/checkpoint-669-*` and
`/tmp/checkpoint-669-coverage.md`. No drawing, screenshot or visual inspection was used.

### Effect technique and pass reflection

| Members / behavior | Decision and evidence |
|---|---|
| EffectPass.Name; EffectPassCollection.Item | The real BasicEffect graph reports pass `P0`; integer and exact-name lookup return the same object, while a case mismatch and negative index return null. |
| EffectTechnique.Annotations; EffectPass.Annotations | Separate authored string annotations remain scoped to their technique and pass. This demonstrates CNA's public reflection storage without implying compiled FX annotations on the stock effect. |
| Collection GetEnumerator members | EffectAnnotationCollection, EffectParameterCollection, EffectPassCollection and EffectTechniqueCollection expose C++ iteration. Their three CLR/C# enumerator inventory entries per type do not justify duplicate demos. |

Focused670 extends `parameters_and_techniques` from4 to8 controls and credits the four remaining
non-enumerator members, moving temporary coverage1772->1776/2326. SOFTWARE and OPENGLES3 pass with
byte-identical stdout and empty stderr; the focused SOFTWARE CTest passes. EffectPass and
EffectTechnique have no generated gap, and both collections retain only the dispositioned CLR
enumerators. Evidence:`/tmp/checkpoint-670-*` and `/tmp/checkpoint-670-coverage.md`. No drawing,
screenshot or visual inspection was used.

### Base Effect apply and lifecycle

| Members / behavior | Decision and evidence |
|---|---|
| Constructor | `custom_apply_lifecycle` directly constructs a derived base Effect and verifies its empty parameter collection plus exact `Default`/`P0` reflection graph. |
| OnApply | The derived override counts one call from CNA's direct Apply extension and one from the current pass. This exercises the protected hook through both real application paths. |
| Dispose | Disposal marks the Effect before raising one Disposing event; both direct and pass application then throw ObjectDisposedException without entering OnApply, and repeated disposal is idempotent. |

Focused671 adds one coherent six-control ID and credits Effect's last three inventory members,
moving temporary coverage1776->1779/2326 at630 demos. SOFTWARE and OPENGLES3 pass with byte-
identical stdout and empty stderr; the focused SOFTWARE CTest passes. Effect has no generated
member gap. Evidence:`/tmp/checkpoint-671-*` and `/tmp/checkpoint-671-coverage.md`. The workflow
does not draw, take a screenshot or claim visual behavior.

### Stock-effect interface state and EnvironmentMapEffect cloning

| Members / behavior | Decision and evidence |
|---|---|
| AlphaTestEffect World/View and fog family | The interface workflow applies nonidentity world/view plus enabled 6..16 fog through IEffectMatrices/IEffectFog, reads the exact values from AlphaTestEffect and draws the fogged sphere. |
| EnvironmentMapEffect fog and IEffectLights state | The same real draw reads exact matrix/fog state, enabled lighting and nonzero default ambient from the concrete effect. IEffectLights AmbientLightColor/LightingEnabled are therefore covered through the interface and concrete implementation. |
| EnvironmentMapEffect Clone, DiffuseColor, AmbientLightColor, DirectionalLight0/1/2 | `reflection` now creates the right sphere by cloning the fully configured left effect. The clone retains both exact texture identities, material, ambient and all three enabled lights; independent transform/Fresnel changes then produce the established distinct exact center pixels. |
| OnApply entries | Every stock-effect scene applies a real pass and renders through the protected hook. The hook is an override point rather than a caller-invoked public behavior; do not add a synthetic per-effect metadata check after the base Effect hook workflow already established the dispatch contract. |

Focused675 grows `effect_interfaces` from3 to5 controls and environment reflection from2 to4,
crediting19 members without a new ID and moving temporary coverage1791->1810/2326. Both IDs pass
on SOFTWARE and OPENGLES3 with byte-identical stdout and empty stderr; both focused SOFTWARE CTests
pass. Evidence:`/tmp/checkpoint-675-*` and `/tmp/checkpoint-675-coverage.md`. Pixel validation used
programmatic readback; no screenshot or visual inspection was used.

### AlphaTestEffect alpha, vertex color and clone

`cutout` now creates the rendered right-hand effect by cloning a fully configured source. Texture,
projection, compare/reference threshold, alpha and vertex-color state survive, then source changes
prove the clone independent. Its colored vertex format supplies cyan with half red, and Alpha=.75
premultiplies the opaque texture center to exact RGBA(96,191,191,191). The discarded near corner
reveals the farther orange circle as exact RGBA(96,124,0,191), so both the vertex-color shader
variant and alpha-test/depth behavior affect pixels.

Focused676 grows the ID from2 to5 controls and credits Alpha, VertexColorEnabled and Clone, moving
temporary coverage1810->1813/2326. SOFTWARE and OPENGLES3 pass with byte-identical stdout and empty
stderr; the focused SOFTWARE CTest passes. AlphaTestEffect retains only its protected OnApply hook,
already dispositioned through real pass dispatch. Evidence:`/tmp/checkpoint-676-*` and
`/tmp/checkpoint-676-coverage.md`. Pixel validation used programmatic readback; no screenshot or
visual inspection was used.

### DualTextureEffect matrices, material, fog, vertex color and clone

| Members / behavior | Decision and evidence |
|---|---|
| World / View | The source uses nonidentity +5/-5 X translations that cancel in the real draw. Exact clone readback proves both survive independently rather than relying on the unchanged screen position alone. |
| DiffuseColor / Alpha / VertexColorEnabled | Diffuse(1,.5,1), Alpha=.8 and a packed cyan vertex tint with half red all enter the rendered quad. The brightest pixel's near-equal red/green, doubled blue and alpha204 distinguish their combined terms. |
| FogEnabled / FogColor / FogStart / FogEnd | Enabled black fog from0 to2 attenuates the constant object-space z=-1 quad by one half. Exact clone readback covers every value and the exact pixel result proves the enabled fog path. |
| Clone | The clone preserves both exact texture identities, matrices, material, vertex-color selection and fog state. Subsequent source Alpha/vertex-color/fog mutations do not change the rendered clone. |
| OnApply | The protected hook is reached by every real pass draw and its override dispatch contract was proved by focused671. It is dispositioned without synthetic direct-member credit. |

Focused677 grows `light_map` from2 to4 controls and credits all ten caller-visible gaps, moving
temporary coverage1813->1823/2326. SOFTWARE and OPENGLES3 pass4/4 with byte-identical stdout and
empty stderr; the focused SOFTWARE CTest passes. Exact programmatic readback is brightest
RGBA(102,102,203,204) and dark-corner RGBA(16,16,32,204). A first raw-vertex layout and first
pixel estimate were demo errors corrected before validation; neither exposed a CNA defect.
Evidence:`/tmp/checkpoint-677-*` and `/tmp/checkpoint-677-coverage.md`. No screenshot or visual
inspection was used.

### EnvironmentMapEffect alpha, emissive and environment specular

The existing reflection workflow sets Alpha=.75, zero diffuse, red EmissiveColor=.25 and blue
EnvironmentMapSpecular=.2 before cloning. Exact readback proves the clone retains every value;
temporary source mutations of all three prove independence before the source is restored. Zero
diffuse isolates the new terms from directional and ambient diffuse lighting.

The Fresnel0 centre is exact RGBA(0,191,38,191): lime reflection and alpha produce green191/alpha191,
and blue38 is the independent environment-specular add. At the facing Fresnel1 centre reflection
falls nearly to zero and exact RGBA(48,1,38,191) exposes the alpha-scaled red emissive surface while
blue specular and alpha remain. Focused678 grows the ID from4 to5 controls, credits all three gaps
and moves temporary coverage1823->1826/2326. SOFTWARE and OPENGLES3 output is byte-identical with
empty stderr; the focused SOFTWARE CTest passes. EnvironmentMapEffect's only remaining inventory
entry is protected OnApply, dispositioned through real pass dispatch. Evidence:
`/tmp/checkpoint-678-*` and `/tmp/checkpoint-678-coverage.md`. Pixel validation used programmatic
readback; no screenshot or visual inspection was used.

### ShaderEffect and shader dialect selection

Source review of ShaderEffect.hpp/.cpp, Effect::Clone's public contract, the four GLSL shader/MRT
demos and the newly public CNA/ShaderDialectEXT.hpp. Existing complete 455 evidence covers the
GL ES path (5/4/2/2 checks for MRT/sprite/toon/displacement) and four declared SOFTWARE skips.
The public-namespace compatibility update now builds and reproduces exactly those focused
results; no new semantic runtime credit comes from changing an include. ShaderDialectEXT's legacy payload dialect and ShaderLanguageEXT's
explicit code/package language are distinct APIs.

| Public members / behavior | Decision and evidence |
|---|---|
| String constructor; GraphicsDevice::GetShaderDialectEXT and ExecutesShaderEffectSourceEXT | Already meaningfully used by all four scenes: choose a GLSL header and refuse a renderer that accepts but does not execute source. Use the current public CNA::ShaderDialectEXT rather than a renderer-internal type. Existing OPENGLES3 evidence exercises GlslEs, not the desktop GLSL branch or HLSL/MSL/WGSL/SPIR-V. Unknown explicitly means undeclared dialect, not absence of all shader support. No enum-per-demo expansion. |
| ShaderCodeEXT / ShaderPackageEXT constructors; GetSelectedShaderLanguageEXT | portable_package now passes eight checks on OPENGLES3 and declares a SOFTWARE CustomEffects skip. Owned labeled ES/desktop pairs survive author-buffer changes; reversed declaration order selects the same live language, incomplete pairs cannot mix languages, duplicate stage candidates are ambiguous, and refused effect construction yields two owned labeled stage diagnostics inspected after exception destruction. The valid package-created effect retains its selected language and draws the checked RGB-to-GBR channel cycle. The final GL image is RGBA-identical to the initial capture actually inspected. No desktop-runtime, binary-language preference, arbitrary binding kind or bad-syntax compiler-log coverage is claimed. |
| IsEffectValid, GetCompileErrorEXT, GetShaderDiagnosticsEXT | compiler_diagnostics now passes seven OPENGLES3 checks: a valid retained shader has empty diagnostics; a selected explicit pair with a missing fragment semicolon returns an invalid effect, raw compiler text and owned structured errors. The diagnostic survives rejected-effect destruction, while the retained program stays valid and draws checked cyan pixels. Its image was actually inspected. Mesa identifies replacement.frag, Fragment, line5/column52; portable checks do not require vendor wording or optional locations/stages. Deliberate compiler stderr is retained. SOFTWARE is a declared CustomEffects skip. This distinguishes compiler failure from portable_package selection refusal; no hot-reload service, every compiler stage or link-only failure claim. |
| SetUniformFloat, SetUniformInt, SetUniformVec3; remaining scalar/vector/matrix setters and array families | Existing sprite/displacement/toon/MRT scenes exercise scalar floats, sampler integers and vec3 values. uniform_palettes now passes four GL checks: two explicit M11..M44 matrix groups and two vec3 entries use counts 2 (32/6 underlying scalars), draw correctly after caller buffers are overwritten, and update on the same program in a second viewport. Both transforms and colors must change to produce the checked magenta/cyan order; center gaps remain black. Its GL image was inspected, stderr empty, SOFTWARE capability skip. No contiguous C++ Matrix-array layout is assumed. sample_kernel now passes five GL checks: one bright texel uses three scalar weights and three vec2 offsets (six explicit floats), with SetUniformVec2 supplying the non-square image texel size. The same program updates from an identity kernel to horizontal1:2:1 then vertical1:6:1, and caller arrays are overwritten before each draw. The actual image was inspected; all27center/neighbor/off-axis/outer samples are exact, including128/64 and191/32 gray contributions. SOFTWARE declares its CustomEffects boundary; GL stderr is empty. This is a real image filter, not generic uniform-layout or multi-pass blur coverage. uniform_palettes now retains its four original controls and adds two for a shared parent matrix and premultiplied scene tint via scalar SetUniformMat4/Vec4. Explicit16float layout is overwritten before drawing; parent scaling/translation shifts both palette centers, while all four tint channels produce RGB(0,32,128)/(64,0,128) over black. Vacated old centers and a retained gap make the geometry change observable. Focused1004 uploads all four scalar/array values for the first panel before the first Apply; its exact Red/Lime pixels prove the setters select their own program and retain copied values rather than relying on a previously bound shader. Focused1011 follows those valid uploads with zero-count vec3/mat4 updates carrying destructive buffers and a missing scalar name; the same exact pixels prove all three calls leave live state unchanged. Six GL checks pass in ten OPENGLES3 and ten OPENGL33 runs with byte-identical output and empty stderr. SOFTWARE keeps its existing skip. Signed negative C++ pointer counts remain invalid caller lengths outside the public contract; the safe C wrapper already refuses its documented negative/null cases. Other renderer dialects are not implied covered. |
| SetTexture(Texture2D/TextureCube/Texture3D) | Displacement samples a separately bound Texture2D through an explicit sampler unit. texture_cube.sampler_ownership binds a copied cube on unit1 and checks every face after a shared edit/disposal. Four distinct +X corner texels are sampled at exact centers using GLSL's s=-Z/X and t=-Y/X mapping, establishing uploaded orientation. Authored Lime mip1 and Blue mip2 then distinguish `LinearMipPoint` (exact Lime at LOD1.25) from `PointMipLinear` (RGB0,191,64 cross-level blend), and a distinct white cube replaces the first object on the same occupied unit. Authored DXT3 and DXT5 cubes now produce exact alpha-dependent framebuffer values through the same sampler. Nine GL controls pass; SOFTWARE skips CustomEffects. texture3d.sampler_coordinates passes ten GL controls on explicit unit1: the retained slice/W checks plus four exact U/V corner texels and the same authored explicit-LOD filter distinction in a real volume chain. SOFTWARE likewise skips. Together these are representative 2D/cube/volume binding, replacement, format decoding, three-axis coordinate and mip-filter behaviors; arbitrary formats, seams and other renderer dialects remain outside the claim. |
| DeclareUniformBlockEXT | Renderer-specific std140 layout contract. On pinned d23 the public header names the retired IGL/Vulkan backend, every renderer inherits the default no-op except WebGPU, and `linux-full` does not build WebGPU. Keep this intentionally uncredited until a WebGPU/browser run can prove explicit offsets through actual pixels; zero-count clearing and replacement belong to that same workflow. Calling the no-op on a loose-uniform GL renderer would not establish behavior. |
| IEffectMatrices World/View/Projection | Existing 3D draws set these properties and the draw path forwards them to the shader. This establishes their role in those authored scenes; noncommuting transform order, cloned state and alternative uniform naming are not implied by getter presence. Extend an existing scene when these distinctions are useful. |
| Clone and disposal; HasRenderer | clone_state now executes nine checks on OPENGLES3: six pass and three expose ISSUE-055 (identity matrix reset, different viewport pixels, and missing disposed-clone refusal). Both independently compiled programs draw; World mutation is independent, original disposal releases its program, and the clone draws identical surviving pixels. Both frame captures were actually inspected; GL stderr is empty. SOFTWARE is a declared CustomEffects skip. Effect::Clone explicitly promises disposed refusal; matrix retention follows its current-value clone contract and ShaderEffect documents only a program-sharing deviation. No raw uniform/texture-state copy contract or desktop runtime coverage is inferred. |
| Source properties, GetVertexSource/GetFragmentSource, GetTypeName | `compiler_diagnostics` now constructs its valid program from temporary `ShaderCodeEXT` values, overwrites the caller source strings and verifies both concrete properties plus the virtual `Effect` accessors still return the exact original vertex/fragment text. The same retained program then survives the rejected replacement and draws cyan. GetTypeName remains routine reflection identity without a separate check. |
| GetEffectRendererPtr, FillGpuDrawParams, protected OnApply/Dispose and portable fallback constructor | Renderer/subclass integration points. Normal draws already reach the apply/parameter bridge; avoid a game demo that merely exposes internal renderer pointers/structures. Protected fallback construction belongs to a real derived-effect integration, not a synthetic constructor-count example. |

Focused836 closes this table's declaration-level disposition. `sample_kernel` provides actual
float/vec2-array pixels and `uniform_palettes` provides actual vec3/mat4-array pixels, so all four
public array families are sampled; the C workflow owns the empty/null/negative validation cases.
`DeclareUniformBlockEXT` is explicitly a WGSL/WebGPU name-to-offset bridge and a no-op for loose
uniforms. WebGPU is absent from this build, so runtime block pixels remain renderer-specific rather
than being inferred from the C state call or an OpenGL no-op.

All eight ShaderEffect project scenes also run on OPENGL33 under Xvfb. Seven pass and `clone_state`
retains only its three expected ISSUE-055 failures. Statuses, known-issue values and check counts
match the OPENGLES3 Complete744 baseline; the package selects `GlslDesktop`, and the diagnostic and
uniform paths use the desktop compiler/runtime branch. No screenshot or manual visual inspection
was performed. HLSL/MSL/WGSL/SPIR-V execution remains tied to an available renderer, not a reason
to multiply GLSL demos.

Full 461 at a447ad2 passes all 474 enabled CTests and both complete smokes. Every original 459
outcome/child check is retained; GL adds exactly the three clone assertions (38 ->41), SOFTWARE
keeps 37. Stable selected runtime hashes and exact skip/compiler stderr deltas are archived.

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

### Game loop, services and window callbacks

| Members / behavior | Decision and evidence |
|---|---|
| Game.BeginDraw / EndDraw | `game.lifecycle.method_order` records two complete draw cycles through the real GraphicsDeviceManager before checking EndRun and disposal-time UnloadContent. Each cycle requires one or more fixed-step Updates followed by BeginDraw -> Draw -> EndDraw: SOFTWARE normally records one Update per draw, while OPENGLES3 may catch up with an additional Update without violating Game-loop ordering. Both renderers pass the same four controls. |
| Game.GraphicsDevice, Tick and Disposed | `method_order` proves the Game and its manager expose the same device. `run_one_frame` initializes through RunOneFrame three times, then invokes Tick directly; each variable-time call produces one Update and Draw. A subsequent Dispose raises Disposed exactly once. |
| Game.Finalize | CLR finalizer inventory entry. C++ destruction and explicit Dispose are already demonstrated; no artificial finalizer demo. |
| Game.ShowMissingRequirementMessage | Platform/host presentation hook. The current base implementation ignores the exception and returns false, and no runtime path calls it. A derived wrapper asserting that no-op would add no meaningful behavior; leave uncredited until a host implements or invokes it. |
| GameServiceContainer constructor | `game.services.service_container` directly constructs an independent container and proves it starts empty before retaining the established add/find/remove workflow on Game.Services. |
| GameWindow BeginScreenDeviceChange / EndScreenDeviceChange | GraphicsDeviceManager's real creation/reset path brackets changes. The back-buffer workflow now observes ClientSizeChanged after its attached OPENGLES3 window is resized; SOFTWARE has no attached window and retains the offscreen behavior. The screen-device workflow also uses both End overloads on a detached window and proves a changed name raises once while an unchanged name does not repeat. |
| GameWindow OnClientSizeChanged / OnScreenDeviceNameChanged | Demonstrated by the event observations above. The screen-device ID passes three controls on OPENGLES3 and is a declared `Need::Window` skip on SOFTWARE. |
| GameWindow OnOrientationChanged / SetSupportedOrientations | `game.window.orientation` already switches from portrait-only to LandscapeLeft through GraphicsDeviceManager, observes exactly one OrientationChanged event and matching presentation orientation, and now names the protected implementation path. OPENGLES3 passes six controls; SOFTWARE is a declared window skip. |
| GameWindow SetTitle | `game.window.title` writes and reads a changing live caption every frame; the public property routes through the protected virtual SetTitle. Both renderers pass ten controls. |
| GameWindow OnActivated / OnDeactivated / OnPaint | These three protected CNA methods are empty and have no internal caller. Game activation is separately handled and demonstrated by `game.lifecycle.activation`. Calling empty hooks from a probe would only inflate metadata, so they remain intentionally uncredited. |

Focused661 adds13 member credits without a new ID, moving temporary coverage1689->1702/2326.
Five portable IDs have byte-identical renderer output; the two window-only IDs pass on OPENGLES3
and declare SOFTWARE skips. The back-buffer ID retains only ISSUE-083's expected query mismatch.
No screenshot or manual interaction was used.

### Graphics resource, buffer and device-service lifetime

| Members / behavior | Decision and evidence |
|---|---|
| GraphicsResource.GraphicsDevice and ToString | `graphics.graphics_resource.lifetime_events` now proves a Texture2D retains its creating device, an unnamed resource formats as its runtime type and a named resource formats as its debug name before the established Tag/event/idempotent-disposal checks. Both renderers pass10 controls. |
| GraphicsResource.Finalize | CLR finalizer inventory entry. Explicit/idempotent disposal, C++ destruction and device ResourceDestroyed are demonstrated; no artificial finalizer demo. |
| VertexBuffer.BufferUsage and Dispose | The readable-buffer workflow checks its `BufferUsage::None` property, performs complete and ranged readback plus drawing, then explicitly disposes the buffer during UnloadContent. Both renderers pass5 controls. |
| IndexBuffer.BufferUsage and Dispose | The indexed-cube workflow checks its 36-entry 16-bit WriteOnly shape, performs the actual indexed draw and explicitly disposes during UnloadContent. Both renderers pass2 controls. |
| DynamicIndexBuffer.ContentLost | The dynamic board attaches the public event, uses the CNA test seam to deliver one content-loss notification, checks event plus lost state, then proves its next SetData clears the flag before actual indexed drawing. This validates recovery semantics without claiming either exercised renderer really lost a device. Both renderers pass3 controls. |
| RenderTarget2D.ContentLost and IsContentLost | The render-to-texture workflow similarly observes one explicit notification and lost flag, then proves binding for replacement rendering clears the flag before the target is drawn and sampled. Both renderers pass3 controls. No physical/context device loss is claimed. |
| RenderTarget2D bound identity, Dispose, destruction and C++ moves | The workflow now executes seventeen controls. A 32x16 target bound with a depth buffer refuses public Dispose and move assignment atomically: the wrapper stays live and bound, both move operands retain their shape/properties, and an authored Gold pixel survives. Moving the active 128x128 source first constructs and then assigns new wrappers while `GetRenderTargets` follows each new identity; a subsequent Orange clear through the transferred binding reads back exactly after unbinding. A separate optional-backed target is destroyed while active: its dangling public binding disappears while Present remains refused until explicit unbind restores the complete back-buffer viewport. Reconstructing at the same address produces a real new binding and exact CornflowerBlue read-back. Focused1010 assigns an occupied primary-device wrapper from an actively bound secondary-device source: ownership and complete shape move, the old primary texture slot detaches, the secondary public target binding follows the new identity, and explicit payload survives and remains writable after unbinding. SOFTWARE passes17/17; OPENGLES3 passes those sixteen controls but exposes ISSUE-112 before the move because secondary `SetRenderTarget` misses its renderer-context lease. Copying is unavailable by public type design. No successful secondary EasyGL render is claimed. |
| RenderTargetCube bound Dispose and destruction | The dynamic environment workflow now passes thirteen controls on both renderers. It binds the NegativeZ face, clears it Gold and verifies public Dispose is refused while the target stays live and `GetRenderTargets` retains the exact cube/face pair. After unbinding, one exact Gold texel proves the refused call did not release or replace the face storage. A separate optional-backed cube is then destroyed while PositiveY is active: its dangling public binding disappears while Present remains refused until an explicit unbind restores the complete back-buffer viewport. Reconstructing at the same address produces a genuine NegativeX binding with the expected square viewport and exact CornflowerBlue face readback. RenderTargetCube exposes no target-level copy/move operations; its base TextureCube wrapper behavior is covered separately. |
| IGraphicsDeviceService lifecycle events | `game.graphics_device_manager.device_events` now subscribes through the interface itself before device creation. It observes created, resetting, reset and disposing around actual manager creation, ApplyChanges and Dispose, retaining its six passing lifecycle controls on both renderers. |

Focused662 adds12 member credits without a new ID, moving temporary coverage1702->1714/2326.
All six IDs pass on SOFTWARE and OPENGLES3 with byte-identical output and empty stderr; all six
focused CTests pass. GraphicsResource retains only CLR Finalize by disposition, and the other five
target types have no generated member gap.

### VertexBufferBinding values and draw consumption

| Members / behavior | Decision and evidence |
|---|---|
| VertexBuffer / VertexOffset | `several_streams` prefixes both position and color buffers with a decoy element, binds each at offset1 and reads back both exact identities/offsets. The unchanged exact white centre and clear exterior prove the draw consumes the shifted streams together. |
| InstanceFrequency | `draw_instanced_primitives` reads back frequency0 for the mesh and frequency1 for the matrix stream before one call spreads100 individually transformed cubes across both screen halves. This ties the value to actual instanced stepping. |

Focused680 keeps `several_streams` at3 controls and grows instancing1->2, crediting all three members
and moving temporary coverage1831->1834/2326. Both IDs pass on SOFTWARE and OPENGLES3 with empty
stderr; both focused SOFTWARE CTests pass. Streams output is byte-identical; the time-varying
instancing diagnostic differs by one covered pixel while both controlled outcomes pass. Evidence:
`/tmp/checkpoint-680-*` and `/tmp/checkpoint-680-coverage.md`. Pixel validation used programmatic
readback; no screenshot or visual inspection was used.

### Presentation and render-target value objects

| Members / behavior | Decision and evidence |
|---|---|
| PresentationParameters constructor and DeviceWindowHandle | `graphics.device.presentation_parameters` directly checks the 800x480 Color/no-handle defaults, then proves the live device parameters carry the presenting GameWindow's handle alongside the established size/format/bounds/clone workflow. Both renderers pass7 controls. |
| RenderTargetBinding.RenderTarget | The MRT workflow constructs named 2D bindings, submits them, and proves both the caller values and `GetRenderTargets` preserve the exact two target identities. OPENGLES3 passes5 controls; SOFTWARE is the established custom-effects capability skip. |
| RenderTargetBinding.CubeMapFace | The cube-target workflow constructs a NegativeZ binding and proves it retains both the exact RenderTargetCube and face before the established six-face rendering/reflection path. |
| RenderTargetCube depth/MSAA/usage values | The live128x128 cube reports its applied Depth24, single-sample count0 and PreserveContents mode before it renders all six faces. Existing dedicated targets demonstrate nonzero multisampling; this workflow relates the cube-specific getters to the target actually sampled. |
| RenderTargetCube ContentLost / IsContentLost | One explicit CNA notification raises the public event and lost flag; binding the first face clears it before drawing. This is a deterministic recovery-boundary test, not a claim that either desktop renderer physically lost its device. |
| RenderTargetCube PreserveContents behavior | On frame2 the +Z face is rebound without clear or draw. The reflective sphere still reads exact green centre RGBA(30,110,40,255), so preservation affects real cube sampling rather than getter state alone. |
| DisplayMode.TitleSafeArea and ToString | `graphics.graphics_adapter.display_modes` adds an authored 1920x1080 Bgr565 value. Its safe area is the full rectangle and its exact text includes dimensions, named format and derived aspect ratio in XNA order. Both renderers pass7 controls with identical output. |
| Viewport.Y | The properties workflow now assigns a nonzero Y together with X/width/height through Bounds and checks all four components before the existing safe-area/default behavior. Both renderers pass7 controls with identical output. |

Focused663 adds7 member credits without a new ID, moving temporary coverage1714->1721/2326.
PresentationParameters, RenderTargetBinding, DisplayMode and Viewport have no generated member gap.
All five focused SOFTWARE CTests pass (MRT skipped by declared capability). No screenshot or manual
visual inspection was used.

Focused679 grows the cube ID from3 to7 controls and credits its five remaining members, moving
temporary coverage1826->1831/2326. SOFTWARE and OPENGLES3 pass7/7 with empty stderr; their two
new controlled pixel lines match. The retained orbit diagnostic differs at431/0 versus445/12.
The focused SOFTWARE CTest passes. A first 4x-MSAA/30-frame draft was stopped after more than40
seconds on SOFTWARE; the final coherent cube remains single-sample and automation runs the three
frames required by its preservation check. Evidence:`/tmp/checkpoint-679-*` and
`/tmp/checkpoint-679-coverage.md`. Pixel readback was programmatic; no screenshot or visual
inspection was used.

### GraphicsAdapter native identities and device-selection flags

| Members / behavior | Decision and evidence |
|---|---|
| VendorId / DeviceId | The pair must either contain bounded16-bit PCI values or both use the documented unavailable zero. This environment reports1002:15BF under both selections; no particular vendor is required. |
| Revision / SubSystemId | CNA documents these unavailable platform values as zero; both are checked exactly. |
| MonitorHandle | Treated as opaque. DefaultAdapter matches Adapters[0], nonzero identities are unique, and zero remains valid for a displayless fallback. SOFTWARE observes0 while Xvfb OPENGLES3 observes1. |
| UseNullDevice / UseReferenceDevice | A bracketed sequence reads/writes both process-wide flags, proves cleared flags mean Hardware/current renderer, Reference maps to SOFTWARE, and Null wins over a simultaneously retained reference flag and maps to HEADLESS. Original state is restored. No alternate device construction is claimed. |

Focused681 grows `display_modes` from7 to14 controls, credits all seven members and moves temporary
coverage1834->1841/2326. SOFTWARE and OPENGLES3 pass14/14 with empty stderr; the focused SOFTWARE
CTest passes. Display/mode output differs by the real fallback versus Xvfb environments as expected.
Evidence:`/tmp/checkpoint-681-*` and `/tmp/checkpoint-681-coverage.md`. No drawing, screenshot or
visual inspection was used.

### Two-component packed-vector value contracts

HalfVector2, NormalizedByte2, NormalizedShort2, Rg32 and Short2 stay in their existing coherent
precision-family demos. Each now copies exact PackedValue state, checks == and != together with
typed and boxed Equals, rejects a boxed sibling packed type, requires equal hashes and validates
its XNA text form. A second value is addressed through IPackedVector, receives a Vector4 and returns
the type's XY with canonical Z=0/W=1. This covers value semantics as one contract per type rather
than creating method-count variants.

Focused682 credits all40 remaining members across four IDs. Half precision and signed normalized
pass8/8; wide UNorm and integer formats pass6/6. SOFTWARE and OPENGLES3 output is byte-identical
with empty stderr; all four focused SOFTWARE CTests pass. Temporary coverage moves1841->1881/2326.
Evidence:`/tmp/checkpoint-682-*` and `/tmp/checkpoint-682-coverage.md`. These are CPU value tests;
no renderer, vertex fetch, screenshot or visual result is claimed.

### Four-component packed-vector value contracts

HalfVector4, NormalizedShort4, Rgba1010102 and Short4 remain in the same precision-family demos.
Each now copies exact PackedValue state, checks == and != together with typed and boxed Equals,
rejects a boxed different type, requires equal hashes and validates the expanded or packed-hex XNA
text form. A second value is addressed through IPackedVector and preserves all four components.
This completes each type as a coherent value contract while retaining its precision and clamping
context.

Focused683 credits all28 remaining members across four IDs. Half precision and signed normalized
pass10/10; wide UNorm and integer formats pass8/8. SOFTWARE and OPENGLES3 output is byte-identical
with empty stderr; all four focused SOFTWARE CTests pass. Temporary coverage moves1881->1909/2326.
Evidence:`/tmp/checkpoint-683-*` and `/tmp/checkpoint-683-coverage.md`. These are CPU value tests; no
renderer, vertex fetch, screenshot or visual result is claimed.

### Complete packed-vector value contracts

Alpha8, Bgr565, Bgra4444, Bgra5551, Byte4, HalfSingle, NormalizedByte4 and Rgba64 remain in five
existing precision-family demos. Each now copies exact PackedValue state, checks == and != together
with typed and boxed Equals, rejects a boxed different type, requires equal hashes, validates exact
XNA text and packs through IPackedVector. The scalar and RGB types also prove their interface-only
Vector4 expansion: Alpha8 supplies zero RGB, Bgr565 supplies opaque alpha and HalfSingle supplies
canonical Y=Z=0/W=1. The existing interface workflow uses CNA's IPackedVectorT<uint16_t> to prove
typed packed storage; this is the C++ representation of the CLR generic IPackedVector<T>.PackedValue
inventory item.

Focused684 credits all52 remaining namespace members across six IDs. Compact color and half
precision pass12/12; integer formats and wide UNorm pass10/10; signed normalized passes12/12; the
interface workflow passes5/5. SOFTWARE and OPENGLES3 output is byte-identical with empty stderr;
all six focused SOFTWARE CTests pass. Temporary coverage moves1909->1961/2326 and PackedVector
reaches164/164 members. Evidence:`/tmp/checkpoint-684-*` and `/tmp/checkpoint-684-coverage.md`.
These are CPU value tests; no renderer, vertex fetch, screenshot or visual result is claimed.

### Complete PacketReader typed payloads

`net.packet.send_and_receive` now places a double, Vector2, Vector4, Quaternion and Matrix after its
established position, scalar, string and Color fields. A real local System Link hop carries the
141-byte packet between two local signed-in gamers. The receiver reads every field in order,
requires exact equality, verifies the sender and consumes the full packet. This closes the five
remaining PacketReader typed-read members while retaining the real PacketWriter, SendData and
ReceiveData path.

Focused685 passes6/6 on SOFTWARE and OPENGLES3 with byte-identical stdout and empty stderr. The
focused SOFTWARE CTest passes outside the socket-restricted sandbox; its first sandboxed attempt
failed at ENet host creation before demo behavior. Temporary coverage moves1961->1966/2326.
Evidence:`/tmp/checkpoint-685-*` and `/tmp/checkpoint-685-coverage.md`. The current four-byte Color
round trip also contradicts stale CNA header/example text; CNA-DEMOS-ISSUE-084 records that
documentation defect. No screenshot or visual inspection was used.

### Complete Design converter behavior

The three existing text, property-grid and constructor-descriptor workflows now cover every Design
converter constructor and direction. Point, Vector2, Vector3, Vector4, Quaternion and Color parse
and emit invariant text; non-parsing Rectangle, BoundingBox, Plane, Ray and Matrix emit exact
structural text. InstanceDescriptor reconstructs BoundingSphere directly and lets BoundingBox/Ray
ConvertFrom consume their own constructor descriptions. Named property maps call CreateInstance for
every supported scalar, vector, geometry and16-element Matrix shape.

Focused686 credits all32 remaining non-field members. String round trips pass9/9, properties
pass10/10 and instance descriptors pass5/5. SOFTWARE and OPENGLES3 output is byte-identical with
empty stderr; all three focused SOFTWARE CTests pass. Temporary coverage moves1966->1998/2326 and
Design reaches51/51 non-field members. Evidence:`/tmp/checkpoint-686-*` and
`/tmp/checkpoint-686-coverage.md`. MathTypeConverter's two documented protected fields remain
intentionally uncredited: the workflows prove their externally visible parsing and ordered-property
effects, while direct protected-storage exposure would add no independent user behavior.

### ContentSerializerAttribute state and cloning

The existing separate-process intermediate serializer now checks every XNA default on a public
ContentSerializerAttribute, configures AllowNull, CollectionItemName, ElementName, FlattenContent,
Optional and SharedResource together, clones the complete state and resets the original without
changing its clone. The following real XML workflow still consumes a second format object to rename
Health, alongside descriptor-level collection, optional, null and ignored behavior.

Focused687 credits all seven remaining members. The child process passes14/14 controls on SOFTWARE
and OPENGLES3 selections with byte-identical stdout and empty stderr; the focused SOFTWARE CTest
passes. Temporary coverage moves1998->2005/2326. Evidence:`/tmp/checkpoint-687-*` and
`/tmp/checkpoint-687-coverage.md`. This is build-time XML and value-state coverage; no rendering or
runtime content-manager behavior is claimed.

### Remaining ordinary Content members

The custom ContentManager now requires exact IServiceProvider identity and exercises its explicit
ContentLoadException constructor on a missing packed asset. The ResourceContentManager workflow
names its protected OpenStream because both the embedded Bounce attempt and missing-resource path
actually dispatch through that override. These extend existing load behavior without adding wrapper
or exception-only demos.

Focused688 credits the final three ordinary members. Custom OpenStream runs four controls with three
passing and only CNA-DEMOS-ISSUE-012's established Load bypass failing; resource content runs two
controls with only the same issue's reader-registration failure. SOFTWARE and OPENGLES3 stdout is
byte-identical with empty stderr; both known-issue SOFTWARE CTests pass their expected-failure
contract. Temporary coverage moves2005->2008/2326 and Content reaches41/48. Evidence:
`/tmp/checkpoint-688-*` and `/tmp/checkpoint-688-coverage.md`. The remaining seven Content entries
are the four nominal attribute wrappers dispositioned above.

### Mouse window targeting

The existing native-window workflow now requires Mouse.WindowHandle to equal the live
GameWindow.Handle, clears it to zero and restores the exact real token before continuing. This
tests both property directions and leaves the static input target usable for later game frames.

Focused689 credits Input's final member. SOFTWARE is correctly skipped because it exposes no
window; OPENGLES3/X11 passes6/6 with empty stderr. The focused SOFTWARE CTest passes its skip
contract. Temporary coverage moves2008->2009/2326 and Input reaches113/113 members. Evidence:
`/tmp/checkpoint-689-*` and `/tmp/checkpoint-689-coverage.md`. No cursor motion, synthetic mouse
event, screenshot or visual result is inferred from the handle identity checks.

### Graphics-state lifetime and complete sampler presets

| Members / behavior | Decision and evidence |
|---|---|
| BlendState, DepthStencilState, RasterizerState and SamplerState.Dispose | `graphics.blend_state.bound_state_is_immutable` now covers all four state-resource types. Each live binding becomes immutable; four independent states then report disposed immediately and every attempted device/collection binding throws `ObjectDisposedException` before replacing live state. Both renderers pass10 controls. |
| BlendState.AlphaBlendFunction | The five custom blend states now configure the color and alpha equations separately. Their actual pixels include reverse-subtract alpha clamping to zero, while Min/Max use the XNA-required One/One alpha factors. Both renderers pass all six exact controls. |
| SamplerState.AnisotropicClamp | The anisotropic-floor workflow checks that this stock state combines anisotropic filtering, MaxAnisotropy4 and Clamp on U/V/W before retaining its real LinearWrap/AnisotropicWrap draw. Both renderers pass. |

Focused664 adds5 XNA-member credits plus one named stock-state field without a new ID, moving
temporary coverage1721->1726/2326 and fields153->154/753. SamplerState now has no generated member
gap; disposal is closed for all four state classes. All three focused SOFTWARE CTests pass, and
SOFTWARE/OPENGLES3 output is byte-identical with empty stderr. The initial Min/Max factor rejection
and reverse-subtract alpha mismatch were demo expectation errors corrected from the public profile
contract and actual matching renderer output; no CNA issue was filed. No screenshot or visual
inspection was used.

### Stencil masks and failure outcomes

| Members / behavior | Decision and evidence |
|---|---|
| StencilMask and StencilWriteMask | `graphics.depth_stencil_state.stencil_mask` clears stencil to binary10, then writes reference bit0 through a write mask of1. An exact comparison proves the protected high bit remains and the stored value is3; after a color-only clear, a read mask of1 lets stored3 match reference1 and reveal the checker only inside the plus shape. Both renderers pass4 pixel controls. |
| StencilFail and StencilDepthBufferFail | The same plus uses two real failure paths: the vertical arm replaces stencil after a `Never` stencil comparison, while the horizontal arm passes stencil and replaces it after a `Never` depth comparison. The final image proves their union. |
| CounterClockwiseStencilFail and CounterClockwiseStencilDepthBufferFail | New focused ID `graphics.depth_stencil_state.stencil_outcomes` draws explicit clockwise and counter-clockwise triangles. For each winding, one state selects stencil-test failure and one selects depth-test failure; four distinct replacement references are independently revealed as four exact colors, with a clear outside control. Both renderers pass5 controls. |

Focused665 adds one coherent ID and6 member credits, moving the staged catalog to629 demos and
temporary coverage1726->1732/2326 at155/753 fields. DepthStencilState now has no generated member
gap. Both IDs have byte-identical SOFTWARE/OPENGLES3 output and empty stderr, and both focused
SOFTWARE CTests pass. Pixel validation used programmatic readback; no screenshot or visual
inspection was used.

### Sample coverage and slope-scaled depth bias

| Members / behavior | Decision and evidence |
|---|---|
| BlendState.MultiSampleMask | `graphics.render_target.multisampling` now renders the same triangle into a fourth 4x target with mask0. Both renderers resolve that target to one clear level while the all-samples target resolves to five levels, proving the mask rejects every covered sample. |
| RasterizerState.MultiSampleAntiAlias | The same demo renders another 4x target with this property false. SOFTWARE changes from five resolved levels to the two all-or-nothing levels of the single-sample control. OpenGL ES retains five because its profile cannot disable multisampling through rasterizer state; the demo records that renderer-specific limitation rather than claiming a toggle. |
| RasterizerState.SlopeScaleDepthBias | The decal workflow grows2->3 pixel controls. Its third panel places a white sloped decal slightly behind a green sloped floor; a negative slope-scaled state makes the exact center white while the unbiased flat control remains hidden and the fixed-bias control remains visible. Both renderers pass. |

Focused666 adds3 member credits without a new ID, moving temporary coverage1732->1735/2326.
RasterizerState now has no generated member gap and BlendState retains only the three per-MRT color
write masks. Both focused SOFTWARE CTests pass. Stderr is empty; the depth-bias output is identical,
while the multisampling diagnostic intentionally reports SOFTWARE2 versus OPENGLES35 grey levels
for the disabled-rasterizer case. Pixel validation used programmatic readback; no screenshot or
visual inspection was used.

### Indexed MRT color-write masks

| Members / behavior | Decision and evidence |
|---|---|
| BlendState.ColorWriteChannels1/2/3 | `graphics.render_target.multiple_render_targets` now performs a four-output probe before its established two-target G-buffer draw. All targets start at RGBA(10,20,30,40). Slot1 writes only the normal output's red channel, slot2 only the constant material output's green channel and slot3 only the constant emissive output's blue channel; one-pixel texture readback proves every enabled channel changes and every disabled channel preserves the clear value. OPENGLES3 passes8 controls, including the original albedo/normal pixels and binding identities. SOFTWARE retains its declared custom-effects skip. |

Focused667 adds3 member credits without a new ID, moving temporary coverage1735->1738/2326.
BlendState, DepthStencilState, RasterizerState and SamplerState now have no generated member gap.
The focused SOFTWARE CTest passes as a declared skip; OPENGLES3 stderr is empty. Pixel validation
used programmatic texture/backbuffer readback; no screenshot or visual inspection was used.

### GraphicsDevice live display and standalone state

| Members / behavior | Decision and evidence |
|---|---|
| DisplayMode, GraphicsDeviceStatus | The presentation workflow relates the windowed device's exact mode to its adapter's current mode and requires Normal status on both active desktop renderer paths. |
| MultiSampleMask | The MSAA workflow keeps its BlendState mask case and adds a separate device-property path. Mask zero produces one clear level on both renderers, then restoring -1 is observed through the public getter. |
| ReferenceStencil | The stencil workflow deliberately binds reference 1, overrides it to 3 through GraphicsDevice after immediate-mode state binding, then reveals only the stored stencil value 3 with exact pixels. |

Focused672 credits four GraphicsDevice members without a new ID, moving temporary coverage1779->
1783/2326. Presentation passes9/9 and stencil4/4 with byte-identical renderer output. MSAA passes
6/6 on both; its device-mask result is one level on both while its established rasterizer-off
diagnostic remains2 on SOFTWARE and5 on OPENGLES3. All stderr is empty and all three focused
SOFTWARE CTests pass. Evidence:`/tmp/checkpoint-672-*` and `/tmp/checkpoint-672-coverage.md`.
Pixels were inspected through programmatic readback; no screenshot or visual inspection was used.

### GraphicsDevice vertex-stage collections

| Members / behavior | Decision and evidence |
|---|---|
| VertexTextures | A real Single-format Texture2D binds by exact identity in vertex slot1 while the same pixel slot remains null. Replacing it with Color throws NotSupportedException and preserves the prior binding; disposing the bound texture clears the slot. |
| VertexSamplerStates | PointClamp in vertex slot1 reports exact filter/address state while the corresponding pixel sampler remains LinearWrap, establishing independent collections. |

Focused673 adds one four-control state ID and credits both members, moving temporary coverage1783->
1785/2326 at631 demos. SOFTWARE and OPENGLES3 pass with byte-identical stdout and empty stderr;
the focused SOFTWARE CTest passes. The workflow validates binding/format/lifetime state and does
not claim compiled vertex-shader sampling. Evidence:`/tmp/checkpoint-673-*` and
`/tmp/checkpoint-673-coverage.md`. No drawing, screenshot or visual inspection was used.

### GraphicsDevice construction, reset, present and disposal

| Members / behavior | Decision and evidence |
|---|---|
| Constructor, Present | `owned_lifecycle` constructs a separate HiDef device with requested 256x144 presentation state, clears and presents it, and observes Normal status afterward. It presents again after reset. |
| Reset | A 320x180 reset raises DeviceResetting then DeviceReset, updates presentation parameters and viewport, and retains a live owned texture. |
| Dispose, Disposing, IsDisposed | Explicit disposal marks the device before raising one event, disposes its texture once, rejects later Present/Reset and is idempotent. |
| DeviceLost | CNA documents this event as never raised on desktop. Reset and disposal keep its count zero on SOFTWARE and OPENGLES3; that absence is recorded without claiming an actual loss event. A supported real loss path remains platform/renderer-specific. |
| Finalize | CLR finalizer machinery represented by the C++ destructor; no independent demo or member credit. Explicit disposal is the deterministic public workflow. |

Focused674 adds one seven-control lifecycle ID and credits six members, moving temporary coverage
1785->1791/2326 at632 demos. SOFTWARE and OPENGLES3 pass with byte-identical stdout and empty
stderr; the focused SOFTWARE CTest passes. Every ordinary GraphicsDevice member now has evidence;
only the dispositioned DeviceLost and Finalize inventory entries remain. Evidence:
`/tmp/checkpoint-674-*` and `/tmp/checkpoint-674-coverage.md`. Clear/present executed, but no pixel,
screenshot or visual inspection was performed.

### ContentManager (including CNA extensions)

| Members / behavior | Decision and evidence |
|---|---|
| Constructors, RootDirectory | `content.content_manager.several_managers` shows separate roots and caches, and `missing_assets` shows slash normalization. The CNA default constructor and nullable service provider support console data loading; they do not need independent demos. `cache_and_unload` now tries to change the root while assets are loaded (CNA wrongly accepts it, ISSUE-039), then switches successfully after Unload and loads a new relative name. All 5 checks run on both renderer selections with just that one expected failure; screenshots inspected. |
| ServiceProvider | Several managers receive the game's services; `game.services.graphics_device_service` explains device service lookup. The getter is ordinary identity plumbing, intentionally without an independent demo; it is not currently named as a demonstrated member. |
| Load (cache, type and name resolution) | `cache_and_unload` counts created textures, `custom.cnb_loader` counts factories, and `several_managers` distinguishes per-manager caches. `missing_assets` checks failures and backslash names. asset_precedence now loads competing original XNB/CNB/CNJ/native files under one logical name with distinct 400/300/200/100 values: compiled priority, cache retention after file removal, fresh-manager selection, Unload, explicit native filenames and repair/retry are verified (13 passes, one public exception-type failure ISSUE-045, both displayless renderer selections). Invalid compiled files do not silently fall through to valid lower tiers. A literal native filename is tested without suffixed compiled neighbors; packaged assets and every alias/case rule remain unclaimed. cnj_loader now loads the same name and CNJ type string into Enemy and Nameplate with independent factories/cache counters; an unregistered requested C++ type is refused and Unload clears both entries (11 checks total). |
| Unload, Dispose() and post-disposal calls | `cache_and_unload` shows rereading after Unload; `unload_disposes` explicitly fails on undisposed textures (ISSUE-011). `read_asset_ownership` now dispatches Dispose through `IDisposable` into a derived protected `Dispose(true)` hook, then repeats public Dispose: the override observes both calls while base cleanup remains idempotent and caller-owned resources survive. Its disposed ReadAsset refusal has the exact ObjectDisposedException and no stream/callback side effect. `several_managers` adds the complementary Load check; both renderers reproduce ISSUE-010 because Load throws std::runtime_error instead of ObjectDisposedException before any asset read. |
| OpenStream, ReadAsset | `open_stream_override` serves an in-memory XNB via ReadAsset, then demonstrates that Load bypasses the override (ISSUE-012); resource_content_manager exposes the embedded-resource equivalent. `read_asset_ownership` now has eighteen controls, seventeen passing plus ISSUE-011's default-recorder failure: the actual nested typed shared_ptr<IDisposable> reaches a supplied callback, while its null sibling/value root do not; repeated reads produce independent resources and each short-read source stream is destroyed. The caller retains resources across manager Unload/Dispose, explicitly disposes their buffers and finally releases them, verified by weak handles. Empty/disposed reads are rejected before opening a stream or calling the recorder. This demonstrates uncached repeat reads, not interaction with an already-populated Load cache. |
| Disposable recorder boundaries | `read_asset_ownership` passes twenty of twenty-one controls; only the established ISSUE-011 default-recorder assertion fails under both selections. Its generated level has two typed fields that resolve to one disposable identity: the supplied callback observes both read occurrences while the caller deduplicates them into one ownership slot. A later truncated field leaves that already delegated resource usable for explicit cleanup, and a deliberate recorder exception propagates while the short-read source stream and unclaimed object unwind. `content.xnb.shared_resources` adds the complementary supplied-recorder case: its typed inline Item is recorded, but both disposable shared-table Items are omitted (ISSUE-087). Raw-object reads intentionally bypass recording in both the inspected contract and implementation. The public ReadAsset recorder boundary is now dispositioned. |
| RegisterTypeReader, LooseFileContentTypeReader::Read / GetExtensions | `custom.loose_file_reader` teaches a tile-map format and checks caching. Ordinary adapter methods need no independent demos. |
| RegisterCnbLoaderEXT | `custom.cnb_loader` now passes twenty-five checks in an isolated process. It registers two observably different factories for one custom ID/name: the registry retains the first factory's owned capture, immediately releases the ignored replacement capture, decodes through the first factory and keeps it live across destruction/recreation of managers while their caches remain independent. Eight gated threads then register one new custom ID/name with distinct owned captures; every public call succeeds, exactly one capture remains after join, and a real CNB decodes through that winner exactly once. Empty factories, CNA built-in/reserved identifiers and a custom ID paired with the wrong canonical name are refused without disturbing the first registration. The existing game-defined schema, optional/mandatory chunks, repair/retry, unsupported version, real hash collision and missing/duplicate/truncated/trailing RULE refusals remain intact. Malformed container tables are separate. |
| RegisterCnjLoader, CnjLoaderFn | `content.custom.cnj_loader` uses two named factories for the same Enemy type, parses authored fields with nlohmann-json (an existing README/CI prerequisite, now explicitly linked for content), rejects duplicate registration and a missing payload field, and shows manager-local registration surviving Unload. All 18 checks pass on both renderer selections, including the same authored file supplying a separately cached UI Nameplate type. Private unknown-type and future-version documents are refused before factory invocation; repairing each exact path and retrying it through the same manager proves failures are not cached. Empty type names/factories are rejected before claiming the type, and a built-in Texture2D reader cannot be shadowed by a custom registration. Later valid Enemy registration/use proves the guard failures did not poison manager state. No CNA Internal parser is used; the public registration contract is semantically closed. |
| GetContentManifest, GetXnbReaderUsageSummary | Existing `content_manager.manifest` enumerates logical asset names and format flags, independently aggregates per-file XNB reader references, verifies loading leaves the inventory unchanged, and checks the loaded SpriteFont reader family is available. The 456 CNA contract registered built-ins on Load<T>(); the later in-flight registry refactor also materializes declared built-ins when queried. The demo checks current post-load availability without requiring a particular initial registry state or proving a lazy transition. All 13 checks pass on SOFTWARE/OPENGLES3 with empty stderr. The old blanket built-in availability assertion failed in isolated CTest and was corrected without changing CNA or adding a known-issue marker. Pipeline bookkeeping files remain visible (ISSUE-013). |
| RefreshContentManifest | `manifest` now adds/removes a private CNJ file between scans, loads the new file before refreshing, and confirms refresh does not discard the loaded asset. All seven new snapshot/cache checks pass on SOFTWARE and OPENGLES3. |
| setGraphicsDevice, getGraphicsDeviceInternal | CNA integration helpers; normal game demos use IGraphicsDeviceService through the supplied provider. No separate demonstration unless a standalone graphics-content scenario needs explicit device injection. |

### CNA binary content: writer, document and primitive cursors

This initial CNA-specific review covers `CnbWriter`, `CnbDocument`, `CnbByteWriter`,
`CnbByteReader` and their format/limit helpers. These extensions are outside the XNA member
percentage; their presence in the unmatched-name list is not evidence of an API spelling error.

| Members / behavior | Decision and evidence |
|---|---|
| CnbWriter constructor, SetMetadata, AddChunk, WriteToFile; CnbAssetTypeIdFromName, MakeChunkId | `content.custom.cnb_loader` writes a game-owned rules schema and loads it through the typed registration. It now refuses missing canonical metadata and a name that hashes to another identifier at Build. A fixed pair of distinct canonical names both hash to `0xB412AA0B`; the writer can validly publish either, while ContentManager compares the full file name with the registered one, refuses Beta before invoking Alpha's factory, then loads Alpha after same-path repair. This demonstrates why the name is part of identity rather than merely displaying the hash. |
| Writer Build, SchemaChunkCount, deterministic output | WriteToFile uses Build internally, but the demo does not directly inspect an in-memory image or compare two generated images. `pipeline_components` now compares two full builds' resulting CNB bytes; it does not claim every compression/platform setting is deterministic. SchemaChunkCount is ordinary inspection and needs no independent demo. |
| CnbDocument::RequireAsset | The custom loader checks the correct type/schema on every successful decode, directly refuses a valid document requested under another custom type ID, and rejects a future schema version. Canonical metadata is enforced separately by CnbWriter and ContentManager dispatch because RequireAsset intentionally checks the numeric asset identity and schema range only. |
| RequireMandatoryChunksUnderstood | `custom.cnb_loader` now loads the same schema with an unknown optional chunk and verifies intact data, refuses the same extension marked mandatory before decoding, then repairs that filename and successfully retries through the same manager without Unload. All 8 checks pass on SOFTWARE and OPENGLES3; failed loads do not poison this cache path. |
| RequireSingle, OpenChunk; primitive WriteString/WriteF32/WriteU32/Take and ReadString/ReadF32/ReadU32/RequireExhausted | The rules fields round trip through a bounded chunk cursor. The existing cnb_loader now passes 12 checks on both renderer selections, including missing/duplicate RULE chunks, a truncated lives integer and trailing schema bytes. Each file first passes ParseFile and its RULE count is inspected; the typed loader then refuses it with file/chunk context before returning a Rules value. These are valid containers with invalid game schemas, not malformed container tables. cnb_read_limits now separately verifies schema ReadString limits after successful container parsing. |
| Other numeric primitive reads/writes, WriteBytes/WriteZeros, ReadBytes/Skip, cursor Size/Position/Remaining/Context, writer Size/View | Ordinary encoding/cursor mechanics; no independent example for every width or accessor. Add them only when a real asset schema needs those representations. They remain uncredited until actually used. The View/ReadBytes spans borrow storage and must not outlive their owner. |
| ReadCount, Fail, IsWellFormedUtf8 | Schema validation helpers. The patrol loader uses ReadCount(8, "waypoints") before allocating/reading coordinate pairs. The tile-layer limits example rejects 64 tiles under a 16-element budget before allocation. Physical-fit and arithmetic-overflow cases are parser-hardening unit tests and do not add a distinct game-facing concept; a stand-alone UTF-8 predicate would likewise duplicate parser tests. |
| Parse/ParseFile, HasMagic, Origin, container versions, asset identity, ChunkCount/ChunkAt/ChunkData, FindAll/FindSingle, Metadata | cnb_read_limits explicitly Parses a complete image, inspects each ChunkAt entry, proves an authored 64-byte alignment in the published entry and physical offset, prints stored/logical sizes and shows that HasMagic remains true after a checksum-invalid payload mutation. cnb_external_references uses ParseFile and inspects the actual XREF table. CNA's container tests exhaustively cover truncated headers/tables/payloads, bad checksums and sizes, offset overflow, overlap, order, alignment, padding and reserved bits. Those byte-level corruption variants are intentionally not repeated in demos; the checksum case is the representative diagnostic behavior. Other format/version/lookup getters need no separate example unless their behavior becomes useful to a real asset workflow. |
| SetExternalReferences, ExternalReferences, ExternalReferenceAt, CnbLogicalNameProblem | cnb_external_references writes two encounters sharing an immutable profile through the loader's owning ContentManager. Eleven checks pass on both renderer selections: typed table data, content-root names, shared/local caches, invalid index and game-schema type declaration, missing child/retry and writer rejection of `..`. A real DecoyProfile file at the declared child name reaches its registered loader but fails ContentManager's requested-type conversion before the parent is published; replacing that exact file with the expected profile lets the same manager retry child and parent. The container intentionally does not traverse custom schemas: the loader validates the declared expected type and calls owner.Load. Runtime cycles are therefore schema-authored recursion with no XREF cycle contract, and automatic child compilation belongs to BuildAsset/nested-build workflows already demonstrated elsewhere. CnbLogicalNameProblem is exercised by the writer; its standalone predicate needs no separate demo. |
| SetCompression, IsCnbCompressionSupported, CompressCnbChunk/DecompressCnbChunk | Build-dependent optional codecs. A compression example should query support and compare logical payloads, document the stored-size versus load-time tradeoff, and avoid claiming a tiny chunk must compress. Raw codec helper calls need no separate example if the writer/document path teaches the behavior. cnb_read_limits now queries Zstd support, writes a compressed image, compares every restored chunk with the original and inspects per-chunk codec/storage. Here the aligned file shrinks 468 to 340 bytes while logical chunk data remains 207 bytes; metadata stays uncompressed. A 206-byte aggregate cap rejects both writing and parsing despite the stored image fitting its file cap. SetCompression(None) restores the original bytes. The complete workflow passes 16 checks on both renderer selections; unsupported builds explicitly leave the compressed path unexecuted. No load-time speedup is claimed. |
| CnbReadLimits / DefaultCnbReadLimits, writer SetLimits/Limits, document Limits | cnb_read_limits passes 16 checks on both renderer selections with Zstd available. It now exercises all seven public bounds through meaningful data: file size, per-chunk size, aggregate logical size, chunk count and alignment plus a CMET type-name string and a two-row XREF table. Each container-level boundary is refused symmetrically by writer and reader; restoring defaults preserves the original valid bytes. Separate post-Parse schema controls still prove that custom chunk payloads must call ReadString/ReadCount themselves. Compressed expansion remains checked against a fitting stored-file budget. Physical truncation, arithmetic overflow and malformed table/header layouts remain parser-hardening paths rather than public-limit gaps; add only a concise diagnostic workflow if it teaches more than CNA's unit tests. Limits getters are ordinary inspection and need no separate demo. |
| ChunkIdToString, IsWellFormedChunkId, IsCustomAssetTypeId, AssetTypeIdToString and format constants | Diagnostic/format plumbing; use within inspection or schema authoring when it clarifies the example. No separate enum/constant demos. |

### Custom content pipeline and intermediate serialization

The public experimental `CNA::Content::Pipeline` contracts live in the runtime content module;
the XNA-shaped XML serialization/importer/processor adapters live in the separate build-time
`cna_content_pipeline` target. `content.custom.pipeline_components` now executes the native
three-stage route (23 checks pass on SOFTWARE and OPENGLES3). The separate XNA adapter tool
now executes registered typed components through the same real coordinator. It has 34 passing child
checks and two refusals (ISSUE-047/048) under both displayless selections.
The native patrol has 23 checks, the nested-build tool has 23, the incremental compiler has 32,
multi-artifact publication has 17, the dependency-graph tool has 16, and native build diagnostics
has nine checks. Source-root capabilities adds eleven native checks on the Linux checkpoint host;
complete checkpoint 445
preserves all of these under both renderer selections. Remaining gaps are not implied covered.

| Members / behavior | Decision and evidence |
|---|---|
| ContentPipelineRegistry RegisterImporter/RegisterProcessor/RegisterWriter; ContentPipeline::Build | `pipeline_components` registers all three components, lets Build select them by declared types, precomputes an original patrol's open/closed lengths and loads the writer's CNB output. Selection identities are checked; no direct writer invocation is substituted for the coordinator. It now also drives one request through importer, processor and writer ambiguity in order, resolving each stage by stable name until the output matches the original bytes. Explicit processor/writer names with incompatible route types fail at Selection rather than falling back. |
| Registry Freeze/IsFrozen/Registry, ResolveImporter/Processor/Writer, HasImporterForSource, inventory views | incremental_compiler observes automatic freeze and refused late registration. Default routing is exercised through real Build; the XNA adapter chooses a declared default among two compatible processors, and compiler discovery ignores unsupported sidecar extensions. The native patrol proves name-only importers/processors are excluded from ordinary resolution, then explicitly selects both and obtains identical output. Direct Resolve/HasImporterForSource calls and sorted Importers/Processors/Writers views are ordinary discovery/inspection and need no isolated demo. Registry() and repeated idempotent Freeze() are trivial access/lifecycle controls, not reasons for new IDs. |
| DocumentAbsentWriter/AbsentWriterReason/AbsentWriters | The existing native patrol registers its intentional XNB absence before the registry freezes. A real XNB Build request reports the game-authored CNB-only decision in a Selection error with exact source/asset context; AbsentWriterReason returns that reason for XNB and none for the working CNB route. This fifteenth check passes on both selections. It is separate from the XML compiler's CNB refusal. Sorted absence enumeration is ordinary inspection and needs no isolated demo. |
| ContentBuildEnvironment targetPlatform/targetProfile/buildConfiguration/outputDirectory/intermediateDirectory; OutputFormat | resize_to_power_of_two retains its rendered 128x64 check and adds six profile controls: XNB-targeted Reach processing refuses 100x60 mipmaps, HiDef keeps the original seven levels, resizing permits Reach with eight levels, and disabling mipmaps permits unscaled Reach. Fresh resized base pixels match the loaded precompiled CNB. A real CNB Build/decode retains 100x60 and the same seven levels despite Reach intent. The XNA adapter now sends WindowsPhone, Profile and a private intermediate directory through a real Build. Its importer and processor receive the same scratch path, the processor creates an environment file there, and the emitted CNB proves the XNA processor and environment-aware native writer saw the same platform/configuration/path. Output-directory/output-format and strictness evidence remains in the established branches. All new controls pass on both renderers. The blanket public comment that built-ins ignore environment fields is stale (documentation ISSUE-049). CLI-managed scratch placement is separate from explicit in-memory environment propagation. |
| ContentBuildLogger Log; processor LogInfo/LogImportant/LogWarning; writer warnings | pipeline_diagnostics now builds an original opacity strip and verifies actual CNB 0/128/255 quantization, ordered Info/Important/Warning messages with all context fields, and equality between the supplied sink and result messages. Nine checks pass under both displayless selections, including null-sink retention, exact endpoints with no loss warning, prior importer messages surviving a refused input, and repair without cross-build accumulation. XnaCompatible separately exposes parameter warnings. The synchronous copying sink does not establish concurrent callback safety, logger-thrown exceptions, CLI verbosity or every context/severity convenience method. |
| ContentValue Create/Get/StableType/CppType/RawData/Empty | Existing pipeline_components now has 23 passing checks on both displayless selections. It distinguishes empty and undeclared importer results (Import), a correctly named importer value carrying the wrong C++ type (the processor Get, Process), empty/undeclared processor results (Process), and a correctly named wrong C++ payload reaching the writer Get (Write). All six failures check exact source, asset and component context through the real Build coordinator. A copied carrier keeps its shared immutable patrol alive after resetting the original; Empty/StableType/CppType/RawData are inspected together and the raw address is used only with the matching C++ identity. These helpers need no independent demo. Empty-name construction and ordinary accessors are trivial guards, not separate semantic coverage claims. |
| ContentProcessorParameters Set/Find/Values; ValidateParameters | The patrol checks a boolean loop parameter changing length 12 to 7 and rejects a string value through ContentPipelineError. The XNA adapter example additionally checks defaults after an overridden build, Double-to-Single binding and rejection of unknown/unconvertible parameters. The compiler extension now verifies writer-version invalidation with identical output bytes; typed per-asset configuration now changes actual manifest parameters/fingerprints and rebuilds only that node; it is separate evidence from explicit Build controls. |
| ContentBuildResult output/schema/component identities and deterministic bytes | The patrol example checks selected components, compares identical Build outputs and loads the produced bytes. Build returning equal bytes does not establish incremental skipping; incremental_compiler now separately verifies it through actual RunContentCompiler invocations and component counters. |
| ContentPipelineError Stage/Component/Source/LogicalName | The native patrol checks all four fields for six Import/Process/Write component-result refusals and for importer, processor and writer ambiguity plus incompatible explicit processor/writer routes at Selection. Selection errors have an empty component, preserve the exact canonical source and logical name, and name their candidate or incompatible route. The route-summary extension checks the missing label's process context. The actual XML compiler's unsupported CNB route reports Selection; the multi-artifact compiler's original stderr records Publish/CNA.AtomicPublisher and Graph/CNA.ContentBuildGraph refusals with source/asset context. Those CLI cases observe formatted diagnostics rather than directly catching a ContentPipelineError object. AddDependency records a contained path and does not itself promise existence. Every error-string variant is not inferred covered. |
| ExternalSourceRoots, ContentSourceRootCapabilities, ResolveSourceDependency | source_root_capabilities grants a disjoint shared-art root and lets the built-in CNJ importer resolve @art/image.png. Eleven checks pass on both displayless selections: decoded CNB pixels match the original shared checker, PrimarySource/SourceFile dependencies retain canonical paths and alias, remapping records the new location with identical bytes, and an unqualified name reads a different local sprite strip. Missing grants, alias-qualified parent traversal, absolute authored names, a symlink escaping the granted physical root and a root equal to the primary are refused with source/asset/stage/component context; valid retry and private cleanup pass. The symlink check is conditional on platform support and executed on the Linux checkpoint host. `nested_builds` proves that XNA `BuildAsset` automatically carries the parent's grant into a nested built-in CNJ importer. incremental_compiler now parses `sourceRoots`, records alias-relative dependencies, rebuilds both nodes when external bytes change and skips both with a byte-identical manifest when the alias is remapped to identical bytes. No direct custom resolver call or GPU validation is claimed. Platform path differences remain separate. Add/Find/Entries/Empty and alias spelling alone need no independent demo. |
| Source dependency capabilities, runtime references, nested builds, deployment/additional outputs | The XNA adapter tool records its original patrol.route as PrimarySource and patrol.scale as SourceFile through importer AddDependency, with empty runtime/deployment lists. The separate nested_builds tool now verifies actual compiled child outputs, parent XREFs and coordinator runtime names in memory. The route processor now demonstrates an actual label dependency and generated deployment file in the source-contained control, while the configured separate output-root case fails (ISSUE-047). nested_builds connects returned parent/child bytes to runtime loaders via caller-managed publication. incremental_compiler now exercises the real CLI coordinator for serial single-output nodes, failed-import preservation and ownership-aware clean. multi_artifact_publication now exercises actual native additional outputs and contained-source deployment through the real compiler, with a deliberate Publish failure and retry. Its graph also refuses two changed nodes claiming the same support path before either publishes. Concurrent compiler invocations in one process and across a synchronized child process prove same-root lease exclusion, complete owner publication and a later skip. Whole-node transactionality is explicitly disproved by the deterministic later-output failure; true process termination between replacements has no public synchronization point and belongs to CNA fault-injection tests. |
| ContentWriteResult additionalOutputs; ContentTypeWriter OutputSchemaIdentities; native AddDeploymentFile | multi_artifact_publication has 22 passing child checks under both displayless selections. One original encounter creates a wave CNB, a reward-table CNB with its own declared schema, and an original briefing deployed to a separate output root. Both actual CNB values, XREF, manifest ownership and digests are checked. Unchanged builds skip the writer; a missing companion or changed support input rebuilds. A directory at the deployment filename causes an actual Publish/CNA.AtomicPublisher failure: new CNBs already exist while the last successful manifest remains; retry completes the set and restores skipping. A held importer proves that a concurrent same-root invocation is refused before writer entry or any artifact, the lease owner then publishes all three files plus its manifest, and an unchanged follow-up skips. A second held importer runs in a child process and proves the same exclusion across an OS process boundary; the child publishes the complete set after release and a parent follow-up skips. A third child exits directly from Import after acquiring its lease but before writing; the parent claims the unlocked stale lease, publishes the complete set and removes the child's isolated staging root. Two changed source nodes claiming the fixed briefing destination receive Graph refusals before changing either prior artifact set. Removing the second source and explicitly restoring the first input permits its original skip. Manifest-less clean preserves all files; restoring the exact saved manifest permits ownership-aware removal of both CNBs and the deployment file, retaining original sources/unowned note. No hidden internal publisher call, runtime loader, whole-node rollback or crash-consistency claim. |
| AddContentBuildDependency, direct/effective fingerprints, graph refusal and worker scheduling | dependency_graph passes 16 child checks on both displayless selections. Battle has both a build edge and runtime XREF to enemy; hud has only the XREF, and neither processor reads enemy's source. Editing enemy 7 -> 9 recompiles enemy/battle while hud skips. Battle's direct fingerprint and bytes stay equal but its effective fingerprint changes; hud's effective fingerprint and bytes remain equal. Fresh ContentManager loads resolve both budgets as 27/18 rather than 21/14. Cyclic and missing-primary-node build dependencies fail before replacing the valid artifacts/manifest; explicit source repair restores old complete skips. An independent two-worker output root uses a bounded importer rendezvous to observe actual overlap, produces byte-identical manifests/CNBs and skips all importers/writers unchanged. This validates these graph behaviors, not writer invocation order, every graph shape, arbitrary runtime cycles, scalability or simultaneous independent compiler processes sharing an output root. |
| Sibling item recognition versus repeated nested builds | nested_builds passes five sibling controls: an empty assetName recognizes a matching ContentBuildSibling and records its actual logical name without child compilation or additional output. The budget's BuildAndLoadAsset still reads the profile source, but the sibling reference adds no content-build edge or child writer schema. A parent-only runtime load fails; the host's separate item build/publication enables retry and shared cache identity. Different Scale parameters prevent sibling matching and produce the requested profile under a generated name. All fifteen older controls and three later capability-propagation controls remain (23 total). This is separate from duplicate-request deduplication and does not demonstrate the CLI scheduling those sibling items. |
| Registry freezing/concurrent builds, RunContentCompiler embedding | incremental_compiler now links CNA::ContentCompiler and runs its real registry-factory overload in a private workspace (32 passing child checks per displayless renderer selection). Unchanged invocations create a fresh registry but enter no importer/processor/writer. The retained accepted registry reports IsFrozen and refuses late RegisterWriter with its frozen diagnostic. Its pre-built-registry overload refuses both FX and XMA service selection with status 2, names `ContentPipelineRegistryFactory` and enters no component; ISSUE-106 records that the XMA diagnostic still names only FX options. One option-complete invocation passes every service/font/configuration/XNB choice to a factory that registers built-in and game routes before CNA freezes it and builds the score. Two synchronized threads then overlap inside one shared factory on disjoint source/output roots: distinct `--fx-compiler` values reach separate registries, CNA freezes both, their manifest writer identities remain isolated and each publishes only its expected score. A separate node records a contained generated factor, skips while its bytes are unchanged and rebuilds to a different compiled value after an edit. CNA explicitly performs game-owned registration and no public dynamic-plugin loader exists, so external-plugin discovery is intentionally not demonstrated. This concurrency result is limited to custom routes and disjoint roots; it does not claim process-global built-in service isolation, same-root publication or independent processes. |
| ContentBuildManifest Parse/Find/Entries, published digests; incremental invalidation and compiler clean | The compiler tool parses the actual manifest, finds the arena node and compares its recorded output digest with ContentFileSha256. One edited primary source rebuilds only that node; changed shared and generated dependencies rebuild their affected nodes, while unchanged generated bytes skip every component. Remapping alias `shared` from `shared-a` to `shared-b` with identical bytes skips both ordinary nodes and preserves the exact manifest, proving the physical root is not unstable cache identity. A refused import preserves prior artifact and manifest bytes, repair succeeds, and a deleted output invalidates its skip. Real clean removes owned compiled files while preserving an unowned note and authored sources; private-directory cleanup is a separate final check. `content.custom.build_manifest` separately covers direct Set/replacement/Clear, ordered Entries/Find, deterministic Serialize/Parse, direct and effective fingerprints, every fingerprint-state domain and malformed ownership/schema/dependency/digest refusal. A writer-version change rebuilds both nodes and changes the recorded fingerprint despite identical output bytes. Same-length output tampering with restored mtime rebuilds only the damaged node; a corrupt manifest triggers two fresh builds and replacement. Clean refuses changed owned bytes before deleting either a valid peer or its manifest; the original bytes are explicitly restored after that assertion for the independent successful-clean control. The public ContentBuildConfiguration Parse/Find/SourceRoots route verifies the external mapping and a typed i64 Bonus in the conventional .cna-content.json. Introducing and changing the parameter rebuilds only arena and changes its fingerprint; whitespace-only rewriting skips both nodes with identical manifest bytes. Removing menu.score collects its proven obsolete output while preserving the live node and unowned note; reintroducing it publishes only menu. Both outputs are restored before the older guarded/successful-clean controls, which use the latest owned arena bytes. The separate multi_artifact_publication tool covers declared additional artifacts, deployment-source invalidation, graph ownership conflict, per-file publication failure/recovery and same-root output lease exclusion within and across processes. Clean without its ownership manifest preserves all artifacts rather than guessing by extension. Process termination between replacements needs CNA-internal fault injection and is intentionally not represented by a timing-dependent demo. |
| RegisterXnaImporter/RegisterXnaProcessor/RegisterXnaProcessorWithoutAttribute; adapter metadata; ProcessorParameterBindings | `xna_pipeline_components` registers one typed importer, two attributed processors and one attribute-free processor under two catalogs. It directly inspects their XNA class/catalog/attribute/type/parameter metadata, selects the importer's default processor despite multiple candidates accepting the same input, binds Padding and Scale properties, and inspects compiled CNB bounds. It checks typed parameter conversion, default isolation and contextual rejection through real ContentPipeline::Build. It does not call Import/Process directly in place of the coordinator. |
| ToXnaTargetPlatform/FromXnaTargetPlatform; ToOpaqueData/ToProcessorParameters | All three supported platform values round-trip. The five canonical parameter alternatives retain exact boxes and values; 32-bit signed/unsigned and float boxes widen to the canonical alternatives, `Vector2` becomes its parseable invariant text and an unsupported game object is refused by stable type. This covers direct helper contracts separately from their use inside adapter contexts. |
| PipelineComponentScanner | Both constructors, all nine result properties and both Update overloads have direct behavior. Complete and catalog-filtered scans preserve sorted names, attributes, stable input/output types and parameter descriptions. Repeating an unchanged scan returns false; a known dependency catalog is accepted while one missing scanned catalog produces exactly one named error; a detached scanner stays empty. Dynamic assembly loading is deliberately replaced by registered catalog enumeration in CNA. |
| XnaBridgeLogger/XnaBridgeImporterContext/XnaBridgeProcessorContext | The registered route constructs both context adapters and both logger overloads through the real coordinator. The importer reads IntermediateDirectory and OutputDirectory, records a dependency and emits Info. The processor reads BuildConfiguration, IntermediateDirectory, OutputDirectory, OutputFilename, Parameters, TargetPlatform and TargetProfile; its existing Convert, AddDependency and AddOutputFile paths remain active, while the new HiDef request emits Important and Warning messages. The result proves exactly three messages with the canonical Import/Process stage and component context, including warning help-link formatting. BuildAssetCore and BuildAndLoadAssetCore are protected forwarding implementations exercised by `nested_builds`; they do not need substitute direct calls. The canonical context aliases add no behavior. |
| ContentBuildEnvironment strictness; ContentBuildResult messages/parameters | The existing XNA adapter contrasts Strict refusal with XnaCompatible warning/default behavior: two invalid entries produce two source/asset/stage/component warnings and byte-identical default bounds, a valid Padding override survives dropping an unknown entry, the caller request stays intact, and missing processor input still fails. Of 36 child checks, 34 pass; older output-directory ISSUE-047 remains and ISSUE-048 exposes returned "effective" parameters retaining the dropped entries. Compiled bounds and four environment controls are correct. No manifest/cache corruption or genuine-XNA compatibility oracle is claimed. The Song and Video component demos directly catch ContentParameterError and distinguish `UnknownName` through both Fault and Parameter. The second enum value is exercised by XnaCompatible conversion refusal; a per-enum demo is unnecessary. |
| ContentProcessorContext AddDependency/AddOutputFile/BuildAsset/BuildAndLoadAsset/Convert | The bounds adapter tool demonstrates Convert with typed parameters and importer AddDependency for its real scale sidecar. `nested_builds` now uses the real coordinator for BuildAndLoadAsset versus BuildAsset: the original eleven child checks remain passing under both displayless selections. A processed profile sets the parent budget without child compilation/runtime references; two identical BuildAsset requests produce one independently parsed child CNB, matching the parent's XREF and coordinator runtime name. Both source dependencies and writer schemas are recorded; different processing under one name is refused and a following valid build is deterministic. The caller writes the actual returned bytes into a private content root: parent-only loading fails, publishing the child permits retry without Unload, direct/parent loads share the profile cache, and scoped cleanup is verified. Five sibling-item controls bring the older total to20; three more now prove missing-grant refusal and automatic propagation of a parent source-root capability through BuildAsset into a real nested CNJ texture, including exact output/dependency/XREF evidence (23 total). The route adapter separately reads a label through processor AddDependency, generates a real summary and calls AddOutputFile. Its separate output-root refusal is ISSUE-047; an explicitly separate source-contained control records the actual deployment file and dependencies. Changed-label explicit rebuilds preserve CNB bounds, missing input reports full process context, and cleanup passes (the original 12 passes/ISSUE-047 remain; the parameter extension adds five passes and ISSUE-048). The compiler tool separately covers serial incremental publication/skipping and clean; native multi-artifact publication failure/recovery and same-root output lease exclusion and pre-write stale-lease recovery across processes are covered separately. Abrupt termination inside the publisher requires an internal fault-injection hook; a timing-dependent demo would add no stable public contract. |
| RegisterXnaXmlSourceContentPipeline/RegisterXnaXnbOutput, custom type writers | xml_xnb_pipeline executes the real source route using RunContentCompiler, a pre-registered described authoring serializer and ContentCompiler::AddTypeWriter. Thirty-one of thirty-three child checks pass under both displayless selections. ProfileWriter exposes the exact ProfileSource TargetType, runtime type, version-two reader and default existing-object policy. Initialize supplies its compiler before Write; Write then resolves a registered Vital writer and uses explicit WriteObject/WriteRawObject routes whose distinct values load through matching game readers. A base-typed RareBadge uses dynamic writer/reader dispatch: its inline value is independent while two WriteSharedResource calls resolve to one deferred runtime identity. WriteExternalReference turns an absolute sibling XNB name into the exact relative extensionless path. The edited document then proves ordinary and shared null references plus an empty external reference round-trip without invoking deferred fixups. Focused public Compile probes additionally refuse raw null, a derived reference with no exact writer and an explicit writer belonging to another compiler. The retained successful compiler reports a sorted KnownTypeNames inventory containing built-ins and all four game writers; its completed compile refuses late registration while known GetTypeWriter remains usable and an unknown type is refused. The Profile writer observes requested Windows Phone/HiDef context and declines requested LZX; a separate default Vital writer accepts the same compression request, sets the XNB compression flag and produces a payload that ContentManager restores exactly. Direct `RegisterXnaXnbOutput` installs exactly one XNB writer per compiler type, preserves an existing compatible route, is idempotent, exposes the option digest/schema and refuses a null compiler. A resolved custom writer produces a compressed XNB that restores its typed value. ISSUE-111 records that its environment overload instead keeps the registration options' Windows/Reach target. Direct XML registration now proves one shared importer plus one pass-through processor/XNB writer per type, extends that live route with a second compiler's new type, remains object-idempotent on repetition, builds and loads the extension value, and refuses null before mutation. ContentTypeWriterAttribute is an empty C++ registration marker, so its construction has no separate property behavior to exercise. A bounded probe exposes ISSUE-090: retrieving the nested writer from Initialize as documented deadlocks AddTypeWriter. Manifest importer/digest, unchanged skip, edited-source rebuild and runtime cache versus Unload remain verified. CNB is separately refused at writer selection because this arbitrary-object route has no CNB schema. Public factory options.xnbContainer supplies the requested options; no direct InvokeWrite substitute, internal XML parser, GPU or genuine-XNA oracle. |
| XNA ContentCompiler/ContentWriter implementation-facing helpers | TypeWriterRegistry and FindWorker/FindWorkerForObject/FindWorkerFor/ActiveWriter connect the compiler's façade to CNA's canonical XNB writer; the successful nested/dynamic/shared graph necessarily exercises that bridge, but direct calls would expose implementation identities rather than another content behavior. ContentWriter::Output is the same canonical bridge. Flush and Close intentionally do nothing because Finish owns the buffered file, and Dispose only unregisters the short-lived façade; isolated calls would not change bytes. Primitive Write overloads are represented by string/int payloads here and the broader stock XNB assets; repeating every scalar and math overload belongs to CNA's byte-level writer tests. These members are intentionally not inflated into independent demo controls. |
| ContentSerializerAttribute, IntermediateReader/Writer, IntermediateSerializer, ContentTypeDescriptor | Real public APIs, not inert metadata. Current sources use format attributes and fluent field descriptors for element names, optional/null/ignored/flattened content and collection item names. `content.custom.intermediate_serializer` links the separate build-time target and now passes 16 child checks: renamed and implicit field names, optional scalar and empty-collection omission, custom collection item names, round trip with constructor defaults, present/explicit-null values, required-null refusal on read/write, missing required data and ignored-field rejection. No Internal parser is borrowed. properties checks flattened spawn coordinates, shared_resources checks cyclic fixups with explicit C++ ownership cleanup, and intermediate_polymorphism checks concrete base-typed values and invalid type declarations. The external-reference tool records nine passes plus native alias ISSUE-046. Version/runtime-type descriptor metadata is separately classified below. |

#### `ContentPipeline.hpp` declaration-level disposition

The pinned `d23e139de` header has 234 public declarations under a mechanical Clang AST count:
199 record fields, constructors, destructors and methods; 23 enumerators; seven namespace
functions; four constants; and one parameter-value alias. The count deliberately includes deleted
context copy operations, pure-virtual interface methods, overloads and plain result fields. This is
a disposition audit, not a claim that calling every spelling would add distinct coverage.

| Declaration family | Disposition |
|---|---|
| Source-root capabilities and validation | Real CNJ compilation resolves local and aliased files, records the canonical path and alias, survives a byte-identical remap, and refuses missing, overlapping, absolute, traversal and symlink-escape cases. Configuration parsing directly inspects ordered Entries/Find/Empty and refuses an invalid alias. `ContentSourceRootAliasProblem`, `ResolveContentSourceRootCapabilities`, equality and `MaxContentSourceRoots` are the validator, normalization, value and stress-limit layers of that same workflow; independent predicate, equality or 33-entry demos would add no content behavior. |
| Output format, target platform, stages and labels | Both CNB and XNB routes execute through real builds. The strict configuration parser accepts both stable format tokens and refuses an unknown one through `TryParseContentOutputFormat`; published filenames establish the two extensions. Windows Phone reaches a component, Reach/HiDef change texture processing, and all six stages occur across selection, component, graph and publication workflows. Name/extension functions and enum values are stable diagnostic labels; representative `ContentPipelineStageName` output is already visible. No enum-per-value demo is warranted. |
| Environment, component/schema identities, messages and dependency value records | Actual builds compare component identities, persist schema/codec identities, carry every environment field, retain ordered messages, and publish PrimarySource, SourceFile, ContentBuild and now Generated dependencies plus runtime references and deployment files. Their default fields, equality and ordering operators are value plumbing already observed through those real results and deterministic manifests. |
| ContentDependencyCollector | Importer/processor context calls feed the collector in real builds. The Song workflow directly inspects runtime/deployment copies; compiler manifests inspect sorted build dependencies; duplicate references collapse in nested and graph workflows; conflicting deployment ownership is refused before publication. Focused959 adds the only missing semantic kind: `AddGeneratedDependency` records `Generated`, unchanged bytes skip, and edited bytes rebuild to a changed CNB. Raw Add/accessor calls and maximum-count saturation remain helper/stress variants. |
| Parameters, ContentValue and pipeline errors | All five parameter alternatives round-trip through configuration; real processors accept, reject and consume them. Song and Video inspect `ContentParameterError::Fault` and `Parameter`; XnaCompatible exercises the unconvertible class. ContentValue's empty, stable/C++ identity, shared lifetime and checked cast are direct. All four ContentPipelineError accessors are checked across Selection, Import, Process and Write, while compiler tools expose Graph and Publish diagnostics. Constructors, default comparisons and every diagnostic string do not need synthetic controls. |
| Importer and processor contexts | Custom components read source paths, logical names, parameters and environment, resolve primary/external dependencies, emit all three component log severities, add build/runtime/deployment/generated dependencies, and perform nested builds. The XNA bridge consumes Pipeline, Dependencies, Logger, ExternalSourceRoots, OutputFormat, SiblingsShared, nested output and nested schema services; `nested_builds` verifies their observable artifacts, schemas, sibling reuse and propagated capability. Direct getter-identity assertions, deleted copy operations and duplicate convenience log calls are forwarding/lifetime declarations, not additional pipeline behavior. |
| Importer, processor and writer interfaces | Registered game components exercise every required virtual stage. DefaultProcessor and name-only selection alter real routing; both writer formats execute, the environment-aware overload changes XNB behavior, declared schemas invalidate manifests, additional outputs and warnings reach publication/results. Virtual destructors, default CNB/ordinary-selection returns and directly invoking a component in place of Build need no separate demo. |
| Registry and coordinator | Registration, automatic freeze, late-registration refusal, conventional and explicit-name selection, ambiguity/incompatibility, intentional absence, concurrent read-only use, Build and ImportAndProcess all have runtime evidence. Direct ResolveImporter is also used by the Song route. HasImporterForSource, sorted inventory/absence views, Registry, repeated Freeze and the other direct Resolve spellings are discovery or identity views over the same configured routes; they remain intentionally uncredited unless a user-facing inventory needs them. |
| Request, build/process results and size bounds | Real workflows populate every request control and consume build source/name, identities, format, schemas, parameters, dependencies, messages and primary/additional output. ImportAndProcess consumes the processed value under several target profiles; nested workflows consume its identities, messages and nested outputs through the bridge. Plain aggregate defaults and the currently-always-true `built` flag are inspection only. `MaxContentBuildOutputs` and `MaxContentDeploymentFiles` are denial-of-service bounds covered by CNA tests; saturating 256 entries would not teach another authoring concept. |

No ordinary public `ContentPipeline.hpp` behavior remains undispositioned after the generated-
dependency extension. Platform path rules, concurrent logger callbacks, maximum-count saturation,
publisher crash injection and arbitrary malformed component declarations retain their explicit
platform, stress, CNA-test or internal-fault-injection classifications.

#### `ContentCompiler.hpp` declaration-level disposition

The pinned header exposes 13 declarations: eight `ContentCompilerOptions` fields and its equality
operator, `RegisterBuiltInContentPipeline`, the registry-factory alias, and two
`RunContentCompiler` overloads. Focused960 exercises them through the existing incremental compiler
rather than adding a CLI-options-only demo.

| Declaration family | Disposition |
|---|---|
| ContentCompilerOptions service and font/configuration fields | One real invocation passes distinct effect executable/launcher, XMA executable/launcher and two argument templates, two font directories and `Debug`. The factory receives every value exactly before registering built-in and game routes. No selected external executable is launched by the score route, so this establishes command-line-to-factory propagation and registration configuration, not an effect/XMA encoding or font-resolution result. Those host/tool-dependent consumers remain CNA integration-test territory until a portable authored asset needs one. |
| ContentCompilerOptions xnbContainer | The same factory receives explicit DesktopGL, version 5, HiDef, LZ4, portable reader names and the unverified-Xbox opt-in. `xml_xnb_pipeline` separately supplies the XNA-compatible Windows Phone/HiDef/LZX request to a game writer and inspects the actual platform/profile/compression result. Container-option validation and legacy/extended permutations belong to the XNB writer audit; this carrier needs no duplicate output matrix. |
| ContentCompilerOptions equality | A container-only compression change compares equal, while a build-configuration change does not. This matches the documented ownership split: the coordinator applies container options after registration, while registry-affecting service/configuration choices require a new factory result. Every field need not be toggled independently after exact propagation is established. |
| RegisterBuiltInContentPipeline | The option-aware factory invokes the public registration helper, adds its game route, and returns one registry. After the coordinator freezes it, `HasImporterForSource` recognizes representative built-in `.fx`, `.spritefont` and `.png` sources plus the game `.score`; the real score builds and decodes to 10. Other content demos separately execute the individual built-in routes. Enumerating every registered component would duplicate their registration helpers rather than add compiler behavior. |
| Factory RunContentCompiler overload | The existing compiler workflows establish one callback per invocation, fresh/frozen registries, exact option propagation, serial skips/rebuilds, graph scheduling, concurrent calls with disjoint roots, publication and clean. Empty/null factory guards and every malformed CLI spelling are ordinary boundary tests already covered by CNA's own suite. |
| Pre-built-registry RunContentCompiler overload | A service-free registry executes the full incremental workflow. FX and XMA selections are both refused before discovery because they could not have configured that already-built registry; the factory overload is named and no component runs. ISSUE-106 records that the XMA refusal's text still names only the two FX options. XNB container settings are intentionally excluded from option equality because the coordinator can add/apply writers after parsing. |

All ordinary `ContentCompiler.hpp` declarations now have runtime evidence or an explicit
integration/boundary disposition. Focused996 now covers the effect compiler seam and source route
with a controlled service and committed genuine bytecode, while external compiler execution remains
tool-dependent. Focused997 covers the XMA encoder seam with controlled in-process and external-
process backends while explicitly making no genuine XMA claim. Font-host results still require a
portable host; option propagation alone does not claim those outputs.

#### `CnjContentPipeline.hpp` declaration-level disposition

The pinned header exposes 79 declarations: ten stable in-memory type constants, 19 fields across
eight source/processed aggregate values, 49 virtual component methods and
`RegisterCnjContentPipeline`. Focused961 adds `content.cnj.pipeline_assets` for the three source
routes that previously had no direct demo workflow.

| Declaration family | Disposition |
|---|---|
| CnjImporter identity, extension, output inventory and Import | The new demo inspects the stable importer identity, ordinary `.cnj` route and deterministic eight-type output list, then selects it through three real builds. `source_root_capabilities` and `nested_builds` already compile Texture2D CNJ through local/aliased dependencies; the project content build compiles Curve and SpriteFont CNJ and their demos consume the exact authored semantics. SoundEffect CNJ is a descriptor over the same shared WAV front end demonstrated by `content.sound_effect.pipeline_components`; Model CNJ selects the same canonical model document and processor exercised by the model and C CNJ workflows. New wrapper-only SoundEffect/Model IDs would duplicate those processing concepts. Envelope corruption, future versions and chaining remain parser boundary tests except where source-root containment is already demonstrated. |
| Texture3D imported value, processor and writer | A real 2x2x2 raw sidecar becomes one Rgba8 representation with exact dimensions, one face, one mip and every authored byte. The build verifies selected identities, source dependency, logical CNB metadata and repeat-byte determinism. Direct inspection covers the stable input/output/schema declarations, and one unsupported parameter is classified as `ContentParameterFault::UnknownName`. `additionalRgbaMipLevels` is meaningful only when another importer supplies mips; the adjacent XNB source audit owns that case. |
| TextureCube imported value, processor and writer | A real DXT1 DDS sidecar decodes through CNA's shared importer, stays one canonical Rgba8 representation and reaches the native writer. The decoded CNB proves 4x4 dimensions, six faces and exact red/green/blue/yellow/magenta/cyan order. Selected component/schema identities and the absorbed dependency are checked. Repeating direct class calls or every malformed DDS belongs to the shared DDS codec tests. |
| AnimationClip imported/processed values, processor and writer | A self-contained two-key SceneNode clip builds through the registered route. Decoding proves duration, target space, bone index, key times and translations, while the sole dependency remains the primary document. An XNB request reaches the registered intentional-absence explanation because standalone `AnimationClipEXT` has no XNA reader. Sidecar-form parsing is the same semantic value with one additional source dependency and is already covered in CNA tests; it adds no separate demo concept. |
| Curve and SpriteFont values/components | `content.cnj.curve` and `content.cnj.sprite_font` are project-built from authored CNJ and verify the resulting Curve and SpriteFont values, including curve looping/evaluation and atlas/glyph/crop/kerning/default-character behavior. Their importer, processor and writer stages are necessary to those generated CNBs. Constructing the plain source/processed carrier structs or repeating identity getters would add no behavior. |
| Fixed parameters, constants and registration | The three new routes are resolved from one registry created only by `RegisterCnjContentPipeline`; existing Curve/SpriteFont project builds and the Texture2D registry composition cover the other registrations. All four non-font fixed processors share the same unknown-name helper, represented by Texture3D; SpriteFont's equivalent empty-only policy is inherent in its parameter-free project build and covered at the CNA boundary. Stable type strings drive actual route resolution. Per-constant assertions or repeating the identical refusal for every processor are intentionally omitted. |

All ordinary public `CnjContentPipeline.hpp` behavior now has runtime evidence or an explicit
shared-route/boundary disposition. This is CPU compilation and CNB decoding evidence; it does not
claim Texture3D/TextureCube GPU upload, rendering or visual quality.

#### `XnbContentPipeline.hpp` declaration-level disposition

The pinned header exposes 13 declarations: the imported-Video type constant, both
`ImportedXnbVideo` fields, four `XnbImporter` methods, five `XnbVideoProcessor` methods and
`RegisterXnbContentPipeline`. Focused962 adds `content.xnb.pipeline_transcode` and runs every one
of the importer's nine advertised root types through the public build coordinator.

| Declaration family | Disposition |
|---|---|
| XnbImporter identity, source route and output inventory | The demo checks identity version 3, the exact `.xnb` extension, ordinary automatic selection and the deterministic nine-type output order. One registry composed from the XNB slice and each shared destination slice resolves the route and performs every build. Direct `Import` calls would bypass the selection, dependency and writer behavior the example is intended to teach. |
| Texture2D, SpriteFont, SoundEffect and Curve roots | Project-built original XNB fixtures become native CNB through the shared processors. Decoding proves the 128x128 texture has real pixels, the font retains complete parallel glyph tables, the 22.05 kHz mono sound retains all 6,615 frames and the three-key curve keeps its midpoint evaluation. Rebuilding the texture is byte-identical. Runtime loading/rendering/playback remains with the existing focused asset demos. |
| Texture3D and TextureCube roots | Original bounded wire fixtures exercise both previously unshown XNB source routes. The volume preserves every byte across 4x2x2, 2x1x1 and 1x1x1 mip levels. The cube preserves every RGBA byte of all six distinct 4x4 faces in canonical order. These are headless import/process/write/decode checks; GPU upload and sampling are separate graphics workflows. |
| Model root and schema selection | The project-built five-bone, two-mesh fixture requires Model schema 2. The output declares schema 2 and the matching public decoder proves its hierarchy, one shared vertex buffer, one shared index buffer and two independently coloured effects. This intentionally covers the fidelity fallback rather than forcing the asset through schema 1; the existing runtime XNB Model scene owns draw and pixel evidence. |
| Song root | An original root with an authored `.wma` path selects the available `.ogg` sidecar, preserves its 4,321 ms duration and logical name, and records the selected source separately as build dependency, runtime XREF and deployment output. The sidecar is deliberately opaque non-playable data, so no media decoding or playback result is claimed. |
| ImportedXnbVideo and XnbVideoProcessor | An original Video root populates both imported fields and reaches the dedicated adapter. Exact original duration, dimensions, frame rate, soundtrack type and selected `.ogv` reference survive. A second build supplies all six standard overrides and proves the changed metadata, XREF logical name, original deployment source and new output path. Identity/input/output are explicit; validation distinguishes `UnknownName` from a zero-dimension range error. Other type/range permutations are boundary-test variants. |
| Registration, malformed input and compatibility limits | `RegisterXnbContentPipeline` is the sole source of the importer and Video adapter in the demo registry; established pipeline slices supply destination processors/writers. Header/compression/type-table corruption, unsupported readers, invalid model subsets, missing media, traversal and size ceilings are extensive CNA parser/pipeline tests rather than useful demo concepts. Existing runtime XNB demos retain genuine loader and GPU/audio behavior. |

All ordinary public `XnbContentPipeline.hpp` declarations now have runtime evidence or an explicit
shared-route/boundary disposition. The result establishes deterministic CPU transcoding and exact
native-data semantics for all supported roots; it makes no genuine-XNA execution, GPU, audio,
video-decoding, playback, screenshot or visual-quality claim.

#### `XnbOutputContentPipeline.hpp` declaration-level disposition

The pinned header exposes eleven persisted `XnbOutputAssetId` values and three public functions.
Focused963 adds `content.xnb.output_pipeline` for the target-format decision surface around the
already demonstrated XNB source/compiler/runtime workflows.

| Declaration family | Disposition |
|---|---|
| XnbOutputAssetId | One grouped assertion freezes all append-only values: Texture2D through Effect are 1 through 10 and XnaObject is 11. The registered built-in writers independently publish the matching first ten IDs and exact runtime type names in their schema identities. XnaObject belongs to the separate game-defined `ContentCompiler` façade; `content.custom.xml_xnb_pipeline` executes that real writer/reader path, so registering it as a built-in asset writer here would be incorrect. |
| XnbOutputOptionsDigest | Default Windows/version5/Reach/uncompressed/XNA-name options produce `w5run`; DesktopGL/version5/HiDef/LZ4/portable options produce `d5h4p`. The latter appears in the resolved writer identity and emits an actual `d`/5/`0x41` container deterministically. Limits and the Xbox opt-in control validation rather than successful byte spelling and intentionally do not add digest characters. ISSUE-107 records that the public header's impossible `w5rn-x` example describes neither grammar. |
| ConvertCnbTextureToXnb | A dual-representation 4x4 texture proves ordered, version-aware choice: version 5 selects first-listed Rgba8/Color bytes, while legacy version 4 skips those and selects second-listed Bgra8/ColorBgraEXT bytes. A BC7-only version-5 request is refused with logical name, format and container version, and a non-square cube is refused with its exact 4x2 shape. The remaining face/depth/empty-data permutations repeat the same public shape guard and stay boundary-test cases. Exact CNJ volume/cube input bytes and runtime DXT/cube upload behavior remain covered by their respective source and graphics workflows. |
| RegisterXnbOutputContentPipeline | One frozen registry exposes exactly ten format-XNB writers: Texture2D, Texture3D, TextureCube, SpriteFont, SoundEffect, Song, Video, Model, Curve and already-compiled Effect. Every input type, writer identity, output format, asset ID/schema/type name and shared serializer identity is checked. A dual registry resolves the ordinary CNB and XNB Texture2D writers independently by format. The custom option-bound Texture2D writer executes twice with identical bytes and reports the root reader stored in its file. Existing `content_xnb` builds and runtime demos execute original Texture2D/SpriteFont/SoundEffect/Curve/Model outputs; Song/Video and model-loss boundaries remain CNA integration tests until a project authoring workflow needs those target artifacts. |

All public `XnbOutputContentPipeline.hpp` identities and functions now have direct runtime evidence
or an explicit shared-workflow/boundary disposition. The emitted probe is decoded structurally by
its header/result identities only; genuine Microsoft XNA loading, GPU use, audio/video playback and
visual quality are not claimed.

#### `EffectContentPipeline.hpp` declaration-level disposition

The pinned header exposes sixteen public declarations: two stable type identities, the compiled
byte carrier and its equality, one signature recognizer, four importer methods, five processor
methods, the registration function and the shared no-CNB explanation. Focused964 extends the
existing `content.xnb.output_pipeline` workflow because the useful behavior ends at the already
audited Effect XNB writer; a second catalog ID would split one coherent source-to-output route.

| Declaration family | Disposition |
|---|---|
| ImportedCompiledEffectType, ProcessedCompiledEffectType; ImportedCompiledEffect::bytecode/operator== | The demo reads the committed 5,000-byte `CnaConformanceEffect.fxb`, checks both stable type strings and exact owned bytes, and distinguishes a one-byte mutation through the default equality operation. The same bytes later survive the complete importer/processor/writer route as an exact XNB suffix. This is byte ownership and identity evidence, not shader semantic validation. |
| IsCompiledEffectBinary | The real fx_2_0 fixture plus synthetic bare and XNA-wrapped Effect Framework 9.1 signatures are accepted. Empty, foreign and deliberately unaligned wrapper inputs are rejected. The helper is a signature preflight by contract; malformed instruction streams after a valid signature belong to the shader compiler/runtime boundary and do not justify misleading demo assertions. |
| CompiledEffectImporter methods | Identity, `.fxb` extension and sole imported output type are exact, and an ordinary named source selection executes `Import` against the real fixture. The source bytes observed at final output prove that the importer did not transform them. Source-extension case and unreadable/traversing paths repeat shared pipeline selection and source-root guards. |
| CompiledEffectProcessor methods | Identity, input/output identities and exact pass-through are exercised in the real build. A supplied `optimize` parameter is refused with the documented reason that compiled bytes pass through exactly. Empty/wrong boxed inputs are contract boundary tests already owned by the CNA suite rather than separate user concepts. |
| RegisterCompiledEffectContentPipeline | A registry containing only this source route plus the standard XNB outputs resolves and builds the real fixture through `CNA.CompiledEffectImporter`, `CNA.CompiledEffectProcessor` and `CNA.XnbEffectWriter`. Two builds are byte-identical; the output reports Effect asset ID 10, the framework Effect type and `EffectReader`. Registration is therefore observed through behavior rather than registry presence alone. |
| CompiledEffectHasNoCnbSchemaReason | The registry reports the exact shared reason for its deliberate missing CNB writer, and an actual CNB build fails at writer selection with that reason. The sibling `.fx` compiler-service route reuses the same public explanation; compiling source is already demonstrated separately and need not be duplicated here. |

All ordinary public `EffectContentPipeline.hpp` declarations now have direct build-time evidence or
an explicit boundary disposition. The fixture is serialized but never executed: neither configured
renderer exposes compiled-Effect execution in this environment, so this checkpoint makes no GPU,
visual, shader-correctness or genuine-XNA compatibility claim.

#### `Texture2DContentPipeline.hpp` declaration-level disposition

The pinned header exposes 48 public declarations under the same counting used above: eight stable
identities/parameter keys, six build-format values, two format functions, the encoder callable,
four pixel helpers, eight imported-image fields, three import/canonicalization helpers, four
importer methods, two processor constructors plus five methods, four writer methods and one
registration function. Focused965 adds `content.texture_processor.pipeline_contract` as the one
headless component-contract demo; the existing four processor scenes and XNB DXT scene retain the
user-facing visual behaviors.

| Declaration family | Disposition |
|---|---|
| ImportedImageType, ProcessedTexture2DType; six parameter keys | One grouped assertion freezes both routing identities and the exact `colorKey`, `textureFormat`, `generateMipmaps`, `premultiplyAlpha`, `resizeToPowerOfTwo` and `passThrough` spellings. The existing color-key, mipmap, premultiplied-alpha, resize/profile and DXT scenes demonstrate the first five policies in use. The new DDS control demonstrates pass-through semantically by preserving every authored BC1 block and bypassing the injected encoder. More combinations would multiply policy permutations rather than add concepts. |
| TextureBuildFormat values; TryParseTextureBuildFormat/TextureBuildFormatName | All six values round-trip to their canonical names; mixed-case `DxtCompressed` parses and unknown `Bc7` remains absent. Existing `content.xnb.dxt_texture` supplies real automatic DXT selection and runtime SurfaceFormat evidence. Separate demos for explicit Dxt1/Dxt3/Dxt5 spelling would only repeat one parser/encoder route. |
| TextureBlockEncoder; both TextureProcessor constructors | The default component identity is inspected and its strict parameter validation is exercised. A parameterized processor registered through the public helper calls an injected encoder once with exact BC1 format, 64x64 dimensions and all 16,384 RGBA source bytes; its recognizable 2,048-byte result becomes the sole processed level. Those probe blocks are deliberately not treated as a valid visual texture. The existing XNB DXT scene is built by the tool's real encoder and owns actual compressed loading/render evidence. |
| NextPowerOfTwoDimension, ResampleRgbaImage, PremultiplyRgbaAlpha, GenerateRgbaMipChain | Exact controls cover 0/1/non-power/exact-power sizing, awkward 3-to-2 area averaging, interpolated growth with fixed endpoints, partial/opaque/transparent alpha plus a trailing partial texel, rectangular halving to 1x1 and the 1x1 no-op. Invalid dimensions/buffer lengths and arithmetic ceilings are useful library safety tests, not additional demo concepts. The resize/mipmap/premultiply scenes connect the helpers to real content and rendering. |
| ImportedImage fields; DecodeImportedImage | A real 64x64 RGBA PNG produces exact dimensions and 16,384 bytes with empty mip/compression/policy side channels. A real 16x8 DXT1 DDS exposes decoded pixels plus its authored BC1 format and every compressed mip for pass-through. A synthetic two-level carrier then proves canonical level ownership/order. The authored colour-key and premultiply optionals are inter-component policy carriers: CNJ and XNB importers populate them, and the existing CNJ source-root and nine-root XNB transcode workflows execute those shared processors. Directly assigning the optionals without an importer would add no author-facing route. |
| ApplyPngFileGammaEXT | A minimal structurally sufficient RGBA PNG header with `gAMA=45000` maps RGB 128/64/255 to 127/63/255 while retaining alpha 77; non-PNG input is unchanged. Full PNG decoding is separately exercised through a real fixture. sRGB/iCCP, truncated chunks and every PNG colour type remain format-parser boundary tests rather than demo variants. |
| BuildCnbTexture2DData | A 2x2 level plus its 1x1 child becomes one Rgba8 representation with depth/face count one and exact level order. The full registered PNG build then decodes the actual CNB and matches every original opaque pixel. Invalid mip geometry is a codec validation boundary already owned by CNA tests. |
| ImageImporter methods | Identity/version, all fourteen routed extensions and the three possible output identities are exact. The registered build executes `Import` for real PNG, ordinary DDS and DDS pass-through; `content.cnj.pipeline_assets` separately exercises the cube-shaped DDS output. PNG and block-compressed DDS represent the ordinary raster and structured-surface branches. One demo per interchangeable JPEG/BMP/TGA/GIF/PSD/HDR/PIC/PNM/DIB/PFM/PPM decoder front end would inflate count without teaching another pipeline concept. |
| TextureProcessor methods and parameter policy | Identity/version and input/output types are exact; registered `ImportAndProcess` executes ordinary Color, injected Dxt1 and pass-through paths. Validation distinguishes `UnknownName` from `UnconvertibleValue` structurally. Existing focused scenes cover color key, default/explicit premultiplication, mip generation, power-of-two resize and Reach/HiDef/CNB policy. Size/aspect/DXT alignment, wrong boxed types and every malformed spelling are boundary matrices for the CNA suite. |
| Texture2DContentWriter methods; RegisterTexture2DContentPipeline | Writer identity/input and the frozen Texture2D asset/schema/runtime-type/codec tuple are exact. A registry built only through the public registration function performs the real PNG import, processing and write twice with byte-identical CNB, then public decoding proves dimensions, one Rgba8 level and every source pixel. The injected encoder and DDS pass-through use that same registered processor. Existing runtime texture demos own GPU upload, sampling and visual output. |

All ordinary public `Texture2DContentPipeline.hpp` declarations now have direct evidence, a shared
workflow owner or an explicit boundary disposition. The new demo is CPU-only and uses synthetic
encoder output solely to expose the callback contract; it makes no visual-quality, GPU-format,
cross-platform decoder or genuine-XNA execution claim.

#### `CnbTextureFormat.hpp` declaration-level disposition

The pinned header exposes 36 public declarations: 28 frozen format values, the maximum identifier
and seven classification/size/mapping functions. Focused967 extends the stable
`content.texture_processor.pipeline_contract` demo because these persisted identifiers are the
codec and processor's shared format axis.

| Declaration family | Disposition |
|---|---|
| CnbTextureFormat values, CnbTextureFormatMax, IsKnownCnbTextureFormat | One loop visits every identifier 1 through 27, with explicit anchors at Unknown=0, Rgba8=1, Bc1=22 and Bc7Srgb=27; zero and 28 are rejected. This freezes the persisted range without one assertion/demo per spelling. The existing runtime `surface_formats`, `floating_formats` and `normalized_formats` scenes supply actual transfer/sampling behavior for the classic uncompressed families, while the XNB DXT scene supplies compressed runtime evidence. |
| CnbTextureFormatToString, IsBlockCompressedCnbTextureFormat, CnbTextureFormatUnitBytes | Every assigned value has a nonempty diagnostic name and nonzero unit size; exactly 22 through 27 classify as block formats. Individual diagnostic strings and all repeated unit values are table data, so the grouped exhaustive loop is stronger and smaller than format-specific demos. Unknown-format diagnostics and zero sizes remain error-boundary unit tests. |
| CnbTextureLevelByteSize | Exact controls cover 1x1 and partial-block BC7 rounding, an 8x8 BC1 level, an awkward 3x5 Rgba8 level and a 4x4x4 volume. Zero dimensions, unknown formats and 64-bit overflow are parser/allocator safety boundaries retained by CNA tests. |
| CnbTextureFormatToSurfaceFormat/SurfaceFormatToCnbTextureFormat | All 27 values round-trip through the runtime enum bijectively. Actual GPU support remains renderer-dependent and is separately queried/used by graphics format demos; the persisted mapping itself must not be mistaken for hardware support. |

All public `CnbTextureFormat.hpp` declarations now have exhaustive value/mapping evidence or a
named runtime/boundary owner. Only CNA's compiled renderer set was available; no claim is made that
every mapped format can be created on every backend.

#### `CnbTextureCodec.hpp` declaration-level disposition

The pinned header exposes 31 public declarations: nine persisted chunk/schema/limit constants, two
representation fields, six texture-data fields, two mip-colour-space values and twelve functions.
Focused967 grows the same stable texture contract from 14 to 24 controls.

| Declaration family | Disposition |
|---|---|
| CnbTextureChunk constants; schema/stride/face/limit constants | `TEXH`, `TEXR`, `TEXD`, schema 1, both 24-byte strides, six cube faces, sixteen mip levels and eight representations are asserted exactly. These are persisted grammar values and therefore grouped in one control. Ceiling-plus-one and malformed chunk-table matrices belong to the codec safety tests. |
| CnbTextureRepresentation and CnbTextureData fields | Default construction proves Unknown/empty representation state and 0x0x1, one-face, one-mip empty texture state. Real controls then populate every field: generated two-level 2D, six-face cube, depth-two volume and a three-representation selection carrier. Field-per-demo expansion would obscure the relationships the shapes enforce. |
| CnbTextureLevelDimensions, MakeRgba8Texture2DData | The helper constructs a validated 2x2 Rgba8 image, and level one reports exactly 1x1x1. A 4x2x2 volume reports a 2x1x1 child, proving depth halves with the other axes. The Texture2D pipeline demo's real PNG carrier supplies the larger ordinary case. Zero/mismatched inputs are explicitly codec-boundary tests. |
| CnbMipColorSpace, GenerateRgba8MipChain | The same half-black/half-white 2x2 source produces a stored-code linear average near 128 but an sRGB-light average near 188; alpha remains near 128 in both. Both chains terminate at 1x1 and the linear chain survives deterministic encode/decode exactly. Existing processor mip scenes retain rendered/runtime evidence; the two public generators intentionally have different filtering contracts. |
| Encode/Decode Texture2D | A generated two-level texture encodes twice byte-identically with exact built-in asset/schema/type/content metadata and decodes every field/level byte. The registered real-PNG build independently exercises the writer adapter. A payload one byte too long is refused before output. |
| Encode/Decode TextureCube | Six distinct 2x2 faces encode and decode in exact face-major order; a non-square description is refused. The CNJ/XNB pipeline demos separately retain real DDS cube bytes and the graphics cube demos own GPU sampling. Other face/mip-count corruptions are the same shape guard's test matrix. |
| Encode/Decode Texture3D | A synthetic 4x2x2 volume with an exact 2x1x1 child round-trips both levels byte-for-byte. The CNJ/XNB source demos separately preserve real authored volume bytes; native unsupported-3D policy and supported renderer uploads remain graphics responsibilities. |
| AppendEmbeddedTexture2DChunks/ReadEmbeddedTexture2DChunks | The two-level texture is embedded under a real SpriteFont asset identity through a public `CnbWriter`, parsed and recovered exactly. Existing SpriteFont content demos load/draw the user-facing font; duplicating glyph behavior here would mix schema concerns. |
| SelectCnbTextureRepresentation | A synthetic ordered BC7/Rgba8/Bgra8 set selects index zero when all are supported, index one when block formats are unavailable and `size()` when none match. CNB schema 1 currently writes one Rgba8 representation, while the XNB output demo supplies a real version-aware multi-representation consumer; this control deliberately tests the future-compatible ordering contract without claiming a schema-1 multi-format encoder. |

All public `CnbTextureCodec.hpp` declarations now have direct value/round-trip evidence, a shared
runtime owner or an explicit safety-boundary disposition. Synthetic cube/volume/selection data is
decoded on CPU; existing demos retain actual source import and GPU behavior, and no screenshot,
visual-quality or genuine-XNA container claim is added.

#### `CnbCurveCodec.hpp` declaration-level disposition

The pinned header exposes six public declarations: two schema chunk IDs, the schema version and key
stride, plus the encoder and decoder. Focused970 extends the stable `content.cnj.curve` workflow
because it already owns the authored CNJ, compiled CNB, runtime object and evaluation semantics.

| Declaration family | Disposition |
|---|---|
| CnbCurveChunk values, schema and key stride | `CRVH`, `CRVK`, schema 1 and the 20-byte five-field key row are asserted exactly. The real project-built asset has one of each schema chunk, the Curve asset identity, exact canonical type/logical name metadata and a direct decode equal to the `ContentManager` result. |
| EncodeCurveToCnb and DecodeCurveFromCnb | The real three-key bounce encodes twice byte-identically and restores both loop policies plus every position, value, tangent and continuity. A compact matrix round-trips all five persisted loop values and both continuity values; an empty Curve separately retains a valid zero-row key table and safe defaults. Wrong asset identity, future schema, missing/duplicate mandatory chunks, invalid loop and continuity ordinals, and a header/key-byte count mismatch are refused through independently constructed valid containers. Generic malformed table/checksum/limit cases remain the shared `CnbDocument` audit. |

All six public declarations now have direct native schema and round-trip evidence. Existing math
Curve demos retain detailed evaluation, tangent, collection and loop behavior; the XNB transcode
workflow retains a separate genuine-XNB source. This extension adds no screenshot, visual or
genuine-XNA execution claim.

#### `CnbAnimationClipCodec.hpp` declaration-level disposition

The pinned header exposes eleven public declarations: three schema chunk IDs, the schema version,
two table strides, three shared scalar/keyframe helpers and the complete encoder/decoder.
Focused971 extends `content.cnj.pipeline_assets`, whose real inline AnimationClip already owns the
registered source route and exact decoded runtime value.

| Declaration family | Disposition |
|---|---|
| Chunk IDs, schema and strides | `ACLH`, `ACLT`, `ACLK`, schema 1, the 12-byte track row and 48-byte key row are asserted exactly. A real pipeline result is re-encoded with exact canonical type/logical name metadata and one of each mandatory schema chunk. |
| ReadCnbSeconds | Direct bounded decoding preserves 1.25 seconds exactly and consumes the whole scalar. NaN, positive infinity and a truncated value are separately refused before `TimeSpan` construction; the full decoder exercises the same helper for duration and every key time. Negative finite time is retained by the keyframe helper rather than confused with a representation error. |
| WriteCnbKeyframe and ReadCnbKeyframe | A deliberately asymmetric key with negative time, signed translation, four distinct quaternion components and nonuniform scale writes exactly 48 bytes and restores every field with no unread bytes. Model and standalone AnimationClip schemas share these helpers, so a second embedded-model byte spelling would duplicate the same implementation rather than add behavior. |
| EncodeAnimationClipToCnb and DecodeAnimationClipFromCnb | The real two-key SceneNode clip encodes twice byte-identically and restores exact duration, target space, bone index and key fields. A second JointPalette clip preserves a populated track plus a distinct negative-index empty track; a wholly empty clip preserves its default target and zero tables. Wrong identity/version, missing/duplicate mandatory tables, invalid target space, mismatched table counts, an out-of-range key slice, and non-finite duration/key time are independently refused. Generic malformed container layout and array-budget saturation remain the shared document/read-limit audits. |

All eleven public declarations now have direct native helper/schema/round-trip evidence. Existing
model animation demos retain sampling, hierarchy, interpolation and rendered behavior; this CPU
codec audit adds no screenshot, visual-quality or genuine-XNA execution claim.

#### `CnbSpriteFontCodec.hpp` declaration-level disposition

The pinned header exposes 21 public declarations: five chunk identifiers, six schema/stride/limit
constants, eight decoded-data fields and two codec functions. Focused968 extends the stable
`content.cnj.sprite_font` workflow because it already owns the real authored font, runtime object,
text layout and rendered pixel evidence; a second codec-only catalog ID would repeat that concept.

| Declaration family | Disposition |
|---|---|
| CnbSpriteFontChunk values; schema/stride/limit constants | `FONT`, `GLYP`, `CROP`, `KERN` and `CHAR` are asserted as exact four-byte IDs alongside schema 1, the 24/16/12/4-byte strides and 65,536-glyph ceiling. Missing/duplicate chunks, bad flags, oversized counts and mismatched raw lengths are hostile-container matrices retained by CNA's codec tests. |
| CnbSpriteFontData fields | Default construction proves the safe empty atlas, four empty parallel tables, zero metrics and absent fallback. Decoding the committed eleven-glyph score font then verifies every character, source rectangle, crop, kerning triple, line spacing, spacing and fallback. Its embedded 204x32 atlas is byte-identical to the separately compiled Texture2D. |
| EncodeSpriteFontToCnb/DecodeSpriteFontFromCnb | The decoded real asset encodes twice byte-identically and the result restores the atlas and every table/metric exactly. A second round trip preserves an absent fallback independently of the first encoded character. Direct refusals cover a short parallel table, unsorted characters, an absent fallback target and a non-2D atlas. The C API font codec demo independently exercises the same native implementation through ownership, sizing and output-buffer boundaries; the XNB transcode demo retains a separate genuine-XNB source. |

All public `CnbSpriteFontCodec.hpp` declarations now have direct native value/round-trip evidence,
shared wrapper/runtime evidence or an explicit parser-boundary disposition. Both compiled renderers
load and draw the resulting font with matching programmatic pixel checks; no screenshot, manual
visual assessment or genuine-XNA execution claim is added.

#### `SpriteFontContentPipeline.hpp` declaration-level disposition

The pinned header exposes 44 public declarations under this audit's convention: one imported-type
identity, four style values, 12 aggregate fields/equality operators, eight free functions, four
importer methods, five description-processor methods, one font-sheet parameter identity, the
font-sheet helper, six font-sheet-processor methods and the registration function. Focused969 adds
the standalone `content.sprite_font.pipeline_contract` build-time workflow because this header
belongs to `CNA::ContentPipelineBuild`, not the runtime library linked into the main demo browser.

| Declaration family | Disposition |
|---|---|
| Imported identity, style values and description carriers | Safe defaults cover every field of `FontCharacterRegion` and `FontDescription`, all four exact style ordinals and both equality operators. Parsing the committed XML restores font name, size, spacing, kerning, style, fallback and its deliberately out-of-order regions exactly. Direct region expansion then sorts, deduplicates an overlap and inserts an out-of-region fallback. Malformed XML and the 8,192-glyph ceiling are refused. ISSUE-108 records that a directly constructed inverted region is silently ignored despite the expansion function's documented refusal; the built-in parser separately rejects that ordering. |
| Font discovery and rasterization | A committed, unmodified OFL Liberation Mono face makes the result independent of host-installed fonts. Explicit search directories round-trip, beside-family and configured-system searches resolve that exact file, and a missing family returns empty. Direct FreeType rasterization produces the exact sorted eight-character set, one Rgba8 atlas, complete parallel tables, valid packed geometry and authored metrics. Strict mode refuses a missing CJK glyph; XNA-compatible mode retains the notdef glyph with a warning. A requested unavailable bold-italic face is explicitly synthesized and warned. Builds without the optional rasterizer skip the tool rather than making a false runtime claim. |
| FontDescriptionImporter and FontDescriptionProcessor | Every identity, source/output/input type and ordinary-selection value is exact. Parameter validation accepts the empty description policy and returns structured `UnknownName` state for an extra value. A registry configured through the public registration function imports and processes the real `.spritefont`, records the TrueType face as a source dependency and reproduces direct rasterization. Two complete CNB builds have identical bytes/dependencies and decode to the same font; an XNB build selects DXT3/BC2 atlas data and the SpriteFontReader root. |
| FontTextureFirstCharacterParameter, BuildFontFromTextureSheet and FontTextureProcessor | Exact component identity/types and named-only selection accompany the accepted three-parameter surface. A 40-glyph synthetic magenta-separated sheet proves sequential characters and the distinct HiDef multiple-of-four versus Reach power-of-two atlas heights. Direct processor calls consume both string and integer first-character forms, preserve or premultiply exact RGBA values, and produce the complete expected BC2 block grid for XNB. Empty first-character and glyphless direct/processed sheets are refused. Existing bitmap SpriteFont demos retain runtime loading, measurement and drawing. |
| Registration, deployment and licensing boundaries | `RegisterSpriteFontSourceContentPipeline` supplies the importer and both processors; shared CNJ and XNB registrations supply the two writers used by the end-to-end builds. The tool runs in its own executable linked to `CNA::ContentPipelineBuild`, preserving the project's runtime-only browser boundary. The fixture's full SIL OFL 1.1 text, provenance, upstream/package origin and SHA-256 are committed beside it and listed in `THIRD_PARTY_NOTICES.md`. Host font enumeration permutations, shaping, arbitrary Unicode coverage, raster quality and platform-specific font backends remain integration/manual concerns rather than extra demo variants. |

All 44 public declarations now have direct behavior, shared runtime evidence or an explicit
integration/boundary disposition. The workflow establishes deterministic CPU conversion and
container semantics. It makes no screenshot, manual raster-quality, host-font portability or
genuine-XNA execution claim.

#### `CnbModelData.hpp` / `CnbModelCodec.hpp` declaration-level disposition

The pinned headers expose 35 public named declarations under this audit's convention: sixteen
data/type/bound declarations, eleven schema chunk IDs, six schema/stride/sentinel constants and
the encoder/decoder. Focused972 extends the stable `content.model.pipeline_contract`; its real
glTF builds retain the source-pipeline boundary while a direct complete value isolates schema-1
codec behavior without adding another catalog ID.

| Declaration family | Disposition |
|---|---|
| `CnbModelChunk` IDs; schema, strides and sentinel | All eleven four-byte IDs (`MDLH`, `MSTR`, `MBON`, `MMSH`, `MMAT`, `MVTX`, `MIDX`, `MMRP`, `MSKL`, `MANM`, `MLIT`) are asserted exactly with schema 1, the 72/16/56/368-byte fixed rows, `CnbNoIndex`, seven texture slots and effect maximum 5. The complete document has exact canonical metadata and expected singleton/repeated chunk cardinalities. |
| `CnbModelBone`, `CnbModelMesh`, `CnbModelPart`, `CnbEffectKind` | A two-bone parent-before-child hierarchy preserves names, indices and nonidentity transforms. One mesh retains its parent and ordered references to six parts. Those parts exercise every effect ordinal, external-effect reference, alternating vertex-colour/unlit flags, exact vertex/index bytes, stride/count/topology and derived primitive count. Direct refusals cover a later/self parent, bad geometry length, wrong primitive count and out-of-range mesh part. |
| `CnbMaterial`, `CnbTextureTransform`, `CnbSamplerState` | One fully populated material preserves all eight logical texture names, three factor arrays, seven scalar factors, alpha mode and double-sided state. Each of seven slots has a distinct coordinate set, five-field UV transform and four-field sampler record. Reuse by six parts also exercises encoded material interning without changing values. Runtime effect application and sampling remain the existing focused graphics/model demos. |
| `CnbMorphTarget`, `CnbMorphWeightKey`, `CnbMorphData` | Two parts distinguish cubic and step weight tracks. Sparse position/normal/tangent target streams, vertex count, flat-normal recomputation, default weights, finite key time and value/in/out-tangent arrays all round trip. A short delta stream is refused by the encoder. Actual blending, upload and animated rendering remain `graphics.model.morph_targets`. |
| `CnbModelSkeleton`, `CnbModelAnimation`, `CnbModelLight`, `CnbModelData` | The complete value preserves two-joint hierarchy plus bind, inverse-bind and optional root-prefix matrices; one named JointPalette clip and its exact key; two directional lights; and both model policy flags. The wholly empty model separately proves every optional collection/payload absent with safe defaults. Existing model demos own pose sampling, multiple skins, lighting and rendered behavior. |
| `EncodeModelToCnb`, `DecodeModelFromCnb` | Encoding the complete value twice is byte-identical and decoding restores every field above. Wrong asset identity/schema, missing or duplicate header, and missing string table are refused. ISSUE-109 records five public values the encoder currently accepts but its paired decoder refuses: negative parent sentinels below -1, an out-of-range decoded index, alpha mode 3 and coordinate set 2. The passing diagnostic keeps that asymmetry visible; arbitrary hostile chunk mutation and resource-limit saturation remain CNA test-suite work. |

All 35 public named declarations now have exact constant, field, round-trip, refusal or shared
runtime evidence. This audit establishes deterministic CPU/container behavior and does not add a
GPU, screenshot, visual-quality or genuine-XNA execution claim.

#### `CnbModelV2Data.hpp` / `CnbModelV2Codec.hpp` declaration-level disposition

The pinned headers expose 35 public named declarations under this audit's convention: twelve data
types, eleven chunk IDs, ten schema/stride/sentinel constants and the encoder/decoder. Focused973
extends the stable `content.xnb.pipeline_transcode` workflow because its committed twin-panel XNB
already selects schema 2 and proves the compatibility source route, shared buffers and distinct
effects. A complete direct value supplies the cases that one small real model does not contain.

| Declaration family | Disposition |
|---|---|
| `CnbModelV2Chunk` IDs; schema, strides and sentinel | All eleven exact IDs (`M2HD`, `M2ST`, `M2BN`, `M2MS`, `M2PT`, `M2VD`, `M2VR`, `MVTX`, `M2IR`, `MIDX`, `M2FX`) are frozen with schema 2, the 64/72/32/32/16/20/16/96-byte rows and `CnbModelV2NoIndex`. The complete document has one of every table chunk and two correctly aligned payload chunks of each resource kind. |
| `CnbModelV2VertexFormat`, `CnbModelV2VertexUsage`, element/declaration/buffer types | Exact contiguous ordinals cover all twelve formats and thirteen usages. Two declarations retain authored element order, offsets, usage indices and strides; two vertex resources preserve declaration indices, counts and every byte. Two index resources distinguish 16-bit and 32-bit widths. The public defaulted equality operators compare the complete nested model after decode. Invalid overlap, payload length and selected index are refused by the shared validator. |
| Effect kind and effect data | One complete model includes Basic, Skinned, DualTexture, AlphaTest and EnvironmentMap payloads with only their legal active fields: typed 2D/cube references, vector/scalar values, weights per vertex, alpha comparison/reference and vertex-colour flags. All five ordinals and every active field survive exactly; a noncanonical inactive DualTexture emissive value is refused. Existing graphics demos retain GPU effect behavior. |
| Bone, part, mesh and model data | Two parent-before-child bones preserve names/transforms and explicit root; two bounded meshes carry six contiguous draw records over shared resources and exact spheres. The committed real XNB-derived model separately re-encodes byte-identically with its five-bone/two-mesh shared-resource graph. A graph-only root is the valid minimum. Invalid root/resource references and negative sphere radius are refused. `nameIsNull` is explicitly in-memory-only: the audit proves a null name becomes the documented empty name after CNB while default equality still distinguishes the two carriers. |
| `EncodeModelV2ToCnb`, `DecodeModelV2FromCnb` | Both the real XNB-derived value and the exhaustive synthetic value encode twice byte-identically, carry exact metadata and round trip through complete public equality. Wrong asset identity/version and missing/duplicate headers are refused. Encoder and decoder both call the same validation function; unlike schema 1, no producer/consumer mismatch was found. Arbitrary reserved-word/chunk mutation and configured-limit saturation remain CNA test-suite work. |

All 35 public named declarations now have exact constant, enum, complete-value, equality,
round-trip, refusal or shared runtime evidence. The source is a committed genuine XNB fixture, but
the audit makes no genuine-XNA execution, GPU, screenshot or visual-quality claim.

#### `CnbModelFromCnj.hpp` declaration-level disposition

The pinned header exposes six public declarations: the result aggregate, its three fields and two
path overloads. Focused974 extends `content.model.pipeline_contract`, which already owns the real
glTF-to-staged-CNJ route and schema-1 encoder, so the front-end seam can be observed directly
without a fabricated second Model workflow.

| Declaration family | Disposition |
|---|---|
| `CnbModelFromCnjResult::model` | Directly reading the importer's live staged document produces populated parts and meshes that can be encoded by `EncodeModelToCnb`. The existing registered build independently processes the same staging output and remaps its texture child. Minimal version-1 and version-2 documents synthesize the documented identity root and preserve their distinct XNA versus glTF lighting policies. |
| `absorbedFiles` and `externalReferences` | Every returned absorbed geometry sidecar resolves to a real file inside the staging root, while the generated PNG remains exactly one external logical asset. The lists agree across both overloads. The original CNJ is intentionally absent from the absorbed list; pipeline dependency collection owns that primary source separately. |
| Filesystem and narrow-string `BuildCnbModelFromCnj` overloads | Both overloads read the same real importer-produced document and return byte-identical encodable models plus equal file lists. The filesystem overload remains the Unicode-safe primary API; this Linux ASCII-path run establishes compatibility delegation, not Windows Unicode behavior. |
| Refusal behavior | Direct files independently prove refusal of the wrong asset type, unsupported Model `sourceFile` indirection, material variants that schema 1 cannot retain, a missing mesh table and a sidecar path escaping the content root. Arbitrary JSON/type/sidecar corruption remains CNA parser-test work. |

Every declaration now has direct value, overload, file-classification or refusal evidence. The
front end is CPU/filesystem-only and adds no GPU, rendering, screenshot or genuine-XNA claim.

#### `CnjToCnb.hpp` declaration-level disposition

The pinned header exposes seven public declarations under this audit's convention: the result
aggregate, its five fields and `CompileCnjToCnb`. Focused975 extends
`content.cnj.pipeline_assets`, which already owns the registered Texture3D, TextureCube and
AnimationClip source routes and the codecs used to inspect their output.

| Declaration family | Disposition |
|---|---|
| `CnjToCnbResult::bytes`, `assetTypeId` and `assetTypeName` | One matrix directly compiles Curve, AnimationClip, Model, Texture2D, Texture3D, TextureCube, SpriteFont and SoundEffect documents. Every result has nonempty bytes, the exact built-in asset ID and canonical type name, and a parsed container carrying those same values plus the requested logical name. Public decoders recover the five routes newly introduced in this control with exact curve, geometry, image, font and PCM values. |
| `absorbedFiles` and `externalReferences` | Every result lists its CNJ first. Texture/image/font/sound sidecars and the Model's vertex/index payloads follow under their authored names. Only the Model's shared texture remains external, proving that the two lists describe distinct deployment outcomes rather than duplicate dependency inventories. |
| `CompileCnjToCnb` all-eight dispatch | Direct output for the three extended types is byte-identical to the existing registered pipeline output at the same logical name. Calling the function twice with omitted root/name uses the CNJ parent and stem and remains byte-deterministic. A representative unsupported type, non-Model version 2, missing image and root-escaping sidecar are independently refused. Type-specific codec relationship and hostile-container matrices remain their focused audits. |

All seven declarations now have direct value, dispatch, default, file-classification or refusal
evidence. This is deterministic CPU/filesystem compilation; existing demos retain runtime loading,
GPU/audio construction and rendered/playback behavior. No screenshot, visual-quality, listening,
Windows Unicode-path or genuine-XNA execution claim is added.

#### `CnbSourceImport.hpp` declaration-level disposition

The pinned header exposes 16 public declarations under this audit's convention: the image-options
aggregate and field, twelve function overloads and the two `SoundEffectLoopPolicy` values.
Focused976 extends the three stable workflows that already own the corresponding source and codec
behavior instead of adding an importer-only catalog ID.

| Declaration family | Disposition |
|---|---|
| `CnbImageImportOptions`; both `ImportImageAsCnbTexture2D` overloads | Safe defaults carry no color key. Native-path and narrow-string calls on the committed 64x64 checker are field/byte-identical to each other and to the public pipeline image decoder. An explicit key preserves every RGB byte and every nonmatching alpha while clearing all matching opaque pixels. A missing file is refused. The existing registered pipeline retains PNG gamma, processing and deterministic CNB evidence; the C importer demo independently uses a one-pixel BMP for matching and nonmatching keys. |
| `DecodeDdsAsCnbTextureCube`; both `ImportDdsAsCnbTextureCube` overloads | Memory, native-path and narrow-string routes over the committed DXT1 cube all produce the same 4x4, six-face, one-mip Rgba8 data as the registered CNJ pipeline. Truncated memory and a missing file are refused. The C importer demo additionally owns caller-buffer lifetime, non-cube/malformed cases and exact zero-block decompression; graphics demos retain actual GPU upload and sampling. |
| `DecodeWavAsImportedSound`; both `ImportWavAsImportedSound` overloads | A committed mono 16-bit WAV returns identical source-oriented encoding, rate, frame/loop counts and bytes through memory, native-path and narrow-string routes. The same demo already covers safe `ImportedSound` defaults and every supported source encoding's width/name/narrowing policy. |
| `ProcessImportedSoundEffect`; both policy values; `DecodeWavAsCnbSoundEffect`; both `ImportWavAsCnbSoundEffect` overloads | `AsAuthored` preserves the absent loop while `WholeSoundWhenUnset` assigns every frame. The compatibility memory decoder and both file overloads exactly equal the explicit whole-sound result. Direct synthetic values retain 8/16-bit bytes, deterministically narrow 24/32-bit integer and float data, and refuse short/unknown input. Truncated WAV memory and a missing path are refused; the C importer workflow separately proves a parsed PCM8 `smpl` loop plus compressed/malformed bounds. |

All 16 declarations now have direct value, overload, option/policy or refusal evidence, with deeper
shared C-wrapper cases explicitly owned by `c_api.content.cnb_importers`. These are headless CPU
imports. No graphics/audio device, rendering, playback, screenshot, visual/listening quality,
Windows Unicode-path or arbitrary decoder-format portability claim is added.

#### `CnbFormat.hpp` declaration-level disposition

The pinned header exposes 45 public declarations under this audit's convention: nine container
constants, the chunk-ID carrier/value/equality plus three helpers, three flag constants, seven
compression values/aliases, fourteen asset-type constants, four asset/logical-name helpers and two
container chunk IDs. Focused977 extends `content.custom.cnb_loader`, where these wire identities
already govern a real game-defined schema and runtime dispatch.

| Declaration family | Disposition |
|---|---|
| Floating representation and `Format` constants | The header's compile-time assertions require binary32/binary64 representation and byte spelling. Runtime controls freeze `CNB\x1A`, 64-byte header, 48-byte TOC row, checksum coverage/offset 44, 16 reserved bytes, container 1.0 and TOC offset 64. The existing writer/parser round trip proves those constants describe accepted bytes. |
| `CnbChunkId`, `MakeChunkId`, string/well-formed helpers | `RULE` packs as exact little-endian `0x454C5552`, compares equal and renders literally. A deliberately malformed four-byte value is rejected and renders control/non-ASCII bytes as `?`, preserving safe diagnostics. Existing schema chunks prove the same values through writer, parser and mandatory-chunk dispatch. |
| `CnbChunkFlags`; `CnbCompression` values and aliases | None, Mandatory and KnownMask retain exact 0/1 values. None/Lz4/Zstd/Deflate retain 0..3 and all three former Reserved aliases equal their assigned codecs. Actual compression capability, stored/logical sizes and expansion budgets remain the focused read-limit workflow rather than being inferred from identifiers. |
| `CnbAssetTypeId` constants and helpers | Invalid and all eleven assigned built-ins retain exact 0..11 values and diagnostic names; reserved/custom ranges begin at `0x40000000`/`0x80000000`, with boundary classification and unknown/custom hexadecimal rendering. `CnaDemos.GameRules` hashes deterministically to `0xE7632F12`, while an empty canonical name is refused. The existing collision pair proves why full metadata identity remains required after hashing. |
| `CnbLogicalNameProblem`; `CnbContainerChunk` IDs | Contained and embedded-dot names pass. Empty, malformed UTF-8, backslash, absolute, drive-qualified and parent-segment forms receive their documented classifications. Existing XREF writing independently proves enforcement. `CMET` and `XREF` IDs are exact, and the writer/parser workflows prove their singleton metadata/reference roles. |

All 45 declarations have exact constant, value, helper, enforcement or compile-time evidence. This
is a byte/container contract audit; it adds no claim about a platform whose compiler fails the
header's IEEE assertions or about an unavailable compression implementation.

#### CNB primitive arithmetic, byte cursor/writer and CRC disposition

Focused978 extends `content.custom.cnb_read_limits`, whose accepted and corrupt custom containers
already consume this primitive layer. It dispositions both public `CnbArithmetic.hpp` helpers, the
complete public `CnbByteWriter.hpp` and `CnbByteReader.hpp` surfaces, and all five public
`CnbCrc32c.hpp` functions.

| Declaration family | Disposition |
|---|---|
| `CheckedAdd`, `CheckedMultiply` | Exact operations reach `UINT64_MAX`, multiplication by zero remains zero, and one-past addition plus doubled maximum multiplication throw before wraparound. Existing document/count parsing supplies real file-declared uses. |
| `CnbByteWriter` constructors, writes and buffer accessors | A writer starting with one byte appends U8/U16/U32/U64/I32, binary32/binary64, a two-width UTF-8 string, raw bytes and zeros into one asserted 44-byte little-endian image. `Size` and `View` expose that image; `Take` transfers it exactly and leaves an empty observable buffer. Malformed UTF-8 is refused before output. |
| `CnbByteReader` construction, state and reads | One bounded cursor restores every value from those exact bytes, reports context/size/position/remaining state, returns a raw span, skips the zero tail and requires exhaustion. Static UTF-8 validation accepts one-through-four-byte sequences and rejects overlong, surrogate, truncated and above-U+10FFFF forms. Truncation and direct `Fail` retain context and exact cursor offset. Existing schema controls separately exercise `ReadCount`, bounded `ReadString` and trailing-byte refusal under copied limits. |
| `Crc32c`, incremental/seed, portable and hardware-path accessor | Empty input is zero and `123456789` produces standard CRC-32C `0xE3069283`; two-span continuation equals the one-shot value and the portable reference. The runtime reports whether this host selected hardware. The generic writer's stored header checksum equals a direct CRC over exactly the public 44-byte coverage, while the existing one-byte mutation is refused by the parser. |

The full primitive public surface now has exact byte/value, cursor, safety-boundary or real-container
evidence. Hardware and portable CRC results are compared on this host; no performance claim or
execution on the alternate host path is made.

#### CNB limits, compression, document and writer disposition

Focused979 completes the remaining generic container audit across `CnbReadLimits.hpp`,
`CnbChunkCompression.hpp`, `CnbDocument.hpp` and `CnbWriter.hpp`. It extends the same
`content.custom.cnb_read_limits` workflow, retaining the existing custom loader and external-
reference demos for file I/O, registry dispatch and mandatory-chunk behavior.

| Declaration family | Disposition |
|---|---|
| `CnbReadLimits`, `DefaultCnbReadLimits` | Every public field has its exact default: 512 MiB file, 65,536 chunks, 384 MiB per chunk, 1 GiB aggregate decoded bytes, 1 MiB strings, 16 Mi elements and 4096-byte alignment. Existing small injected limits prove symmetric reader/writer enforcement. Writer and document controls now also prove that each stores its own copy rather than retaining the caller's mutable object. |
| Compression helpers | Every codec name and implemented/unsupported result is exact. `None` copies losslessly. When Zstandard is present, a compressible 4096-byte value shrinks, restores byte-exactly and refuses both a wrong decoded size and an undersized ceiling; LZ4 and Deflate are refused in this build. ISSUE-110 records that the direct `None` decompression branch returns before the two checks promised by its public contract. The ordinary document path performs those structural checks earlier and remains bounded. |
| `CnbChunkEntry`, `CnbExternalReference`, `CnbMetadata` | Default values are safe and exact. Real GRID/NOTE entries expose expected IDs, flags, offsets, stored/logical sizes, CRCs, compression, alignment and mandatory classification. Parsed CMET identity/provenance is exact and an absent XREF is observably empty; the focused external-reference workflow owns populated fields and indexed lookup. |
| `CnbDocument` | Parse/HasMagic, chunk count/data/cursors, lookup, required asset/mandatory chunks, metadata/references and corruption/limit behavior already run against accepted and damaged images. The extension adds exact origin, container versions, asset identity/schema, all remaining TOC accessors, present/absent `FindSingle`, copied limits and every out-of-range accessor. Existing custom demos retain `ParseFile`, populated XREF and duplicate/unknown-chunk cases. |
| `CnbWriter` | Constructor validation, default/current limits and schema-only chunk count are direct. Metadata, references, chunk addition, compression, building and file writing retain real round trips. New controls refuse malformed IDs, unknown flags, non-power-of-two/excessive alignments and container-owned IDs, and repeated `Build` with copied limits is byte-identical. Existing custom-type collision, unsafe reference and budget controls retain deeper writer/parser symmetry evidence. |

All public declarations in these four headers now have direct state, round-trip, refusal or named
shared-workflow evidence. The compression checks describe this build's available codecs only; they
make no portability, throughput or alternate-build claim.

#### `CnbLoaderRegistry.hpp` declaration-level disposition

Focused980 extends the own-process `content.custom.cnb_loader` workflow from 32 to 42 controls and
dispositions all ten public declarations. The existing end-to-end path still owns typed
`ContentManager` loading, cache behavior, concurrent first-wins registration and a genuine 31-bit
name collision; the added controls expose the underlying registry contract directly.

| Declaration family | Disposition |
|---|---|
| `CnbLoaderRegistry`, `LoaderFn`, `Register` | A direct loader consumes the parsed document, manager reference and exact logical asset name, then returns the expected boxed type. Equivalent repeat registration retains the first callable. Invalid, built-in, reserved, unnamed, differently hashed and empty-loader registrations are refused. Two distinct canonical names with the same valid custom ID exercise the separate conflicting-name refusal. |
| `IsRegistered`, `Find`, `RegisteredTypeName` | A real `ContentManager` declares all ten built-in IDs with exact canonical names, and each lookup yields a callable copy. Custom registration reports the same exact state. The existing eight-thread registration gate retains one capture; direct lookup followed by removal proves the returned function remains valid independently of later table mutation. |
| `Remove`, `Clear` | First removal reports true, repeated removal false, and every query reports absence afterward. `Clear` removes both custom entries and declared built-ins in the isolated child process, including a false removal from the empty state. A copied loader remains callable after its table entry is gone. |
| `ResolveForDocument` | A matching custom document resolves and decodes through both the ContentManager-installed wrapper and a direct loader. Missing registration and a real colliding full name are refused before the wrong loader executes. For a built-in type, the numeric ID remains authoritative even when optional CMET text differs. |
| `RegisterBuiltIns` | After `Clear`, two calls idempotently install exactly the documented device-independent Curve and AnimationClip loaders, with exact names and lookup functions. Texture2D remains absent because the other eight loaders belong to `ContentManager`; direct built-in removal has the same true/false lifecycle. |

Every public declaration now has direct state, lifetime, identity or refusal evidence. The demo's
own-process boundary isolates the intentionally destructive global-registry reset. No claim is made
about private registration entry points or thread-scheduler ordering.

#### `ModelContentPipeline.hpp` declaration-level disposition

The pinned header exposes 33 public declarations: two variant aliases, three stable identities,
nine imported-document fields, two child fields, three bundle fields, four importer methods, five
processor methods, four writer methods and the registration function. Focused966 adds
`content.model.pipeline_contract` for the build-time carrier/component boundary. The existing 30
graphics/model demos retain model interpretation, animation, material, GPU and pixel evidence.

| Declaration family | Disposition |
|---|---|
| CanonicalModelValue; ImportedModelDocumentType, ProcessedModelType, ModelGenerateChildAssetsParameter | The exact two carrier identities and child-policy spelling are frozen. A real glTF produces the schema-1 alternative; `content.xnb.pipeline_transcode` separately proves schema-2 selection on the twin-panel XNB model. The processor accepts a bool child policy and distinguishes an unknown name from a mistyped value. Repeating true/false spelling through more source shapes adds policy permutations rather than a new concept. |
| ImportedModelDocument staging fields | A direct public importer invocation produces one canonical Model CNJ, one generated embedded texture, the source-stem prefix, a common contained intermediate root and a live opaque owner. It has no additional model/animation document, authored-sidecar flag or canonical XNB value for this source. Destroying the carrier removes the whole staging tree, directly proving the ownership field rather than only observing successful processing. Authored Model CNJ sets `recordAuthoredSidecars`; the shared dependency collector behavior is already demonstrated by source-root/nested-build workflows, so directly toggling this inter-component flag would not add an author-facing route. |
| ImportedModelDocument::canonicalModel | The glTF route deliberately leaves it empty and stages CNJ. The nine-root XNB transcode demo supplies the alternative directly, selects schema 2 and verifies the complete hierarchy/shared geometry/effects after native decoding. Schema-1 XNB conversion is the same carrier alternative and existing runtime/source model workflows own it. |
| ProcessedModelChildValue/ProcessedModelChild | The embedded-texture source produces a deterministically named Texture2D child whose value reaches a decodable additional CNB and a typed runtime reference. The animated slide-rig source retains its embedded animation and emits a separately decodable AnimationClip child. The remaining Model alternative belongs to multi-model glTF splitting; CNA's integration test proves it, but this repository has no authoring fixture that intentionally splits into multiple models. It remains explicit integration-test evidence rather than a fabricated aggregate-only demo claim. |
| ProcessedModelBundle fields | Real processing yields a schema-1 primary, no XNA-shaped carrier and one sorted child. The XNA `.x` source workflow behind `graphics.model.xnb_model` supplies the `xnaModel` fidelity path to the XNB writer, while the XNB-to-CNB demo supplies the canonical schema-2 primary. An absent primary is the deliberate EffectMaterialContent CNB refusal and is best retained as a writer boundary test until such a project source is added. |
| GltfImporter methods | Identity/version, `.gltf`/`.glb` routes and sole output identity are exact; direct `Import` executes the shared converter against the committed embedded-texture glTF. `.glb` changes container packaging, not the importer/component contract, and existing CNA parity fixtures cover both spellings. Source-side buffers, images, unicode, invalid accessors and glTF extensions already have focused model demos or CNA parser tests. |
| ModelProcessor methods | Identity/version and input/output identities are exact. Registered `ImportAndProcess` consumes the owned staging documents, remaps the generated texture reference, creates the primary/child variants and reports typed runtime references. A second real source creates the animation child. Multi-model collisions, output ceilings, staging escapes and malformed generated CNJ are safety/integration matrices rather than additional demonstrations. |
| ModelContentWriter methods; RegisterModelContentPipeline | The writer declares the exact sorted Texture2D, Model schema-1, Model schema-2 and AnimationClip asset/schema/type/codec tuples. A registry configured only through the public function performs real import, processing and writing twice; primary and child bytes, dependencies and runtime references are identical. Public decoders recover a nonempty Model whose material names the child, the exact Texture2D child and the standalone AnimationClip. Existing model scenes own runtime loading and drawing. |

All public `ModelContentPipeline.hpp` declarations now have direct evidence, a named shared-workflow
owner or an explicit intentional disposition. This checkpoint performs CPU conversion and CNB
decoding only; it adds no GPU, animation-playback, screenshot, visual-quality, external-tool or
genuine-XNA execution claim.

### Runtime XNB readers (separate from intermediate XML)

`content.xnb.custom_type_reader` supplies the game's LevelReader and original generated XNB.
The stock XNB demos load framework assets. The custom reader now directly covers the useful fixed
scalar/math helpers, while object graphs and container orchestration remain separate decisions.

| Members / behavior | Decision and evidence |
|---|---|
| ContentTypeReader<T> construction/Read; AddTypeCreator | LevelReader is registered under the exact reader-table name and reads every field in the authored order. Name, rows and gem count are checked. The example counts Read calls: first Load invokes the reader once and second Load returns equal rows without another invocation. The extended workflow passes10 checks on both renderers. |
| ContentReader AssetName/ContentManager; version/platform extensions | The loader passes the logical asset name and owning manager to the reader. LevelReader now observes both, and the demo checks the exact logical name and owner pointer after loading. Container version/platform inspection is routine and does not deserve a separate demo. |
| ReadSingle/Double/Color/Matrix/Quaternion/Vector2/Vector3/Vector4/BoundingSphere | The generated Level XNB carries one coherent authored state block after its map: exact single/double scalars, nontrivial RGBA bytes, all16 row-major matrix elements, quaternion XYZW and every component of Vector2/3/4. Focused930 appends a game-level encounter volume and calls public `ReadBoundingSphere` directly, preserving center `(6.5,-7.5,8.5)` and radius `9.25`. This covers the useful math helpers in one schema without scalar-read-per-demo expansion. Evidence:`/tmp/checkpoint-648-content-reader-*` and `/tmp/checkpoint-930-content-sphere-*`. |
| ReadBytesExactOrThrow and inherited BinaryReader primitives | `content.content_manager.read_asset_ownership` now reads its real variable-length tile payload through `ReadBytesExactOrThrow`. A separate authored XNB declares nine bytes but ends after eight; the helper raises `EndOfStreamException` naming the reader before constructing or recording a resource. The normal aliased payload retains its exact bytes. Existing schemas directly use inherited `ReadString`, `ReadInt32`, `ReadByte`, scalar and math operations; individual controls for every integer width would only repeat BinaryReader encoding mechanics. Evidence:`/tmp/checkpoint-931-content-exact-*`. |
| CheckCollectionElementCount, CheckDecodedByteSize | The same game-owned Level/Tiles readers apply CNA's public untrusted-size guards before their row loop and byte allocation. Original XNBs declare10,000,001 rows and268,435,457 decoded bytes; actual `ContentManager::ReadAsset` paths raise contextual `ContentLoadException` before iteration, allocation or resource recording. The valid zero-row level and eight-byte tile paths remain intact. Evidence:`/tmp/checkpoint-932-content-limits-*`. |
| ReadObject overloads, ReadRawObject and type-erased ReadObject | shared_resources checks a separately allocated inline Item. existing_objects now checks indexed and explicit-reader raw payloads filling existing slots without replacing identity or runtime-only fields, zero-reference fallback, a fresh std::any weapon tag, safe wrong-type probing, empty tags and exact trailer alignment. Eleven behavior checks pass under both renderer selections; two more expose GetTypeReader and implicit ReadRawObject<T>(existing) ISSUE-043 (13 checks total). Actual automatic raw lookup fails without advancing the stream; only then does the explicit named-reader fallback allow the remaining payload example to run. No hidden target-name alias is registered; disposable recording and every overload remain separate decisions. |
| ReadSharedResource/ReadSharedResources | content.xnb.shared_resources executes 17 console checks under both renderer selections: sixteen pass for deferred root callbacks, all objects read before fixups, alias identity, forward and weak backward edges, zero-index callback omission, independent inline objects/cache/manager graphs, invalid index/type refusal, released lifetime and supplied-recorder delivery of the typed inline disposable. The seventeenth fails as ISSUE-087 because both IDisposable shared-table values bypass that recorder. Callback captures own stable objects; the back edge is weak. ReadSharedResources orchestration remains owned by ContentManager, not invoked manually inside the root reader. All three custom readers are explicitly registered; Game startup normally registers built-ins, and a bare console manager does not. No GPU/visual validation or general container fuzzing is claimed. |
| ReadExternalReference | external_references passes 11 checks per renderer: relative sibling PNG via the supported Texture2D template, compiled custom label via std::any, empty references, slash/backslash normalization, typed owner-cache sharing with direct Load and erased-cache sharing across different parents. Partial texture edits render through both parents; inspected screenshots are RGBA-identical. Absolute/above-root paths and missing children are refused. The erased cache is a separate route, not a claim of shared entries with typed Load of the same custom type. Only Texture2D, TextureCube and shared_ptr<Effect> have instantiated template forms in current source; custom labels use the non-template route. Path hardening is CNA behavior, not measured genuine-XNA behavior. |
| TypeVersion/SupportsVersion, Initialize/GetTypeReader | existing_objects verifies that both table readers are constructed before root Initialize and that each table reader is initialized once. Its distinct target/factory names expose GetTypeReader returning null (ISSUE-043); a named factory produces a correct independent stateless reader for the remaining example. Current returned readers are fresh instances, so do not assume per-file initialized state is reused. reader_versions passes 17 checks under both renderer selections without a display: an actual v0 Health-only record retains default Armor, v1 carries Armor, and v2 is refused before Read. The initial base equality policy rejects v0; a replacement explicitly accepts v0/v1. Payload flags select the supported fields; SupportsVersion is a pure predicate and XNB container version remains five. This does not imply arbitrary schema migration or container compatibility. |
| AddTypeCreator repeated registration, RemoveTypeCreatorEXT, CreateReader/IsRegistered | The public contract keeps the first factory for a repeated key and allows single-key withdrawal. reader_versions checks duplicate registration preserves the original strict policy, rejected captures are released, single-key withdrawal releases its owned immutable policy, an already-created reader survives, cached assets remain usable, uncached loads fail until re-registration, and scoped cleanup releases the replacement policy. Weak pointers establish data ownership; this does not demonstrate unloading dynamically linked reader code. ClearTypeCreators/ClearTargetTypeAssociationsEXT are test-isolation helpers and must not erase unrelated registrations in the browser. |
| Declared built-in readers, withdrawal across loads and a later Game | reader_versions now retains all17custom-policy checks and adds six built-in lifetime controls (23passing checks in both requested renderer environments, empty stderr). A real signed Int32 XNB first loads under a Game declaration; withdrawing its factory removes direct construction while preserving the cached value. An uncached manager and the original manager after Unload both refuse the asset without silently reinstalling its reader. A subsequent Game permits a fresh load BEFORE a registry query could materialize declarations. Console ownProcess keeps this built-in mutation out of the parent/browser registry; no Internal helpers, blanket clearing or replacement of an unknown caller factory is used. The new deterministic90byte fixture is staged identically and all three old fixtures remain byte-identical. The two Games never Run: no rendering/visual or hardware credit. Constructor declarations, query materialization and ordinary custom-key withdrawal remain distinct behaviors; this example does not establish concurrent registry changes or unloadable reader code. |
| InitializeTypeReaders/ReadAsset, constructor limits | Public integration surfaces for the container loader/custom readers. Independent low-level container orchestration is intentionally deferred; ordinary demos should let ContentManager own parsing. Focused931-932 cover the application-facing exact-byte and size guards through real custom readers; arbitrary limit injection and whole-container fuzzing remain CNA test-suite work. |

### Intermediate XML serialization (member and behavior decisions)

The first tool deliberately demonstrates one small authored encounter, not the whole serializer.
The following finer decisions come from the public IntermediateSerializer/Reader/Writer and
ContentTypeDescription headers. Source inspection is not runtime coverage of the pending rows.

| Members / behavior | Decision and evidence |
|---|---|
| Serialize/Deserialize, Field, Format/ElementName, member CollectionItemName | The existing tool checks emitted HitPoints/Drop names and round-trips the encounter. Equivalent fluent ElementName spelling needs no new demo; the explicit public format object keeps ContentSerializerAttribute visible. One reusable Optional format deliberately omits ElementName; a present integer collection emits under its own described Patrol name, proving Format preserves the binding's fallback rather than sharing or erasing it. |
| Optional, AllowNull, default values | The tool omits an empty optional hint and observes a nonempty reader constructor default, then checks a present value replacing that default and explicit Null clearing it. Missing required HitPoints is rejected despite its scalar default. An Objective with a nullable C++ carrier and AllowNull(false) round-trips an authored value while explicit Null is refused on read and write. The empty Optional Reinforcements collection is omitted and deserialization preserves its nonempty constructor default, while a present Patrol collection round-trips. Separately, the custom DiceRoll serializer's typed ObjectIsEmpty override marks count zero as empty: an invalid zero-count optional field is omitted without invoking Serialize, while a valid nonempty bonus emits compact text and round-trips. These cases do not establish every optional/null rule. |
| Ignore | The tool verifies omission and rejects an injected ignored field. This is descriptor behavior; it does not instantiate the separate ContentSerializerIgnoreAttribute wrapper. Ordinary constructor defaults versus serialized runtime-only state are meaningful and already checked. |
| FlattenContent; collection-level CollectionItemName | `content.custom.intermediate_properties` emits nested SpawnPoint X/Y members without a Spawn wrapper and round-trips their values. Source audit at clean CNA d23e139de finds that type-level ContentTypeDescriptor::CollectionItemName is stored, exposed and inherited by BaseType when the derived value is empty, but no content/content-pipeline implementation consumes ContentTypeSerializerBase::CollectionItemName. A round-trip therefore cannot observe either the value or inheritance. It is dispositioned as metadata awaiting a real consumer; direct getter assertions would not demonstrate serialization behavior. Member-level item naming remains meaningfully covered by the Drop/Room XML. |
| Property, ReadOnlyProperty, BaseType | `content.custom.intermediate_properties` now checks one concrete DungeonConfiguration: the inherited Name is emitted first even when BaseType is declared after derived bindings; Difficulty loads through its setter exactly once and the setter rejects an invalid value; get-only Rooms keeps its constructor Entrance and appends Shop/Arena from XML (5 child checks overall, including flattening). This demonstrates append semantics for std::vector, not iterator stability or every managed collection carrier. The separate intermediate_polymorphism tool now demonstrates base-typed dynamic loading; these property checks do not imply it. |
| SharedResource; WriteSharedResource/ReadSharedResource | `content.custom.intermediate_shared_resources` checks a Resources section, forward fixups, preserved alias identity, separate equal-valued tables and independent inline copies; mutating one loaded alias affects only its shared partner. It verifies a null shared table, missing/duplicate IDs and incompatible resource types. Two LootTable resources then form a cyclic fallback graph with exact loaded forward/backward closure. A root LootTable that points to itself follows measured XNA behavior: XML emits the root again as one resource, deserialization yields a distinct root and resource copy, and the self-cycle closes on the resource. Scoped unlinking and weak handles verify that all authored and loaded cycles release. A non-shared self-cycle reaches the writer's public 1024-level guard and is refused with an actionable diagnostic (20 child checks total under both displayless renderer selections). The descriptor invokes Reader/Writer internally; their direct custom-serializer forms are not independently credited. XML support does not make C++ reference cycles self-releasing. |
| ExternalReference, WriteExternalReference/ReadExternalReference, referenceRelocationPath | `intermediate_external_references` now distinguishes filename references from an embedded preview, deduplicates identical target/file entries, relocates relative paths, preserves separate wrapper identity and nulls, and refuses missing location/ID/incompatible type. Nine child checks pass under both displayless selections. Its tenth check exposes native emitted TargetType aliases not resolving on read (ISSUE-046); original XML/failure stay visible and independent relocation controls explicitly use canonical target names. The builtin ExternalReference serializer invokes Reader/Writer internally, so their direct custom-serializer calls are not separately credited. This XML tool records filenames only; the separate nested_builds tool now exercises imports, dependencies, child compilation and caller publication. Those capabilities are not implied by XML fixups. The separate xml_xnb_pipeline now demonstrates a simple custom root through XNB, but does not establish external-reference graph compilation. |
| AddTypeSerializer, TypeSerializerFor, GetTypeSerializer and registry lookup helpers | Described types register automatically in the current process. `content.custom.intermediate_type_serializer` registers a compact NdM dice serializer and proves that the returned and looked-up instance reports the exact DiceRoll `TargetType` and `dice` `XmlTypeName` used by its root XML and round trip. Dispatch from a described Attack field reaches the same serializer. A custom typed envelope obtains nested codecs from the active writer/reader Serializer context, emits dynamic type names and resolves them through ReadTypeName. Its ScanChildren override reports an Attack through the object serializer, causing the namespace prepass to declare `xmlns:CnaDemos` before WriteTypeName emits the nested custom type; both payloads round-trip. MoveToElement plus the reader Xml surface support exact refusals for absent/missing/unknown/unrelated typed payloads. The DiceRoll ObjectIsEmpty override additionally controls optional omission. The documented Initialize callback never runs, exposing ISSUE-089, so the diagnostic serializer must fall back to a lookup during Serialize. Re-registering a target preserves the first serializer, while a distinct type's colliding `dice` shortcut still exposes ISSUE-088. Eighteen of twenty child checks pass. Ordinary spelling-only lookup variants need no extra checks. |
| ReadObject/WriteObject versus ReadRawObject/WriteRawObject; existing instance overloads | intermediate_existing_instances directly exercises ordinary and raw public templates (12 passing child checks per displayless selection). Both fill preallocated StatSlot references, retain their pointers and nonserialized owner tokens, and leave authoring objects independent. Ordinary Null returns an empty reference without modifying its supplied instance or misaligning following fields; an unrelated known Type is refused. Raw reading ignores Type and Null wrapper attributes while consuming its declared payload. A field-specific explicit raw serializer hint emits compact integer text beside ordinary described fields and fills a separate existing instance; it never replaces the global descriptor. A second custom serializer advertises CanDeserializeIntoExistingObject and is reached through a described read-only property: the reader passes the object's constructor-created MeterSlot, the codec fills it, and its pointer plus runtime-only token survive while the authoring instance stays independent. These are XML controls, not runtime XNB ReadRawObject/ISSUE-043 evidence. The typed-envelope workflow now supplies the distinct explicit-hint ordinary schema; boxed Core forwarding needs no duplicate demo. |
| Descriptor RuntimeType/TypeVersion | Source review at CNA 9e340c876: these setters store metadata and their getters expose it; no consumer of the getters was found in the content/content-pipeline implementations. The repository's runtime_type.xml/type_version.xml reference fixtures retain the authored XML type and ordinary fields, without runtime-name/version attributes; those fixtures were inspected, not executed here. Intentionally no independent setter/getter demo: connect them to a real writer consumer before claiming semantic support. This is separate from the runtime XNB reader-version enforcement actually exercised by reader_versions. |
| Dynamic types, namespace/type names | `content.custom.intermediate_polymorphism` explicitly registers two concrete serializers and round-trips EnemySpawn/TreasureSpawn through an abstract Spawn list. Eight child checks pass under both displayless renderer selections: actual XML Type attributes, null entry, list order, fresh concrete objects with inherited and derived data, and unknown/unrelated/abstract type refusal at the intended diagnostic gate. Prefix spelling comes from emitted XML, not a hard-coded namespace. No GPU, XNB output or genuine-XNA execution is claimed. Ordinary name getters and spelling-only variants do not justify independent demos; descriptor RuntimeType metadata does not establish polymorphic dispatch. |
| Boxed *Core extensions, serializer/XML accessors, MaxNestingDepth | Integration routes and implementation-facing public helpers do not need one demo per forwarding function. The typed-envelope serializer now uses both Serializer properties, both Xml surfaces, MoveToElement, WriteTypeName and ReadTypeName as one real nested dynamic-schema workflow. Writer `MaxNestingDepth` is observed through a non-shared self-cycle in `intermediate_shared_resources`: the exception names 1024 and explains the invalid edge. Reader `MaxNestingDepth` remains intentionally uncredited because tinyxml2 rejects ordinary element nesting beyond 500 first; CNA's own hardening suite covers that parser boundary and a 400-level readable control. The reader guard exists behind flattened-member recursion, but fabricating a 1025-deep reflected schema would add no user-facing XML behavior. Boxed `*Core` forwarders and structural text helpers remain implementation seams unless a custom schema needs them. |

### Texture2D (overload families and ownership)

| Members / behavior | Decision and evidence |
|---|---|
| Constructors, Width, Height, Bounds, inherited Format / LevelCount | `texture2d.set_data`, `surface_formats`, `mipmaps` and `from_stream` show allocated images, packed formats, mip levels and decoded bounds. Empty C++ handles are normal members in many demos; no separate constructor-per-overload demos. |
| SetData / GetData, whole image | `set_data` round trips procedurally generated pixels; `get_data_collision` puts readback to use for collision. These show the Color path, not arbitrary element layouts. |
| Rectangle, mip level, CPU startIndex / elementCount | `set_data_rectangle` checks a stroke and an untouched adjacent pixel; `mipmaps` writes every level and reads level 3. `set_data_rectangle` uploads from CPU offset 1 and reads into offset 2 with untouched prefix/suffix checks. Its seven controls now also prove atomic rectangle/count refusal and disposed-resource guards on both transfer directions under SOFTWARE and OPENGLES3. |
| Packed and floating-point data overloads | `surface_formats` uploads Bgr565/Bgra4444/Bgra5551 and reads Bgr565 back. `floating_formats` round-trips exact Single/Vector2/Vector4 and HalfSingle/HalfVector2/HalfVector4 data, preserves raw HDR/negative four-channel values, rejects an over-wide element without mutation and proves sampled channel expansion plus final Color-buffer clamping. `normalized_formats` round-trips NormalizedByte2/4, Rgba1010102, Rg32, Rgba64 and Alpha8, then distinguishes signed output clamping, wide UNorm precision, two-bit alpha and alpha-only expansion through real sampling. `render_target.hdr_blendable` separately reads HalfVector4 render-target values past one. These coherent format families cover every classic uncompressed packed layout; do not infer arbitrary element compatibility from the collapsed SetData/GetData member count. |
| Bound-texture update and transfer rejection | `set_data_rectangle` explicitly binds the texture and checks InvalidOperationException before unbinding for updates. One partly outside rectangle refuses SetData without changing its intersecting edge texel and refuses GetData without touching any destination element. A valid 4x4 rectangle with a logical count of15 in physically sufficient storage is likewise atomic in both directions. A separate disposed texture rejects both routes with ObjectDisposedException and preserves the read destination. Null pointers, negative indices and every rectangle permutation are routine variants already pinned by CNA tests; no exception-per-demo series is justified. |
| FromStream, resize/crop overload | `from_stream` compares original PNG dimensions, aspect-preserving fit and zoom/crop. It now also decodes an in-memory 2x2 palette GIF and checks every transparent/opaque RGBA texel, then requires InvalidOperationException from a recognized but truncated GIF. `save_as_image` supplies pixel-exact PNG and bounded-loss JPEG round trips, so all three public classic containers have executed evidence. CNA deliberately excludes broader decoder formats and routes DDS through DDSFromStreamEXT; those boundaries are pinned by CNA tests and do not justify one demo per rejected signature. |
| SaveAsPng / SaveAsJpeg | `save_as_image` compares lossless and lossy in-memory round trips. Filename convenience overloads need no independent example. save_as_image decodes both PNG and JPEG requested at 64x32 from a 128x128 source and checks that the source pixels/dimensions remain unchanged. Focused1020 adds a 64x32 split texture whose two halves carry the same magenta RGB at alpha0 and alpha128. PNG decodes the first half to exact transparent black and preserves every nonzero-alpha magenta texel; JPEG produces an opaque black center versus opaque magenta after dropping alpha. Ten checks pass on both renderers. Earlier labeled thumbnails were reviewed and the rendered source/lossless PNG panels were pixel-identical; Focused1020 used decoded public pixels only. The JPEG comparison verifies shape before indexing. Exact resampling rules, other formats and encoder failure paths remain unclaimed. |
| DDSFromStreamEXT | dds_stream now passes 14 checks per renderer with an original 16x8 DXT1 file and complete five-level chain. A packed offset and seven-byte short reads load the full asset; both tested renderers retain public Dxt1 format and all four lower mips return exact authored blocks. Fit preserves four base stripes at 32x16; centered crop retains middle stripes at 32x32; both are single-level Color and render correctly. Truncated final mip and incomplete declared chain are refused. Inspected screenshot pairs are RGBA-identical. This does not prove physical GPU compressed storage, untested renderer behavior, DXT3/DXT5 alpha paths or the fallback Color-loading branch (both selected Dxt1 here). |
| C++ copy / move / assignment and shared resource ownership | `texture2d.copy_ownership` now passes ten checks on both renderers. Its original copy construction proves Name/Tag isolation, a shared partial edit, simple full-Color upload detachment, source disposal and exact drawing by both survivors. A new occupied/bound primary target is copy-assigned from a 2x2 secondary-device source: owner/bounds/payload change and the old primary slot clears. A partial source edit remains shared; a full Color upload through the assigned target detaches only that wrapper, leaving the source exact. This does not generalize the simple full-upload overload to every mip/raw-data path, disabled context recovery or RenderTarget2D aliases. Active-binding move transfer is the same GraphicsDevice route already proved for cube/volume wrappers, so syntax-only repetition is not warranted. |
| Dispose, inherited GraphicsResource properties/events | `graphics_resource.lifetime_events` checks explicit disposal, resource events, Name/Tag and IsDisposed. copy_ownership checks that disposing one texture wrapper leaves its shared sibling readable/drawable, then separately disposes its cross-device target and copy-assigns the live source back into it; the target becomes live, registered to the secondary device and readable with the shared payload. Secondary-device teardown is deliberately left until after the last primary frame because the dimension-independent OPENGLES3 failure is already isolated as CNA-DEMOS-ISSUE-086. |
| SetDataRGBA, CreateFromPixels | CNA convenience paths; ordinary uploads already show the concept. Byte layout may fit a future C/C++ interop example, but no independent demo merely for the helper name. |
| GetRenderer / GetRendererWeak / GetCpuPixelsWeak / HasRenderer, ReconstructFromCache | Publicly exposed CNA integration and cache hooks, not ordinary game texture operations. Intentionally no independent demo unless a renderer/cache integration scenario requires them. |
| CreateCpuOnlyForTests, CreateWithRendererForTests | Explicit test seams; intentionally not game demonstrations. GetTypeName is routine runtime type identification, not an independent texture behavior. |

### TextureCube (face transfers, formats and stream boundaries)

Audit of TextureCube.hpp/.cpp against reflection, render_target_cube and the new face_transfers.
The focused transfer scene now passes all twelve checks on SOFTWARE. OPENGLES3 passes its ten
transfer/lifecycle controls but fails both established atlas checks after secondary-device teardown
because of CNA-DEMOS-ISSUE-086; the diagnostic frame was inspected and is completely black. The
pre-lifecycle 800x480 atlas captures were inspected and are RGBA-identical. The atlas draws public
readback through Texture2D; it does not establish direct GPU readback or cube sampling.

| Public members / behavior | Decision and evidence |
|---|---|
| Constructor, Size, inherited Format/LevelCount; CubeMapFace | reflection already authors six solid Color faces and checks actual +Z reflection and Fresnel contrast. render_target_cube demonstrates attachment/rendering through the derived type; its upright reflected-face checks now pass on EasyGL after the ISSUE-021 fix. face_transfers now checks size 4, Color format and the full 4x4/2x2/1x1 chain as part of actual transfer behavior; no independent enum/size getter demos. |
| SetData/GetData Color overload families | face_transfers writes six independent faces, updates +Y's central rectangle through source offset 1, reads it to destination offset 2 and checks allocated guard elements. All six base faces are read and compared after the patch, mip edits and a refused write. The +Y atlas checks four texels and the untouched blue border. Both renderer selections pass; EasyGL's public GetData uses its CPU shadow, so this is not proof of GPU readback. |
| Explicit mip-level transfers | face_transfers explicitly uploads and reads a cyan 2x2 level and magenta 1x1 level, independent of the patched blue base face. Null rectangles select those level dimensions, and actual atlas pixels show the two authored colors on both renderers. sampler_ownership now also samples authored Lime/Blue levels at explicit LOD1.25: point mip selection returns Lime, while linear mip selection returns their predicted3:1 blend. That filtering evidence is OPENGLES3-only because SOFTWARE lacks CustomEffects. |
| Typed packed, float, raw-value and byte transfers | compressed_blocks passes nine checks on SOFTWARE/OPENGLES3 with empty stderr. Original DXT1 faces use 32 bytes for 8x8; a 4x4 region copies 8 bytes through guarded source/destination windows, while 4x4/2x2/1x1 authored mips each retain one padded 8-byte block. Short writes and misaligned writes/reads are refused, preserving every neighbor block/face/mip and read guard. DXT3/DXT5 sixteen-byte alpha-plus-color blocks round-trip exactly; sampler_ownership now adds the actual decoded-alpha shader evidence described below. Only constructor capability refusal can skip. These are public compressed-byte and sampled results, not physical GPU readback or DDS decoding. typed_payloads adds eight passing checks on both selections: four Bgr565 PackedValues transfer as explicit little-endian 16-bit words through guarded windows, while Vector4 payloads preserve negative/above-one channels, expose sixteen scalar elements for four texels and accept a one-texel typed patch. Oversized element types and short logical scalar counts leave caller buffers unchanged. The observed Bgr565 C++ object size is 16 bytes here, but transferred word width is two; no ABI-size assertion or C++ object reinterpretation is used. Both stderr files are empty. Other packed/half/raw-value types and sampling remain separate; no format-per-demo series is justified. The Color path repacks CNA's polymorphic Color; never reinterpret those objects as texture/vertex bytes. |
| Refused transfer and caller-output preservation | Header promises complete transfer or an exception before publishing partial/fabricated output. Actual guards cover disposed/null, active-resource use, face/level, range/rectangle and element width. face_transfers now refuses an undersized logical write count using different replacement colors and an out-of-bounds read rectangle. The entire guarded read destination and all six original/patched base faces survive; physical arrays are fully allocated. Other refusal families remain separate. Capability refusals are renderer-specific, not a substitute for testing supported transfers. |
| DDSFromStreamEXT | dds_streams now executes five focused checks on both renderer selections: an original 506-byte RGB24 cube has six 4x4 faces and authored 2x2/1x1 levels, all read back as opaque Color. Full and prefixed-full streams match all eighteen face/level images; ordinary one-byte truncation throws FormatException. ISSUE-054 records the two remaining failures: seven-byte reads are attempted only once, and the identical truncation at position sixteen is accepted with incorrect pixels because total Length creates zero padding. Both final stderr streams are empty. The demo's initial wrong exception catch was corrected and retained separately; it is not a CNA defect. These checks establish public decoded data, not direct GPU readback, compressed cube formats or cube sampling. |
| Copy/move wrappers and Dispose | sampler_ownership passes nine GL checks: copied wrappers keep independent Name values; a complete Color face upload through the source remains shared; disposing the source leaves its sibling readable; custom samplerCube draws all six surviving faces including the changed orange +Z face; four authored +X corners preserve orientation; authored mips distinguish point/linear level selection; DXT3/DXT5 alpha decodes before blending; and a distinct white cube replaces that survivor on the same shader unit while an untouched red region remains exact. face_transfers adds four renderer-independent lifecycle controls: cross-device copy assignment replaces an occupied target, changes its device/shape/payload and detaches its former primary slot; a later source face edit is visible through the assigned wrapper; move construction transfers an active secondary slot and retains the shared payload; explicit secondary-device disposal marks both registered wrappers disposed and a rejected read leaves output untouched. All four pass on both renderers. Plain unbound move syntax and same-device copy assignment are now semantic duplicates rather than independent demo concepts. |
| Device teardown and context independence | Wrapper registration/disposal succeeds on both renderers, but the next primary OPENGLES3 frame is wholly black after the secondary device is destroyed. Both atlas assertions fail with CNA-DEMOS-ISSUE-086, while SOFTWARE renders the expected atlas. The existing Texture3D two-device workflow independently produces a black diagnostic GL screenshot despite its non-framebuffer assertions passing, so this is a common EasyGL multi-device teardown defect rather than cube payload loss. Device-loss/reset recovery and other renderer backends remain separate platform decisions. |
| GetTypeName, AsTextureCubeEXT, GetRenderer, protected renderer constructor/Dispose hook | Reflection identity and AsTextureCubeEXT returning this cube are trivial inspection/dispatch hooks, not separate game concepts. GetRenderer is the internal renderer bridge, and protected construction supports RenderTargetCube; no independent game-facing examples are justified for these implementation entry points. |

Full 462 at bbddd34 passes all 475 enabled CTests and both complete smokes. The new retained-cube
scene adds five GL checks/SOFTWARE skip; six automatic-Draw child checks preserve every older
C control. Original 461 raw failures stay 37/41; selected hashes stay stable and GL stderr is
unchanged. No broader compressed/typed-format coverage follows from the Color sampler scene.

### Texture3D (volume transfer versus sampling)

Reviewed Texture3D.hpp/.cpp, the sole existing volume_data demo and the custom shader binding
path initially at CNA 90bd2d33c. sampler_coordinates has focused native evidence through
clean CNA d23e139de: ten OPENGLES3 checks with programmatic back-buffer readback; SOFTWARE declares
its CustomEffects boundary. Other rows remain classifications unless explicitly tied to execution.

| Members / behavior | Decision and evidence |
|---|---|
| Constructor, Width, Height, Depth; inherited Format / LevelCount | volume_data now passes25 checks on both renderers. Its original32x32x4 fixture and non-cubic8x4x2 four-level Color chain establish dimensions, format and count. A DXT1 volume is refused, width257 exceeds the HiDef ceiling, and a separate Reach device rejects even1x1x1 Color. Vector4 remains the representative non-Color storage path. Nonpositive dimensions are routine argument guards; renderer-specific ceilings and device-loss recovery are platform work rather than independent scenes. |
| Color SetData: whole volume and sub-box | volume_data uploads slice-major data and overwrites exactly an8x8 box in slices1/2 using source offset1 with both guards retained. A level1 Z overflow, invalid level, short count, over-wide element and bound resource are refused without mutation. Repeating the same common box validator for negative/equal/outside X and Y boundaries would add assertion count without new transfer semantics. |
| Color GetData: whole volume, destination windows and boxes | volume_data retains its whole-volume read/2D atlas and reads the selected128-color box into output offset2, preserving two guards on either side. Short count, invalid level and over-wide element refusals leave all caller bytes unchanged. Symmetric X/Y/Z box-coordinate permutations share the same validator and are dispositioned rather than multiplied. Public getter success is not evidence of direct GPU readback. |
| Authored mip chain and non-cubic volume bounds | volume_data authors distinct payloads for 8x4x2,4x2x1,2x1x1 and1x1x1. A level1 box extending to Z=2 is refused at its own depth1 boundary; all four complete payloads remain exact. Reading nonexistent level4 throws without touching physically allocated output/guards. Seven additions preserve the three original checks (ten total on both renderers). The sampler workflow below separately establishes authored mip sampling; this transfer row alone does not imply filtering or other refusal families. |
| Typed packed, binary32 and raw-value transfers | volume_data now passes20 checks on both renderers. Its signed4x3x2 Vector4 field uses guarded24-vector/96-scalar views, a compact two-slice box and atomic short-count refusals. A separate2x1x2 Color volume round-trips four arbitrary trivially-copyable four-byte words through guarded windows in X-fastest/slice-major order. Eight-byte elements are refused atomically for both upload and readback, preserving resource and caller output. This is public transfer evidence, not direct GPU readback or float sampling. Packed and half-float format conversions use the same packed-value serialization paths already exercised across every classic Texture2D format; repeating the matrix per texture dimension would add count without new semantics. |
| SetDataPointerEXT | A public native-buffer integration route with a byte count, separate from the safe typed element interface. Ordinary Color uploads invoke it internally for Color storage, which is not direct game-facing demonstration. Intentionally defer a standalone example until an actual native producer needs this route; do not replace typed calls merely to increase metadata counts. |
| ShaderEffect::SetTexture(Texture3D), sampler coordinates, addressing and filtering | sampler_coordinates passes ten GL controls. A real sampler3D binds a 4x4x2 Color chain to explicit unit1; PointClamp selects red at Z=.375, linear filtering yields RGB(191,0,64), and both authored slice centers remain exact. Changing only AddressW to Wrap selects the opposite endpoint beyond the range. White/Yellow/Cyan/Magenta slice-zero corners sampled at exact U/V texel centers preserve uploaded row order. Authored uniform Lime mip1 and Blue mip2 plus explicit LOD1.25 distinguish `LinearMipPoint` exact selection from `PointMipLinear` RGB(0,191,64) cross-level interpolation. SOFTWARE declares CustomEffects absence. This is programmatic back-buffer evidence; no manual image inspection, non-Color sampling or direct GPU readback is claimed. |
| Move construction/assignment and Dispose | sampler_coordinates proves unbound move construction and occupied move assignment around explicit source Dispose. volume_data binds a2x1x2 volume and shows that move construction transfers the active binding, then uses two live devices: an occupied target bound on the primary receives a bound2x1x2 source from the secondary, reports the secondary owner, keeps all four colors and still refuses upload until the secondary slot is unbound. This establishes same-device and cross-device moves with both inactive and active bindings. External-reference rebinding, direct GPU readback and device/context loss remain separate platform decisions. |
| GetTypeName, AsTexture3DEXT, GetRenderer and protected disposal | Reflection, identity dispatch and the renderer bridge are implementation-facing inspection hooks. They do not justify independent game concepts or metadata credit merely from source review. |

### SpriteBatch and SpriteFont

Reviewed against current public headers and the 19 sprite-batch / 7 sprite-font demos. This is
an overload-family and behavior audit, not a claim that every picture has a pixel assertion.
In particular, a successful bounded Game run does not itself verify the rendered layout.

| Members / behavior | Decision and evidence |
|---|---|
| SpriteBatch(GraphicsDevice), empty constructor, renderer-injection constructor | Device construction is shown in `sprite_batch.draw_texture` and throughout the graphics demos. Empty handles do not need their own example. The header explicitly labels renderer injection test-only; intentionally leave mock renderer construction to CNA's tests. |
| Begin/End sequencing | `begin_end_rules` now executes fourteen controls on both renderers. It retains Draw before Begin, End without Begin, duplicate Begin and concurrent-batch rules, then proves post-Dispose Begin, End and Draw all refuse with ObjectDisposedException. A disposed texture is also refused at Draw before queueing, and the still-open deferred batch accepts a valid draw and End afterward. DrawString has its own public entry point but forwards to the same full overload and begun guard; repeating the identical pre-Begin/disposed permutations would add syntax, not behavior. CNA's cross-batch Immediate guard remains its own device-coordination contract rather than a claim about undocumented genuine-XNA internals. |
| Begin overloads, blend/sampler/depth/rasterizer states and transform matrix | `blend_states`, `sampler_states`, `multiple_batches` and `transform_matrix` demonstrate the coherent settings, not every progressively longer overload. Null/default forwarding and pointer-versus-value syntax do not warrant independent demos. multiple_batches now passes six checks on both renderers: background End retains opaque/PointWrap, a deferred world Begin leaves those states until End, world End retains alpha/PointClamp plus depth-none/cull state, and default HUD End installs LinearClamp. Two pixel checks verify camera-transformed world and screen-fixed HUD positions. Both captures were inspected; 3,108 pixels differ across renderers by at most one channel level. These representative states do not establish every state object, Immediate timing or automatic restoration of unrelated device bindings; callers still own manual state restoration when needed. |
| Deferred, BackToFront and FrontToBack sorting; layerDepth | `layer_depth_sorting` checks each mode's overlap plus all unobscured colors. Three equal-depth submissions additionally expose XNA 4.0's intentionally unstable three-item Array.Sort reversal rather than promising submission stability. Matching FrontToBack panels under DepthStencilState None/Default then prove painter order leaves far blue visible without depth testing while real layer Z preserves near red with it. The demo also mixes nonzero-depth DrawString glyphs and ordinary sprites in the same queue. Seven controls pass on both renderers. |
| Texture sorting | `sort_mode_texture` reads overlaps and checks that Deferred alternates while Texture groups them, deliberately leaving the inter-texture group order unspecified. It does not measure draw-call savings or prove a particular texture ordering. The grouped result check now requires an authored red/blue color as well as uniform overlaps, so a blank row cannot pass. Both checks pass on SOFTWARE/OPENGLES3; screenshots inspected. |
| Immediate state changes | `immediate_mode.blend_state` verifies Opaque then Additive with pixels. The parallel sampler example fails visibly with ISSUE-004. A failed sampler path must not be credited as working renderer support. |
| Draw positions, destination/source rectangles, tint and alpha | Existing `draw_texture`, `destination_rectangle`, `source_rectangle`, `tint_and_transparency`, `blend_states` and `loose_png_alpha` expose these inputs directly. `source_rectangle` point-samples four enlarged atlas selections and checks each frame's exact solid color, transparent-background preservation and the unscaled whole-strip order from `std::nullopt` on both renderers. `tint_and_transparency` point-samples an authored white texel under four opaque tints and four `Color::White * opacity` values; the latter match premultiplied AlphaBlend over DimGray on both renderers within two channel levels. `destination_rectangle` proves that all four destination bounds are filled without touching adjacent pixels and that horizontal 4x/2x and vertical 0.75x/6.25x resampling retain the complete checker's normalized pattern. Degenerate rectangles are deliberate unchecked numeric input and oversized source rectangles are the sampler-addressing contract already shown by `tiling_with_wrap`; they are not argument-refusal gaps. `begin_end_rules` covers disposed-texture rejection without poisoning the batch. New `cross_device_texture` constructs two owned devices, refuses the foreign texture at Draw, then accepts a same-device texture, End and another Begin/End; the invalid command therefore never reaches the deferred queue. |
| Rotation, origin, scalar/Vector2 scale, SpriteEffects | `rotation` replaces its timing-dependent picture with six deterministic point-sampled draws. Four 0/90/180/270-degree centre-origin arrows keep their yellow centre on the requested pivot while the asymmetric red marker moves clockwise through the predicted quadrants; two more 90-degree arrows prove centre versus top-left origin placement. `scale` correlates an authored red/yellow texel pair with scalar0.5/1/3 spacing and independent Vector2(5,1)/(1,5) spacing. `sprite_effects` now locates its asymmetric red marker in all four expected corners, proves only horizontal flipping reverses the arrow direction, and keeps all variants inside identical128x128 bounds. Their three/two/three aggregate controls pass on both renderers. Cropped-source origins, negative scale combined with flips and transformed clipping deserve extensions only when they teach a distinct interaction, not an overload-per-demo expansion. |
| Wrapping and animated source rectangles | `tiling_with_wrap` demonstrates oversized source rectangles with wrapping; `sprite_sheet_animation` derives frames from game time. These are coherent uses of Draw rather than new API types. The latter is intentionally unsuitable as a fixed-frame pixel baseline because Update catch-up changes elapsed game time. |
| Begin Effect argument: stock, source and compiled | `sprite_batch.stock_effect` passes two framebuffer controls on SOFTWARE and OPENGLES3. AlphaTestEffect Projection places a red sprite in the top-right model-space quadrant; BasicEffect applies the same Projection plus a Y-flipping World and places blue in the bottom-right, while the left half and original pixel-space corner remain black. This proves stock effects own SpriteBatch vertex placement in both Deferred and Immediate modes. `shader_effect.sprite_shader` separately checks grayscale luma through a GLSL ShaderEffect, and `portable_package` selects/executes an owned GLSL package; both correctly skip SOFTWARE's missing CustomEffects capability. Compiled XNA effects cannot form a current two-renderer demo: this baseline has both `CNA_EASYGL_COMPILED_EFFECTS` and `CNA_SOFTWARE_COMPILED_EFFECTS` OFF. `c_api.graphics.effect_core` already supplies the valid `CnaConformanceEffect.fxb` and checks explicit capability refusal; CNA's renderer suites contain compiled SpriteBatch, multi-pass ordering and effect-alternation contracts for enabled builds, but they were inspected rather than executed here. Do not add a demo that only repeats the current refusal or another GLSL spelling. Queued compiled-effect mutation remains intentionally renderer/build-specific. |
| Dispose, inherited IsDisposed/Disposing; queued resource lifetime | `begin_end_rules` directly credits public SpriteBatch.Dispose and projects XNA-member coverage1437->1438/2326. Dispose raises Disposing exactly once before IsDisposed becomes true, a repeated call is inert and every batching entry point refuses afterward. A temporary 1x1 orange Texture2D wrapper leaves scope between deferred Draw and End; retained renderer payload produces exact orange/black framebuffer samples on both renderers. Explicitly disposing another texture before Draw now raises ObjectDisposedException before queueing, and the same deferred batch remains usable for a valid Draw/End. ISSUE-082 is the one failing control: disposing an open deferred batch clears local state but leaks the GraphicsDevice active-batch count, so a second batch's Immediate Begin is permanently refused. Both renderers now reproduce thirteen passes plus that unchanged failure. GetTypeName and routine destructor syntax need no separate demo. |
| SpriteFont constructor, atlas, glyph bounds/cropping/kerning | `glyph_constructor` now passes nine controls on both renderers. Its public inspector round-trips the exact 30x5 atlas and all four ordered tables after the caller mutates its source vectors. It explicitly disposes the caller atlas and proves the font's independent Texture2D value wrapper stays live, retains every texel and supplies the later exact DrawString pixel. Digit `1` carries crop(-2,1,3,7) and kerning(-1,3,-2): single/repeated measurement establishes signed edge/interior rules and height7, while exact framebuffer samples prove the shifted/downward ink placement. The equal-table-length rule is an explicit constructor precondition; CNA declares no refusal contract, and deliberately building a value whose later indexed access is invalid would not be a useful demo. `content.cnj.sprite_font` separately shows file-authored glyphs. Device teardown remains lifecycle scope. |
| Characters, DefaultCharacter | `default_character` now passes six controls. It retains the missing-BMP-glyph refusal and substitution, rejects a fallback absent from Characters without replacing the valid `?`, clears the optional fallback and proves refusal returns, then restores `?` for drawing. The accented UTF-8 input demonstrates BMP decoding, not broad localization. CNA's public UTF-8 bridge deliberately maps invalid/out-of-BMP input to `?`; supplementary-code-point policy does not justify pretending SpriteFont's UTF-16-code-unit map accepts a new glyph kind. |
| MeasureString(string), MeasureString(StringBuilder) | `measure_string` now passes seven controls: fixed-width advances, multiple/widest lines, trailing newline, exact zero for empty text and ignored carriage returns both alone within a line and as CRLF. `string_builder` retains equivalence with ToString. `glyph_constructor` supplies the unusual signed-bearing/tall-crop metrics and matches actual drawn placement. These representative rules cover the coherent layout contract; arbitrary glyph metric permutations add no concept. |
| Spacing, LineSpacing, copy independence | `spacing` now passes four controls. Positive inter-character and doubled line spacing remain exact; changing the copy leaves the source getters unchanged. A second copy with Spacing=-2 reduces exactly two gaps in `ABC` while the one-glyph extent is unchanged. Texture storage through the constructor's independent value handle is now covered by `glyph_constructor`; it is not inferred from these spacing copies. |
| DrawString overloads and transforms | `draw_string` now proves a repeated compiled-glyph prefix lands at exact LineSpacing intervals under mixed CRLF/LF input, with real ink under all four displayed tints; `string_builder` covers mutable text. `layer_depth_sorting` proves depth0.25/0.75 glyphs share BackToFront ordering with a depth0.5 ordinary sprite independent of submission order. `transformed_text` point-samples compiled-font output: scalar versus Vector2 scale changes complete-layout bounds, a90-degree turn rotates both bounds and ink-centre offset around the `MeasureString` origin, and H/V/combined flags produce exact whole-layout occupancy reflections of asymmetric `LJ`. Redundant overload syntax needs no separate demo. |
| getTextureEXT, getGlyphBoundsEXT, getCroppingEXT, getKerningEXT | `glyph_constructor` now directly reads all four extensions and correlates the atlas, bounds, cropping and kerning entries by the Characters order. Caller-vector mutations prove the tables are retained values. This is the intended custom-layout/binding inspection workflow; no getter-per-demo expansion. Returned references must not outlive the font. |

### GamePad value identity and disconnected capabilities

Source review at CNA `09c3108e5`: GamePadState and its four component value implementations,
GamePadCapabilities, the SDL population path, and the existing `input.gamepad.state_values` and
`capabilities` demos. No physical controller was present in this environment.

| Members / behavior | Decision and evidence |
|---|---|
| GamePadState Equals, GetHashCode and ToString | `state_values` retains all eleven construction, clamping, virtual-button and operator controls and adds typed/boxed equality with wrong-type rejection, an equal hash and XNA's exact fully-qualified type-name text. The renderer-free direct run passes18/18 with empty stderr. PacketNumber already has a separate polling workflow; arbitrary packet values add no identity concept here. |
| GamePadButtons/GamePadDPad/GamePadThumbSticks/GamePadTriggers identity and text | One coherent set of controls compares complete authored copies through typed and boxed Equals, equality/inequality operators and equal hashes, while default values remain distinct. Exact diagnostics are `{Buttons:A}`, `{DPad:Up}`, the two labelled stick vectors and the two trigger scalars. These are the four nested parts of the demonstrated state, not separate value-only demos. Together with GamePadState they add twenty XNA-member credits. |
| HasLeftStickButton and HasRightStickButton | `capabilities` now reads both properties explicitly in its connected summary and all-controls predicate. Its GamePad need was a demo metadata defect: the source already promised meaningful disconnected behavior but preflight prevented that code from running without hardware. With the need removed, all four absent player slots report disconnected and no controls, including both stick buttons. SOFTWARE and OPENGLES3 each pass eight checks with byte-identical stdout and empty stderr. This demonstrates the false/disconnected contract only; it does not claim either button exists on a real controller. |

Temporary generated coverage moves1438->1460/2326 XNA members without changing the627-demo,
297-type or C-route totals. `capabilities` will change one no-gamepad smoke result from skip to pass
per renderer at the next complete checkpoint. No screenshot or visual inspection was performed.

### KeyboardState and MouseState value identity

| Members / behavior | Decision and evidence |
|---|---|
| KeyboardState.GetHashCode | The existing `input.keyboard.state_values` workflow now compares the same A/LeftShift set constructed in opposite orders and checks equal hashes; the empty set hashes to zero. This complements its constructor, query, equality and recorded-transition controls without adding a second keyboard demo. |
| MouseState constructor, fields and value identity | New console demo `input.mouse.state_values` passes eight controls under both renderer selections. The default and public eight-argument constructor expose exact coordinates, vertical wheel and five buttons. Typed/boxed equality, both operators, equal hashes, wrong/empty boxes, a changed coordinate and exact stable text establish the complete XNA value contract. CNA's nine-argument constructor also proves the horizontal wheel is preserved but deliberately excluded from XNA equality, hashing and text. This is authored state, not live mouse hardware. |
| Mouse.WindowHandle | `platform.window.native_handle` now requires automatic identity with the live GameWindow token, clears the static target and restores that same real token before continuing. OPENGLES3/X11 passes; SOFTWARE has its declared no-window skip. Restoration preserves subsequent input targeting. Evidence: `/tmp/checkpoint-689-*`. |

Focused649 raises the staged catalog to628 demos and temporary coverage from1527 to1534/2326.
`Microsoft.Xna.Framework.Input` reaches112/113 members; only the dispositioned WindowHandle remains.
Evidence:`/tmp/checkpoint-649-*`. No physical input, screenshot or visual inspection was performed.

### Input small-state enum values

The authored MouseState workflow directly distinguishes `ButtonState::Pressed` and `Released`
across five button positions, defaults, equality and exact text. The authored KeyboardState workflow
likewise distinguishes `KeyState::Down` and `Up` through its indexer while its query and replay
controls establish the same state changes. Keyboard gamepad emulation deterministically reports
`GamePadType::GamePad`; the four disconnected capability slots now explicitly report
`GamePadType::Unknown` together with no controls. The other eight GamePadType identities require
specific controller hardware, and CNA has no path that reports AlternateGuitar separately, so they
remain uncredited rather than reduced to integer checks.

The existing dead-zone workflow names and calls `None`, `IndependentAxes` and `Circular` in one
frame. Both focused renderer runs pass its three scalar helper controls, but no physical controller
was connected, so the three returned live stick values were neutral and did not distinguish the
filters. Their side-by-side behavioral comparison remains a manual hardware observation; metadata
credit records the real public calls, not a claim that the modes were visually or numerically
separated in this session. All five exact SOFTWARE CTests pass ten repetitions. Metadata validates
659/0, both validator suites pass and coverage moves175->184/753 fields, with Input at9/202.
Evidence:`/tmp/checkpoint-897-*`. No screenshot, physical-input or visual-result claim is made.

### Buttons flag values

`input.gamepad.state_values` now distinguishes every one of the25 public XNA Buttons flags without
adding per-button demos. One authored state holds all fifteen physical face, menu, shoulder, stick-
click and D-pad flags; one aggregate control requires every `IsButtonDown` result and every matching
GamePadButtons/GamePadDPad property to be Pressed. A positive-axis state derives both right/up flags
for both sticks plus both trigger flags, while a negative-axis state derives the four left/down
flags and each state requires all opposite directions to remain up. This connects each constant to
observable GamePadState behavior rather than checking numeric masks.

Both renderer-labelled console runs pass21/21 with byte-identical output and empty stderr, retaining
all18 prior construction, clamping, identity and text controls. The exact SOFTWARE CTest passes20
repetitions. Metadata validates659/0, validator suites pass5/5 and4/4, and generated coverage moves
184->209/753 fields with Input at34/202. Evidence:`/tmp/checkpoint-898-*`. No physical controller,
rendering, screenshot or visual judgment is involved.

### TouchLocation and TouchCollection value behavior

| Members / behavior | Decision and evidence |
|---|---|
| TouchLocation equality, hash, operators and text | `input.touch.touch_values` now compares its actual moved location with a pressure-bearing copy. Typed/boxed equality, wrong/empty boxes, both operators and equal hashes prove that CNA pressure is retained but excluded from the XNA identity. A changed previous position is unequal, so equality includes history, while exact text reports only `{Position:{X:130 Y:90}}`. False TryGetPreviousLocation and FindById calls also verify their assigned invalid sentinels. |
| TouchCollection CopyTo and mutation methods | The same workflow copies its two-touch snapshot into the middle of a preseeded vector and checks insertion order. A copied collection still reports IsReadOnly=true while Add/Insert/Remove/RemoveAt/Clear all mutate it, matching the documented FNA advisory flag rather than inventing a refusal contract. Removal distinguishes present from absent equal values and every ordering transition is checked. |
| Enumerator/interface aliases | C++ range iteration already traverses real TouchCollections in `input.touch.get_state`. The remaining explicit nested Enumerator and non-generic IEnumerable members are CLR interop mechanics and do not justify parallel controls, especially because short `Enumerator` metadata would ambiguously credit unrelated nested types. |
| TouchPanel.WindowHandle | `platform.window.native_handle` writes the live GameWindow token, observes exact identity, clears it and restores the previously observed process value. OPENGLES3/X11 passes7/7; SOFTWARE correctly skips without a window. This proves opaque property state, not touch delivery. Evidence: `/tmp/checkpoint-693-touch-window-*`. |

Focused650 extends the existing ID from eight to17 controls and moves temporary coverage from1534
to1545/2326; the Touch namespace is41/48 with every remaining member dispositioned. Evidence:
`/tmp/checkpoint-650-touch-*` and `/tmp/checkpoint-650-coverage.md`. No touch hardware, screenshot or
visual inspection was used.

### Input.Touch public enum values

`input.touch.touch_values` already constructs and distinguishes Invalid, Released, Pressed and
Moved through previous-location sentinels, current/history states and collection mutations; its
metadata now names all four TouchLocationState values. The gesture-queue workflow now combines all
ten nonempty GestureType flags and proves None remains the empty mask, then enqueues one sample of
each nonempty type. Ten distinct timestamps, primary/secondary positions and primary/secondary
deltas return in exact order, so HorizontalDrag, VerticalDrag and both completion values are
observed through the same public queue as the familiar tap/drag/pinch values. This is deterministic
replay/queue evidence and does not claim that a recognizer produced the gestures from touch motion.

Both renderer-labelled console runs pass14 gesture and17 touch-value controls with byte-identical
output and empty stderr. Both exact SOFTWARE CTests pass20 repetitions. The clean `-j3` build,
metadata659/0 and both validator suites pass. Generated fields move209->224/753 and Input.Touch
reaches15/15; the two secondary payload properties were already credited elsewhere, so the non-
field member total remains2114/2326. Evidence:`/tmp/checkpoint-899-*`. No touch hardware,
rendering, screenshot or visual judgment is involved.

### Keys public values

`input.keyboard.state_values` now partitions all160 public `Keys` values into seven coherent
control/navigation/IME, alphanumeric/system, numeric-keypad, function/lock/modifier,
browser/media/launch, OEM/chatpad and legacy process/editing families. Each family constructs a
real `KeyboardState`, requires `GetPressedKeys` to return the complete numerically ordered set and
requires both `IsKeyDown` and the indexer to report every member down. This gives each constant
state behavior instead of merely casting or listing its integer value. The existing recorded-input
sequence still demonstrates transition logic separately.

Both renderer-labelled console runs pass16/16 with byte-identical stdout and empty stderr. The
exact SOFTWARE CTest passes20 repetitions. The warning-free `-j3` build, metadata659/0 and both
validator suites pass; the catalog contains exactly160 unique `Keys::` entries. Generated fields
move224->384/753 and Input reaches194/202. The remaining eight Input fields are the hardware-
reported GamePadType identities AlternateGuitar, ArcadeStick, BigButtonPad, DancePad, DrumKit,
FlightStick, Guitar and Wheel; they stay intentionally uncredited because this environment has no
corresponding devices and CNA has no deterministic public injection path. Evidence:
`/tmp/checkpoint-900-*`. No physical key delivery, keyboard-layout mapping, rendering, screenshot
or visual judgment is claimed.

### Root Framework public fields

The root `Microsoft.Xna.Framework` namespace now has a semantic decision for all78 inventory
fields. Existing geometry workflows already compare `BoundingFrustum::CornerCount` with the
returned corner vector and classify a box behind a plane as `PlaneIntersectionType::Back`; their
metadata now records those direct uses. The value workflows add compact constructor round-trips
for every Matrix, Quaternion, Rectangle and Vector2/3/4 public component. Matrix's check maps the
distinct values1..16 across all row-major fields, while the vector, rectangle and quaternion
checks preserve distinct per-component values rather than testing only equality.

`input.gamepad.capabilities` now sends the named PlayerIndex One, Two, Three and Four values to
real `GetCapabilities` and `GetState` calls instead of casting loop indices. The window-orientation
workflow proves the manager's initial Default policy, its existing Portrait-to-LandscapeLeft
transition and a second live transition to LandscapeRight, including presentation state and one
additional event. Eight renderer-free workflows pass on both labels; capabilities passes8/8 on
both; orientation passes9/9 on OPENGLES3 under Xvfb and keeps SOFTWARE's declared no-window skip.
All ten exact SOFTWARE CTests pass or take that one declared skip through ten repetitions. The
warning-free build uses `-j3`, metadata remains659/0 and both validator suites pass. Fields move
384->426/753 and the root namespace closes at78/78. Evidence:`/tmp/checkpoint-901-*`. No physical
controller, screenshot or visual-quality claim is made.

### Audio public fields

The five previously uncredited Audio fields now attach to existing workflows. The XACT play-cue
demo checks `AudioEngine::ContentVersion` against version46, the generation encoded by the original
XGS/XWB/XSB assets it then successfully loads. The cue-state workflow already contrasts
`AudioStopOptions::AsAuthored` with Immediate using no-release, authored-fade and ReleaseTime-RPC
cues, so its metadata now credits the behavior it directly observes.

The manual microphone workflow now requires a nonempty public `Name`, starts in Stopped, enters
Started, returns to Stopped and starts again before its ordinary capture loop. With SDL's dummy
recording device it passes7/7 under SOFTWARE and OPENGLES3/Xvfb; this validates enumeration and
state transitions without claiming physical input or recording quality. Both XACT workflows pass
under both renderer labels, and their exact SOFTWARE CTests pass ten repetitions. The warning-free
build uses `-j3`, metadata is659/0 and both validator suites pass. Fields move426->431/753 and Audio
closes at11/11. Evidence:`/tmp/checkpoint-902-*`. No listening or microphone-hardware claim is made.

### Design protected fields

`design.type_converter.properties` now includes a focused `MathTypeConverter` subclass that uses
the two documented protected field names directly. Toggling `supportStringConvert` changes the
public `CanConvertFrom(string)` answer from false to true. Assigning Color's real descriptor
collection to `propertyDescriptions` makes the same count and first descriptor identity visible
through public `GetProperties`. These checks demonstrate one shared state source for each field,
which is the subclass-extension contract; they do not merely read protected storage.

Both renderer-labelled runs pass12/12 with byte-identical output and empty stderr. The exact
SOFTWARE CTest passes20 repetitions. The warning-free build uses `-j3`, metadata is659/0 and both
validator suites pass. Fields move431->433/753 and Design closes at2/2. Evidence:
`/tmp/checkpoint-903-*`.

### Net public fields

Nine of the fourteen previously uncredited Net values now connect to observable workflows. The
local-session path proves the exact public capacities31 and100, atomically refuses a requested
capacity32 and bounds its current PreviousGamers view. The game-flow path directly observes Lobby
and Playing. The known join-refusal workflow observes CNA's SessionNotFound result and keeps the
expected SessionNotJoinable result visible as CNA-DEMOS-ISSUE-032. The packet workflow sends three
distinct payloads with InOrder, ReliableInOrder and Chat and receives each from the selected local
sender; this is evidence for public option handling and local delivery, not real-network reliability
or ordering.

Both renderer runs pass the three positive workflows at12/12,8/8 and10/10. The known-issue join
workflow retains its reviewed failures on both. All four exact SOFTWARE CTests pass ten repetitions,
the warning-free build uses at most three jobs, metadata is659/0 and both validator suites pass.
Fields move433->442/753 and Net reaches17/22. SessionFull needs a second live join against a full
host; Ended and HostEndedSession need a remote host closure; Disconnected needs actual transport
loss; ClientSignedOut needs service identity loss. Those five values remain intentionally
uncredited until their causes can be observed. Evidence:`/tmp/checkpoint-904-*`.

### Blend, colour-write and cull values

The blend-functions workflow now sends all13 Blend factors through actual render-target blending
with distinct source, destination and constant RGBA components. It checks the complete resulting
Color rather than enum integers. Its visible blend-mode row also adds source-minus-destination, so
all five BlendFunction values have independent pixel evidence. The colour-write workflow adds
Alpha and All to its existing masks and compares every RGBA byte, including preservation of the
translucent destination. The cull-mode workflow already renders both triangle windings under all
three modes; metadata now credits the two observed values that were previously unnamed.

Both renderers pass the blend20/20, write-mask7/7 and cull3/3 controls with identical reported
pixels. Their exact SOFTWARE CTests pass ten repetitions. The warning-free build uses at most three
jobs, metadata is659/0 and both validator suites pass. Fields move442->457/753 and Graphics reaches
132/192. The first compile's mixed integer/byte Color overload and the first Subtract alpha oracle
were demo defects fixed before the validated runs; no CNA defect appeared. Evidence:
`/tmp/checkpoint-905-*`.

### Compare and stencil-operation values

The depth-function workflow now uses equal submitted depths to distinguish strict Less/Greater
from inclusive LessEqual/GreaterEqual before its original hidden-object Greater scene. The stencil-
counting workflow seeds exact stencil values and then verifies the four previously missing
transformations through equality-gated coloured readback: decrement saturation keeps0, increment
saturation keeps255, invert maps0x0f to0xf0 and zero maps123 to0. Together with the existing depth,
stencil-fail, depth-fail, winding and overlap workflows, every value in both enums now has actual
buffer behavior.

Both renderer runs pass7/7 depth and8/8 stencil controls, and both exact SOFTWARE CTests pass ten
repetitions. The warning-free build uses at most three jobs, metadata is659/0 and both validator
suites pass. Fields move457->464/753 and Graphics reaches139/192. Evidence:
`/tmp/checkpoint-906-*`. No screenshot or visual-quality judgment is claimed.

### Effect parameter class and type values

The existing typed-parameter workflow now checks the complete reflection value families. Loaded
BasicEffect parameters supply real Scalar, Vector, Matrix and Texture2D metadata. Authored public
EffectParameter storage connects Object and Struct plus Bool, Int32, Single, String, generic
Texture, Texture3D, TextureCube and Void to the value paths already exercised. A generic Texture
parameter retains the actual Texture2D object. An authored Texture1D parameter preserves its exact
reflection identity; XNA/CNA has no public Texture1D resource class or getter, so this is explicitly
metadata coverage rather than a sampling claim.

Both renderers execute31 controls and retain only the established ISSUE-034/-035 failures. The
exact known-issue SOFTWARE CTest passes ten repetitions. The warning-free build uses at most three
jobs, metadata is659/0 and both validator suites pass. Fields move464->479/753 and Graphics reaches
154/192. Evidence:`/tmp/checkpoint-907-*`.

### Vertex element format and usage values

The skinned-colour workflow already draws through Byte4 BlendIndices, Vector2 texture coordinates
and Vector4 BlendWeight; its metadata now records the missing values attached to that real skinning
path. The custom-vertex workflow adds a compact declaration that pairs the other six missing
formats with the other six missing legacy usages. Exact offsets and documented element widths
produce stride48, the declaration preserves every descriptor, and a readable HiDef VertexBuffer
accepts it and returns a distinct 48-byte record unchanged. The demo's original Position/Single/
Color declaration continues to drive a checked draw. The rare semantics are declaration and buffer
contract evidence; no stock shader is claimed to consume Binormal, Depth, Fog, PointSize, Sample
or TessellateFactor.

Both renderers pass10/10 custom-layout and3/3 skinned controls, and both exact SOFTWARE CTests pass
ten repetitions. The warning-free build uses at most three jobs, metadata is659/0 and both
validator suites pass. Fields move479->495/753 and Graphics reaches170/192. Evidence:
`/tmp/checkpoint-908-*`.

### Texture filter values

The SpriteBatch sampler workflow now gives all four mixed min/mag/mip values sampled behavior. A
generated two-colour board magnified24x yields exactly zero intermediate pixels for each MagPoint
value and7488 for each MagLinear value on both renderers, while the public SamplerState property
retains each complete enum identity. Point and Linear form the controls. Existing texture3D/cube
workflows already distinguish LinearMipPoint from PointMipLinear, so this batch does not duplicate
their mip-selection scenes. The anisotropic floor binds the real AnisotropicWrap state and checks
AnisotropicClamp's filter before drawing; this remains a functional sampler path without a fixed
cross-renderer sharpness claim.

Both renderers pass6/6 sampler and1/1 anisotropic controls with identical measured counts, and both
exact SOFTWARE CTests pass ten repetitions. The warning-free build uses at most three jobs,
metadata is659/0 and both validator suites pass. Fields move495->501/753 and Graphics reaches
176/192. Evidence:`/tmp/checkpoint-909-*`.

### Cube faces, depth, compressed formats and target usage values

The existing cube-transfer workflow uploads a different Color image to every face, reads all six
back exactly and builds its displayed atlas from those results. Its metadata now names every
CubeMapFace value attached to that behavior. The compressed-cube workflow adds Dxt3 and Dxt5
resources beside Dxt1; each reports its requested format and round-trips a distinct sixteen-byte
alpha-plus-colour block exactly. This is public compressed storage and transfer evidence, without
a claim about sampling, DDS decoding or direct GPU readback.

The render-target depth workflow now runs identical overlapping geometry through None, Depth16 and
Depth24 resources. Exact target readback is blue without depth and red with either real depth
attachment. The usage workflow adds a PlatformContents target, checks its retained policy and
observes its first red drawing after an unbind/rebind before adding blue, matching CNA's documented
desktop policy. This closes all values in CubeMapFace, DepthFormat, SurfaceFormat and
RenderTargetUsage.

SOFTWARE and OPENGLES3/Xvfb pass3/3 depth,4/4 usage and9/9 compressed controls. The face-transfer
workflow passes12/12 on SOFTWARE; OPENGLES3 retains only the established ISSUE-086 post-secondary-
device pixel failures while all transfer checks pass. All four exact SOFTWARE CTests pass ten
repetitions. The warning-free build uses at most three jobs, metadata is659/0 and both validator
suites pass. Fields move501->509/753 and Graphics reaches184/192. Evidence:
`/tmp/checkpoint-910-*`.

### Presentation, device status and dynamic upload values

The owned-device workflow takes Default, One and Two through the same real Reset and Present path
as its original Immediate setting. Each value survives in the device's applied presentation
parameters, the device remains Normal and each cycle emits exactly one ordered Resetting/Reset
pair. This demonstrates selection and usable presentation; refresh timing is platform-controlled
and no wall-clock synchronization claim is made. GraphicsDeviceStatus::Normal is observed after
construction, ordinary and region presentation, and every reset.

The dynamic-vertex workflow now streams its actual waving strip through a three-segment buffer.
None initializes the first segment, NoOverwrite fills disjoint segments while the previous one may
still be consumed, and Discard starts each wrap. A direct bound-buffer control rejects None and
accepts NoOverwrite into the next segment. CNA currently composes windowed writes in its CPU shadow
and re-uploads the whole buffer, so the semantic result is established without claiming a native
driver performance difference.

SOFTWARE and OPENGLES3/Xvfb pass12/12 lifecycle and2/2 buffer controls; both exact SOFTWARE CTests
pass ten repetitions. The warning-free build uses at most three jobs, metadata is659/0 and both
validator suites pass. Fields move509->515/753 and Graphics reaches190/192. Lost and NotReset are
the complete intentional remainder: CNA assigns them only from a real renderer device-event
callback, and neither tested renderer can deterministically produce or publicly inject that
environmental transition. Evidence:`/tmp/checkpoint-911-*`.

### Complete912 generated runtime checkpoint

The complete pinned-CNA build and outer-Xvfb validation absorb focused893-911. Metadata is659/0;
all672 enabled CTests pass, with three manual entries disabled. SOFTWARE reports560 pass/52
expected fail/44 skip/3 manual and OPENGLES3 reports600/53/3/3. Both execute all659 IDs with zero
unexpected results. Against Complete892, the only status changes are the two new expected-failure
Video IDs and the reviewed ISSUE-099 transition of the existing VideoPlayer contract workflow;
exactly eleven new failed-assertion groups match ISSUE-099/-100/-101. Runtime-backed coverage is
302/329 types,2114/2326 members,515/753 fields and3078/3205 C routes. Evidence:
`build/linux-full/reports/checkpoint-912-complete-*`.

### Avatar preset, skeleton, expression and renderer-state values

The existing avatar workflows now attach all public avatar values to real data paths. Every one of
the31 stock presets constructs an AvatarAnimation and yields a positive-duration midpoint sample
with71 transforms and bounded facial state. Female and Male requests survive random description
creation. All55 named bone indices address distinct slots in the Ready renderer's real71-element
parent/bind-pose data, with valid hierarchy and finite matrices. All14 mouth,14 eye and5 eyebrow
values round-trip through the public AvatarExpression properties before the demo restores and draws
its asymmetric expression.

AvatarRendererState is covered as a lifecycle rather than an ordinal list: real runs observed the
recognized description in Loading, later asserted Ready and used its bind pose, while a nonzero
1021-byte description that satisfies AvatarDescription.IsValid but cannot be decoded produces
Unavailable deterministically. Both renderer selections pass the complete five-demo avatar batch;
the changed workflows pass16,6 and9 controls. This moves fields515->639/753 and GamerServices
1->125/223. It does not claim a fixed asynchronous loading duration, image-quality inspection or
compatibility with Xbox LIVE avatar bytes. Evidence:`/tmp/checkpoint-913-avatar-*.json`.

### GamerServices defaults, presence, account policy and leaderboard values

Three isolated local profiles carry Easy/Low/Back, Normal/Medium/Front and Hard/High/Inside through
the production profile JSON reader, real sign-in and public GameDefaults getters. This closes all
nine values in those three families. The live presence workflow observes None, retains AtMenu and
then combines Stage with a changing PresenceValue. The other57 presence labels select strings for
the same private service publication path; cycling their integer identities would add no distinct
offline behavior, so they remain intentionally uncredited. Similarly, a local profile proves the
actual Everyone privilege result and Unknown gamer zone. Blocked/FriendsOnly and the four named
zones come only from service account state in pinned CNA and remain service-test scope.

All four stock leaderboard keys now create exact key strings and distinct game modes through
LeaderboardIdentity.Create. Four real local gameplay writers persist rows whose public dictionary
columns return Win, Tie, Loss and explicit None after sorted paging. Both renderer runs reach all21
controls and retain only the established ISSUE-085 stream-column failure. This moves fields
639->661/753 and GamerServices125->147/223. Evidence:
`/tmp/checkpoint-914-gamer-services-*.json`.

### Guide notification positions, message-box icons and final GamerServices field disposition

A single live sign-in toast now traverses all nine NotificationPosition cells. Back-buffer footprints
under both renderers are identical:9882 non-background pixels with distinct exact bounds for every
row and column. Four asynchronous message boxes render None, Error, Warning and Alert before their
simulated choice; pairwise comparison in the icon/card region distinguishes every rendering while
the existing visibility, callback state, selected index and overlapping-dialog refusal controls
remain active. The changed workflows pass14 and15 controls, the full Guide category passes5/5 on both
renderers, and ten repeated SOFTWARE CTests are stable.

GamerServices reaches160/223 fields. The remaining63 are a deliberate semantic remainder. Fifty-
seven GamerPresenceMode labels map to canned service strings but otherwise use the same setter,
revision and publication path already exercised by None, AtMenu and Stage; pinned CNA exposes no
local public readback of the formatted string. GamerPrivilegeSetting::Blocked/FriendsOnly and the
four named GamerZone values are supplied only by a configured service account, whereas the local
profile paths establish Everyone and Unknown. Integer round-trips would not demonstrate those
policies. Evidence:`/tmp/checkpoint-915-guide-*.json`.

### OcclusionQuery (result observation and resource lifetime)

Source review at CNA `e842e7473`: public OcclusionQuery.hpp, Xna/OcclusionQuery.cpp and
`graphics.occlusion_query.visible_pixels`. Complete469-shadow already records the existing
three visibility controls passing on SOFTWARE and OPENGLES3. The same example now adds eight
ordering/disposal controls, passing eleven checks on both selections at CNA `dc9c34f62`.
All four actual before/after captures were inspected; stderr is empty. The entire original
Update/Draw/EndRun source remains unchanged. The moving light is time-dependent, so no
whole-image identity or fixed GPU completion-latency claim is made.

| Members / behavior | Decision and evidence |
|---|---|
| Constructor, Begin and End | visible_pixels creates three queries and brackets actual draws of hidden, open and moving lights. Its fixed controls establish depth rejection and visibility. Constructor refusal under Reach or absent renderer capability is a separate documented path: declared needs/profile selection do not execute every constructor error. A shared profile/capability example could add it if useful; no separate constructor-only scene. Focused1012 also brackets a distinct owner/contender pair and exposes ISSUE-113: both tested renderers accept the contender's Begin while the owner is active instead of throwing the header's documented InvalidOperationException. Safe contender disposal plus owner End leave every later query usable. |
| IsComplete, PixelCount and result reuse | The fixed queries are read only after both complete, and the moving query is reused after observing completion. EndRun requires the fixed results to arrive during the automated run. This establishes a bounded polling workflow, not fixed GPU completion latency. A scratch-query preflight now checks fresh IsComplete=false, refused PixelCount without a result, End before Begin, repeated Begin and reuse after End without an IsComplete observation. The successful End after a refused repeated Begin confirms the active pair survives that refusal. With Focused1012 the scene executes thirteen controls: twelve pass and the distinct-active Begin check retains ISSUE-113 on both selections. The original hidden/open counts remain 0/4422 on SOFTWARE (precise) and 0/1 on OPENGLES3 (boolean). The source records an IsComplete observation even when it returns false; the reuse guard is observation, not a promise that earlier GPU work completed. Do not infer that premature reuse is safe on every renderer or require an initially false GPU poll. |
| isPixelCountPreciseEXT | The existing example selects either a real tally greater than 100 or the documented boolean value 1 for the open light; the hidden light must always yield zero. The extension prevents interpreting an ES3 boolean result as a coverage ratio. This does not establish exact cross-renderer raster counts, sample-versus-pixel behavior with MSAA, or all renderer implementations. |
| Dispose, IsDisposed, HasRenderer and disposed operations | Normal query ownership is demonstrated by unique_ptr cleanup. The scratch-query preflight ends one native pair, calls public Dispose, verifies IsDisposed and HasRenderer=false, and requires ObjectDisposedException from subsequent Begin and PixelCount. Focused694 corrects metadata to name `OcclusionQuery::Dispose` as well as the inherited resource path. Focused1009 adds a distinct query disposed while its Begin/End pair is active; the later hidden/open queries both complete, proving that SOFTWARE and EasyGL release their single native active-query slot. Ten Focused1009 runs per tested renderer passed12/12 with stable output and empty stderr; Focused1012 retains all twelve passes beside its separate expected Begin failure. Repeated Dispose and query destruction after earlier device/context teardown remain separate; no device-loss or all-backend teardown proof is claimed. HasRenderer is a small lifetime diagnostic, not an independent scene. |
| GetTypeName, inherited Name/Tag and protected Dispose(bool) | Reflection identity and inherited resource bookkeeping do not justify duplicate demos per subclass. The protected hook is framework lifecycle plumbing; applications use public Dispose. The public preflight avoids internal query handles and renderer bridges. |

### VertexElement and custom vertex declarations

Source review began at CNA `cc547f5224` and Focused1013 rechecked the public lifetime/draw paths at
pinned CNA `d23e139de`: public VertexElement/VertexDeclaration headers, FNA's retained-declaration
reference path and the existing `graphics.vertex_declaration.custom_vertex` example. The example
now executes16 controls under SOFTWARE and OPENGLES3. Ten runs per renderer have byte-identical
stdout and empty stderr.

| Members / behavior | Decision and evidence |
|---|---|
| VertexDeclaration construction, VertexStride and GetVertexElements | custom_vertex keeps its plain20-byte Position/Glow/PackedTint structure and its original stride check. Three added controls compare every returned element against the actual `offsetof` values, storage formats and Position0/TextureCoordinate1/Color0 semantics. The layout is then passed directly to DrawUserPrimitives. |
| VertexElement Offset, VertexElementFormat, VertexElementUsage and UsageIndex | The controls read all four public fields for all three descriptors. A separate mutable value is authored through all four setters and compared with the declaration's middle element, proving the property round trip without changing the declaration used for drawing. |
| Equals, equality/inequality, GetHashCode and ToString | The declaration's complete element vector equals an independently authored value vector; the matching middle elements satisfy typed Equals and share a hash, while a distinct element compares unequal. The edited descriptor produces the exact readable text `{Offset:12 Format:Single Usage:TextureCoordinate UsageIndex:1}`. These operations belong together as value identity and diagnostics; they do not justify separate demos. |
| Actual layout consumption | Focused1013 gives buffered and user-memory paths separate first-frame readbacks. Each finds a white center and black exterior on both renderers. Together they prove that Position and packed Color at the audited offsets drive the visible fan while the extra TextureCoordinate1 field remains legal and unused by BasicEffect. Pixels were read programmatically; no screenshot or visual inspection was performed. |
| VertexDeclaration Dispose and inherited resource plumbing | A separate complete20-byte declaration is disposed twice, reports disposed and is refused with ObjectDisposedException when a VertexBuffer tries to bind it. Focused1013 also binds a live buffer first, disposes its source declaration and observes the shared disposed identity through the retained `VertexBuffer::VertexDeclaration` without disposing or unbinding the buffer. The active buffer still produces the exact fan pixels, matching FNA's retained pinned-layout path. Protected hooks, type names and teardown after device loss remain framework or destructive lifecycle decisions. |

### DirectionalLight construction defaults

`graphics.basic_effect.default_lighting` now constructs an unbound public DirectionalLight and
requires XNA's exact defaults: disabled, Down direction, One diffuse and Zero specular. The same
scene then retains its real three-light rig and programmatic sphere readback. Both renderers pass5/5
with byte-identical stdout and empty stderr. This credits construction without claiming that an
unbound light affects an Effect. Evidence:`/tmp/checkpoint-694-default-light-*`.

### Stock vertex declarations and value semantics

Source review at CNA `09c3108e5`: the four stock vertex headers and implementations, `IVertexType`,
and the existing `graphics.vertex_declaration.stock_vertex_types` example. The example retains its
original rendered checker/tint workflow and now has eight focused controls. SOFTWARE and
OPENGLES3 pass all eight through the project build, with byte-identical stdout and empty stderr.

| Members / behavior | Decision and evidence |
|---|---|
| Static VertexDeclaration and IVertexType property | Each of VertexPositionColor, VertexPositionColorTexture, VertexPositionNormalTexture and VertexPositionTexture returns the same stable declaration object through its static member and IVertexType property. The declarations are compared element-for-element: packed strides are 16, 24, 32 and 20 bytes, with exact Position, Color, Normal and TextureCoordinate offsets, formats, usages and usage indices. The explicit CLR inventory aliases for interface implementation remain intentionally uncredited because the same C++ property path is executed directly. |
| Constructors and public fields | Independently authored Position, Color, Normal and TextureCoordinate values survive all four public constructors and are read back from every public field. These checks document actual packed-value state in the same workflow; constructor or field-per-demo expansion would add no coherent concept. |
| Equality, inequality, typed/boxed Equals and GetHashCode | Copies of every stock value compare equal through the operators and typed/boxed Equals, while changing one field makes each value unequal. Boxed equality rejects another stock vertex type, and equal copies produce equal hashes. This is one shared value-identity contract rather than four independent examples. |
| ToString | Exact field-labelled output is checked for all four values, including Normal and TextureCoordinate. Diagnostic formatting belongs with value semantics and does not justify a separate demo. |
| Actual declaration consumption | The retained VertexPositionColorTexture quad still reaches DrawUserPrimitives without an explicit declaration argument. Programmatic samples find the same white and orange checker/tint pixels under both renderers, tying the audited declaration to drawing. No screenshot or visual inspection was performed. |

Temporary generated coverage moves the complete634 XNA-member baseline from1417 to1437/2326;
the demo count, type count and C-route count do not change. The projection will replace generated
`docs/COVERAGE.md` only at the next complete runtime checkpoint.

### Model, ModelBone and collection pose workflow

Source review at CNA `49b1d3a998`: public Model/ModelBone collection headers and implementations,
plus the existing `graphics.model.bones` and XNB model examples. The bones demo now adds eight
focused controls and passes all12 checks under both requested renderers through an isolated relink
against the last complete libraries. The stdout files are byte-identical and stderr is empty.

| Members / behavior | Decision and evidence |
|---|---|
| Bones, Meshes, Root; bone Name/Index/Parent/Children; collection Item/Count, TryGetValue and Contains | bones retains its authored four-node hierarchy and all four original checks. Exact bone and mesh lookups return the same objects as indexed access; missing names clear their output pointers; membership rejects detached/null objects; and Forearm is found through UpperArm.Children. Existing C++ range-for loops traverse both collections. The explicit CLR-style Enumerator structs remain language interop machinery and do not justify separate demos. |
| CopyBoneTransformsFrom, CopyBoneTransformsTo and CopyAbsoluteBoneTransformsTo | A complete local array moves the root by +1 on X while preserving every descendant local value. Local readback is exact, the Forearm's absolute translation gains exactly UnitX through hierarchy composition, an extra source value is ignored by CNA's documented C++ safety behavior and a larger destination keeps its sentinel tail. The original authored array is restored exactly before LoadContent returns. |
| Short transform arrays and atomicity | A short source throws ArgumentOutOfRangeException before changing any local transform. Short local and absolute destinations both throw while preserving every sentinel element. This is the useful array-boundary contract; null/raw arrays do not exist in the C++ vector API, and repeating each possible deficit adds no behavior. |
| Model constructors, Tag, owned resources and drawing | Existing procedural mesh and C ABI workflows cover constructor graph ownership; node_animation/skinned_animation use typed Tag payloads; Model.Draw has exact pixels in draw and mesh_effects. Arbitrary Tag setter/getter identity and setOwnedResources are carrier/plumbing operations and do not justify independent scenes. `mesh_parts` already clears one part's Effect, verifies its omission from `ModelMesh::Effects` and proves by exact pixels that only the sibling draws; the earlier null-effect gap is closed. |
| ModelCameraEXT projection metadata and live placement | `gltf_cameras` now loads finite perspective, orthographic and infinite perspective cameras in source-node order. It verifies authored versus absent aspect, exact orthographic projection with perspective-only fields at defaults, and exact reconstruction of an absent-zfar projection through `CreateInfinitePerspectiveFieldOfViewEXT`. An imported Dolly scene-node clip targets the camera's exact Model bone; sampling it moves the live absolute transform from x0 to x0.8 while `WorldTransform` remains the import-time snapshot, and the live view supplies the actual left-viewport draw. Both renderers execute 13 controls with only established compiled-camera ISSUE-023; inspected captures are byte-identical. Camera setter-only replacement, invalid projection-builder permutations already pinned by CNA tests and another static framing variant do not justify separate demos. |
| GltfImportReportEXT and GltfImportDiagnosticEXT | `import_report` now reads all twelve structural fields across real camera, shared-skin and punctual-light scenes. Its camera report distinguishes four instances/distinct meshes, three cameras and one retained clip; the skin report distinguishes two placements of one shared mesh and two skins; the light report distinguishes two authored light nodes from two retained effect slots. Existing robot, material-variant, topology and skin-weight fixtures supply Information, GeneratedData, DroppedData and Approximation entries. A loose copy of the existing lit sphere deliberately adds stale accessor bounds and an optional unknown extension for InvalidSourceData and UnsupportedFeature. The demo checks all eight diagnostic payload fields, both severities, all six kinds and exact aggregate semantics: WarningCount counts entries while DroppedFeatureCount and ApproximationCount sum occurrences. The compiled robot proves every field and aggregate of the empty default. SOFTWARE and OPENGLES3 pass all fourteen controls; inspected captures are byte-identical. Setter-only synthetic report replacement and every individual importer diagnostic code do not justify separate demos after the complete public data contract and representative real outcomes are covered. |
| Rigid glTF node instances, scene selection and composed transforms | New `node_instances` imports one shared mesh at five nodes through hierarchical TRS, an explicit glTF matrix and odd/even composed mirrors. It checks exact bone ancestry, world centres and transformed axes, unique placement names/parents, source counts and exclusion of a mesh-bearing scene-zero decoy while explicit scene one is selected. Optional `EXT_mesh_gpu_instancing` retains only the node placement and produces distinct unsupported-extension plus dropped-instance diagnostics; the two authored GPU translations remain visibly absent. A fixed glTF cull state draws an ordinary placement but drops the odd mirror, while choosing CullClockwise/CullCounterClockwise from each public world determinant draws all five placements. Its extension proves implicit first-scene selection when the default index is absent, de-duplicates a repeated root shared by two scenes, preserves transformed root hierarchies when the whole scenes array is absent, and attaches two bare meshes to the identity root when neither scenes nor nodes exist. A parsed orphan node cycle is rejected through ContentLoadException before traversal. SOFTWARE and OPENGLES3 run24 controls with one retained failure: CNA-DEMOS-ISSUE-092 records that an empty selected scene revives an unselected mesh at the synthetic root and reports NodeCount0 with MeshInstanceCount1. Pathological-depth and pointer-corruption stress cases remain CNA internal tests; the meaningful public scene/node-graph boundary is otherwise closed. |

### ModelMesh and ModelMeshPart

The existing model examples load authored glTF/CNB geometry. That does not establish every
mesh-part member merely because the loader populated it internally. This review uses the
public ModelMesh/ModelMeshPart headers and their Draw/setEffect implementations at the same
CNA revision as the other tables. The new mesh_parts demo now supplies targeted evidence
for the shared-buffer/material rows; other proposals remain unexecuted.

| Members / behavior | Decision and evidence |
|---|---|
| ModelMesh::Draw, Effects, Model::Draw | Existing mesh_effects changes each loaded BasicEffect's tint/alpha and checks rendered pixels; skinned_animation explicitly calls mesh.Draw after assigning bone transforms. Model.Draw supplies world/view/projection to effects before calling each mesh. This does not demonstrate changing a part's Effect pointer. |
| MeshParts, NumVertices, PrimitiveCount, StartIndex, VertexOffset, VertexBuffer, IndexBuffer | mesh_parts now passes 10 checks on SOFTWARE/OPENGLES3: two parts share a vertex/index buffer but select distinct triangle halves using StartIndex 0/3 and VertexOffset 0/4. Actual colored and untouched pixels distinguish both ranges; screenshots were inspected and are RGBA-identical. Existing imported-model draws alone did not teach those ranges. PrimitiveCount is topology-dependent; the CNA extension can carry lines/points, so multiplying every count by three is incorrect. |
| ModelMeshPart::Effect setter and ModelMesh::Effects membership | mesh_parts verifies one Effects entry for a shared effect, two after replacing one part, then removal/deduplication when the last use of the old effect changes. Its focused extension clears only the right part's Effect: green leaves Effects, the left triangle still draws red and the right sample stays at the clear color. Restoring green returns normal drawing. Both renderers pass this exact pixel and membership control. |
| CNA constructors and SetVertexOffset/SetNumVertices/SetStartIndex/SetPrimitiveCount/SetVertexBuffer/SetIndexBuffer | Public CNA extensions support procedural geometry/content loaders. The shared-buffer example uses the constructors; independent setter-per-demo variants are unnecessary. Its focused extension temporarily sets the left PrimitiveCount to zero: the red region stays clear, the green sibling still draws and red remains in Effects, distinguishing geometry omission from effect detachment. Buffer/effect pointers are stored, so the demo owns those objects for the whole mesh lifetime rather than implying ownership transfer. |
| ParentBone, BoundingSphere | bones now reassigns the loaded Forearm mesh from its authored parent to bone zero and proves `Model::BoundingSphereEXT` changes immediately; assigning null reproduces `Model::Draw`'s bone-zero fallback, and restoring the parent restores the exact authored aggregate bound. morph_targets separately proves caller-managed mesh-bound replacement after procedural deformation. frustum_culling uses the resulting loaded bounds. Independent getter-only demos are unnecessary. |
| Name, Tag; mutable Effects extension | Names are diagnostics and Tag carries game data; no getter/setter-only demo. Do not mutate the Effects collection directly to simulate an Effect setter: that would bypass the part/collection relationship being demonstrated. |
| PrimitiveTypeEXT and imported sampler-state extensions | mesh_parts draws LineList and TriangleList parts in one mesh, sharing buffers with different ranges. Its line uses PrimitiveCount=1 and two vertices; three pixel columns verify it above the triangles. part_sampler_state passes 10 checks for stored PointClamp metadata versus application PointWrap and explicit selection, plus an actual unlit glTF with NEAREST/CLAMP metadata, owned texture and preserved out-of-range UVs. imported_topology passes 39 checks per renderer with all seven glTF modes: exact public topology/count/index data, loop closure, strip/fan conversion, preserved POINTS, a direct open LINE_STRIP and one reported/dropped tail from each LINES/TRIANGLES run. Actual point, open/complete line and filled-region pixels pass on SOFTWARE/OPENGLES3; inspected captures are byte-identical. `specular_sampler_state` separately imports nearest-clamp strength and nearest-repeat colour metadata through the two-entry `SpecularSamplerStatesEXT`, then selects slots5/6 explicitly. Exact black/green41/red41 reflections pass on OPENGLES3/OPENGL33 with byte-identical inspected captures; SOFTWARE declares its PBR sampling limit. |
| Model material-variant complete state | `material_variants` now passes 22 controls on SOFTWARE/OPENGLES3. Its red default and textured Gold are unlit 24-byte colour layouts, while Ocean selects the 60-byte coloured PBR layout. Selection replaces the exact Effect, VertexBuffer, sampler array, layout-specific MorphTargetDataEXT Tag and sole `ModelMesh::Effects` member; `-1` restores all original identities and linear-wrap state. Gold's independent half-weight morph state survives a round trip through default and is then reset. Its owned 2x1 texture at out-of-range U=1.125 renders cyan under an explicit PointWrap control and gold under the imported PointClamp state; default red, lit-blue Ocean and unmapped black wheels remain visible. Both final-state screenshots were inspected. The first GL threshold assumed an unlit blue intensity even though Ocean deliberately became PBR; hue-correct output identified a demo oracle error, not a CNA defect. The C sibling `c_api.content.model_loading` now likewise binds PointClamp explicitly and uses a blue-dominance oracle, restoring its 30/30 SOFTWARE/OPENGLES3 result after the shared fixture deepened. This establishes complete replacement through the public loaded-model boundary rather than inferring it from CNA importer unit tests. Compiled variants remain unsupported by the content format and are not claimed. |
| SkinnedModelEXT, PartEXT and attachment APIs | skinned_parts now passes 11 checks on both renderers: AddPartEXT transfers procedural buffer/part/texture ownership; AttachPartEXT moves donor parts, replaces by name and retains the receiver's clip; donor destruction precedes actual rendering. A mismatched bone count is refused before either part list moves. RemovePartEXT removes a temporary center part; surviving colored pixels and the clear center are checked. Both screenshots were inspected and are RGBA-identical. This does not measure GPU deallocation/leaks, nor validate same-count skeleton/index-order compatibility; callers must supply the same canonical skeleton. The separate skinned_manifest now demonstrates imported manifest sidecars and rendering. |
| SkinnedModelEXT::ComputeBoneTransformsEXT | skinned_parts samples a one-bone linear translation at 0.5 s and verifies the nonidentity inverse-bind correction both numerically and through actual shifted geometry/vacated pixels. palette_sampling now has fifteen SkinnedModelEXT CPU checks plus five AnimationPlayer checks: three-bone parent composition, untracked bind pose, interpolated scale/quaternion rotation/translation, inverse binds, positive/negative loop and clamp, missing clip, self-parent and truncated arrays with preserved output. Its new control appends empty, negative-index and past-end tracks and proves they are safely skipped without changing the valid pose. Unsorted keys violate BoneTrackEXT's documented ascending-time precondition; duplicate-time import behavior is already covered by node_animation, while arbitrary duplicate tracks for one bone have no promised merge rule and should be normalized by the producer. These malformed hand-authored variants are intentionally not presented as supported behavior. |
| SkinnedModelEXT testing counters, move-only boilerplate | GetOwned*CountForTesting explicitly exposes test instrumentation: intentionally leave direct counter assertions to CNA tests. Useful public ownership evidence should come from the moved/replaced/removed parts and rendered results. Deleted copy operations and empty construction need no standalone demo. |

The generated inventory leaves only the collection `GetEnumerator` members and `ModelMesh::Tag`
uncredited in the core Model family. The former are CLR/C# interop machinery already represented
by real C++ range iteration. The latter is an arbitrary application-data carrier equivalent to the
demonstrated model and mesh-part tags. These have explicit dispositions and do not justify more
model demos, so the audit proceeds to other public-member families.

### AnimationPlayer, SkinningData and clip target spaces

Source review: AnimationPlayer.hpp/.cpp plus existing skinned_animation/node_animation demos,
CNA a796ef491 (library code unchanged from e60fa35f7). These are CNA extensions inspired by
XNA's sample code, not framework types shipped by XNA. Shared keyframe aliases do not make the
Model/SkinningData player and the separate SkinnedModelEXT evaluator the same behavior path.

| Public members / behavior | Decision and evidence |
|---|---|
| AnimationPlayer constructor, StartClip, Update, GetSkinTransforms | skinned_animation loads SkinningData from Model.Tag, starts Bend, seeks absolutely to 0.5 seconds and validates the joint angle plus a rendered bent tip. It now passes 10 checks on both renderer selections, explicitly covering relative accumulation versus absolute seek, positive wrap, end clamp and restart. Before/after first-frame RGBA pixels are identical on each renderer; cross-renderer silhouettes agree but pre-existing lighting differs (8,216 pixels, maximum channel delta 38). No lighting-equivalence claim. |
| GetBoneTransforms / GetWorldTransforms / GetSkinTransforms | World transforms supply the checked upper-joint angle and skin transforms feed SkinnedEffect. palette_sampling now reuses its three-bone authored hierarchy through a separate SkinningData/AnimationPlayer, checks local versus ancestor-composed origins and inverse-bind point coordinates, then compares both evaluators on joint and offset points. All nineteen CPU checks pass under both renderer selections, including its fourteen original SkinnedModelEXT controls. The owner/clip outlive the borrowed player; no extra rendering scene is needed. |
| CurrentClip, CurrentPosition and reference lifetimes | StartClip retains a pointer and the player retains its SkinningData owner by reference; keep both alive. State before StartClip, borrowed active-clip identity, reset-to-zero on restart and seek-versus-advance are checked in skinned_animation. animation_player_validation additionally edits the selected clip's final translation after StartClip and proves the next sample observes it, then restores the value and pose. Dangling-reference misuse is intentionally not demonstrated. |
| SkeletonRootPrefix and rig-root metadata | The new fixture exercises a root joint below a non-joint ancestor and an authored transformed mesh node. Root name/index are lookup metadata, not instructions to truncate ancestry. multiple_skins now verifies ancestors at -0.5/+0.5, retained authored mesh nodes at +4/-4, identity actual mesh parents and ancestor-only prefixes. Root animation retains the prefix and actual pixels confirm the positions. ISSUE-042 records comments describing the older inverse-mesh-prefix representation. More complex ancestry, nonuniform scale and other content carriers remain distinct gaps. |
| ApplyBindPoseBoneTransformsEXT, multiple skins | multiple_skins checks both imported bind palettes, distinct skeleton/effect objects despite shared authored geometry, two primitive/material parts per placement, animation of the left root, deliberate disturbance and selective restoration of both right-palette effects, preservation of both left effects and live bounds. Its 21 checks pass on SOFTWARE/OPENGLES3 with the shared source animation. New multi_skin_animation_report proves that one animation split across two palettes is retained as two independently playable one-track clips. Its public report correctly distinguishes one source animation from two retained clips but falsely counts each channel as unavailable to the other palette, producing two dropped occurrences despite complete aggregate retention (ISSUE-095); both renderers pass8/9. skin_weight_import separately supplies one five-joint skin across four mesh placements. This closes the meaningful multiple-part and multiple-palette pose mapping behavior; more part-count variants add no new contract. |
| ClipTargetSpaceEXT / ApplyClipToBonesEXT and rigid glTF channel retention | node_animation now passes nineteen checks on both renderers. Its original contract controls cover a translated untracked base carrying an animated descendant, copied JointPalette refusal, preservation of every local transform on refusal, and negative/past-end clamping. A loose four-animation fixture retains exactly three clips: disjoint translation/scale times become a four-key union with the rest rotation, duplicate adjacent times keep the earlier value, the resulting pose applies exactly, and an unnamed animation becomes stable `Clip3` with its nonzero first-key hold and interior interpolation. An off-scene target, unsupported morph-weight path and now-trackless animation are omitted with exact GltfImportReportEXT codes and aggregates. Existing earlier first-frame captures remain the visual evidence for the unchanged turntable; Focused849-850 made no screenshot or visual-quality claim. ApplyClipToBonesEXT owns the documented cross-space refusal because its input can be either clip kind. AnimationPlayer and SkinnedModelEXT are joint-palette carriers and expose no target-space exception contract; deliberately placing a SceneNode clip in those palette collections would be invalid application routing, so no demo asserts an invented refusal. Revisit only if CNA gives those evaluators an explicit guard. |
| Loose glTF animation sampler validation | New animation_validation passes four of five controls on both renderers. The public ContentManager loader gives a contextual strict-order refusal for decreasing input times, structurally refuses an output accessor shorter than its input and remains usable for a valid load. An unknown interpolation token is silently parsed as LINEAR with no warning; the required-refusal check alone fails as CNA-DEMOS-ISSUE-093, while an adjacent control pins the exact two-key midpoint fallback. Standard missing-reference/type/count permutations remain cgltf structural-test territory rather than separate application scenes. |
| Mixed skinned and rigid scene-node tracks | skinned_animation now passes fifteen checks on both renderers. A loose `MixedMotion` source targets one real joint and one rigid node. SkinningData retains exactly the JointPalette track, AnimationPlayer reaches its final Y=.2 pose and the rigid ModelBone stays at rest X=.5. The public report distinguishes the unavailable palette target from the final `rigid-animation-dropped-on-skinned-model` carrier loss, each count one, and names the affected clip. This establishes CNA's current one-Tag retention boundary without presenting the dropped rigid motion as supported. Multiple-skin duplicate accounting remains separate. |
| MorphTargetDataEXT and morph-weight animation carriers | New morph_animation_import passes six of seven controls on both renderers. The same original asset's compiled and loose part Tags preserve identical base bytes, stride, two targets, zero defaults and four-key linear track. Evaluation at1.5 seconds yields weights(1,.5), CPU blends match and SetMorphWeightsEXT plus raw VertexBuffer readback prove exact loose upload. The required report invariant alone fails: the loose loader labels this retained playable channel unsupported and its animation empty, producing two false dropped occurrences (CNA-DEMOS-ISSUE-094). The exhaustive C morph workflow separately covers owned linear/STEP/CUBICSPLINE tracks, Hermite tangents, normal/tangent deltas, flat-normal recomputation and invalid inputs; duplicating each value permutation in C++ adds no semantic behavior. |
| glTF STEP/LINEAR/CUBICSPLINE Model carriers | clip_interpolation now runs twelve controls through byte-equivalent compiled and loose forms of the same generated source. Both expose `Slide`, agree exactly at.25 seconds and reach the common authored key at one second; the loose report retains one clean clip with no warning/loss. LINEAR gives the correct -1.5 in both paths. Four required assertions fail under CNA-DEMOS-ISSUE-031: STEP gives -1.5 rather than holding -2, and CUBICSPLINE gives -1.5 rather than Hermite -1.6875 through each carrier. This is direct public runtime evidence, not inference from shared source. Multi-skin animation/report accounting remains separate. |
| Malformed arrays / parent ordering | New animation_player_validation passes nine of ten controls on both renderer selections. Construction refuses required-array size mismatches and forward parents. Each Update revalidates later borrowed hierarchy/array mutation, and restored data recovers on the same player. Empty root prefixes mean identity and complete prefixes compose exactly. The sole failed invariant is ISSUE-096: a nonempty partial SkeletonRootPrefix is silently treated as entirely absent both at construction and after mutation, rather than receiving the same size validation. No destructive dangling-data probes are needed. |
| Model::BoundingSphereEXT with live skin palettes | multiple_skins reads its current aggregate sphere and chooses a temporary joint pose guaranteed outside it using the observed center/radius. A new sphere includes that moved center; restoring the palette excludes it again before the final pixel checks. skin_weight_import reads every imported influence and proves joint four has no nonzero weight, then moves only that unused joint from identity to X=8. The model-bound radius grows from 1.037 to 4.857 while all established rendered positions remain exact, demonstrating the documented safe full-palette overbound. morph_targets separately covers caller-managed morph bounds. |
| Keyframe/AnimationClip aliases, GetTypeName | Aliases share the EXT data representation and need no duplicate examples. Reflection names are trivial unless used in an actual content dispatch workflow. |
| Loose SkinnedModelEXT content bundle | Source/documentation supports Load<shared_ptr<SkinnedModelEXT>> with manifest-relative skeleton, geometry, texture and clip files; it validates truncated data. skinned_manifest now passes 12 checks per renderer with an original nested bundle: loaded hierarchy, mesh ranges, texture, binary clip, rest/final palette and truncated-skeleton refusal. Actual stationary/moved/vacated pixel checks pass; inspected screenshots are RGBA-identical. The eight sidecars regenerate byte-identically. This is separate from the existing glTF-to-Model/SkinningData route and does not establish XNB ModelReader coverage. |
| XNB Model source route | The existing content_xnb CMake target already requests FORMAT xnb. Source review of XnaModelSourceContentPipeline confirms `.x` resolves to CNA.XImporter and CNA.XnaModelProcessor, whose canonical model graph feeds the XNB writer. xnb_model now loads an original `.x` compiled by that target and passes 11 checks per renderer: actual XNB signature, five named bones/two meshes, hierarchy composition, shared vertex/index buffer pointers, independent ranges and distinct material effects. Actual two-color placement/gap checks pass; inspected captures are RGBA-identical. This is CNA-produced XNB evidence, not an execution by genuine XNA, and does not demonstrate custom reader fixup callbacks. |

### SkinnedEffect (palette contracts versus rendered behavior)

Read against SkinnedEffect.hpp/.cpp and the actual two_bones, skinned_animation,
effect_interfaces and parameter_values sources. The palette extension and SkinnedModelEXT example now have targeted runtime evidence on both renderers. A valid joint palette alone does not establish lighting,
weighted interpolation or all shader variants.

| Members / behavior | Decision and evidence |
|---|---|
| Constructor, World/View/Projection, Texture, SetBoneTransforms | two_bones draws an authored strip with two joint matrices; skinned_animation feeds an imported clip through AnimationPlayer. Its three pixel checks establish the stationary lower tip, moved upper tip and vacated old tip. Matrix/property forwarding does not need separate getter-only examples. |
| WeightsPerVertex, VertexPositionNormalTextureSkinned | two_bones selects two influences and checks filled/empty joint-edge pixels from its equal-weight center row. `graphics.skinned_effect.weight_channels` renders one/two/four active channels with nonzero unused lanes for one/two and four permuted indices, including the last palette slot; analytic samples detect dropping any contribution or failing to renormalize. `graphics.model.skin_weight_import` adds the importer boundary: normalized UBYTE weights with USHORT `{3,1,2,0}` joints read back as exact fractions, malformed and post-truncation 0.6 sums become `{.5,.5,0,0}`, and a zero sum stays zero. Its public report counts eight repaired vertices, one dropped second set with worst share .4 and four zero-weight vertices. Three controlled palette draws and the full-palette-bound controls pass24/24 on SOFTWARE/OPENGLES3; Focused839's inspected captures remain the visual evidence for the unchanged scene. This establishes all four imported channels and repair/truncation policy, not arbitrary weights, rotation/normal blending or every encoding/error permutation. |
| GetBoneTransforms, MaxBones, invalid palette/weight changes | two_bones now passes 13 checks on both renderers: GetBoneTransforms round-trips the uploaded two-matrix pose; empty and oversized palettes and three weights are refused with their declared exceptions; the valid palette/weight count survives and still draws the expected geometry. Both screenshots were inspected: silhouettes match, with 1,342 pixels differing by at most one color-channel level. Zero and oversized GetBoneTransforms counts are now refused with ArgumentOutOfRangeException. An accepted MaxBones-sized palette round-trips its original first matrices and a distinct final translation, then the original two-bone pose is restored before rendering. The existing weight_channels demo now also draws a fourth mesh with indices 2,0,MaxBones-1,1 and a distinct last-slot transform. All six original checks remain and five center/near-edge probes form a seventh passing assertion on both renderers. Both inspected captures are RGBA-identical; exactly 7,504 background pixels become cyan versus original checkpoint-448, with all older colored regions unchanged. The original short palette is restored each frame before those controls. This establishes actual last-slot addressing for the authored translated four-influence mesh; arbitrary joint rotations/normals remain separate. ISSUE-035 concerns EffectParameter.Elements introspection, not proof that palette uploads fail. |
| EnableDefaultLighting, DirectionalLight0/1/2, AmbientLightColor, LightingEnabled | Default lighting is used in two_bones and skinned_animation. clone_ownership verifies ambient-only material colors after disabling all three directional lights. `normal_lighting` now isolates light 0 with zero ambient/specular: an unchanged normal, a 60-degree joint rotation, a shorter equal identity/120-degree blend and that blend under nonuniform World scale yield the analytically expected diffuse colors. Five checks pass on both renderers, including declared NotSupportedException for LightingEnabled=false with lighting retained. The metadata name SkinnedEffect::LightingEnabled intentionally names CNA's actual public IEffectLights override; it is unmatched by the XNA inventory, so no extra XNA member credit is inferred. Both 800x480 captures were inspected: identical silhouettes; 31,104 middle-card pixels differ by one red level (235 SW / 234 GL), within original tolerance 1. This establishes transformed normals, post-blend normalization and World inverse-transpose behavior for the authored fixture. It does not establish nonuniform joint-scale correction or arbitrary weights. The specular_lights scene below now also isolates both lights' independent diffuse terms and adds them to the retained specular controls. |
| DiffuseColor, Texture and fog members | clone_ownership renders distinct DiffuseColor values with ambient-only light and one shared texture. `graphics.skinned_effect.fog_depth` now shares a Z=0 bind mesh across bone-only depths 2/4/6, with World only separating cards sideways. Four checks pass on both renderers: the disabled-fog control preserves Gold at every depth, then FogStart/midpoint/FogEnd yield Gold, half-blended color and CornflowerBlue. Both 800x480 captures were inspected; each card has 39,168 pixels and only the middle blue channel differs (119 SOFTWARE, 118 OPENGLES3), within the original tolerance 1. Stderr is empty. This establishes post-skin view-depth fog under an orthographic projection, not every fog range, material/alpha combination or renderer. BasicEffect fog driven by World transforms alone could not establish this behavior. Selected emissive/ambient/alpha, texture/vertex alpha and specular behaviors now have the focused rows below. effect_interfaces checks SkinnedEffect's interface availability but draws its separate Basic/AlphaTest/EnvironmentMap scene, so that scene does not validate skinned fog or per-pixel lighting. Other lighting combinations remain separate; scalar getter/setter permutations are trivial. |
| EmissiveColor, AmbientLightColor, DiffuseColor, Alpha | `graphics.skinned_effect.material_colors` shares one bone-translated mesh and texel (128,255,64) across emission-only, ambient-diffuse-only, summed and half-alpha cards. Four checks pass on both selections: (32,128,48), (51,26,10), (83,153,58), then (54,89,85) from premultiplied AlphaBlend over MidnightBlue. Directional/specular terms are disabled, so the controls isolate the actual material arithmetic. Both 800x480 captures were inspected: identical silhouettes, with only 41,472 ambient-panel pixels differing green 26 SW/25 GL within original tolerance 1. Empty stderr. This establishes global material Alpha with opaque texture texels, not texture/vertex-alpha combinations, fog interaction or every renderer. |
| SpecularColor, SpecularPower, DirectionalLight1/2 and independent diffuse contributions | `graphics.skinned_effect.specular_lights` retains its four black-texture specular controls: light1 red, light2 blue, their sum and power8 produce RGB(102,0,0), (0,0,51), (102,0,51), (52,0,26), within tolerance1. A labeled second row uses white texture, material DiffuseColor(.5,1,.25), light colors(1,.5,0)/(0,.5,1), and N dot L=.6. Its independent diffuse, summed diffuse and diffuse+specular centers are (77,77,0), (0,77,38), (77,153,38), (179,153,89), also within1. All eight checks pass on both renderers; four actual before/after lighting images were inspected. Original assertion labels are retained in source and all four old center controls still pass after resizing; successful C++ checks are counted in JSON, not printed child-log lines. Empty stderr. The two green .3 terms sum before byte conversion. One bone moves the bind mesh to Z=-3; zero ambient/emission and disabled light0 isolate these terms. The layout changes from one row to two, so no whole-image identity is claimed. This establishes the selected per-pixel diffuse/specular arithmetic, not vertex-lighting specular, arbitrary normals/weights, transparency interactions or all renderers. |
| PreferPerPixelLighting (skinned shader variants) | BasicEffect's coarse-sphere example and constant-normal skinning scenes cannot establish this shader-path distinction. `graphics.skinned_effect.per_pixel_lighting` now renders the same coarse skinned quad with authored endpoint normals (-.8,0,.6)/(.8,0,.6). Vertex diffuse stays .6, while normalized per-pixel interpolation gives full center light and approximately .832 off-center. Three pixel checks pass on both renderers; both actual 800x480 captures were inspected and are RGBA-identical, with the expected constant panel/gradient. One bone moves bind Z=0 into view, and the same effect selects false then true every frame. Stderr is empty. This establishes the diffuse lighting modes for this one-influence fixture, not specular power, alpha, arbitrary joint blends or all renderers. |
| Clone and SetOwnedTexture | clone_ownership passes fourteen checks on both renderers: two clones initially copy the original palette/weight count and reference the same owned texture; changing one clone and then the source leaves the sibling palettes independent. After the source and caller's strong texture reference are released, both clones render their own positions and material colors multiplied by the retained non-white texel. A separate raw-setter probe retains exactly one caller owner and expires after the pointer is safely cleared. Explicitly disposing the cloned texture makes the same disposed object visible from both clones while their `shared_ptr`s keep its wrapper alive; the next pass Apply throws ObjectDisposedException through the real texture-binding path. Clearing only the raw texture properties leaves ownership intact, so a weak_ptr survives the first clone's destruction and expires after the last. Earlier inspected captures establish the rendering; Focused1015 adds programmatic pixel checks only. This is one-influence skinning and C++ ownership; other effect families and subclass cloning remain separate. The protected copy constructor is a subclass hook, not an independent public construction example. |
| VertexColorEnabled extension | `graphics.skinned_effect.vertex_colors` supplies a plain 56-byte custom vertex with packed RGBA COLOR alongside position/normal/UV/weights/indices. It uses the actual public CNAEXT VertexColorEnabled field, unmatched by the XNA member inventory. Six rendered checks pass on both selections: disabled color retains uniform material times non-white texture (64,255,96), enabled tint gives center (36,96,54) and distinct quarter-point colors (50,80,33)/(22,112,75). Three black-source AlphaBlend controls then leave only exact destination attenuation visible over MidnightBlue: half vertex alpha yields (12,12,56), multiplying half material alpha yields (19,19,84), and also multiplying half texture alpha yields (22,22,98), all within tolerance1. The earlier 800x480 captures were actually inspected for the original flat control and gradient; only 864 pixels differed by one blue level. Focused1019 used programmatic readback only. A bone moves every bind mesh into view, so this is actual skinned shader evidence, independent of importer COLOR_0 metadata. Arbitrary palettes, imported color-channel fidelity, non-black translucent source RGB and alpha under fog/lighting variations remain separate. |
| GetTypeName; inherited Name/Tag/disposal and protected OnApply | GetTypeName is CNA reflection identity plumbing and does not justify an independent demo. Inherited graphics-resource lifecycle has its own examples; it does not automatically prove every disposed SkinnedEffect operation. OnApply is reached through the visible EffectPass::Apply calls, not invoked directly by a game. No metadata credit is added merely for this source review. |
| Single-map shadow reception: map, light matrix, enabled flag and depth bias | shadow_receiver now passes four paired-pixel controls on OPENGLES3 with empty stderr. The actual800x480image was inspected and eight selected pixels match the independently predicted colors within tolerance1. An authored8x4Color map stores red depths64/255and192/255; a bone moves the receiver toZ=-3 and the public light matrix maps its post-skin depth to.5. Enabled reception removes direct diffuse on the occluded half while ambient/emission remain; disabling reception, bias.3 and map detachment each restore direct light. This is actual sampling, not getter-only credit or shadow-map generation. SOFTWARE explicitly skips the unsupported stock path; OPENGL33 shares the declared EasyGL scope but has no runtime evidence here. GraphicsCapability has no shadow query, so the demo declares its renderer restriction instead of misusing CustomEffects. These original diffuse controls do not establish specular shadowing; the isolated extension below does. Other bone configurations and renderers remain separate. Plain shadow-property getters need no independent scene. |
| Shadow filter radius: single comparison, 3x3 and 5x5 PCF | The same shadow_receiver now adds a lower row with three kernel controls. At the marked world X near -.125, the authored depth edge gives 0/1, 3/9 and 10/25 lit comparisons, hence visibility 0, 1/3 and 2/5. Predicted RGB values (20,28,26), (71,62,43), (82,69,46) all match within tolerance 1. All seven GL checks pass with empty stderr. The actual 800x480 capture was inspected; eight original control pixels were rechecked at their new top-row coordinates and three kernel samples were verified. Original four assertion labels remain exact. The layout changed, so no whole-image identity is claimed. SOFTWARE retains the exact declared skip. These are comparison averages, not interpolation of the final depth result. Uniform map rows isolate the horizontal edge; other edge orientations, out-of-range radius clamping and other renderers remain untested. Isolated specular reception now has the separate controls below. No extra ID or metadata-only member credit. |
| Direct specular attenuation by a single shadow map | shadow_receiver now has nine passing GL controls: its seven original receiver/kernel checks plus two isolated specular panels. Diffuse, ambient and emission are zero; material SpecularColor(.3,.15,.05), power1, white texture and light0 specular One produce paired highlights RGB(76,38,13) within tolerance1. Disabled reception preserves both highlights; enabled reception turns only the low-depth half black. The actual three-row800x480 image was inspected and all15 selected pixels were checked, including relocated original controls. Original seven assertion labels remain exact; GL stderr is empty and SOFTWARE retains its explicit stock-path skip. This demonstrates single-map per-pixel directional specular attenuation, not per-vertex, texture-alpha, cascade or punctual-specular behavior. The layout changed, so no whole-image identity is claimed. |
| ShadowCascadeStateEXT and setShadowCascadesEXT: atlas matrices, split distances, CameraView, BlendBand and Count0 | shadow_cascades now passes six GL pixel controls with empty stderr. Two authored atlas slices store red depths64/255 and192/255, and their WorldToAtlas matrices directly produce normalized atlas coordinates with receiver depth.5. Bone depths2 and3 select the near shadowed slice, including the exact split; depth4 selects the lit far slice. Depth2.5 in a one-unit blend band produces visibility.5 and RGB(97,79,51). A separate fitting CameraView translates depth4 to fitting depth2 while the rendering View remains identity, selecting the near shadow. Count0 restores an explicitly different single-map transform and its shadow. Focused916 reruns these controls on OPENGLES3 and OPENGL33; the inspected 800x480 captures are byte-identical and all selected pixels match predicted floor/lit/blended colors within tolerance1. SOFTWARE explicitly skips the stock path. This is authored two-slice sampling, not a shadow-map producer, all four cascades, invalid states or DebugTint proof. The shared-edge extension below separately addresses atlas isolation. Plain state getters and field permutations do not justify separate scenes. |
| Cascade atlas boundary isolation | The same shadow_cascades retains its original six controls and adds two with identical constant atlas UV(.5,.5), receiver depth.5 and requested radius2. Bone depths2/4 select near/far slices. The near result remains the occluded floor RGB(20,28,26); the far result remains lit RGB(173,130,77), demonstrating that each slice excludes its neighbor's opposite depth at their shared boundary. All eight GL checks pass with empty stderr. The actual three-row800x480 image was inspected, and all six original relocated centers plus both new centers match within tolerance1. SOFTWARE retains its explicit stock-path skip. This establishes slice isolation, not independently all25filter weights inside a cascade, arbitrary atlas layouts, other boundary orientations or other renderers. No whole-image identity is claimed after the layout change. |
| Fourth cascade at maximum count and selected DebugTint | shadow_cascades now passes ten GL pixel controls. Count=kMaxCascades=4 and fitting depth7 select slot3 after splits2/4/6/8. Its matrix produces comparison depth.5 for lit RGB(173,130,77), then depth.9 plus the selected yellow DebugTint produces shadow-floor RGB(20,28,15), within tolerance1. Both actual800x480 images were inspected; all eight original control pixels and loops remain, and regions outside the replaced ninth cell are RGBA-identical. SOFTWARE retains its exact stock-path skip, GL stderr is empty. This demonstrates fourth-slot addressing/depth and its diagnostic color, not every four-cascade index/palette color, invalid counts, arbitrary atlases, a producer or other renderer. No whole-image identity is claimed. |
| Cascade-selected directional specular | Focused916 extends shadow_cascades to12 controls with a final isolated-specular row. Diffuse, ambient and emission are zero; material SpecularColor(.3,.15,.05), power1 and light0 specular One leave only the highlight. View depth2 selects the low-depth near slice and produces black, while depth4 selects the high-depth far slice and preserves RGB(76,38,13), both within tolerance1. OPENGLES3 passes in eleven runs including the captured run, and OPENGL33 passes once; both inspected800x480 captures are byte-identical. SOFTWARE retains its declared stock-path skip. This proves that cascade selection attenuates directional specular as well as diffuse light; it does not establish per-vertex, alpha-dependent, arbitrary-normal or generated-map behavior. Evidence: `/tmp/checkpoint-916-*`. |
| All four cascade selections and DebugTint colors | Focused918 extends shadow_cascades to16 controls. With Count4, fitting depths1/3/5/7 independently select slots0/1/2/3. Every matrix uses comparison depth.2 so each strip remains lit and the four diagnostic tints are isolated as RGB(173,78,46), RGB(104,130,46), RGB(104,78,77) and RGB(173,130,46), within tolerance1. OPENGLES3 passes in eleven runs and OPENGL33 once; both inspected800x480 captures are byte-identical. SOFTWARE retains its declared skip. This completes the four-slot selection/palette behavior without one demo per field value. Evidence:`/tmp/checkpoint-918-*`. |
| PunctualLightEXT: point/spot authored distance reception | Focused917 grows punctual_shadows from8 to10 GL pixel controls. No map contributes diffuse light, authored constant-distance cube/2D maps remove it, and normalized bias.3 restores it. Separate range/cone exclusion controls retain only ambient/emission. Bone depth3/Range6 gives distance.5, while the spot matrix deliberately gives projecteddepth.1, distinguishing the compared quantity. At linear light intensity4, the original pixels match litRGB74/64/43 or floor20/28/26 within1. A second 4x4 map has low-depth left columns and high-depth right columns; the center 3x3 spot kernel sees six lit taps and yields RGB(56,52,37). A map-free direction whose cosine is exactly halfway between the .3-radian inner and .5-radian outer limits produces the shader's squared one-quarter transition at RGB(34,37,30). OPENGLES3 passes in eleven runs and OPENGL33 once; the inspected800x480 captures are byte-identical. SOFTWARE explicitly skips. This establishes reception, fractional spot PCF and cone transition for the authored geometry, not shadow production, all cube face directions/seams, arbitrary filter patterns or other renderers. PunctualLightEXT is diffuse only; getters/field permutations do not justify extra scenes. Evidence:`/tmp/checkpoint-917-*`. |
| Remaining shadow behavior and FillGpuDrawParams | Single-map PCF/specular, all four cascade selections/tints, two-slice blending/specular/boundary isolation and punctual fractional filtering/cone transition now have focused evidence. Oversized Count is not safe validation territory: source review proves that Count>4 overruns fixed public and draw-parameter arrays before backend clamping, recorded as ISSUE-102 without executing undefined behavior. Descending/nonfinite splits, matrices and negative BlendBand have no documented validation/refusal contract and would only demonstrate arbitrary invalid carrier values. Cube face direction/seam behavior is already a texture-sampling concern rather than another light-state variant. Source audit confirms PunctualLightEXT exposes no punctual specular field. IShadowReceiverEXT permits game-authored maps without an engine producer. FillGpuDrawParams is the internal renderer bridge; intentionally avoid a demo around internal draw structures. The observable shadow-receiver audit is therefore semantically closed for the supported EasyGL paths, without claiming generated maps or other renderers. |

### SoundEffectInstance and DynamicSoundEffectInstance

These are source/behavior reviews. The automated checkpoint uses SDL's dummy audio driver;
state and buffer checks can pass there, but audible quality, panning and Doppler are not validated.

| Members / behavior | Decision and evidence |
|---|---|
| SoundEffect::Play overloads and MasterVolume | `sound_effect.play_parameters` keeps its three successful fire-and-forget calls at the exact volume/pitch/pan endpoints and now executes twelve controls. Finite pitch outliers clamp and still play, while pan beyond either endpoint throws ArgumentOutOfRangeException. A separately disposed effect makes both Play forms return false while CreateInstance throws ObjectDisposedException. MasterVolume starts at one, round-trips one-half, accepts exact zero and one, then rejects high, negative and NaN values while retaining one-half; the original process-global value is restored and rechecked. Both focused800 dummy-audio runs pass with byte-identical output and empty stderr. This establishes public return/property/range/disposal behavior, not audible volume, pitch, pan or the mixer's effect on already-playing tracks. |
| SoundEffect raw PCM constructors and sample helpers | `sound_effect.from_pcm_buffer` now executes eight controls. Its original generated mono buffer retains exact quarter-second duration and playback. A padded source constructs the same duration from an aligned nonzero window. Static helpers establish XNA's nearest-millisecond duration rounding, whole-frame float sizing including the 44.1 kHz mono one-second result, and zero results for nonpositive sizes/rates/channel counts. Negative offsets and partial-frame counts throw ArgumentException, while a 7999 Hz rate and undefined channel identity throw ArgumentOutOfRangeException. Both focused801 runs pass with byte-identical output and empty stderr. This covers public buffer ownership/validation and arithmetic, not decoded sample equality or audible quality. |
| SoundEffect authored loop region | `sound_effect.loop_region` now uses a nonterminal region: a quarter-second intro, quarter-second loop and separate three-quarter-second tail. It remains Playing beyond the complete 1.25-second buffer duration, then pauses and releases through `Stop(false)`. Six of seven controls pass, including pause/property retention, bounded natural stop, replay/immediate Stop and the independent past-end/negative coordinate refusal matrix. ISSUE-114 is the sole failure: after resume the instance stops at the loop boundary instead of remaining active halfway through the authored tail. Ten runs per renderer label reproduce the same result with empty stderr. This establishes constructor validation and retained looping state plus the broken release boundary, not output-sample identity, click-free continuity or exact audio timing. |
| SoundEffect::FromStream, Duration and Name | `sound_effect.from_stream` now executes eight controls. The original loose WAV retains exact half-second duration, empty/mutable Name, instance playback and bounded natural completion. A prefixed in-memory stream is positioned at the WAV start; the returned caller-owned unique_ptr keeps the decoded half-second effect after that source stream is destroyed. Empty, non-audio and half-truncated WAV inputs each throw NotSupportedException, with any unexpected raw return immediately owned for cleanup. Both focused802 runs pass with empty stderr and matching output after normalizing the measured completion time. This establishes current-position consumption, independent result lifetime and malformed-input classification, not arbitrary stream implementations, every WAV codec/chunk or audible quality. |
| SoundEffect::CreateInstance; Play, Pause, Resume, Stop(), State | `sound_effect_instance.playback_state` now has eleven controls. Pause before first Play is a quiet Stopped no-op, while Resume before first Play follows XNA by starting playback. After Stop, Pause and Resume leave the retained track Stopped. A two-second instance played at half speed is explicitly still Playing at its two-second midpoint, receives Play again, then stops within three more seconds; a restart would require about four seconds after the second call. This wider window replaces a fixed 1.25+1.0-second observation that flaked once under a full three-job CTest load while passing serially. The normal Play/Pause/Resume/Stop sequence, replay and bounded natural completion remain. Fifteen concurrent focused repetitions, both direct renderer selections and the repeated complete CTest pass with dummy audio. State evidence does not measure audible latency or quality. Play-after-dispose is separately covered by lifetime; remaining disposed-call behavior is classified there. |
| IsLooped and Stop(false) release | `sound_effect_instance.looping` retains its original three controls: looping survives multiple tone lengths, mutation after first Play is refused, and immediate Stop ends it. It then replays, pauses and calls Stop(false): the voice remains Paused with configured IsLooped still true, Resume starts it, and it naturally stops within a3s steady-clock deadline. All six checks pass in ten dummy-audio repetitions (five per requested renderer), with empty stderr. The complete original function prefix through its three checks remains unchanged. Pausing before release avoids assuming the callback cannot reach the end between Stop(false) and observation. This demonstrates the generic whole-sound release/state behavior, not output samples, audible content or exact completion latency. Observed post-Resume waits are recorded in checkpoint-470-release-focused-review.json. `sound_effect.loop_region` now supplies the distinct nonterminal authored-tail discriminator and exposes ISSUE-114. |
| Volume, Pitch, Pan | `sound_effect_instance.volume_pitch_pan` executes eleven controls. It retains the initial values and live changes, accepts the exact lower and upper endpoints for Volume0..1 and Pitch/Pan-1..1, rejects the existing finite outliers and NaN for every property, and proves all six refused writes preserve the last valid values together. Both dummy-audio renderer selections pass with byte-identical output and empty stderr. This establishes public storage and atomic range behavior, not mixer gain, pitch or pan output. The public Pan/3D mode lifecycle has separate focused evidence below, including a stored Pan change on mode refusal. |
| Apply3D overloads, listener/emitter state | `sound_effect_instance.apply_3d` verifies the exact listener/emitter default state, round-trips an independently authored listener Velocity and emitter Up, and passes those values to Apply3D while stopped before retaining its original moving-emitter playback. The later overload audit below covers reference, vector and pointer/count forms plus their refusal boundaries. Public state still does not expose dominant-listener selection, mixed Doppler or orientation output. |
| Dispose, IsDisposed, protected disposal hook; C++ moves | The focused lifetime demo now passes 17 checks under both renderer selections with dummy audio. A voice retains playable data after destruction of a temporary C++ SoundEffect wrapper; SoundEffectInstance move construction/assignment transfer live playback and parent registration. SoundEffect parent move construction likewise preserves its active child's registry so disposing the target cascades correctly. Parent move assignment transfers the incoming registry; replacing the old destination wrapper does not explicitly dispose its prior child, which remains live until caller cleanup, while disposing the assigned target reaches only the incoming child. Instance Dispose is idempotent; Play and Resume throw ObjectDisposedException, while Pause and Stop remain safe cleanup calls. Volume, Pitch and Pan setters also throw ObjectDisposedException and preserve their values. Because the voice had played, IsLooped retains its one-way mutation lock and throws InvalidOperationException without clearing true. Explicit SoundEffect.Dispose cascades to both a moved playing child and an unplayed child and is idempotent. Both focused804 runs are byte-identical with empty stderr. These C++ wrapper/Dispose semantics do not override the C ABI parent-release guards. The protected disposal hook is implementation machinery; GetTypeName needs no independent demonstration, and no audible quality is claimed. |
| Dynamic constructor, Play, SubmitBuffer, BufferNeeded, PendingBufferCount | `dynamic_sound_effect_instance.streaming` starts an isolated mono PCM16 instance with no queued data, calls Play plus the public Update forwarder, and verifies exactly three canonical BufferNeeded callbacks synchronously refill it to PendingBufferCount=3 while state remains Playing. A one-shot listener removes itself during the first dispatch while the independent refill listener still receives all three calls, leaving only that refill subscription. Its original continuous tone still prequeues and refills from the game-thread event; renderer-dependent callback totals are reported rather than treated as exact timing. Both dummy-audio selections pass these starvation/recovery and listener-mutation controls. This establishes empty-stream recovery, subscriber independence and the callback sender/arguments, not concurrent producers, audible continuity or real-time callback cadence. |
| GetSampleSizeInBytes, GetSampleDuration | Streaming checks that 50 ms of 24 kHz mono PCM16 needs 2400 bytes and converts that count back to 50 ms through the instance duration helper. SoundEffect's static helpers are demonstrated in `from_pcm_buffer`, including XNA nearest-millisecond duration rounding, whole-frame float sizing and invalid-format zero results. |
| SubmitBuffer offset/count; SubmitFloatBufferEXT | The focused buffer_formats console example now passes25checks in both requested environments with dummy audio and empty stderr. One stereo instance stages a middle integer byte range, refuses offsets/counts between complete stereo frames without queue changes, plays/pauses/resumes, and refuses float input while paused without altering its state/pending count. After stopping and clearing old data, it accepts a float sample range; the reverse live-paused integer refusal and stopped return to integer playback also pass. Two independent two-second queues now close the live boundary: a Playing integer stream refuses float data and a Playing float stream refuses integer data, with state and pending count preserved in both directions. GetSampleDuration retains its 16-bit PCM meaning after float submission (4800bytes still means50ms at24kHz stereo). The original mono event-refill source remains unchanged. Four float-range refusals preserve staged queues: negative offset and oversized count before integer playback, then negative count and a nonempty range at the vector end after float staging. The range and paused-clear extensions retain all prior assertion calls in order. These are accepted ranges, queue preservation and public state/guard observations: no output-sample readback, audible quality, partial-float-frame output or concurrent-producer proof. Stop-before-switch avoids mixing already-queued encodings. The different units and a real format lifecycle justify one focused scenario rather than overload-count variants. |
| Float range bounds versus integer frame alignment | Source review at CNA eff3131017 distinguishes the contracts: SubmitFloatBufferEXT checks negative/out-of-vector ranges before selecting float mode and queuing data, while the integer path also requires whole PCM frames. The existing CNA unit test explicitly accepts three float samples for a stereo instance; its public header specifies sample units without an alignment refusal. Do not turn the earlier untested-alignment note into an invented expected exception or a CNA issue. buffer_formats demonstrates four float-bounds refusals with state/queue preservation and now both live-playing cross-format refusals. Partial-frame output behavior still needs separate output observation and is intentionally not fed to playback by that example. The alignment classification itself adds no native or audible coverage. |
| Dynamic IsLooped / Stop(false) | Streaming now demonstrates accepting IsLooped=false, rejecting true (the header contradiction remains ISSUE-037), refusing Stop(false) while continuing playback, and immediate Stop clearing pending buffers. Post-Dispose PendingBufferCount and IsLooped must throw by both the XNA spec and local genuine-XNA source; CNA instead returns 0/false (ISSUE-038). Both renderer selections execute 13 checks: eleven pass and only these two expected checks fail. No listening or genuine-XNA runtime execution is claimed. |
| QueueInitialBuffers, ClearBuffers, Update (CNA extensions) | buffer_formats now demonstrates paused queue cancellation without Stop: ClearBuffers leaves Paused/Pending0; a valid float submission still throws InvalidOperationException and retains that state; same-format refill with explicit QueueInitialBuffers leaves Paused/Pending1 before the original Resume/Stop path. Both focused dummy-audio runs pass all23checks with empty stderr, and all20prior assertion calls remain verbatim/in order. This establishes queue clearing versus stopping and the retained format constraint. Pending counts combine staged/submitted buffers, so they do not independently prove QueueInitialBuffers transport or output samples. Its call remains visible as ordinary paused-refill usage. The mono streaming example still demonstrates managed refill through Game/FrameworkDispatcher; a separate Update-forwarder demo adds no independent game concept without a specific consumption/refill scenario. No listening or concurrent-producer claim. |

### AudioListener, AudioEmitter and public spatial-audio behavior

Source review at CNA `eff3131017`: the public listener/emitter headers and implementations,
`SoundEffectInstance::Apply3D`, and the existing `apply_3d` / `positional_cue` demos. This
classifies remaining work. Source inspection alone adds no runtime, listening or metadata
coverage credit; the Pan/3D mode row now separately records actual focused native evidence.

| Members / behavior | Decision and evidence |
|---|---|
| Listener/emitter constructors and Position | apply_3d now checks both exact default records together: zero Position/Velocity, Forward/Up basis and emitter DopplerScale1. It still moves the emitter relative to the origin listener. Further vector permutations remain trivial storage operations and do not justify standalone property demos. The dummy driver does not establish audible distance attenuation or panning. |
| Listener Forward and Up | apply_3d explicitly selects the default basis. Source uses their cross product to derive listener-relative right; this does not prove behavior for a rotated or degenerate basis. Extend the existing audible spatial scene with a rotated listener when listening or a supported output observation is available. Do not claim orientation coverage merely from assigning the defaults. |
| Emitter Forward and Up | The reviewed SoundEffectInstance spatial path does not consume emitter orientation. apply_3d now authors a nondefault emitter Up, reads it back exactly and submits the record successfully before playback. The separate Cue::Apply3D extension observes Forward through public OrientationAngle0/90degrees. Neither establishes an acoustic emitter cone or an Up effect; backend-specific cone/orientation behavior requires separate support and evidence. |
| Listener/emitter Velocity and emitter DopplerScale | apply_3d authors and reads back a nonzero listener Velocity and submits it successfully before playback, while positional_cue explicitly sets approaching emitter velocity and observes its public raw Cue DopplerPitchScalar. Focused798 additionally round-trips emitter DopplerScale2.5, accepts the zero boundary, rejects a negative value without changing zero, then restores one before the sweep. Both runs pass18/18 with byte-identical output and empty stderr. Source combines emitter and global Doppler scales with both velocities. These are input/state and Cue-variable observations, not mixed-output Doppler evidence. An acoustic approaching/receding or zero-scale output comparison still needs listening or a supported public observation route. |
| SoundEffect DistanceScale, DopplerScale and SpeedOfSound | apply_3d now retains its default-speed control and directly round-trips DistanceScale2, DopplerScale0 and SpeedOfSound300. DistanceScale0 becomes the smallest positive float; negative distance, negative/NaN Doppler and zero/NaN speed are refused without changing the valid state. The demo restores and rechecks all three original process-global values. Both focused796 runs pass14/14 with byte-identical output and empty stderr. These are global property, range and restoration observations; dummy audio does not establish their attenuation or pitch effect. |
| Apply3D reference, vector and pointer/count overloads | apply_3d now uses a stopped probe to pass two independently positioned listeners through both the vector and pointer/count overloads. It distinguishes empty-vector and zero/negative-count ArgumentOutOfRangeException from null-pointer ArgumentNullException, and all refusals preserve Stopped state and the public Pan value. Both focused797 runs pass16/16 with byte-identical output and empty stderr. Source approximates XACT multi-listener DSP by selecting the nearest listener, but these accepted calls and refusal observations do not establish that selection or output equivalence; actual dominant-listener output still needs a supported observation route or listening. |
| Apply3D versus Pan after playback starts | apply_3d retains its original audible sweep/default-speed check and six public mode controls, the exact default record, authored listener-Velocity/emitter-Up round-trip and stopped Apply3D acceptance. Four global-scale, two overload and two emitter-scale controls bring focused798 to eighteen passing checks in each of two dummy-audio runs with byte-identical stdout and empty stderr. A paused pan voice refuses Apply3D and retains its stored pan; Stop permits starting/pausing the same voice in3D. Pan then throws InvalidOperationException while the voice stays Paused, but its public getter exposes the newly assigned .75 value. Reapplying Apply3D succeeds while paused, and Stop permits switching back to pan, restoring the Apply3D refusal. These are actual state/refusal/property observations, not mixer-output panning, orientation, Doppler or listening evidence; SmokeOnly remains appropriate for the audible sweep. The stored Pan change is demonstrated CNA behavior, not by itself a diagnosed CNA defect or acoustic change. No extra ID was added. Evidence: `/tmp/checkpoint-798-audio-emitter-doppler-*`. |
| Internal spatial calculators and mixer test access | Publicly exposed INTERNAL calculation/test hooks do not represent normal game use. Keep them out of demos rather than substituting private mixer state for the missing public output observation. Existing CNA tests may validate those internals; they do not confer demo coverage. |

### XACT engine, banks, cues and categories

| Members / behavior | Decision and evidence |
|---|---|
| AudioEngine constructors, RendererDetails, ContentVersion | `xact.play_cue` creates the settings/banks and pumps Update. `renderer_details` now passes ten controls under both dummy-audio renderer selections. It distinguishes an empty settings path (`ArgumentNullException`), a missing file (`FileNotFoundException`) and an existing non-XGS file (`InvalidOperationException`). It lists actual output IDs/names, copies the first record, and establishes typed/boxed equality, equal operators/hash, wrong-box rejection and FriendlyName diagnostic text. The copy retains both strings after the discovery engine is disposed. A separate engine accepts a nonzero look-ahead and unknown renderer ID through CNA's documented single-backend fallback before the selected-renderer engine plays a cue. `bank_lifetime` adds the complementary object behavior: disposal clears the engine's renderer list while its getter remains safe. Only CNA's one returned SDL3_mixer record was available, so no unequal-device comparison is claimed. The expected corrupt-file probe emits one stable parse diagnostic to stderr in each focused run. ContentVersion is a format constant and needs no independent demo. |
| AudioEngine::GetCategory, AudioCategory::Name / equality / Pause / Resume / Stop / SetVolume | `xact.categories` distinguishes Music/SFX names, controls their cues independently and rejects an unknown name. It establishes typed/boxed equality, wrong-type rejection, equal hashes and exact name text, plus same-name equality for SFX records from two different engines while their controls remain attached to their own engines. A punctual-scope control pauses SFX with no live cue, then proves a cue started afterward plays unpaused: category Pause affects only cues active at that call, matching FACT rather than creating persistent category state. `bank_lifetime` proves a valid category lookup becomes ObjectDisposedException after engine disposal. SetVolume is called, but audible attenuation is not measured by the dummy-driver checkpoint. |
| Authored category hierarchy | categories now passes thirteen checks in two dummy-audio runs with byte-identical stdout and empty stderr. Its independent Music/SFX workflow includes the punctual Pause scope above. A separate160byte XGS makes Master parent of SFX. Five hierarchy controls start with live paused direct/descendant voices, show parent Resume/Pause reaching both, child Resume leaving the direct voice paused, and parent Stop ending both from a verified paused state. Nine old bank files remain byte-identical; all ten source/staged pairs match. No extra ID. Ordinary category Stop forwards the option to each cue; category-authored fades, deep trees and mixed-output volume remain separate. Current evidence:`/tmp/checkpoint-809-xact-category-pause-scope-*`. |
| GetGlobalVariable / SetGlobalVariable; Cue::GetVariable / SetVariable | `global_variables` passes eight controls: authored initial/mutable values, both clamp bounds, ArgumentNullException/InvalidOperationException symmetry across Get and Set for empty/unknown names, and a PUBLIC|READONLY global whose attempted write is a silent no-op preserving its authored25 value. The generator updates all three XGS variants coherently. `cue_variables` passes twelve controls: independent per-cue storage, both clamp bounds, live mutation, bidirectional cue/global namespace separation, empty/unknown-name refusals and ObjectDisposedException from both valid-name access directions after cue disposal. `bank_lifetime` separately proves both engine-global access directions reject a valid name after engine disposal. These state/refusal checks do not demonstrate an RPC curve changing audible volume/pitch or selecting an interactive variation. |
| SoundBank constructors, PlayCue overloads, GetCue, IsInUse | `play_cue`, `positional_cue` and `cue_states` distinguish fire-and-forget playback, positional playback and controlled cue instances. GetCue returns caller-owned C++ objects; the workflows use unique_ptr so Dispose releases playback and scope exit deletes the object. play_cue checks both lookup and fire-and-forget boundaries: an empty name throws ArgumentNullException, an unknown name throws InvalidOperationException, and the following valid LowTone still enters InUse and returns to idle. Its original three predicates retain a3s steady-clock completion limit after an earlier full CTest exposed an unsupported700ms assumption. `bank_lifetime` adds constructor distinctions: null engine and empty filename are ArgumentNullException, a missing eager file is FileNotFoundException, while an existing non-XSB file creates a live stub whose cue lookup is InvalidOperationException. It also proves both valid-name routes throw ObjectDisposedException after explicit SoundBank disposal. This is state/refusal/retry evidence, not audible-duration measurement. |
| Cue::Name, state flags, Play/Pause/Resume/Stop/Dispose | cue_states now passes20checks under both dummy-audio renderer selections with byte-identical stdout and empty stderr. It proves repeated Play is a state-preserving no-op while Playing and Paused, and a stopped Cue is single-use. After disposal Play throws ObjectDisposedException while Pause/Resume/Stop remain safe no-ops and preserve disposed/stopped state. The simple no-fade cue stops immediately with AsAuthored. An original three-second PCM bank and complex single-sound cue with500ms fadeOutMS show Stopping then bounded Stopped, and a fresh cue's release is interrupted with Immediate. The observed ~505ms state wait includes caller scheduling/polling; no acoustic envelope, audible quality or sample timing claim. Transient preparation flags are classified separately below; RPC-only release and category-authored fades remain separate. Current evidence:`/tmp/checkpoint-812-xact-cue-transport-*`. |
| Cue::IsCreated and IsPreparing | Intentionally no independent demo for true transitional states in the current implementation. Source audit at clean eff3131017 finds the private Cue constructor changes its initial Created field to Prepared before SoundBank returns it, and no assignment enters Preparing. Getter-only false checks would not establish preparation behavior. Revisit if CNA exposes an asynchronous preparation lifecycle; this is a source classification, not new runtime coverage or a diagnosed defect. |
| Cue::Apply3D | positional_cue now adds6public controls to the complete unchanged original audible loop; SmokeOnly remains. Two dummy-audio runs pass with empty stderr and still execute all8sweep positions. BeforePlay, a caller-owned Cue::Apply3D exposes Distance5, OrientationAngle0 and DopplerPitchScalar1; reorientation changes the angle to90degrees. A paused voice accepts Distance10 and approaching velocity at0.1ofpublic SpeedOfSound, yielding raw scalar10/9. These observations establish explicit Cue positioning/variable updates, not acoustic panning, mixed pitch, RPC curve application or listening. AttackTime/ReleaseTime live clocks are substituted inside RPC evaluation and are not exposed as such by GetVariable; `audio.xact.cue_states` later proves that an RPC-bound ReleaseTime changes the public Stopping-to-Stopped transition. Original source audit remains checkpoint-470-fourth-next-cue-spatial-audit.md; actual focused evidence is checkpoint-471-cue3d-focused-* and Focused861. |
| WaveBank constructors, IsPrepared, IsInUse | `play_cue` loads an in-memory bank; `streaming_wave_bank` uses offset/packet-size construction, plays it and checks release. A separate compatibility bank passes deliberately unusual negative offset/packet inputs and still prepares, demonstrating that CNA/FNA's standard file I/O path does not consume these arguments. `bank_lifetime` adds null-engine and empty-filename ArgumentNullException controls for both constructor families. A missing eager file throws FileNotFoundException; an existing non-XWB file and a missing streaming file instead create live unprepared stubs, matching the two distinct load paths. The streaming constructor's relative-path restriction remains ISSUE-028. These public prepared/use observations do not expose eager versus lazy resident byte counts; that is internal-test territory. |
| Disposing events, IsDisposed, explicit bank/engine disposal | bank_lifetime passes17public checks in two dummy-audio runs with byte-identical stdout and empty stderr. A paused consuming cue keeps both banks in use; WaveBank disposal disposes it while a prepared never-played cue and SoundBank survive. SoundBank disposal also disposes prepared cues, then valid GetCue and PlayCue calls both throw ObjectDisposedException. Event senders and exactly-once counts survive repeated disposal and deletion of disposed wrappers. Fresh registered banks and both cue states are released by AudioEngine disposal. Afterward the cleared renderer list remains queryable, valid category/global-variable operations refuse the disposed engine and Update is a safe no-op. Caller-owned allocations remain unique_ptr-owned; no global callback-order, concurrent teardown or audible-latency claim. cue_states separately retains its cue-level lifecycle and authored release checks. Current evidence: `/tmp/checkpoint-806-xact-engine-post-dispose-*`. |
| RPC curves, complex variations, instance limits | instance_limits now passes18checks under both dummy-audio renderer selections with byte-identical stdout and empty stderr. Its category1/Fail and1/ReplaceOldest controls remain, as do five cue-definition1/Fail checks. A deterministic364byte bank authors MidTone as1/ReplaceOldest: LowTone and the first MidTone prove definition-local counting, then a second MidTone stops that older LowTone while the older same-definition cue survives, exposing FACT/FAudio's bank-wide victim search. A separate settings/bank pair gives SFX capacity2/ReplaceLowestPriority and authors priorities High200, Low10 and New100; High starts first, but adding New evicts Low and preserves High, distinguishing priority selection from oldest order. QUEUE and ReplaceQuietest intentionally collapse to ReplaceOldest in FAudio/CNA and do not justify duplicate demos. Source and unit-test review closes the remaining output-only paths: weighted/interactive/track/effect variations expose neither selected-wave identity nor their mixer result through public Cue state, while volume/pitch/filter RPC results likewise require listening or private mixer access. `AttackTime` and `ReleaseTime` are substituted only during internal RPC evaluation; public GetVariable returns the authored variable value rather than a live clock. Focused861 closes the meaningful ReleaseTime candidate through its public Stopping-to-Stopped interval. Category fade envelopes remain manual/acoustic work. Evidence:`/tmp/checkpoint-815-xact-category-priority-*`, focused816 and Focused861. |
| Remaining generated Audio member/type gaps | The seven non-field gaps are four CLR finalizers (`AudioEngine`, `SoundBank`, `WaveBank`, `SoundEffectInstance`) plus constructors for `InstancePlayLimitException`, `NoAudioHardwareException` and `NoMicrophoneConnectedException`. Existing ownership workflows exercise native destruction and explicit Dispose. Missing audio/microphone facilities remain environmental or manual, and CNA's production XACT capacity path does not throw `InstancePlayLimitException`. Synthetic exception construction and finalizer metadata would add no audio behavior. The Audio generated-member audit is therefore dispositioned at114/121 without claiming acoustic, hardware or service completeness. |

### MediaLibrary and picture streams

Reviewed against CNA ce74063e6 (media source unchanged from e60fa35f7). The standalone
`media.media_library.save_picture` configures Linux XDG before first CNA/SDL use and verifies
both resolved roots before constructing a library. No internal path override or real user
pictures are used. Both renderer selections reproduce 34 child checks with the established
ISSUE-041 failure and two grouped ISSUE-098 failures; this is filesystem/stream evidence, not
visual or GPU validation.

| Public behavior | Evidence / decision |
|---|---|
| SavePicture byte-vector and Stream overloads | One original 640x320 PNG and its generated thumbnail save successfully. The normal memory stream is consumed. A separate valid 13-byte short-read stream exposes ISSUE-041; the independent control reader proves its full bytes are available. No workaround hides the failing CNA call. |
| Pictures / SavedPictures / RootPictureAlbum and child album views | Empty initial collections, new Saved Pictures child, its parent identity and counts across library/album views are checked. A fresh library reindexes both saved files. PictureCollection.Item exposes the two names in either filesystem order and PictureAlbumCollection.Item exposes their shared Saved Pictures parent. The workflow then writes an ordinary `Trips/Day One` hierarchy and a fresh library exposes both levels with exact parent links. Each album contains only its own decoded 640x320 picture, whose Album points back to that node and whose owned GetImage stream matches the original PNG. Filesystem-dependent sibling ordering is deliberately not assumed. |
| Picture Name / Width / Height / Date / Album and token identity | The immediate saved record keeps authored filename/dimensions, the borrowed parent, and a Date bounded by the SavePicture call. Its CNAEXT path token resolves the independent reopened Picture through public MediaLibrary.GetPictureFromToken; an unknown token returns null and the indexed Date is populated. Original/reopened Pictures establish path-based typed/object equality, operators, equal hashes and name text while Preview is unequal. The corresponding Saved Pictures albums establish the same contracts by path and reject a Picture object. Repeated disposal marks only the selected views, but zero of twelve tested Picture operations and zero of nine PictureAlbum operations then produce XNA's required ObjectDisposedException; the two grouped failures expose ISSUE-098. |
| Picture collection disposal | Repeated disposal marks populated PictureCollection and PictureAlbumCollection views without destroying retained Picture/PictureAlbum entries. Count and Item must then throw ObjectDisposedException; each collection instead reports 0/2 exact refusals because CNA clears the view, returns a zero count and takes the bounds-exception path for Item. Windows XNA also guards GetEnumerator; the CLR member remains source-dispositioned because CNA exposes separate CNAEXT range adapters. These two grouped failures extend ISSUE-098. Evidence: `/tmp/checkpoint-886-picture-collections-*`. |
| GetImage / GetThumbnail ownership and content | Caller-owned streams use unique_ptr; full image bytes match exactly and the large panorama's thumbnail is smaller with the same 2:1 aspect. Current 128x64 output is observed, not assumed as a universal contract. A fresh indexed 128x64 replacement then returns the exact original PNG bytes from two distinct caller-owned thumbnail streams, proving the no-upscale/no-reencode branch and per-call ownership. The demo next overwrites that indexed file through its public path token; failed thumbnail decoding returns a stream over the exact current corrupt bytes, closing the fallback branch without treating them as an image. Header contradiction is ISSUE-040. CNA deliberately has no thumbnail cache. |
| Stream positions, premature EOF, unknown length | Source audit at the pinned CNA d23e139de shows the overload allocates the stream's total Length, performs one Read from the current position and ignores its returned count. A nonzero position necessarily produces the same legal short-read mechanism already isolated by ISSUE-041, while premature EOF produces the same ignored-count/zero-padding defect with a stale or contradictory advertised Length; duplicate failing controls would not establish distinct behavior. SharpRuntime Stream makes getLengthProperty pure virtual and defines it as total bytes, so an "unknown length" is not a supported value. A subclass may throw NotSupportedException, which SavePicture naturally propagates before creating a Picture; that capability refusal does not merit a media demo. Later CNA commit a83673fc5 loops to EOF and resizes to received bytes, but this project's validated baseline remains pinned before that fix. |
| MediaSource selection, tokens, same-name saves and disposal | `media_library.collections` now adopts every raw pointer returned by GetAvailableMediaSources, verifies each actual record has matching nonempty Name/ToString, and constructs its MediaLibrary from the returned Local Device record. The library exposes a distinct copied MediaSource with the same name/type/text, while its existing IsDisposed=false check is now credited. Both focused runs used fresh empty XDG music/picture roots and found zero items, leaving both directories empty. This establishes CNA's one returned desktop source and copy ownership, not wider device enumeration. Picture token lookup is demonstrated by save_picture. Saving `Panorama.png` again with the valid thumbnail bytes leaves the physical file count unchanged but appends a distinct live Picture view. The two views compare equal and share a token because identity is path-based, while each retains its creation-time dimensions. A fresh library resolves that token to one 128x64 record whose stream contains the exact replacement bytes. music_index separately proves that repeated MediaLibrary::Dispose sets IsDisposed without cascading into its borrowed graph. Evidence: `/tmp/checkpoint-645-media-source-*`, `/tmp/checkpoint-646-picture-values-*`, `/tmp/checkpoint-877-*`. |
| Platform folder integration | Verified only on Linux/XDG; the demo skips before access if private roots cannot be confirmed. Windows/macOS/device integration requires separate environment validation. |

### Song caller-created value identity

`media.song.properties` creates three caller-owned records: two display names for the same file and
the same display name for two different files. Typed and System::Object equality, both operators
and equal hashes establish that Song identity uses the retained file handle rather than the display
name; null object comparison is false; and ToString returns each independent display name. After
idempotent disposal and peer-isolation controls, all ten ordinary properties, both Equals
overloads, both operators, hash and text remain usable. The resulting `0/16` exact refusals expose
ISSUE-098 while the original seven controls remain passing. Both renderer-labelled streams are
byte-identical and stderr is empty. This is path/value/lifetime behavior; neither file is played
and no decoded or audible identity is inferred. Evidence: `/tmp/checkpoint-642-song-values-*` and
`/tmp/checkpoint-886-song-disposal-*`.

### MediaLibrary tagged music and shared collection views

| Public behavior | Evidence / decision |
|---|---|
| Song metadata, Album/Artist/Genre grouping and back-references | music_index uses three original native FLAC fixtures whose basenames and directories differ from the tags. Public Song names/track numbers, two artists, two same-name artist-specific albums and one genre agree across library/group views. Album/Artist/Genre members and Song back-references share the same borrowed objects. Its five collection Item indexers are credited because these exact accesses establish ordering and shared identity throughout the workflow. Twenty-three child checks pass in both requested environments with exactly six reviewed TitleContainer INFO lines; parent JSON checks are zero. This demonstrates actual populated objects rather than empty collection counts. |
| Playlist::Name and Songs ordering | A private Selected.m3u reverses the first two tracks; the public playlist preserves that order and points to the original Song objects. Other playlist formats, missing/escaping entries and duplicate entry policies remain separate behaviors. |
| Equality, hashing and reflection helpers | A second independent MediaLibrary indexes the same fixture and supplies distinct Album/Artist/Genre/Playlist objects. Album typed/object equality, operators, equal hashes and text establish (title, artist) identity: the mirror Field Notes/Demo Ensemble pair is equal while the same title under Second Ensemble is unequal. Artist, Genre and Playlist establish the corresponding name-based typed/object equality, operators, equal hashes and exact text; Genre/Playlist object overloads also reject the other media type. Different hashes for unequal values are not guaranteed and are not asserted. Reflection names remain intentionally uncredited. Both runs retain exactly six TitleContainer INFO records because the second scan reads the already isolated files directly. Evidence: `/tmp/checkpoint-643-media-values-*`. |
| Group Dispose/IsDisposed | Repeated disposal still proves idempotent flags and sibling isolation. The demo then invokes 13 ordinary Album operations and eight each on Artist, Genre and Playlist. Exact ObjectDisposedException refusals are 0/13, 0/8, 0/8 and 0/8 because CNA only sets each flag; four grouped failures extend ISSUE-098 to these music views. Album's no-art image methods also return the wrong InvalidOperationException after disposal, establishing exception-precedence rather than mere continued property access. Post-destruction borrowed-pointer use remains intentionally excluded. |
| Music collection and MediaLibrary disposal | Repeated disposal marks all five top-level music collections without destroying retained entities, but Count remains readable as zero and Item produces ArgumentOutOfRangeException. Each collection therefore records 0/2 exact XNA refusals. A separately indexed MediaLibrary is disposed without cascading into its retained collection/entity graph, which is correct; all nine library properties, GetPictureFromToken and both SavePicture overloads nevertheless remain callable, for 0/12 exact refusals. Windows XNA 4.0 IL also guards GetEnumerator; CNA's C++ range adapters are CNAEXT conveniences and no separate metadata credit is manufactured for the CLR member. The unguarded picture saves use isolated one-byte inputs and their temporary files are removed. These six grouped failures extend ISSUE-098. Evidence: `/tmp/checkpoint-885-*`. |
| Duration and album art | music_index passes18child checks in two FFmpeg-enabled native runs: all15original relationship checks remain, with positive250/500/375msSong lengths, artist-specific750/375msAlbum sums and750msPlaylist sum excluding the third song. This is import-time metadata, not playback or audible timing; the non-FFmpeg branch has syntax-only evidence. The separate album_art standalone now passes20 identical controls in two native runs: HasArt distinguishes folder, FLAC/MP3 embedded and missing art; both missing-art calls throw InvalidOperationException; the large folder cover returns exact bytes and a public-decodable128x64 thumbnail; and its caller-owned streams survive library destruction. Mixed-case names prove case-insensitive `cover > folder` selection. Fresh scans establish `cover.jpg > cover.jpeg > cover.png` and the lower `front > album > albumart` chain through the final fallback. Same-family aliases use the identical fixed candidate loop and are source-dispositioned. Independent thumbnails preserve an already-small64x64 folder PNG exactly, corrupt-file fallback returns exact current bytes, and tagged FLAC PICTURE plus MP3 APIC fixtures prove byte-exact embedded PNG extraction. Folder-over-embedded precedence is explicit. Visual quality and other platforms remain separate. ISSUE-040 has Album runtime evidence for its stale thumbnail comment. Evidence:checkpoint-473-duration-focused-*, checkpoint-474-art-focused-* and `/tmp/checkpoint-882-*`, `/tmp/checkpoint-883-*`, `/tmp/checkpoint-888-*`, `/tmp/checkpoint-890-*`. |
| Playlist import formats and boundaries | `playlist_import` passes9/9 through public MediaLibrary views. Root-level M3U/M3U8 files preserve order and duplicate shared Song identity, accept comments, CRLF, whitespace, nested relative audio paths and a UTF-8 filename with uppercase extension, and skip a missing member. Separate playlists remain empty for an absolute indexed path and an in-root symlink escaping the playlist directory. PLS and nested playlist discovery are deliberately unsupported. Both direct renderer-labelled runs are byte-identical; exact SOFTWARE CTest passes20 times. Evidence: `/tmp/checkpoint-887-*`. This does not claim file-URI/remote playlist support, legacy non-UTF-8 encodings, playback, listening or other platforms. |
| Audio formats, tags and recursive indexing | `format_index` passes11/11 through public MediaLibrary views. Ogg/OGA, MP3, WAV, FLAC and Opus each produce one Song; Vorbis, ID3v2, native-FLAC and Opus tags preserve exact titles/tracks and shared normalized group identities. Rating80 becomes Song rating8 while an unrated peer remains false/0. A nested untagged WAV derives title/album/artist from its path without a genre, track or rating. A title-only Vorbis record keeps that real field while album/artist fall back independently and genre/track stay absent. AAC/M4A/BIN extensions over valid audio bytes remain absent. A self-directory symlink terminates without duplicates, while a child symlink to an external private directory is followed once and preserves its FLAC tags. All eight durations are positive with this FFmpeg-enabled build. Both direct renderer-labelled runs are byte-identical; exact SOFTWARE CTest passes20 times. Evidence: `/tmp/checkpoint-889-*` and `/tmp/checkpoint-891-*`. Malformed/truncated byte layouts are internal parser-bounds cases whose public result is the demonstrated per-field fallback; this establishes indexing and import metadata, not decoding quality, playback or other platforms. |
| Root isolation, ownership and limits | The standalone workflows select XDG_CONFIG_HOME before first CNA/SDL use and verify public music/picture roots before library access. No real user collection is read or changed. music_index and format_index leave borrowed groups/collections to library lifetime. album_art retains only caller-owned streams after destruction and never touches stale Album/Picture pointers. save_picture verifies fresh-library reindexing of Saved Pictures and an ordinary nested tree. Post-destruction borrowed-pointer use remains intentionally untested. Generated audio encoding/tag inspection is preparation evidence, not playback validation. |

### MediaPlayer and MediaQueue (member and behavior decisions)

Source review at clean CNA eff3131017; checkpoint-473-music-next-player-members-source.json.
This classifies existing evidence and follow-up work; it adds no runtime or acoustic proof.

| Public behavior | Evidence / decision |
|---|---|
| Play single Song / collection / start index; State and Queue | play_song checks initial Stopped, immediate Playing and eventual completion. queue_and_repeat now calls the plain collection overload and proves its zero start plus two ordered queue-owned copies after both caller Songs are destroyed. It later replaces that live queue rather than appending, calls the indexed overload with entry one, and retains the replacement order, active entry and Playing state after the second caller collection is destroyed. This establishes retained handle/name records, not preservation of library group metadata. |
| MoveNext / MovePrevious / IsRepeating | queue_and_repeat checks manual forward movement, repeating end-to-start, staying at the first song when moving backward and returning from the second song. It now also replays a real two-song queue from index zero under bounded dispatcher pumping: completion of the first track advances to index one in Playing, then the final nonrepeating track reaches Stopped without wrapping or changing the active entry. No need for one demo per queue position. |
| IsShuffled | queue_and_repeat now follows the flag round-trip with sixteen actual shuffled MoveNext calls. Every selection retains a valid queue index, makes ActiveSong equal that indexed item, names one of the two members and returns to Playing. The check deliberately accepts a repeated index because CNA's random selection permits it; it establishes branch behavior and queue consistency, not a particular order or fair distribution. |
| MediaQueue indexer, ActiveSongIndex setter, Add and Clear | queue_and_repeat uses Item for both player-owned entries, ties ActiveSong to item0, and verifies negative/count indices throw ArgumentOutOfRangeException without changing either item. A separate scratch MediaQueue now proves the empty constructor state, transfers two ordered Song pointers through Add, selects item1 through the active-index setter, then Clear destroys the owned entries and restores count zero, index -1 and null ActiveSong. Keeping those CNAEXT mutation helpers off the live player's queue avoids corrupting transport state. ISSUE-058's unsafe replay of Queue.ActiveSong remains deliberately uncalled. Evidence: `/tmp/checkpoint-644-media-queue-*`, `/tmp/checkpoint-784-media-queue-mutation-*`. |
| Pause / Resume / Stop and PlayPosition | dispatcher_events lets the real track advance, pauses it and proves PlayPosition remains within 10 ms while another 100 ms of wall time passes. Resume advances more than 50 ms from that frozen value, and Stop publishes Stopped with an exact zero position before the same Song is replayed through its established natural-end path. A second Pause while Paused, Resume while Playing and Stop while Stopped each survive a dispatcher pump without adding an event; the same position controls prove their transport state remains coherent. play_song separately checks positive position during a Game loop. No exact callback-latency claim is made. |
| Replaying a queue-owned Song | A public-API ASan probe now confirms ISSUE-058: passing `MediaPlayer::Queue.ActiveSong` back to `MediaPlayer::Play(Song*)` frees that queue-owned Song in `Clear` and then reads it in `LoadSong`. An independently caller-owned Song with identical path/name succeeds. This is probe-backed lifetime evidence, not a normal demo check; the catalog intentionally does not execute undefined behavior. Evidence: `checkpoint-473-hierarchy-*queue-replay*`. |
| ActiveSongChanged / MediaStateChanged | dispatcher_events establishes deferred delivery through FrameworkDispatcher::Update and now asserts the exact sequence across Play, coalesced Pause/Resume, separately pumped Pause and Resume, Stop, replay and natural end. Active-song delivery precedes state when both are pending; multiple state changes before one pump produce one notification exposing the current Playing state. Every callback receives a null sender and the canonical EventArgs::Empty, and removed listeners observe neither a later Play nor Stop. Mutation during Raise is generic System::EventHandler snapshot behavior rather than a MediaPlayer-specific transport concept and is not duplicated here. |
| Volume / IsMuted / GameHasControl | volume_and_mute now sends -0.25 and 1.25 through the public setter and observes exact clamps to 0 and 1 before restoring 0.5 for playback. It retains stored half-volume, isolation from SoundEffect::MasterVolume and IsMuted preserving that value. The assertion now describes only stored state because dummy audio does not establish attenuation or silence; those and platform interruption need output/manual evidence. GameHasControl is always true in this source; no independent constant-getter demo. |
| IsVisualizationEnabled / GetVisualizationData | visualization first overwrites both public 256-value buffers with nonzero sentinels, leaves capture explicitly disabled and proves GetVisualizationData clears both arrays to zero. It then enables capture, observes positive aggregate spectrum energy and draws frequency/sample arrays. Paused-buffer freshness has no documented public expectation and would overfit mixer timing; FFT frequency accuracy, amplitude ranges and rendered pixel correctness likewise remain unclaimed. Extend with an authored signal and explicit bounds only if those behaviors are the goal; do not infer acoustic accuracy from the current energy sum. |
| CNA initialization/shutdown and internal state hooks | Publicly reachable bridge/hooks should not replace ordinary game transport APIs in demonstrations. Revisit only for an explicit embedding lifecycle; reflection, constant getters and deleted copy operators do not justify standalone demos. |
| C explicit update, ProgramExit and MediaQueue type names | `cna_media_player_program_exit_ext` releases resources during live playback and the same independent caller-owned Song immediately restarts. The final behavioral route is now demonstrated after the game loop: a real 250 ms track finishes while the public state remains Playing, then `cna_media_player_update_ext` consumes the pending native end notification and transitions the one-song, non-repeating queue to Stopped. Both renderer runs retain all92 older controls in order and pass the two additions with byte-identical output and empty stderr. The two remaining routes are ordinary generated size/copy helpers for the fixed `Microsoft.Xna.Framework.Media.MediaQueue` reflection name; CNA's pure-C smoke already asserts the exact text and generic buffer contract, so they remain intentionally trivial and uncredited. Every `media_player.h` route now has a semantic disposition at39/41 demonstrated. Evidence: `checkpoint-474-c-player-program-exit-*`, `checkpoint-521-c-player-update-focused-*`. |

### VideoPlayer validation, transport, frame and lifecycle behavior

The existing `media.video_player.pause_and_loop` keeps its real decode, pause, resume and
end-to-start loop workflow and expands from12 to27 controls. Its21 passing controls establish fresh
defaults, configuration round-trips, first-frame decode, stable paused position, resumed playback,
loop transport, lifetime-monotonic frame generation and nonnegative frame timestamps. A raw
`Video::FromUriEXT` matches the compiled Video's probed dimensions, rate, positive duration, path,
device and type name. Destroying an active raw C++ Video safely stops its dependent player, and all
seven already-guarded transport/texture/CNAEXT track methods throw ObjectDisposedException after
player disposal.

Direct Windows XNA 4.0 assembly inspection adds six grouped compatibility expectations. On pinned
CNA, `GetTexture` before Play returns null, `Play(nullptr)` is ignored, and invalid volumes clamp;
Stop and Dispose clear the last Video pointer; and disposed State/PlayPosition reads plus the three
configuration setters remain callable. Windows XNA throws the exact corresponding validation and
disposal exceptions and retains `activeVideo`. Both renderer-labelled runs reproduce exactly these
six failures, now CNA-DEMOS-ISSUE-099, with all other controls passing. Different wall-clock pause
and wrap positions make byte-identical output inappropriate. Coverage remains2114/2326 because the
XNA members were already credited; eight newly named CNA extensions are explicitly reported as
non-XNA metadata. Evidence: `/tmp/checkpoint-893-*`. Frame pixels continue through the established
draw path, but no screenshot, visual-quality or audible-quality claim is made.

### Video and VideoPlayer multi-track selection

New `media.video_player.track_selection` uses a deterministic original MKV with two conforming
160x90 YUV420 streams (solid red and blue) and two conforming48kHz mono streams. Public player-level
preselection before Play returns blue pixels from stream one. Same-track and invalid video indices
preserve the live Texture2D identity; real, same-format audio switches and their no-op variants do
not replace that unrelated texture or stop transport. Resetting both player preferences to `-1`
selects the default red stream on replay while frame generation stays lifetime-monotonic. Calling
the same selectors on the attached Video reaches its player without stopping playback.

The Video-level pre-Play path exposes CNA-DEMOS-ISSUE-100. FNA documents these methods on Video and
recommends calling them before Play, but pinned CNA decodes red stream zero after the detached Video
selected blue stream one. Source inspection shows the parallel audio field is disconnected too;
because the public surface has no active-audio-track readback, that symmetry is recorded as source
evidence rather than claimed runtime proof. Both renderer-labelled runs execute nine controls,
pass eight and fail only the blue-selection invariant with byte-identical stdout and empty stderr.
Evidence:`/tmp/checkpoint-894-*`. Pixel bytes were read programmatically; no screenshot, listening
test or manual visual-quality judgment was performed.

### Video URI and trusted-metadata construction

New `media.video.uri_and_metadata` closes the remaining public Video construction/failure behavior
in eight controls. Plain absolute raw probing returns the generated fixture's exact metadata; a
missing raw file throws FileNotFoundException. The trusted/XNB-style constructor retains supplied
duration, dimensions, frame rate and soundtrack type without touching its nonexistent path. A
trusted Video whose dimensions disagree with a real decoded stream then raises
InvalidOperationException and leaves the player stopped with no Video.

The other four controls expose CNA-DEMOS-ISSUE-101. FNA resolves relative URIs below TitleLocation,
extracts an absolute file URI's local path, rejects non-file absolute URIs with
InvalidOperationException, and reports a trusted constructor's deferred missing file during Play.
CNA instead treats every factory string as a literal process-relative filesystem path and silently
returns from Play when decoder open fails. Both renderer-labelled runs reproduce all four grouped
failures with byte-identical stdout and empty stderr. `Video::setDurationProperty` is explicitly an
internal-use CNAEXT setter; its trivial assignment does not warrant another demo. Together with
play_video, pause_and_loop and track_selection, the public Video/VideoPlayer construction,
metadata, transport, frame, track, lifetime, disposal and failure surface now has a semantic
disposition. Evidence:`/tmp/checkpoint-895-*`. No network request, audio, screenshot or visual
judgment occurred.

### Media public enum values

All eight runtime-inventory fields in `Microsoft.Xna.Framework.Media` now have a semantic decision.
The established `pause_and_loop` workflow directly observes `MediaState::Stopped`, `Playing` and
`Paused` across construction, playback, pause/resume, stopping and dependent-Video destruction.
The compiled-video workflow now requires its authored/default `VideoSoundtrackType::Music`; the
trusted-metadata workflow reads back explicit `Dialog` and `MusicAndDialog` values before its
separate failure checks. The desktop MediaLibrary workflow requires every returned source to be a
`MediaSourceType::LocalDevice` and still verifies that its copied library source preserves the
same value.

`MediaSourceType::WindowsMediaConnect` remains intentionally uncredited. Pinned CNA only creates
LocalDevice sources, and its public MediaLibrary constructor rejects any other source type; merely
casting the numeric constant would not demonstrate a usable media-source behavior. The focused
runs pass the new positive controls on SOFTWARE and OPENGLES3 while retaining exactly the existing
ISSUE-099/101 failures. The four exact SOFTWARE CTests pass ten repetitions, metadata validates
659/0 and the field total moves169->175/753, leaving Media at7/8. Evidence:
`/tmp/checkpoint-896-*`. No new ID, CNA issue, listening, screenshot or visual claim is made.

### Network simulation (measurement boundaries)

| Members / behavior | Decision and evidence |
|---|---|
| SimulatedLatency, SimulatedPacketLoss, SendDataOptions::None/Reliable | Existing two-process demo now uses 24 authored packet pairs per phase, separate baseline/delay/full-outage phases, and a recovery interval with loss disabled while the sender stays connected. Checks use exact sequence counts, not the span between first/last arrivals, which hid missing endpoints. The latency phase checks a lower bound on every arrival and full reliable delivery; scheduling can add extra delay. The outage checks zero unreliable arrivals and exposes ISSUE-030 as zero reliable deliveries even after recovery. |
| BytesPerSecondSent / BytesPerSecondReceived | The host samples both public rates after every real session Update and retains their maxima across all packet phases. Both become positive in SOFTWARE and OPENGLES3/X11, proving transport totals are published through the NetworkSession properties without asserting a scheduler-dependent exact rate. The control passes before the separate ISSUE-030 reliable-outage failure. Evidence:`/tmp/checkpoint-697-bandwidth-*`. |
| Random loss probabilities, unsimulated unreliable delivery | The old >90% baseline and 50..90% simulated-delivery gates were empirical assumptions, not guarantees of SendDataOptions::None. A full semantic-409 SOFTWARE run observed 79% baseline delivery; a follow-up observed 42% under 0.3 simulated loss. Those raw logs remain archived. Arbitrary probability sampling is now observational/deferred; do not infer statistical distribution coverage from the deterministic endpoint test. |
| Test coordination, deadlines and platforms | Local temporary marker files control phase boundaries independently of the impaired game packets. Wall-clock pacing avoids fixed-step catch-up bursts; both processes pump the session while settling and recovering. Initial 466 SOFTWARE hit the demo's single 20-second aggregate deadline after outage recovery began, before Evaluate; the exact raw comparison rejected that substituted failure instead of accepting its known-issue status. The unchanged isolated demo retained all five original child checks on both selections, as does the repaired version with a 20-second watchdog per finite startup/burst/recovery phase. Resets occur only on real phase transitions, and sign-in cannot bypass the watchdog. All 24 packet pairs, 20 ms pacing, 600 ms settling, 1-second recovery and delivery/latency predicates are unchanged. Both repaired focused stderr streams are empty; the affected CTest passes its expected-failure contract. Full466-timing preserves all five host checks and all37/41 old suite failures. A controlled native SOFTWARE regression adds identical8-second delays at the three host phase transitions: the original sender times out at22.88s and the host evaluates incomplete phases; the repaired37.41s exchange retains all five checks with only ISSUE-030. Both host/child renderer selections are explicit, and selected project binary hashes remain unchanged. This is a demo orchestration fix, not new CNA delivery evidence or a claim of native stalled-peer injection. This is Linux loopback validation, not wide-area QoS or genuine-XNA runtime evidence. Ordinary transport/network-session lifecycle and voice remain separate behaviors. |

### Available sessions and search-result lifetime

`net.network_session.find_and_join` now creates its real System Link host with three total slots,
one reserved private slot and sparse advertised values in properties0 and5. Both requested
renderers pass exact discovery checks for one occupied gamer, one open public slot, one open private
slot and the complete eight-value property shape. The joiner observes `IsDisposed=false`, joins,
then disposes the result collection twice. CNA intentionally leaves the read-only listing storage
visible after disposal, but the shared disposal token also reaches a previously copied listing;
joining that copy throws ObjectDisposedException while the already established session continues.

The same workflow now performs discovery through BeginFind/EndFind and connection through
BeginJoin/EndJoin. CNA's System Link Begin tokens complete synchronously and invoke callbacks with
the supplied state; EndFind performs the network search and EndJoin performs the blocking handshake.
The first run checked BeginFind once per retry, a demo-only nondeterministic count corrected to one
token-contract assertion while retries continue. The listing's QualityOfService also reports
positive downstream and upstream estimates from the real discovery probe trains in both requested
renderer environments.

Together this credits `AvailableNetworkSession::OpenPrivateGamerSlots`, `SessionProperties`,
`AvailableNetworkSessionCollection::IsDisposed`, the four Begin/End members and both QoS bandwidth
properties as coherent lobby, measurement and lifetime behavior. Final SOFTWARE and OPENGLES3/X11
executions pass all19 host/joiner controls with byte-identical stdout and empty stderr; the focused
SOFTWARE CTest passes. Evidence:`/tmp/checkpoint-695-find-*`, `/tmp/checkpoint-699-async-find-join-*`.
Discovery uses Linux local UDP on one machine; no service directory, remote host, screenshot or
visual result is claimed.

### Remote gamer departure and history

The focused extension to `net.network_session.find_and_join` retains the exact remote gamer pointer
raised by GamerJoined. It is initially live and its Session points to the host's real
NetworkSession. CNA moves it out of the active roster before raising GamerLeft; that event supplies
the same pointer with HasLeftSession set, while PreviousGamers contains it as its sole member and
Session still identifies the owner. RoundtripTime remains publicly readable and non-negative after
departure. A zero value is valid on fast local loopback or before a native sample, so this does not
claim positive WAN latency or RTT accuracy.

This credits `NetworkGamer::HasLeftSession`, `Session`, `RoundtripTime` and
`NetworkSession::PreviousGamers` as one coherent production lifecycle. SOFTWARE and OPENGLES3/X11
pass all16 host/joiner controls with byte-identical stdout and empty stderr; the focused SOFTWARE
CTest passes. Evidence:`/tmp/checkpoint-696-find-*`. No stale access after session disposal, remote
machine, service directory or voice state is inferred.

### Ready-state reset and round boundaries

`net.network_session.game_flow` now uses `ResetReady` between two authored lobby attempts. The
first player becomes ready, then the host clears both player flags and IsEveryoneReady together.
Both become ready again and enter Playing. A second reset is refused there with
InvalidOperationException and leaves both flags intact; the existing EndGame transition returns to
Lobby and clears them through the separate round-end path. Both renderers pass8/8 with identical
stdout and empty stderr. Evidence:`/tmp/checkpoint-698-reset-ready-*`.

This credits `NetworkSession::ResetReady` through successful state mutation and its key state guard,
rather than a setter-only assertion. It does not establish cross-machine ready propagation, a
non-host call or online-service synchronization.

### Remaining Net member dispositions after focused699

| Inventory family | Decision |
|---|---|
| `NetworkSession::IsDisposed` | Covered by focused700 in the existing Local-session ownership workflow: a live session is disposed twice, reports disposed, clears Host and all four roster views, and refuses Update. |
| `NetworkSessionProperties` explicit `ICollection`/`IList` members and GetEnumerator entries | Focused701 demonstrates the useful fixed-shape list behavior. Focused1001 additionally exercises CNA's actual public `GetEnumerator` cursor; the remaining ten CLR-mangled interface aliases have no separate C++ entry points and stay intentionally uncredited. |
| GameStarted/GameEnded/GamerJoined/GamerLeft/HostChanged/NetworkSessionEnded event-argument constructors | Framework-created payloads. Existing real events demonstrate their meaningful properties; manually constructing payload objects would add no event behavior. |
| `LocalNetworkGamer::EnableSendVoice`; NetworkGamer HasVoice/IsTalking/IsMutedByLocalUser; `SendDataOptions::Chat` | Voice requires a capture device, a second participant and audible/manual observation. Keep as hardware/manual work; a default-false getter batch would be misleading. |
| `SendPartyInvites` | Focused1001 reaches its real first policy boundary: an actual local session gamer whose offline profile has no other party member gets `InvalidOperationException`. Successful delivery still requires an online service party. |
| InviteAccepted and Join/BeginJoin/EndJoinInvited | Require an online service identity plus another user accepting an invitation. Service/manual; ordinary System Link Join does not imply invitation coverage. |
| WriteArbitratedLeaderboard, WriteTrueSkill, WriteUnarbitratedLeaderboard and WriteLeaderboardsEventArgs properties | Ranked/leaderboard service callbacks. Existing offline sessions cannot raise them; service/manual. |
| `NetworkGamer::IsGuest` | A meaningful positive state requires a service-backed guest sign-in and propagation into a NetworkSession. The current offline profiles only establish non-guest state; defer with the service/Guide guest workflow rather than crediting a constant false. |
| NetworkSessionJoinException constructor/GetObjectData | `join_refused` catches a real exception and observes JoinError. Focused1001 invokes `GetObjectData` on that production exception and proves base-first message/inner/join-error storage. The framework-created constructor remains intentionally uncredited. |
| MaxPreviousGamers/MaxSupportedGamers and remaining enum fields | Constants and enum identities are reported separately from member coverage. Credit only fields used by real behavior; do not create getter-style demos for constants or switch-only labels. Focused702 now creates and disposes `LocalWithLeaderboards`, retaining its exact session type and local roster; the other uncredited enum/constant identities do not justify switch-only controls. |

### NetworkSession disposal state

`net.network_session.local_session` now observes IsDisposed=false on the real two-local-player
session before its existing roster checks. It then calls Dispose twice while retaining ownership of
the NetworkSession object. IsDisposed becomes true, Host becomes null and AllGamers, LocalGamers,
RemoteGamers and PreviousGamers are all empty, so callers cannot observe pointers to gamers released
by disposal. A subsequent Update throws ObjectDisposedException; Unload repeats Dispose safely and
then deletes the object. Both renderers pass10/10 with identical stdout and empty stderr. Evidence:
`/tmp/checkpoint-700-session-dispose-*`.

This credits the ordinary IsDisposed member through state transition and post-dispose safety. It
does not claim transport teardown for System Link, online-service departure or behavior of every
other member after disposal.

### NetworkSessionProperties fixed collection behavior

`net.network_session.session_properties` retains its two authored values and six empty slots while
adding three coherent controls. Contains and IndexOf distinguish values, null and absence, and a
C++ range visits all eight values in order. CopyTo rejects an undersized shifted destination before
changing any sentinel and then writes all eight entries at offset one while preserving the two
outer sentinels. Add, Insert, Remove, RemoveAt and Clear each throw NotSupportedException; Count and
every authored value remain exact. IsReadOnly is false because indexed replacement remains allowed,
even though the collection's shape cannot change.

Both renderers originally passed8/8 with identical stdout and empty stderr. Focused1001 adds the
explicit public enumerator: invalid Current before/after traversal, all eight values in the same
order, Reset and visibility of a fixed-slot write. Both renderers now pass10/10 with identical
stdout and empty stderr. Evidence:
`/tmp/checkpoint-701-session-properties-*`. This demonstrates CNA's actual public C++ collection
surface without falsely naming the inventory's explicit CLR interface aliases. Focused1001 evidence
is under `/tmp/checkpoint-1001-net-members/*`.

### LocalWithLeaderboards offline session type

`net.network_session.online_session_types` now includes the second single-machine session type in
its existing policy comparison. With the same offline local profile that creates a Local session,
LocalWithLeaderboards creates successfully, reports its exact SessionType and contains that one
LocalGamer before disposal. PlayerMatch and Ranked retain their established
GamerServicesNotAvailableException results in the same run.

Both renderers pass5/5 with identical stdout and empty stderr. Evidence:
`/tmp/checkpoint-702-session-types-*`. The leaderboard write/read and paging demos already exercise
the type's gameplay purpose; this control establishes its offline policy branch without claiming a
configured service, transport, leaderboard write, rendered result or additional demo variant.

### LeaderboardReader asynchronous paging and lifecycle

`gamer_services.leaderboard.paging` retains its four real local entries, two-row pages and typed
columns while using BeginRead/EndRead for the initial page. The offline token is synchronously
complete, invokes one callback with exact caller state and produces a reader whose
LeaderboardIdentity retains the requested Key and GameMode. Both page directions now execute their
Begin/End pairs with the same token contract; a subsequent synchronous PageDown/PageUp cycle
preserves the prior direct-method evidence. Repeated Dispose finally publishes IsDisposed.

SOFTWARE and OPENGLES3/X11 pass13/13 with identical stdout and empty stderr. Evidence:
`/tmp/checkpoint-703-leaderboard-reader-*`. Initial read-only lock failures came from redirecting
XDG_CONFIG_HOME while StorageDevice uses XDG_DATA_HOME; isolated writable data roots resolved them.
This demonstrates local-store behavior and the public async contract. It does not establish a
service-backed deferred callback, remote paging, concurrent requests or post-dispose service guards.

### PropertyDictionary typed values and offline stream columns

`gamer_services.leaderboard.paging` writes Outcome, TimeSpan, int32, int64, float, double, DateTime
and string columns before each real rating commit. The first read row recovers all eight values
through typed getters. Writer and reader dictionaries also establish exact Count, present/absent
ContainsKey, successful/unsuccessful TryGetValue with preserved absent output, Item and C++ range
shape. This credits eleven ordinary inventory members without naming the CLR explicit-interface
aliases that CNA does not expose under those names.

The stream column is a confirmed CNA defect. On an offline writer, GetValueStream for a new key
throws `std::out_of_range` because only service-backed entries enable create-on-first-use streams;
source review also shows the offline JSON serializer skips Stream pointers. CNA-DEMOS-ISSUE-085
records the mismatch with the public header. Both renderers execute17 controls, pass16 and fail only
the stream round trip with identical stdout and empty stderr. Evidence:
`/tmp/checkpoint-704-property-dictionary-*`. This is isolated local-store evidence; no configured
service or remote stream-column transport is claimed.

The follow-up collection pass copies those eight loaded columns so mutation cannot affect the live
reader. Keys/Values snapshots agree in sorted order; CopyTo proves atomic insufficient-capacity
refusal and successful offset placement with untouched sentinels. Add, Contains and Remove use full
key/value equality, duplicate Add preserves the original, key Remove distinguishes first/repeated
calls and Clear empties the copy. IsReadOnly remains true despite working explicit mutators, as CNA
documents from FNA. Both renderers retain only ISSUE-085 while all three new controls pass.
Evidence:`/tmp/checkpoint-705-property-collection-*`.

The XNA inventory records five ICollection operations, IsReadOnly and two GetEnumerator forms under
CLR-mangled names. CNA exposes the useful operations as ordinary C++ methods and range iterators,
not as separate CLR entry points. Their behavior is now demonstrated, but those eight generated
names remain intentionally uncredited rather than being placed in metadata synthetically.

### Gamer and GamerProfile identity and disposal

`gamer_services.gamer_profile.get_profile` now establishes that the active offline Gamer is live
and ToString returns the same local identity as DisplayName and Gamertag. Its existing synchronous
and asynchronous reads yield independent GamerProfile objects. Repeated Dispose marks only the
selected profile and makes its GetGamerPicture throw ObjectDisposedException; the other profile
remains live and returns the established null picture for this local account.

Both renderers pass8/8 with identical stdout and empty stderr. Evidence:
`/tmp/checkpoint-706-gamer-profile-*`. This credits Gamer's live IsDisposed state and profile
cleanup/guard behavior. A true disposed Gamer remains tied to signed-out/service lifetime and is
not inferred from profile disposal; no picture asset or remote account is claimed.

### AchievementCollection populated values and lifetime

`gamer_services.achievement.award` now uses its actual three-item catalog for both Item overloads,
their distinct boundary exceptions, atomic insufficient-capacity CopyTo and exact offset copying.
Contains and IndexOf identify copied values. A separate collection copy exercises ordered
Insert/RemoveAt/Add/Remove/Clear while retaining the documented IsReadOnly=true result. Repeated
Dispose clears and marks only the selected result; an earlier snapshot remains live and populated.

Both renderers execute10 controls, pass9 and retain only existing ISSUE-017 with identical stdout
and empty stderr. Evidence:`/tmp/checkpoint-707-achievement-collection-*`. This credits the three
ordinary inventory members Item, Dispose and IsDisposed. The nine explicit ICollection/IList
spellings and two GetEnumerator forms have no distinct CNA C++ entry points; their behavior is
demonstrated but the CLR aliases remain intentionally uncredited.

The same workflow now uses BeginAwardAchievement/EndAwardAchievement for the second real award and
BeginGetAchievements/EndGetAchievements for the populated collection under test. Both local-store
tokens complete synchronously, invoke exactly one callback and preserve caller state; the original
synchronous paths remain in the same run. Both renderers execute12 controls, pass11 and retain only
ISSUE-017. Evidence:`/tmp/checkpoint-708-achievement-async-*`.

This closes the four ordinary SignedInGamer async achievement members. GetFriends/IsFriend and a
positive PartySize require service identities; IsHeadset requires actual microphone association.
Default false/zero getters would not demonstrate those behaviors and remain uncredited.

### AvatarAnimation sampled values and lifetime

`gamer_services.avatar.animation_presets` retains its six concurrently rendered stock clips and
adds an independent Celebrate instance for value and lifetime inspection. At one second its
BoneTransforms collection has exactly AvatarRenderer::BoneCount (71) local matrices with actual
motion. Its Expression contains the clip's keyed laughing mouth and eyes plus both raised eyebrows,
showing that the property carries authored facial state rather than a default placeholder.

The independent instance starts undisposed, accepts Dispose twice, reports IsDisposed and rejects
Update with ObjectDisposedException. Both renderers pass9/9 with byte-identical stdout and empty
stderr. Evidence:`/tmp/checkpoint-709-avatar-animation-current-*`. This credits BoneTransforms, Expression,
Dispose and IsDisposed on the concrete AvatarAnimation. Its protected Dispose(bool) is exercised
through the public disposal path but is not separately nameable; CLR Finalize is represented by
native destruction and remains intentionally uncredited. The run inspects values and automated
rendering, not a screenshot or manual animation quality.

### AvatarRenderer lighting and lifetime

`gamer_services.avatar.render` now supplies distinguishable LightColor, LightDirection and
AmbientLightColor values and verifies their exact getters. Once Update has observed the async
renderer reach Ready, the demo captures the same Wave frame three ways: with the authored lights,
with both light colors zeroed and with the key direction reversed. Each alteration changes thousands
of sampled avatar pixels on SOFTWARE and OPENGLES3, so the properties have demonstrated rendering
effect as well as value storage. The ready renderer finally accepts Dispose twice, reports
IsDisposed and throws ObjectDisposedException from both State and Draw.

Both renderers pass7/7 with empty stderr. Their stdout differs in the randomly generated avatar's
height and exact changed-pixel counts, as expected. Evidence:
`/tmp/checkpoint-710-avatar-renderer-final-*`. This credits the three lighting properties, Dispose
and IsDisposed. Protected Dispose(bool) is reached through the public cleanup path but is not
separately named; CLR Finalize remains intentionally uncredited as native destruction. The pixel
readback is programmatic; no screenshot or manual lighting-quality judgment is claimed.

### AvatarDescription local-profile retrieval and Changed disposition

`gamer_services.signed_in_gamer.automatic_sign_in` now passes the real first-Update local player to
AvatarDescription::BeginGetFromGamer. Its offline token is completed synchronously, invokes one
callback before Begin returns and preserves exact caller state. EndGetFromGamer returns the same
profile's valid stored avatar with the fixed 1021-byte description and positive decoded height.
Both renderers pass8/8 with byte-identical stdout and empty stderr. Evidence:
`/tmp/checkpoint-711-avatar-description-*`.

This credits the Begin/End pair without implying service-backed deferred execution. Changed remains
uncredited by design: production raises it after a configured service observes a new avatar revision
or after the player manually saves in CNA's Guide avatar editor. The ordinary event's public Raise
and CNA's internal local-profile setter can force a callback, but neither is the public gamer-facing
behavior under audit. This needs service or manual UI evidence rather than a synthetic automated
control.

### Achievement catalog-picture streams

`gamer_services.achievement.award` now gives the real `first_steps` catalog entry a small valid
PNG and reads it through `Achievement::GetPicture`. The first returned stream starts at position
zero, has the exact 70-byte asset length and exposes the PNG signature. A second call returns a
distinct caller-owned stream at position zero with the same length, rather than reusing consumed
state. A copied achievement whose catalog entry has no picture throws
`GamerServicesNotAvailableException`.

Both renderers execute 14 controls, pass 13 and retain only ISSUE-017 for the unrelated
`EarnedOnline` value. Their stdout is byte-identical and stderr is empty. Evidence:
`/tmp/checkpoint-712-achievement-picture-focused-*`. This credits `Achievement::GetPicture` with a
real offline asset and both its success and unavailable paths. The PNG is inspected as bytes; it is
not drawn, and no screenshot or manual visual-quality judgment is claimed.

### GamerServicesComponent lifecycle and dispatcher state

`gamer_services.signed_in_gamer.automatic_sign_in` now inspects the actual component installed by
its game base. Before initialization, the collection contains one `GamerServicesComponent`, it
points back to the owning game and the dispatcher is uninitialized. Game initialization then sets
`IsInitialized` and copies the game's exact legacy window handle to the dispatcher.

The demo disables that component for the first game update: the pending local sign-in stays queued
and no event fires. After enabling it, the next update publishes the player and the existing
identity, event and avatar checks pass. Both renderers pass10/10 with byte-identical stdout and
empty stderr. Evidence:`/tmp/checkpoint-713-component-focused-*`. This credits the component
constructor, Initialize and Update plus dispatcher Initialize and WindowHandle. Dispatcher Update
already has direct trial-mode behavior evidence. `InstallingTitleUpdate` remains intentionally
uncredited because CNA installs no title updates and never raises it in production; calling the
generic event's public Raise would not demonstrate that contract.

### Guide platform screen-saver state

`gamer_services.guide.notification_position` now reads the real platform screen-saver state,
sets its opposite value, observes the change, restores the original value and verifies the
restoration. Its existing programmatic framebuffer inspection still proves the sign-in toast is
drawn at TopRight rather than BottomCenter. Both renderers pass5/5 with empty stderr; their stdout
differs only by three rasterized notification pixels. Evidence:
`/tmp/checkpoint-714-guide-state-focused-*`.

This credits `Guide::IsScreenSaverEnabled` through its platform behavior. `DelayNotifications`
remains intentionally uncredited: it only delays game-invitation prompts delivered by an online
service, so an offline call with no arriving invitation has no public effect to observe. The other
remaining social Guide calls likewise require service identities or manual overlay interaction.
Pixel readback was programmatic; no screenshot or manual visual-quality judgment is claimed.

### XACT Cue synchronous preparation states

`audio.xact.cue_states` now checks `IsCreated` and `IsPreparing` alongside its established
prepared/playing/paused/stopping/stopped lifecycle. A Cue returned by synchronous
`SoundBank::GetCue` has already skipped both transient states and is immediately prepared and idle;
the rest of the workflow still covers ordinary playback, repeated disposal, immediate stop and a
real authored release.

Both renderer-labelled dummy-audio runs pass15/15 with empty stderr. Their stdout differs only in
the measured authored-release wait (506 versus508 ms), which the demo already labels as elapsed
control timing rather than output-sample timing. Evidence:`/tmp/checkpoint-715-cue-state-focused-*`.
This credits both remaining Cue state properties without implying that CNA exposes an asynchronous
XSB preparation interval. No listening test or audible-quality result is claimed.

### Microphone enumeration, format conversion and headset classification

The manual `audio.microphone.input_level` workflow now enumerates `Microphone::All` before choosing
Default. With SDL's dummy recording backend, both renderer selections expose the same single
`System audio recording device`; Default is its first entry, `IsHeadset` classifies it as an
ordinary microphone for the UI, and `GetSampleDuration` reverses the exact byte count generated
for a 100 ms mono buffer. Both one-frame direct runs pass2/2 with byte-identical stdout and empty
stderr. Evidence:`/tmp/checkpoint-716-microphone-direct-*`.

This credits All, GetSampleDuration and IsHeadset through an actual enumerated recording-provider
object. The ordinary manual CTest remains declared disabled, as designed. Dummy capture does not
establish physical headset detection, live samples, BufferReady delivery, input-level movement or
recording quality; those remain manual hardware observations.

### C ABI selected diagnostics, ownership, dispatch and math

The 21 standalone C programs cover coherent entry points in each Phase 14 area. The generated
260/3210 route count is metadata coverage, not proof of those routes' full contracts. This first
member review compares `core.h`, `runtime.h`, `content.h` and selected `vectors.h` routes with the actual C demo sources;
the complete ABI still needs a wider audit. C++ behavior does not automatically cover C handles.

| Members / behavior | Decision and evidence |
|---|---|
| C CRT, depth-reduction and ASCII post-process effects | `c_api.graphics.retro_effects` adds24 `graphics_ext.h` routes in32 controls. CRT state verifies all five defaults, independent mutation, zero-to-one clamping, nonfinite/unknown-value atomic refusals, null outputs and ordinary Effect destruction. Depth state round-trips all seven color/palette modes and all three dither modes, preserves the last valid pair across unknown identities and uses the same Effect lifetime. ASCII checks default/mutated state, invalid-input preservation and pre-draw zero grid, uploads a deterministic16x16 color field, then executes explicit-destination Color and whole-viewport monochrome draws yielding exact4x8 and2x2 source grids. Stale source and repeated destroy are refused. SOFTWARE and OPENGLES3/Xvfb pass32/32 with byte-identical stdout and empty stderr. This proves C bridge state and that ASCII quantization/draw completes; it does not duplicate the existing C++ scenes' detailed pixel/visual evidence for CRT and Depth. Evidence:`/tmp/checkpoint-818-retro-*`. |
| C DebugDraw geometry, list routing and frame lifecycle | New `c_api.graphics.debug_draw` covers all15 remaining `graphics_ext.h` routes in28 controls. Begin resets both line lists and depth mode; Clear preserves an open frame and its mode; End draws and clears idempotently. Depth/overlay line routing, count-only and atomic exact vertex copying, malformed inputs, box/frustum/cross geometry and both sphere segment clamps are explicit. The final423-line batch proves exact cross coordinates and all768 maximum-segment bounding-sphere vertices lie at radius two. SOFTWARE and OPENGLES3/Xvfb pass28/28 with byte-identical stdout and empty stderr; the exact SOFTWARE CTest passes. `graphics_ext.h` is42/42 and every public C header now has a demonstrated route. Evidence:`/tmp/checkpoint-819-debug-draw-*`. The existing C++ scene retains detailed pixel evidence; no screenshot or manual visual inspection is claimed here. |
| C renderer metadata and classification | New renderer-independent `c_api.platform.renderer_metadata` covers all11 renderer metadata routes in `core_ext.h`. It checks fixed category/maturity ordinals and exact stable names, classifies all ten live public renderer identities, refuses unknown/retired identities, and correlates the compile-time renderer's identity, exact unterminated build-option name, category and maturity. Short copies are atomic. SOFTWARE- and OPENGLES3-labelled direct runs pass8/8 with byte-identical stdout and empty stderr; the exact SOFTWARE CTest passes. Evidence:`/tmp/checkpoint-820-renderer-metadata-*`. No device is created, so availability and capability remain separate. |
| C renderer selection, latch and fallback diagnostics | New `c_api.platform.renderer_selection` covers all18 production selection/fallback routes in `core_ext.h`. It atomically enumerates the six compiled backends, compares availability across all ten live identities, parses and selects by name/identity, toggles automatic fallback, and rejects malformed requests. A real device request for absent WEBGL2 falls back to the command-line renderer, leaving distinct selected/active identities and one exact NotCompiledIn history record; every mutation is refused after latching. SOFTWARE and OPENGLES3/Xvfb pass18/18 with byte-identical stdout and empty stderr; the exact SOFTWARE CTest passes. Evidence:`/tmp/checkpoint-821-renderer-selection-*`. The sole remaining route, `cna_graphics_renderer_reset_selection_for_tests_ext`, is explicitly test-only and intentionally uncredited. No pixel or visual-quality claim is made. |
| C graphics-extension availability and image-based-light values | `c_api.graphics.image_based_light_values` adds the first three `graphics_ext.h` routes in25 controls. It verifies canonical availability, complete initialization overwrite, structure size/version and null boundaries, empty and partial bundles, mip-count and negative/NaN-intensity refusals, a valid pair of live TextureCube resources plus live Texture2D BRDF lookup, and invalidation after resource destruction. The final wrong-kind control exposes ISSUE-091: three live Game handles incorrectly satisfy the public texture-bundle predicate because the C adapter checks liveness without resource kind. SOFTWARE and OPENGLES3/Xvfb each pass24/25 with the same sole failure and empty stderr. This is value/handle validation; no PBR effect, draw, sampled light or visual result is claimed. Evidence:`/tmp/checkpoint-817-ibl-*`. |
| CNA_Result, error category and message count/copy | `c_api.core.error_handling` checks null outputs, an unissued handle, a short type-name buffer, and an update callback failure. It reads the structured diagnostic and exact byte count, including failed-query preservation and capacity boundaries. These are meaningfully demonstrated together; no result-code-per-demo expansion. |
| Buffer-too-small atomicity and terminators | The same demo now verifies every destination/guard byte remains unchanged after a refused type-name copy. It then copies into exactly the required byte capacity, checks the following sentinel and adds its own terminator. UTF-8 multibyte text remains a distinct encoding case, not covered by the ASCII type name. |
| Launch parameter spans, ownership and enumeration | c_api.game.launch_parameters now passes fifteen child checks on SOFTWARE/OPENGLES3 with empty stderr. Its explicit bounded StringViews survive caller input overwrite, UTF-8 byte counts distinguish four characters from five bytes, exact/short copies preserve guards, and sorted enumeration puts Zeta before lowercase names. Duplicate Add keeps the first value per the documented C contract. Malformed replacement preserves prior values/count, a null zero-length view creates an empty value and an explicit empty list clears the map. Output lengths are preseeded to detect missing writes. Eight routes gain metadata coverage (166/3210 total). No loop/image, arbitrary Unicode collation or every invalid argument permutation is claimed. The prior external build failure is resolved as ISSUE-053; complete 456 passes all 469 enabled CTests and both full smokes, retaining every original 35/36 failure. General dictionary getter variants need no separate examples. |
| Error-query failures preserve diagnostics; per-thread storage | The public error-query contract explicitly preserves the previous diagnostic even when the query itself fails. error_handling now checks that a null information query and a too-short message copy leave the original diagnostic intact (retained within its current 31 child checks). The compatible-output-prefix extension allocates two unknown tail fields after CNA_ErrorInfo: both renderer selections pass the two new checks for known-field population, preserved tail bytes and retained thread diagnostics. All 29 older assertions remain, and focused stderr is empty. `c_api.core.thread_local_errors` retains all eight controls interleaving two distinct worker failures and preserving the caller's diagnostic. Its later owner-only Game fixture now adds nine passing checks: canonical MasterVolume=2 type/parameter/message survive worker C validation, and a repeated owner-native failure does not populate the worker's empty type/parameter fields. Both selections pass all 17 child checks with empty stderr. Error snapshots copy the four public detail queries; C11 atomic phase waits synchronize the interleaving. The worker uses no Game handle and no pump runs, so ordinary owner calls cannot erase the diagnostic being measured. This establishes this canonical-owner/C-validation-worker contrast, not every exception family, two different concurrent nonempty parameter names or foreign dispatch. Runtime evidence is Linux; the target probes C11 thread/atomic support before building. Source audit rejected an invalid perspective-matrix call as a candidate for named-parameter isolation: the current Matrix::CreatePerspectiveFieldOfView throws std::invalid_argument, so it does not establish a canonical ParamName. Choose an actual canonical exception route before extending that claim. The pure C SoundEffect sample-size/duration helpers were also reviewed: invalid/nonpositive rates return zero in the native implementation, while undefined channel identities fail in the C adapter. Neither supplies a canonical parameter-name failure. The executed owner-only fixture avoids those rejected routes and needs no foreign-call dispatch. A future served fixture would still need separate pump/diagnostic semantics. |
| Canonical exception type and parameter-name extensions | `error_handling` calls the four public count/copy routes after rejected MasterVolume=2: INVALID_ARGUMENT/ARGUMENT also describes a native C null-output failure, but the canonical call carries System.ArgumentOutOfRangeException and ParamName=value. A later invalid handle clears both details (31 child checks overall pass on both renderer selections). Focused924 extends the real renderer-selection workflow instead of manufacturing exceptions: an unknown renderer name reports INVALID_ARGUMENT/ARGUMENT plus System.ArgumentException and ParamName=name, while mutation after a real device latch reports INVALID_STATE/STATE plus System.InvalidOperationException and no parameter. Both selections pass20/20 in ten byte-identical repetitions. Focused925 then reaches a real ContentManager `ContentLoadException`; its missing type exposes ISSUE-103 rather than being accepted as an untyped IO error. Other families should be added only through similarly useful production failures; synthetic construction remains inappropriate. |
| Callback failure and release after failure | error_handling observes CALLBACK, the callback's supplied message, CALLBACK again from destroy, and an invalid handle afterwards. A third RunOneFrame returns the same failure without another Update or premature unload. Destruction invokes UnloadContent exactly once with the original caller-owned context and null time; successful cleanup preserves the original callback message. Its three failure-retention/cleanup checks remain within the current 31 passing child checks under SOFTWARE/OPENGLES3. This demonstrates failure retention and cleanup, not recovery from a failed game. Callback success and normal load/update/draw/exit/unload sequencing are checked by `c_api.game.lifecycle_callbacks`. Selected owner-thread reentry is now checked below; other reentrant routes and failure during other lifecycle hooks remain separate gaps. |
| Owner-thread callback reentrancy | lifecycle_callbacks calls RunFrameEXT and Destroy from the first Update and checks INVALID_STATE/STATE plus the specific lifecycle-callback diagnostic. This distinguishes destruction's callback guard from the separate owned-manager release guard. The original sixteen child assertions still pass, proving the outer run, fresh/expired device borrows, fixed timing, exit and unload survive; totals are eighteen without capture and twenty with capture on both selections. The inspected scene remains a solid time-dependent color, so no cross-renderer pixel identity is claimed. Other driving routes, callback locations and permitted nested resource calls remain separate. This owner-thread control does not fix foreign-thread ISSUE-050. |
| Versioned structures and borrowed callback handles | All C demos initialize structure size/version and reserved storage. Lifecycle obtains the graphics device only within callbacks. error_handling now rejects an undersized ErrorInfo declaration and unsupported version with INVALID_ARGUMENT while preserving the original diagnostic (31 child checks overall on both renderer selections). The C object is fully allocated despite declaring a shorter size. The same demo now accepts an actually allocated larger version-one ErrorInfo prefix, fills known fields and preserves its two unknown tail values and the original diagnostic. host_frames independently accepts a larger input callback prefix. Other structure families remain separate decisions; these examples do not establish arbitrary future versions. The lifecycle demo now verifies a device is usable in LoadContent, expired with INVALID_HANDLE in the next Update, and replaced by a fresh usable borrow; after Run, a new borrow outside callbacks returns INVALID_STATE. Including the reentry extension, both renderers pass 20 child checks with screenshots, 18 without in CTest. No borrowed-handle release or lifetime extension is attempted; its old numeric value is retained solely for the negative check. |
| Owned content handles and release order | `c_api.content.content_manager` loads a checker texture and a font plus independently owned atlas, draws them, checks actual checker pixels and font metrics after both Unload and manager destruction, draws the resources and releases the font before its atlas and all children before the game. It now also checks INVALID_STATE when attempting to destroy the live font's retained atlas, then proves the checker and text still render after manager destruction. Including the root-change extension, both renderers pass 41 child checks with screenshots (39 without); rendering remains pixel-identical to the preceding atlas-guard baseline. Other invalid parent/child destruction and active-batch retention remain separate gaps. |
| Content unload/destruction versus loaded C handles | `content.h` explicitly promises that independently owned loaded handles survive manager unload/destruction. The C content demo now exercises both boundaries and verifies the original texels and metrics afterwards. SOFTWARE and OPENGLES3 pass; the latter's image is pixel-identical to the original demo before the lifetime extension. Do not transfer the C++ Unload disposal expectation (ISSUE-011) to this different C ownership contract. |
| RootDirectory and resource-backed managers | C's setter explicitly permits changing the root for uncached loads while preserving cache entries; do not copy the XNA loaded-root guard expectation (ISSUE-039) into this demo. The existing C content demo now changes from assets to assets/textures while data is cached, reuses the caller's root buffer and verifies count/copy returns the original path. A cached checker still loads; an uncached sprite_strip resolves from the new directory. After Unload the old checker name returns IO under that directory, while previously owned handles still draw. Both renderers pass 41 checks with screenshots, and CTest passes 39 without. The three root routes are credited; embedded-resource managers and multibyte root names remain separate gaps. |
| Game-owned content manager borrowing and copying | content_manager now passes eight new child controls on both selections, preserving all39 originals (47 without capture/49 with). A distinguishable initial game root proves the setter actually copies the caller-owned configuration; later source-root changes do not reach it. Repeated borrowing returns the same handle; independent destruction is refused, the borrow survives callbacks/source destruction, and game destruction invalidates it while a stale query leaves its output sentinel intact. Two actual C routes gain metadata. All four before/after C images were inspected and are RGBA-identical; stderr is empty. Copy precedes asset loads, so cached-object sharing, device-handle expiry and loads through the borrow remain separate. |
| C resource-backed content manager | Focused1007 corrects this older disposition: `c_api.content.cnb_loader_registry` already requires successful `cna_content_manager_create_resource`, uses that manager through repeated custom and built-in loader callbacks, observes callback scope and destroys the owner, so the constructor is demonstrated. Embedded-resource loads still return IO because no native store exists; keep that load behavior intentionally unsupported until a real embedded-asset workflow exists. The older C++ virtual-stream behavior/ISSUE-012 is a separate contract and remains independently recorded. |
| C SpriteBatch retention and active-interval destruction | graphics.h explicitly retains submitted texture handles until successful End, and promises that destroying a deferred batch cancels queued commands and releases references. The existing sprite_batch scene now preserves every original child check and adds eight passing controls on both selections (19 without capture, 21 with capture): a temporary red texture and deferred batch are created, its corner command is submitted, texture destruction returns INVALID_STATE while retained, then destroying the active batch permits texture destruction. Cancellation occurs immediately before final-frame readback; the existing clear-corner assertion proves the queued red rectangle was not flushed. Both focused stderr streams are empty. All four before/after 800x480 images were inspected: GL is RGBA-identical; SOFTWARE ring timing differs while stable center/corner pixels remain exact. These are C handle-lifetime controls, not automatic C++ ownership credit. Immediate-mode undo is expressly not promised. Successful End is covered by the later extension to this same scene, below. The same example now adds ten successful-End controls on both renderers, preserving every earlier child label/order (29 without capture,31 with). An independent deferred batch submits a gold bottom-right rectangle, refuses texture destruction before End, then releases its texture after successful End while the batch is still alive. Destroying the completed batch leaves all400gold pixels exact in the final readback; original white-center and cancelled red-corner controls remain. Four actual C before/after images were inspected; time-dependent ring motion prevents a general image-identity claim. All stderr streams are empty. This establishes successful flush/release independently of active cancellation; failed-flush cleanup and Immediate-mode behavior remain separate. The later command-array extension retains every29old child check and adds one invalid-second-texture control (30without capture/32with) on both selections. The valid first command would paint the old clear corner, but the invalid second handle returns INVALID_HANDLE before either enters the batch. A later valid gold submission and successful End retain all400gold pixels while all400left-corner pixels stay clear. Four actual C images were inspected; stderr remains empty. This proves this handle-validation preflight and continued batch usability, not rollback after native draw/flush failure or every invalid POD-field permutation. |
| C audio parent/child destruction | `audio.h` documents an owned SoundEffectInstance as a C child of both its SoundEffect and game, despite retaining native sound data. `c_api.audio.sound_effect` now refuses SoundEffect/game destruction with a live instance (INVALID_STATE), verifies the duration still reads correctly, then plays to completion, releases instance/effect/game in order and refuses a query on the released instance (INVALID_HANDLE). All 19 child checks pass under SOFTWARE/OPENGLES3 with dummy audio. This validates state/ownership, not audible quality; no additional tone variant or demo ID was added. |
| C keyboard snapshot query and enumeration | The existing keyboard/mouse demo now passes three additional controls on both selections while retaining all12older labels in order (15total, empty stderr). A snapshot built from the deliberately unsorted Left/Space input proves `is_key_up` is the exact complement of the existing down query for held and released keys. Copying into a one-key destination returns BUFFER_TOO_SMALL with required count2 and preserves both sentinels; an exact destination then receives Space followed by Left in ascending numeric order. These two routes gain metadata credit. This deterministic POD example does not establish live key transitions, keyboard-layout mapping, thread behavior, null buffers or every invalid-key case. Focused and complete evidence:checkpoint-474-c-input-keyboard-state-*. The full post-build capture saw clean CNA74194f4; concurrent external LINK-050 later changed only CMake tests and final state was cleanf44a8ff. All37selected binaries remained hash-identical, so the accepted run is immutable binary/runtime evidence rather than an atomic external-source snapshot. |
| C touch snapshots and previous locations | `c_api.input.touch_state` uses CNA's public, process-wide test fixture routes to avoid depending on physical hardware. Both renderer selections pass the same30 child checks with empty stderr: connected capabilities report the fixed XNA maximum4, the initial snapshot is empty, exact press/move/release positions appear after explicit frame advances, and a second empty-slot frame retires the released touch. `find_by_id` returns the matching press and a canonical invalid location for a missing id. `try_get_previous` writes invalid/false for the new press, then reconstructs the Pressed predecessor of Moved and the Moved predecessor of Released. Reset runs before game destruction. Four `input.h` and four `input_touch.h` routes gain credit. Complete475 preserves all38/42old smoke failures and protected child evidence while the new ID contributes the same30 checks and parentJSON0;488enabled CTests pass. This does not demonstrate physical-device enumeration, platform event scaling, gesture detection, multiple fingers, pressure or renderer pixels. Focused evidence:checkpoint-475-c-input-touch-lifecycle-focused-*; complete evidence:checkpoint-475-c-input-touch-state-*. |
| C touch value identity and collection operations | The same C demo now retains all30 lifecycle controls and adds30 passing controls on both renderer selections, with identical output and empty stderr. Two locations that differ only in pressure compare equal, hash equally and match collection Contains/IndexOf/Remove, reflecting the canonical identity contract. A two-location local snapshot is disconnected and reports the canonical advisory IsReadOnly=true, yet Insert/RemoveAt/Add/Clear all succeed. A short CopyTo reports the four-entry requirement without changing any destination byte; the exact call inserts both snapshot entries and shifts the existing tail. Fifteen actual `input_touch.h` routes gain credit, raising that header4->19/47 and the total C ABI count239->254/3210. All57input/CLI CTests pass with six declared skips. Complete475 preserves every unrelated parent/child result and38/42failure multiset while only this demo grows30->60; normalized stderr is exact. This is authored POD behavior, not physical pressure sensing, concurrency or an unbounded collection. Focused evidence:checkpoint-475-c-input-touch-values-focused-*; complete evidence:checkpoint-475-c-input-touch-values-*. |
| C authored gesture values and FIFO queue | `c_api.input.gesture_queue` passes31 identical controls under both renderer selections with empty stderr. Reset establishes a None mask and empty queue; Tap|FreeDrag round-trips exactly. Explicit Tap and FreeDrag samples preserve their timestamps, primary/secondary finger ids and all four vectors. Disabling detection after two enqueues leaves them pending; reads return exact FIFO values, availability stays true until the second read, and an empty read returns INVALID_STATE without changing its preseeded output. Six actual routes raise `input_touch.h`19->25/47 and total C coverage254->260/3210. All58input/CLI CTests pass with six declared skips. Complete476 adds exactly these31 controls while preserving all475 prior parent/child outcomes,38/42 failures, protected traces and normalized diagnostics;489enabled CTests pass and39 selected runtime paths stay fixed. The post-build capture saw clean CNAf44a8ff before unrelated external source drift during smoke. This authored queue does not claim physical detection, recognizer timing/thresholds, callback threads or pixels. Focused evidence:checkpoint-476-c-input-gesture-queue-focused-*; complete evidence:checkpoint-476-c-input-gesture-queue-*. |
| C raised-touch display scaling and source boundary | The gesture demo now passes64 identical child checks under both renderers with empty stderr; filtering33 additions restores all31 prior labels in order. Reset exposes width0, height0 and Default orientation. Normalized press/move calls then succeed without queuing a gesture, proving the zero-size drop. At640x480, a large raised FreeDrag returns finger23, exact position(480,240) and delta(320,120); Portrait round-trips independently. The same C call does not populate `CNA_TouchState`, an undefined state is refused, release adds nothing with DragComplete disabled, and reset clears all three display properties. Seven actual routes raise `input_touch.h`25->32/47 and total C coverage260->267/3210. All58 exact input/C-API/CLI tests pass with six declared skips. Complete476 preserves every parent outcome,38/42 failures, unrelated C/custom child list, protected trace and normalized diagnostic;489enabled CTests pass and all39 preflight runtime paths stay fixed. Two CNA libraries already carried unrelated dependency drift before preflight; the full capture/final state is clean bb6820f. ISSUE-060 records that the header calls this both gesture input and an event-map source even though implementation/runtime establish gesture-only behavior. This does not establish physical touch, orientation-driven coordinate rotation, the exact drag threshold, snapshot injection or pixels. Focused evidence:checkpoint-476-c-input-touch-scaling-focused-*; complete evidence:checkpoint-476-c-input-touch-scaling-*. |
| Remaining C `input_touch.h` route decisions | The existing `c_api.input.touch_state` now demonstrates43/47 routes. The direct72-control workflow adds complete default/authored gesture, capability, location and empty-state construction; exact unterminated and atomic-short TouchLocation text; and the live false/true touch-device getter around the already demonstrated public fixture setter. SOFTWARE and OPENGLES3 output is byte-identical with empty stderr. The remaining four routes are intentionally deferred: mouse-touch emulation's getter/setter require manual pointer input, while the opaque window-handle getter/setter are platform integration. Evidence:`checkpoint-535-c-touch-values-focused-*-direct.*`, `checkpoint-536-c-touch-device-exists-focused-*-direct.*`. Project-built runner and generated coverage evidence await a coherent sibling CNA checkout. |
| C mouse value semantics | `c_api.input.keyboard_mouse` retains all15 prior controls and adds six coherent value checks. Default initialization yields the origin, zero wheels and no buttons. The ordinary constructor maps position/vertical wheel/button bits and leaves the horizontal wheel zero; the extension constructor preserves a distinct horizontal wheel. Equality observes the button/horizontal differences while the canonical hash intentionally ignores them and matches for equal position/vertical wheel. Size/copy produces the exact field-bearing text rather than a generic type name. These eight `input_mouse.h` routes are demonstrated without repeating generated null/invalid-bit checks. |
| C live mouse binding, coordinates, modes and clicked-event lifetime | The same program adds eight controls. Both renderers round-trip their current window token and report SetPosition(123,77) in the next live snapshot before restoration; a global desktop warp reaches(17,19) exactly and restores the original position. OPENGLES3 exposes a nonzero window token, toggles/restores relative mode and applies capture/release. SOFTWARE exposes token0, so its void canonical relative setter succeeds without changing false; capture reports applied=false while release reports true. Both are asserted documented capability outcomes rather than hidden skips. A process-wide clicked callback runs synchronously with native button3, unsubscribe blocks button4, and reset drops a second registration while its owned handle still releases safely after restoring the window token. These twelve remaining `input_mouse.h` routes are all exercised; no physical button/wheel transition, pointer confinement, other desktop, renderer or thread/reentrancy claim is made. |
| C cursor ownership and final mouse/cursor route disposition | The same program adds three ownership controls covering all six `input_cursor.h` routes. An owned default cursor installs, disposes twice and destroys; disposing a borrowed stock Arrow is a deliberate no-op and the same handle remains installable; a cursor copies the existing one-pixel texture, installs, then disposes/destroys after restoring Arrow. Both renderer selections pass the same32 total child labels with empty stderr; filtering17 additions restores all15 prior labels. ISSUE-061 records that the C header calls the owned default empty/no-op although its direct public source path constructs Arrow; no pointer-shape visual claim is made. Together the preceding three rows give every public route in `input_mouse.h` and `input_cursor.h` a demonstrated semantic role:20/20 and6/6, raising actual C metadata coverage267->293/3210. Complete476 preserves every parent outcome,38/42 failures, unrelated C/custom child list, protected trace and normalized diagnostic;489enabled CTests pass and all39 selected runtime paths stay fixed at clean CNAbb6820f. Focused evidence:`checkpoint-476-c-input-mouse-cursor-focused-*`; complete evidence:`checkpoint-476-c-input-mouse-cursor-*`. |
| C keyboard values, live slots and key-name mapping | `c_api.input.keyboard_mouse` retains all32 prior controls and adds eight coherent checks. Default initialization writes the versioned empty POD; the indexer distinguishes Down/Up; equality ignores construction order while inequality detects a different set; and single-key A/NumPad1 states prove the documented 32-slot XOR hash collision without treating a collision as equality. Live captures for player slots One and Four equal the ordinary snapshot, modifiers contain only defined combinable bits, physical A's copied scancode name round-trips to A, and physical A's layout-dependent copied key name resolves to the same key as `get_key_from_scancode_ext`. Both renderer selections pass the same40 labels with empty stderr and filtering the eight additions restores every complete476 label in order. These checks demonstrate14 previously uncredited routes plus the existing `init_from_keys`, raising `input_keyboard.h`1->15/17 and total C coverage293->307/3210. At that checkpoint the fixed state-text pair remained uncredited; focused822 later added it coherently to this value demo and closed the header. Complete476 preserves all489 enabled CTests, every parent outcome,38/42 failures, unrelated C/custom list, protected trace and normalized diagnostic; all39 selected paths stay fixed. External LINK-053 accounts for15 additional content-related binary differences from the prior complete baseline, while every relevant keyboard source hash stays exact. No physical key transition, alternate layout, lock-key toggle, scancode mode, other desktop or callback/thread claim. Focused evidence:`checkpoint-476-c-input-keyboard-focused-*`; complete evidence:`checkpoint-476-c-input-keyboard-*`. |
| C gamepad value families and composed snapshots | `c_api.input.gamepad` retains all28 prior controls and adds19 coherent checks covering34 previously uncredited `input_gamepad.h` routes. Button values build from empty/mask/array, query combined bits, compare and expose their raw-mask hash. D-pad construction ignores non-directional array members; queries/comparisons use only four directional bits and the hash uses canonical weights10for Up+Left. Thumbsticks preserve in-range positions, square-clamp outside components and compare exactly; triggers clamp0..1 and compare with epsilon. Equal values have equal hashes. The existing raw state is reproduced from components, all component getters round-trip, packet7 changes equality, and an analog-only difference changes equality while preserving the button/packet-only state hash. The capability default checks every public feature flag, version fields, type and reserved byte. Scalar dead-zone exclusion complements the existing full analog transforms. Both selections pass the same47 labels with empty stderr; filtering19 additions restores all28 complete476 labels. Two watched builds keep3912 CNA inputs fixed, and all58 input/C-API/CLI tests pass with six declared skips. Coverage rises307->341/3210 and `input_gamepad.h`4->38/63. Complete476 preserves all489 enabled CTests, every parent outcome,38/42 failures, unrelated C/custom list, protected trace and normalized diagnostic; only the catalog and C gamepad executable differ, and all39 selected paths stay fixed. Unrelated DirectX11 retirement work appears in CNA during smoke without changing relevant gamepad source hashes or Linux runtime artifacts. At that checkpoint the fixed state-string pair remained uncredited; focused822 later added it to this value demo and closed the header. The other live device operations are covered for disconnected semantics while hardware-success limitations remain manual. Focused evidence:`checkpoint-476-c-input-gamepad-values-focused-*`; complete evidence:`checkpoint-476-c-input-gamepad-values-*`. |
| C disconnected gamepad device contract | `c_api.input.gamepad` adds eight grouped controls and retains all47 previous labels in order. It finds an empty player slot, then demonstrates23 live routes without pretending to have controller hardware: ordinary and trigger vibration return applied=false, the void light-bar call succeeds, gyro/accelerometer return unavailable while preserving poisoned vectors, and the player/power/label/connection queries return -1, Error/-1, Unknown and Unknown. GUID, name, path and serial share one zero-byte size/copy protocol check. Firmware and Steam identities are zero. Touchpad and finger counts are zero, and the unavailable finger query preserves every poisoned field. Both renderer selections pass the same55 labels with empty stderr; the exact58 input/C-API/CLI tests pass with six declared skips. Coverage rises341->364/3210 and `input_gamepad.h`38->61/63. The two remaining routes are fixed GamePadState type-name reflection and remain intentionally uncredited. All hardware-success branches remain manual. Direct compilation is clean and metadata validates476/0, but the ordinary watched/full build is currently blocked before this demo by unrelated in-progress sibling-CNA Vulkan source/header incompatibility at dirty9edbf314; the focused runtime uses a manual link against the last built CNA libraries and is not a complete-build claim. Evidence:`checkpoint-476-c-input-gamepad-device-focused-*`. |
| C explicit live gamepad dead-zone capture | `cna_gamepad_get_state_with_dead_zone` remains intentionally uncredited and manual-deferred. The C API has no keyboard-gamepad emulation switch; without a physical controller, None, IndependentAxes and Circular all return the same disconnected neutral snapshot. `c_api.input.gamepad` already demonstrates all three algorithms over authored analog values through `cna_gamepad_apply_dead_zone`. A meaningful extension needs a physical stick held in a range where the modes differ, with the ordinary and explicit IndependentAxes captures agreeing. Calling the overload on the disconnected slot would only inflate route count. |
| C input state text and touch process properties | Existing input workflows add eight coherent routes. Gamepad and keyboard states size/copy the exact fixed canonical type names, show that connected/pressed fields do not affect the text and preserve short destinations. Touch state toggles and restores mouse-to-touch emulation, atomically rejects a noncanonical boolean, round-trips an authored opaque native-window token and restores its prior binding. SOFTWARE/OPENGLES3 gamepad runs pass57/57 and touch runs75/75 byte-identically; OPENGLES3 keyboard/mouse passes42/42 while SOFTWARE executes the new text controls before its existing no-desktop-pointer skip. All stderr is empty. Evidence:`/tmp/checkpoint-822-input-*`. This closes `input_gamepad.h`63/63, `input_keyboard.h`17/17 and `input_touch.h`47/47. The explicit live dead-zone route remains manual for the reason above. No physical input or visual claim is made. |
| C host input descriptors, enumeration, motion, hot-plug and power | New standalone `c_api.input.devices` passes29 identical controls on SOFTWARE/OPENGLES3 with empty stderr. Default SensorInfo/InputDeviceInfo values are versioned; equality observes sensor id/kind/name and full-width device id/name. Xvfb's three mice and three keyboards execute every info/name size/copy route with exact strings; touch and host-sensor counts are zero, their index-at-count boundaries are refused, and their six populated-metadata routes remain manual/uncredited. Accelerometer and gyroscope report unavailable and preserve poisoned vectors. Four public raised hot-plug events synchronously reach only their handler/id; selective unsubscribe and global reset have distinct observed effects, and registrations release safely after reset. One power snapshot returns Charged,79percent and unknown seconds while satisfying all documented ranges. Twenty-seven actual routes establish `input_devices.h`27/37 and raise total C coverage364->391/3210. Direct compilation is diagnostic-free, metadata is477/0 and all59 exact input/C-API/CLI CTests pass with six declared skips. The standard/full build remains blocked by unrelated in-progress sibling-CNA Vulkan source/header incompatibility, so focused runtime uses the last built libraries and makes no complete-build claim. The four clipboard routes are deliberately separate because they mutate desktop-global state. Evidence:`checkpoint-477-c-input-devices-focused-*`. |
| C clipboard text ownership and byte boundaries | New standalone `c_api.input.clipboard` passes15 identical controls on SOFTWARE/OPENGLES3 with empty stderr. It first captures the desktop-global value through a bounded retry around the count/copy race. An explicit multibyte UTF-8 request makes has-text true and returns its exact unterminated byte count and bytes. A copy with one byte less capacity returns BUFFER_TOO_SMALL, reports the full requirement and preserves all128 destination bytes. Empty text returns false/zero and supports a null zero-capacity copy. Finally, the exact original bytes are restored and reread. This demonstrates all four clipboard routes while acknowledging that another process can still race external state. Direct compilation is diagnostic-free, metadata is478/0 and all60 exact input/C-API/CLI tests pass with six declared skips. Coverage rises391->395/3210 and `input_devices.h`27->31/37. Its last six uncredited routes need a populated host sensor or touch-device enumeration. The standard/full build remains externally blocked as above; focused runtime uses the last built CNA libraries. Evidence:`checkpoint-478-c-input-clipboard-focused-*`. |
| C device-layer clipboard acceptance | The same clipboard demo replaces its original plain probe write with `cna_devices_clipboard_set_text_ext`, requires the platform's returned acceptance flag, and retains the other14 controls: exact UTF-8 read-back, atomic short-buffer refusal, empty text and restoration still pass identically under both renderers with empty stderr. The later empty and restore writes continue to exercise the plain setter. This one route raises C coverage646->647/3205 and `devices.h`58->59/62 without adding a demo or leaving external state intentionally changed. Metadata remains488/0 and the exact focused CTest passes. Another process can still race the shared clipboard. Evidence:`checkpoint-488-c-clipboard-acceptance-focused-*`. |
| Remaining C `devices.h` route decisions | Every one of62 routes now has a semantic disposition. Five focused demos plus clipboard acceptance demonstrate59. The remaining three deliberately cross the process boundary: `cna_url_launcher_open_ext` hands a nonempty URL to another application, `cna_system_tray_create` creates a real host icon, and `cna_camera_create` may activate hardware or trigger a permission prompt. Empty/null refusals alone would not demonstrate those behaviors, so the routes remain manual and uncredited. The immediate dialog, vibration, tray and camera test paths already cover CNA-owned request, state, callback, frame and lifetime behavior without making external side effects. |
| C display and presentation values | New `c_api.graphics.display_values` passes eight identical controls under both renderer selections with empty stderr and needs no game. DisplayMode initialization writes the complete versioned value and computes both 16:9 and the documented zero aspect for zero height; equality observes width/height/format and ignores a poisoned cached ratio. PresentationParameters supplies every XNA-compatible default, accepts a fully edited valid value, derives its zero-origin bounds and refuses a nonzero reserved byte before touching the destination. Its native clone preserves all XNA fields but resets CNA's non-XNA `headless_ext`, matching the implementation and binding documentation. These five routes raise C coverage647->652/3205 and establish `display.h`5/22. Direct C/C++ compilation is clean, metadata is489/0, the exact focused CTest passes and all56 enabled tests in a57-test C/API, adapter, device-manager and CLI selection pass. No device, window, rendering or visual validation is claimed. Evidence:`checkpoint-489-c-display-values-focused-*`. |
| C graphics-adapter snapshot | New `c_api.graphics.adapter_snapshot` uses the active game's own handle before its first frame and passes15 identical controls under both renderer selections with empty stderr. The one Xvfb adapter is a complete default snapshot with nonempty names; exact description/device-name copies preserve terminator guards, while one-byte-short requests report the full requirement without any write. The current mode agrees with the widescreen flag. Complete and Color-filtered mode arrays contain coherent versioned values, short arrays are atomic, and the index-at-count is refused by every snapshot route before its outputs change. This host reports `screen`, `\\.\DISPLAY1` and one1280x1024 Color mode. These seven routes raise C coverage652->659/3205 and `display.h`5->12/22. Direct C/C++ compilation is clean, metadata is490/0, the focused CTest passes and all57 enabled tests in a58-test C/API, adapter, device-manager and CLI selection pass. Complete490 then passes503 enabled CTests and both full smoke suites; all476 prior statuses remain and the only added failed assertions are the two documented ISSUE-062 sensor-lifecycle checks. No rendering, other monitor topology or visual validation is claimed. Focused evidence:`checkpoint-490-c-adapter-snapshot-focused-*`; complete evidence:`checkpoint-490-c-adapter-snapshot-*`. |
| C graphics-adapter capabilities and limitations | New `c_api.graphics.adapter_capabilities` passes16 identical controls under SOFTWARE and OPENGLES3 with empty stderr. Reach support is true; Color/None/0 is an exact render-target and backbuffer selection, and both versioned outputs overwrite poisoned fields and reserved bytes completely. The demo reads both device-preference flags, toggles one, observes both, restores the exact original pair before device creation and confirms it in a fresh snapshot. Native monitor lookup returns NOT_SUPPORTED with zero output. Adapter refresh uses a callback-scoped device and likewise returns its documented NOT_SUPPORTED result, because invalidating the global cache would dangle the active device's adapter. These six routes raise C coverage659->665/3205 and `display.h`12->18/22. Metadata is491/0, the exact CTest passes and all58 enabled tests in the Xvfb-backed59-test C/API/adapter/device-manager/CLI selection pass. A no-display diagnostic run isolates existing desktop clipboard/cursor requirements and is not a regression. No native handle, refresh, pixels or visual inspection is claimed. Evidence:`checkpoint-491-c-adapter-capabilities-focused-*`. |
| C live graphics-device presentation | New `c_api.graphics.device_presentation` passes the same12 checks under SOFTWARE and OPENGLES3 with empty stderr. Its callback-scoped device returns a complete versioned PresentationParameters value. The demo changes the presentation interval, proves the fresh value while every other public field stays exact, restores the original and verifies every field before leaving the callback. Device DisplayMode is complete and coherent; SOFTWARE observes800x480 and OPENGLES3/Xvfb observes1280x1024. The device-level native-window route returns NOT_SUPPORTED and zero, consistent with the game window owning that answer elsewhere. These last four routes raise C coverage665->669/3205 and complete `display.h` at22/22. Metadata is492/0, the exact CTest passes and all59 enabled tests in the Xvfb-backed60-test C/API/adapter/device-manager/CLI selection pass. Complete492 then passes505 enabled CTests and both full smoke suites; compared with complete490, both new IDs pass and all40/44 failed assertions remain exact. No pixels, disclosed native handle or visual inspection is claimed. Focused evidence:`checkpoint-492-c-device-presentation-focused-*`; complete evidence:`checkpoint-492-c-device-presentation-*`. |
| C render-target information, binding arrays and events | New `c_api.graphics.render_target_bindings` passes22 identical controls under SOFTWARE and OPENGLES3 with empty stderr. Discard is the only non-preserving usage. Matching16x16 2D/cube targets report complete versioned applied state and real renderer storage. They bind simultaneously as ordered slots0/1, with the cube's NegativeZ face retained; count is two, a one-entry copy reports two without writing, and an exact copy returns both normalized bindings. A NegativeY single-cube bind replaces the MRT array, while an empty array restores the backbuffer and copies as empty. ContentLost registration is a real owned lifetime, stays silent during ordinary binds on both non-device-losing renderers, releases once and then refuses the stale handle. These nine routes raise C coverage669->678/3205 and complete `render_target.h` at12/12. Metadata is493/0, the exact CTest passes and the Xvfb-backed68-test C/API/render-target/device-manager/CLI selection has no failures; its C++ custom-effect MRT scene declares the expected SOFTWARE skip. No actual device-loss event, pixels or visual inspection is claimed. Evidence:`checkpoint-493-c-render-target-bindings-focused-*`. |
| C graphics-device identity and capability profile | New `c_api.graphics.device_capabilities` passes20 controls under both renderers with empty stderr. It copies the exact renderer name with atomic short-buffer refusal; cross-checks all17 individual legacy capability answers against the versioned summary bits; classifies every live detailed feature and all four reserved gaps; obtains all eight known numeric limits while refusing13 retired identities; and verifies supported usages remain subsets of known usages for all27 surface formats. The detailed texture maximum equals the summary, Color has the baseline storage/render-target/color-transfer classification, the complete English report copies atomically, and logical backbuffer information equals applied presentation state. Renderer-specific results are preserved: SOFTWARE has15 supported detailed features, dialect Unknown and a6427-byte report; OPENGLES3 has20, GLSL ES and6423 bytes. These ten routes raise C coverage678->688/3205 and `graphics.h`14->24/27. The Xvfb-backed68-test C/API/device/device-manager/CLI selection passes all67 active tests with fullscreen disabled. No pixels, report-text protocol or visual inspection is claimed. Evidence:`checkpoint-494-c-device-capabilities-focused-*`. |
| C renderer-independent texture format rules | New `c_api.graphics.texture_formats` passes six deterministic controls directly and through both renderer selections with byte-identical output and empty stderr. Across all27 SurfaceFormat identities it verifies the exact compressed-block texel footprint, texel/block byte size, pixel-store alignment capped at eight bytes, accepted divisor widths and adjacent invalid widths. It also distinguishes Color as the only format accepted by the renderer-independent base Texture contract while every other valid identity returns NOT_SUPPORTED, and invalid identities preserve getter output. Five routes raise total C coverage1084->1089/3205 and `texture.h`2->7/21. All32 focused Texture/CLI CTests satisfy their contracts in34.25s, with two declared SOFTWARE capability skips. This is format-policy evidence without resource allocation or backend support claims. Evidence:`checkpoint-522-c-texture-formats-focused-*`. |
| C Texture2D storage, transfer and image I/O | New `c_api.graphics.texture2d_io` passes27 controls directly and under both renderer selections with byte-identical output and empty stderr. A default standalone texture has canonical Color identity and no storage; a PNG-loaded standalone texture has authoritative CPU pixels and no renderer. Exact full RGBA bytes and a typed2x2 rectangle demonstrate count validation, caller-array offsets, atomic short readback and untouched guards. PNG/JPEG sizing and copies prove atomic buffers and exact signatures; a privately saved PNG decodes again at64x64. Inside one device callback, file and authored RGBA constructors produce renderer-backed textures, raw replacement reads back exactly, encoded PNG memory preserves8x4 dimensions and zoom-decodes to2x3, and every device child releases before callback exit. Twelve routes raise total C coverage1089->1101/3205 and `texture.h`7->19/21. Its only remaining routes copy the fixed Texture2D reflection name and remain intentionally trivial/uncredited. All34 focused C Texture/Texture/CLI tests satisfy their contracts in18.83s, with two declared SOFTWARE capability skips. No draw, screenshot or visual inspection is claimed. Evidence:`checkpoint-523-c-texture2d-io-focused-*`. |
| C Texture3D boxes, cube faces and DDS memory | New `c_api.graphics.texture_volume_data` passes19 controls directly and under both renderer selections with byte-identical output and empty stderr. A HiDef device creates a2x2x2 Color volume whose specific/common metadata agree; typed upload/readback uses an offset eight-element window, short readback is atomic, and an exact32-byte upload replaces all voxels. A2x2 cube reports coherent metadata, and every face round-trips its own offset-window payload before an undersized face read preserves all output. A generated176-byte DXT1 cube DDS decodes from memory to a4x4 one-level public Color cube under both selected renderers; documented NOT_SUPPORTED remains accepted for backends without storage. Eight routes raise total C coverage1101->1109/3205 and `texture_volume.h`4->12/16. The four remaining routes are fixed Texture3D/TextureCube type-name pairs already covered by CNA's generic C smoke and remain intentionally trivial/uncredited. All35 focused C Texture/Texture/CLI tests satisfy their contracts in23.95s with two declared SOFTWARE capability skips. Nothing is sampled or drawn and no visual inspection is claimed. Evidence:`checkpoint-524-c-texture-volume-focused-*`. |
| C caller-built SpriteFont state and glyph-table lifetime | New `c_api.graphics.sprite_font_state` passes26 controls directly and through SOFTWARE/OPENGLES3 with byte-identical runner output and empty stderr. A real8x8 atlas and three deliberately asymmetric glyph records establish that creation copies caller storage; character and complete glyph-table copies remain parallel and atomically reject short buffers; version/reserved output is canonical; and exact multiline measurements observe bearings, cropping height, spacing, line spacing and fallback substitution. Valid setters update all three mutable properties, absent fallback and NaN spacing leave the prior values intact, clearing fallback ignores its value argument and restores missing-glyph refusal. Atlas destruction is refused until the font releases its retained reference. Seven routes raise total C coverage1109->1116/3205 and complete `sprite_font.h`2->9/9. All24 focused C SpriteFont, C++ SpriteFont/CNJ and CLI tests pass in12.78s. Nothing is drawn and no screenshot or visual inspection is claimed. Evidence:`checkpoint-525-c-sprite-font-focused-*`. |
| C SpriteBatch position-and-scale arrays | The existing `c_api.graphics.sprite_batch` grows from30to43 controls on both renderers. A two-entry array with an invalid second texture is refused before its valid first scaled command reaches the frame. The accepted array selects separate cyan and magenta texels, retains their source texture through the active deferred interval and releases it after successful End. Exact readback points prove the first command's one-texel origin shifts its12x8 nonuniform result before its named position, while the second starts at its position and spans6x14. SOFTWARE and OPENGLES3 have identical output and empty stderr; all73 focused C/API, SpriteBatch and CLI tests pass. This one route raises C coverage688->689/3205 and `graphics.h`24->25/27. The only two remaining uncredited routes copy a fixed SpriteBatch reflection name, so every route in the header has a semantic disposition. No screenshot or visual inspection is claimed. Evidence:`checkpoint-494-c-sprite-scaled-focused-*`. |
| C generic graphics-resource lifecycle | New `c_api.graphics.resource_lifecycle` passes25 identical controls under both renderers with empty stderr. A real device-owned Texture2D returns the current callback's borrowed device, starts live with an empty name and null tag, and formats as `Texture2D` until a validated non-ASCII UTF-8 name is assigned. Name and string size/copy pairs prove atomic short-buffer refusal, exact bytes and no appended terminator; embedded NUL is refused without mutation and the full64-bit opaque C tag round-trips. Explicit disposal synchronously raises exactly one callback while the resource still reports live; its separate C handle then reports disposed, repeated disposal is silent, registration release is one-shot and typed destruction invalidates the handle. These12 routes raise C coverage689->701/3205 and complete `graphics_resource.h`. All56 focused C/API, GraphicsResource and CLI tests pass. No rendering or visual inspection is claimed. Evidence:`checkpoint-495-c-resource-lifecycle-focused-*`. |
| C graphics-device resource events and tracking | The existing `c_api.graphics.resource_lifecycle` grows25->30 controls while retaining identical output and empty stderr under both renderers. Before texture construction it records the live tracker count and installs both device resource-event registrations. Creation synchronously reports the complete versioned presence payload after tracking has grown by one. Explicit disposal first invokes the resource Disposing callback, then the device callback observes the exact non-ASCII UTF-8 name while the tracker still includes the resource; its native-tag flag is false because the full-width opaque C tag is owned separately by the C handle table. The count then returns to baseline. Both registrations release once, reject stale handles and silence a second create/destroy cycle. Four routes raise C coverage745->749/3205 and `graphics_device.h`29->33/84; all67 focused C/API, C++ graphics-device/resource and CLI tests pass in39.99s, followed by a clean full build. Nothing is drawn and no visual inspection is claimed. Evidence:`checkpoint-498-c-resource-events-focused-*`. |
| C graphics-device texture-slot bindings | New `c_api.graphics.texture_slots` passes16 identical controls under both renderers with empty stderr. Two high pixel slots on its fresh HiDef device begin empty. One owned Color Texture2D binds with exact native occupancy and C-handle identity, the invalid handle clears only the selected slot, and the same texture occupies both slots. Whole-device unbinding clears both, repeated unbinding is an idempotent no-op, and typed destruction of a rebound texture clears the device and invalidates its stale handle. Every returned slot structure has exact size/version and zero reserved bytes. Three routes raise C coverage749->752/3205 and `graphics_device.h`33->36/84; all68 focused C/API, C++ graphics-device/resource and CLI tests pass in40.15s, followed by a clean full build. No vertex-stage format support, drawing or visual inspection is claimed. Evidence:`checkpoint-499-c-texture-slots-focused-*`. |
| C graphics-device vertex/index buffer bindings | New `c_api.graphics.buffer_bindings` passes20 identical controls under both renderers with empty stderr. A fresh device has no bindings. Two real vertex buffers share one declaration and round-trip through the single, offset and ordered multi-stream routes with exact owned handles, offsets and an instance frequency. A short destination returns the required two entries without mutation; a negative second offset refuses the whole array before changing the applied pair. One real index buffer round-trips by handle. Explicit empty bindings clear all state, while destruction of a rebound vertex owner detaches the whole multi-stream set and destruction of the bound index owner clears its slot. The two simple setters already draw the pixel-validated BasicEffect cube; six newly named routes raise C coverage752->758/3205 and `graphics_device.h`36->42/84. All76 focused C/API, C++ graphics device/resource/buffer and CLI tests pass in59.02s, followed by a clean full build. Complete500 then passes all513 enabled CTests and both full smoke suites; the three post-497 IDs pass and every prior status, known issue and failed assertion remains exact. No buffer data, draw or visual inspection is claimed. Focused evidence:`checkpoint-500-c-buffer-bindings-focused-*`; complete evidence:`checkpoint-500-c-buffer-bindings-*`. |
| C graphics-device buffer clears and windowed readback | New `c_api.graphics.device_clears` passes24 identical controls under both renderers with empty stderr. An explicit HiDef Color/Depth24Stencil8 back buffer lets the demo exercise normalized RGBA, Color-plus-depth and option-selected color/depth/stencil clears. Complete RGBA8 readback samples both pairs of corners and the center after every accepted operation. Non-finite channels/depth and an unknown option bit are refused without changing the established color; the documented empty mask succeeds without changing it. A2x2 source window writes four exact pixels at destination offset2 while retaining guards before and after it; a one-pixel-short destination and malformed version preserve every caller byte. Four routes raise C coverage758->762/3205 and `graphics_device.h`42->46/84. The exact demo, four related C++ clear/readback/depth demos and three CLI tests pass8/8. Pixel values were read programmatically; no screenshot or visual inspection is claimed. Evidence:`checkpoint-501-c-device-clears-focused-*`. |
| C caller-owned graphics-device lifecycle | New `c_api.graphics.owned_device` passes20 byte-identical controls under both renderers with empty stderr. Two HiDef devices coexist outside any Game. The first answers ordinary queries, clears/presents, and raises exact synchronous resetting/reset pairs around both reset forms; changed dimensions, interval and explicit adapter read back exactly, while a null parameter set raises no event. Reset/lost registrations release once. Destroying the second leaves the first live; destroying the first raises one exact disposing callback which releases its own registration before the source dies, and all stale handles are refused. Six routes raise C coverage762->768/3205 and `graphics_device.h`46->52/84; eight focused lifecycle/presentation/state/CLI tests pass. No displayed pixels or visual inspection is claimed. Evidence:`checkpoint-502-c-owned-device-focused-*`. |
| C graphics-device profile and unsupported-3D policy | The existing `c_api.graphics.device_states` grows22->25 controls and retains byte-identical output with empty stderr under both renderers. Its live HiDef device begins with strict unsupported-3D handling. The workflow changes the profile to Reach and the policy to warn-and-stub, reads both back exactly, refuses the first undefined identity for each without mutation, then restores and re-reads both borrowed values before continuing the existing state audit. Three routes raise C coverage768->771/3205 and `graphics_device.h`52->55/84; seven focused C/API, profile, capability and CLI regressions pass. Complete502 then passes all515 enabled CTests and both full smoke suites; both post-500 IDs pass and every prior status, known issue and failed assertion remains exact. Nothing is drawn and no visual inspection is claimed. Focused evidence:`checkpoint-502-c-device-policy-focused-*`; complete evidence:`checkpoint-502-c-device-policy-*`. |
| C graphics-device draw routes and direct pipeline controls | New `c_api.graphics.draw_routes` pixel-proves all five primitive topologies; bound, caller-vertex, caller-indexed and repeated-instance draws; explicit current-effect refusal/selection; and direct blend, depth-test and depth-write toggles. OPENGLES3 passes all24 controls. SOFTWARE passes23 and retains one expected ISSUE-066 pixel failure because its direct blend-enable method is empty: repeated half-alpha instances overwrite to `(255,0,0,128)` instead of accumulating near `(192,0,0,159)`. Both batch wrappers accept exactly their declared outcomes, stderr is empty, and all19 focused C/API, C++ primitive/state and CLI tests pass in23.69s. Nine routes raise `graphics_device.h`55->64/84; the BasicEffect vertex-color setter raises `effects.h`8->9/290 and total C coverage771->781/3205. Pixel values were read programmatically; no screenshot or visual inspection is claimed. Evidence:`checkpoint-503-c-draw-routes-focused-*`. |
| C occlusion-query lifecycle and visible-sample semantics | New `c_api.graphics.occlusion_query` passes23 controls under both renderers with empty stderr. A fresh owned HiDef query has a live renderer, reports canonical precision, is incomplete, preserves the caller's output while refusing PixelCount, and refuses End. Its real full-screen red draw proves Begin/nested-Begin, End/repeated-End and observation-before-reuse ordering, then a one-pixel readback both proves rasterization and gives asynchronous backends a synchronization point. Bounded polling completes in the first draw here. SOFTWARE identifies a precise query and returns exactly384000 samples for the800x480 buffer; OPENGLES3 identifies a boolean query and returns one. Destroy succeeds once and stale queries preserve output while returning INVALID_HANDLE. All19 focused query/capability/CLI tests pass in9.96s. Eight routes raise total C coverage781->789/3205 and `graphics_device.h`64->72/84. No screenshot or visual inspection is claimed. Evidence:`checkpoint-504-c-occlusion-query-focused-*`. |
| C device-wide content-loss notification and restoration | The existing `c_api.graphics.render_target_bindings` grows22->25 controls and retains byte-identical output with empty stderr under both renderers. After its existing real2D/cube binding audit restores the backbuffer, an explicit device notification marks both tracked targets lost and synchronously raises the cube's subscribed event once. Rebinding only the2D target clears only its flag; the cube remains lost until its own successful face bind, which clears it without a second event. The delivered registration still releases once. All28 focused C/API, render-target, dynamic-buffer and CLI tests pass in30.37s with one declared SOFTWARE MRT skip. This one route raises total C coverage789->790/3205 and `graphics_device.h`72->73/84. The explicit deterministic route demonstrates CNA-owned loss propagation and recovery; no real platform context loss is claimed. Evidence:`checkpoint-504-c-content-loss-focused-*`. |
| C render-target truth, effect/IBL execution and display color space | The existing `c_api.graphics.device_capabilities` grows20->24 controls. For all27 surface formats the direct render-target verdict agrees with the detailed supported-usage mask. SOFTWARE truthfully reports no shader-source execution and no IBL; OPENGLES3 reports both. All three display-color-space support probes preserve the original space, every set result agrees with support, any successful alternate would be restored, and undefined ordinal3 is refused without changing outputs or state. Both hosts report current sRGB0 and support `1/0/0` for sRGB/scRGB/HDR10. Direct and runner executions pass with empty stderr; all37 focused capability, presentation, render-target, PBR/shader and CLI tests pass in17.95s with11 declared SOFTWARE skips. Six routes raise total C coverage790->796/3205 and `graphics_device.h`73->79/84. ISSUE-067 records that the C header itself provides no type or names for the three accepted color-space ordinals. No rendering or visual inspection is claimed. Evidence:`checkpoint-504-c-device-capabilities-focused-*`. |
| C resource-free renderer reconstruction | The existing `c_api.graphics.owned_device` grows20->23 controls while retaining every prior label. With zero tracked GPU resources, a negative multisample request is refused without presentation or event changes. A request for4 rebuilds the same renderer family, preserves its capability bits, applies4 samples on both tested backends and raises no reset/lost/disposal event; clear/present remain operational. A second reconstruction restores exact single-sample presentation before the existing two-reset lifecycle continues. Both renderers produce byte-identical23-check output with empty stderr. All27 enabled focused lifecycle, multisampling, selection and CLI tests pass in11.94s; one manual fullscreen test remains disabled. One route raises total C coverage796->797/3205 and `graphics_device.h`79->80/84. No displayed-pixel or visual claim is made. Evidence:`checkpoint-504-c-renderer-recreation-focused-*`. |
| Remaining C `graphics_device.h` route decisions | Every one of84 routes has a semantic disposition:80 are demonstrated and four are intentionally uncredited. The fixed GraphicsDevice type-name size/copy pair is generic reflection text and does not justify independent controls. Context recovery requires an actual renderer-context loss and remains manual/platform-specific. The one-way string marker has no public observable result, so a successful setter call would not demonstrate its effect. This audit assigns work by observable behavior and does not count declarations as coverage. |
| Complete C graphics-device checkpoint | Complete504 passes all517 active CTests plus both504-demo smoke suites. Against complete502, every existing status and known-issue value remains exact; SOFTWARE adds only the documented ISSUE-066 assertion and OPENGLES3 retains its complete failure multiset. This closes the executable graphics-device groups at80/84 without converting the four intentional decisions above into count-only coverage. Evidence:`checkpoint-504-c-graphics-device-*`. |
| C effect annotations and owned collections | New `c_api.graphics.effect_annotations` passes21 byte-identical controls with empty stderr under both renderer selections. It copies a non-ASCII UTF-8 name, semantic, cached string and16-float payload before caller buffers are overwritten; reports complete versioned shape/type metadata; proves atomic short and exact unterminated text copies; exposes the raw value through Boolean, integer-bit, float, Vector2/3/4 and Matrix views; and retains a separate signed-integer value. An ordered collection copies both annotations by value, permits source destruction, returns independent owned index/name copies, distinguishes exact case and absence, refuses its upper bound without publishing a handle, and leaves both copies usable after collection destruction. All22 routes gain credit, raising `effects.h`9->31/290 and total C coverage797->819/3205. This is renderer-independent reflection-value behavior, not compiled-effect annotation discovery or shader execution. Evidence:`checkpoint-505-c-effect-annotations-focused-*`. |
| C effect parameter metadata and typed values | New `c_api.graphics.effect_parameter_values` passes30 controls with byte-identical runner output and empty stderr under both renderer selections. It copies versioned shape/type metadata, name and semantic bytes before caller storage changes; proves atomic short and exact unterminated text copies; round-trips all nine tagged scalar and array families; distinguishes ordinary from transposed non-symmetric matrices; and exercises bounded array output. Its copied string value has the same buffer contract. A CPU-only Texture2D proves separate typed2D and base slots retain the exact resource until both are cleared; the intentionally getter-less base slot refuses retrieval. Sixteen parameter routes and one texture constructor raise `effects.h`31->47/290, `texture.h`1->2/21 and total C coverage819->836/3205. This is ownerless parameter storage behavior, not compiled-effect reflection, shader execution or rendering. Evidence:`checkpoint-506-c-effect-parameter-values-focused-*`. |
| C effect parameter collections and nested lifetimes | New `c_api.graphics.effect_parameter_collections` passes19 controls with byte-identical runner output and empty stderr under both renderer selections. Add-create copies metadata and the first mutable element remains stable after32 more insertions. Index, exact-name and semantic lookups share one state; names remain case-sensitive while semantic lookup ignores case and returns the first of duplicate matches. Bounds publish no handle. Separate element, structure-member and annotation views accept children, and those descendants survive destruction of the root collection, every root element handle and finally their nested collection views. All ten remaining parameter/collection routes raise `effects.h`47->57/290 and total C coverage836->846/3205. ISSUE-068 records the C header's inaccurate "exact" semantic wording. Evidence:`checkpoint-507-c-effect-parameter-collections-focused-*`. |
| C effect pass and technique values, collections and lifetimes | New `c_api.graphics.effect_passes_techniques` passes27 controls with byte-identical runner output and empty stderr under both renderer selections. Default, named and reflected techniques distinguish stable nonzero identity from authored runtime index and empty from canonical-P0 pass lists. Passes preserve copied names, reflected indices and the documented ownerless no-op Apply. Separate annotation views retain values after their parent handles release. Standalone pass and technique collections keep first-element views stable across32 further insertions, provide exact case-sensitive index/name views, refuse bounds atomically and retain a nested P0 through root-collection, parent-view and nested-collection destruction. All31 routes raise `effects.h`57->88/290 and total C coverage846->877/3205. Effect-owned technique validation and shader application remain for the core Effect batch. Evidence:`checkpoint-508-c-effect-passes-techniques-focused-*`. |
| C core Effect lifecycle, owned reflection and compiled-format gate | New `c_api.graphics.effect_core` passes27 controls with byte-identical runner output and empty stderr under both renderer selections. A device-owned empty Effect exposes exact device/type/source/runtime facts, its default technique and P0, rejects null and foreign current techniques without mutation, accepts an added owned technique and applies only through the selected graph. Its clone has a fresh canonical graph, disposes independently and leaves the source usable. Parameter, technique and pass views retain the empty native graph after its root handle is destroyed. The compiled constructor atomically distinguishes empty/malformed input, MGFX and a committed genuine5000-byte XNA Effect Framework binary. Both configured renderers report `CompiledEffects=false` because their opt-in build flags are off, so the valid binary is explicitly refused and compiled reflection/execution remains conditional rather than claimed. These18 routes raise `effects.h`88->106/290 and total C coverage877->895/3205. Evidence:`checkpoint-509-c-effect-core-focused-*`. |
| C concrete EffectMaterial and SpriteEffect ownership | New `c_api.graphics.effect_concrete_types` passes22 controls with byte-identical runner output and empty stderr under both renderer selections. EffectMaterial creation preserves its owner and concrete type, rejects its ownership routes on a base Effect, ignores null texture retention, rejects an undefined family and owns two list entries for one CPU Texture2D after the caller destroys its handle. Its inherited canonical clone is an independent base Effect and remains usable after the original source is destroyed. SpriteEffect reports exact stock identity, applies through the internal sprite path, clones as another exact SpriteEffect and leaves that clone live after source disposal. ISSUE-069 records that the generic C clone comment overpromises same-concrete-type behavior despite EffectMaterial's deliberate XNA exception. These four routes raise `effects.h`106->110/290 and total C coverage895->899/3205. Evidence:`checkpoint-510-c-effect-concrete-types-focused-*`. |
| C BasicEffect state, shared interfaces and light ownership | New `c_api.graphics.basic_effect_state` passes40 byte-identical controls with empty stderr under both renderer selections. It checks the coherent XNA defaults, then round-trips distinct world/view/projection matrices, material channels, shader-selection switches, fog and ambient/lighting state through the concrete and shared interfaces. Separate handles observe one stable directional-light member; the default-lighting preset rewrites all three canonical lights; a standalone light has independent defaults and one-shot ownership; and a nested member view remains mutable after its root Effect handle and callback end. A real same-device Texture2D cannot be destroyed while assigned, clears to an empty slot and then destroys successfully. A plain Effect refuses all three interfaces without publishing a light. Forty-two newly named routes raise `effects.h`110->152/290 and total C coverage899->941/3205. Apply is state-only; there is no draw, pixel or visual claim. Evidence:`checkpoint-511-c-basic-effect-state-focused-*`. |
| C AlphaTestEffect and DualTextureEffect state/texture ownership | New `c_api.graphics.unlit_stock_effects` passes33 byte-identical controls with empty stderr under both renderer selections. It establishes both effects' coherent defaults and independent matrix/fog state; round-trips material alpha without clamping, vertex-color selection and AlphaTestEffect's comparison/reference pair; rejects invalid comparison, Boolean and texture-layer identities without mutation; and rejects each concrete API on the other stock type. One Texture2D is retained by AlphaTestEffect and DualTextureEffect layer0 while a second occupies layer1. Type-preserving clones retain all state and exact handles after both sources are destroyed; the first texture releases only after both clone slots clear and the second releases independently. All22 concrete routes raise `effects.h`152->174/290 and total C coverage941->963/3205. Apply is state-only; there is no draw, alpha-test pixel, sampling or visual claim. Evidence:`checkpoint-512-c-unlit-stock-effects-focused-*`. |
| C EnvironmentMapEffect state, required lighting and texture ownership | New `c_api.graphics.environment_map_effect_state` passes30 byte-identical controls with empty stderr under both renderer selections. It establishes material/map defaults and deliberately unclamped alpha, amount and Fresnel values; exact matrix/fog state; mutable ambient color; the permanent lighting invariant; and the canonical three-light preset through stable light views. A real Texture2D and newly demonstrated TextureCube occupy distinct same-device slots, refuse caller destruction while retained and survive source destruction through a type-preserving clone. Clearing each clone slot releases only its corresponding resource. All17 EnvironmentMapEffect routes plus TextureCube create/destroy raise `effects.h`174->191/290, `texture_volume.h`0->2/16 and total C coverage963->982/3205. Apply is state-only; there is no draw, cube sampling, pixel or visual claim. Evidence:`checkpoint-513-c-environment-map-effect-focused-*`. |
| C SkinnedEffect material, palette boundaries and ownership | New `c_api.graphics.skinned_effect_state` passes35 byte-identical controls with empty stderr under both renderer selections. It establishes all material/switch/interface defaults and mutations, required lighting, accepted one/two/four weight modes and invalid-value preservation. The default 72-bone palette is entirely identity; a maximum-sized authored palette is copied on input, copied out through the final slot, preserved after caller overwrite and returned atomically. Empty and 73-bone requests are refused without state/output mutation. An ownerless Texture2D is rejected, while a same-device texture and the complete palette survive source destruction in a type-preserving clone and release independently. All21 SkinnedEffect routes raise `effects.h`191->212/290 and total C coverage982->1003/3205. Apply is state-only; there is no skinned draw, vertex-weight, pixel or visual claim. Evidence:`checkpoint-514-c-skinned-effect-focused-*`. |
| C ColorMatrixEffect finite state, presets and cloning | New `c_api.graphics.color_matrix_effect_state` passes22 controls under both renderer selections with byte-identical runner output and empty stderr. It proves identity/zero defaults, exact complete finite row-major matrix and unclamped RGBA offset round trips, and atomic preservation when NaN or infinity is refused. The grayscale command installs the exact three Rec. 709 RGB rows, preserves alpha and clears the offset; reset restores identity/zero together. A type-preserving clone retains every authored element after source destruction and applies successfully. All7 ColorMatrixEffect routes raise `effects.h`212->219/290 and total C coverage1003->1010/3205. The existing C++ SOFTWARE demo supplies actual transformed-pixel validation; this C workflow makes no draw, pixel or visual claim. Evidence:`checkpoint-515-c-color-matrix-effect-focused-*`. |
| C PbrEffect material, shared interfaces and clone completeness | New `c_api.graphics.pbr_effect_material_state` runs33 byte-identical controls with empty stderr under both renderer selections. Twenty-nine establish exact core/glTF defaults, deliberately unclamped factors, all alpha-mode identities and invalid preservation, canonical Boolean validation, required lighting, shared matrix/fog state and a surviving type-preserving clone. Four retained failures establish CNA-DEMOS-ISSUE-070: clone resets enabled vertex color, blend mode, cutoff0.125 and double-sided state to their defaults, while preserving metallic-roughness/dielectric values and the separate output-encoding switch. Thirty-one routes raise `effects.h`219->250/290 and total C coverage1010->1041/3205. Apply is state-only; existing C++ PBR demos provide renderer-capability-gated draw coverage. Evidence:`checkpoint-516-c-pbr-material-focused-*`. |
| C PBR texture-slot metadata and retained ownership | New `c_api.graphics.pbr_texture_slots` passes34 controls under both renderer selections with byte-identical runner output and empty stderr. It establishes the texture-transform value contract and all seven slots' empty/UV0/identity defaults; mutates each UV selector and transform independently; limits sRGB state to base-color, emissive and specular-color; and proves atomic invalid slot/channel/Boolean/structure refusal. Five core slots retain one real Texture2D and two specular slots retain another through clone/source destruction; each releases only after its final clone slot clears. Ten routes raise `effects.h`250->260/290 and total C coverage1041->1051/3205. CNA-DEMOS-ISSUE-071 records that the public getter comment still calls the seven defined identities five. No sampling, draw, pixel or visual claim is made. Evidence:`checkpoint-517-c-pbr-textures-focused-*`. |
| C SkinnedPbrEffect shared compatibility and palette ownership | New `c_api.graphics.skinned_pbr_effect_state` passes32 controls under both renderer selections with byte-identical runner output and empty stderr. It proves that shared material, base-texture, matrix, fog, lighting and vertex-color routes accept the skinned type; required lighting remains enabled; and one/two/four weights preserve the last valid mode after refusal. All72 default bones are identity, a complete authored palette is copied from input and through its final slot, short output is atomic, and empty/73-bone requests preserve state. A same-device texture plus material and palette survive source destruction through a type-preserving clone, then release independently. Five routes raise `effects.h`260->265/290 and total C coverage1051->1056/3205. Apply is state-only; the existing skinned-PBR model demo provides renderer-gated deformation/pixel coverage. Evidence:`checkpoint-518-c-skinned-pbr-focused-*`. |
| C ShaderEffect validity, diagnostics and matrix state | New `c_api.graphics.shader_effect_state` passes17 controls directly and under both renderer selections with byte-identical runner output and empty stderr. It selects source from the reported dialect, distinguishes handle creation from canonical validity and renderer ownership, requires compilation and apply whenever source execution is advertised, verifies the exact empty diagnostic view after success and atomically refuses two empty sources. World, View and Projection start as independent identities and round-trip distinct complete matrices without transposition. Ten routes raise `effects.h`265->275/290 and total C coverage1056->1066/3205. Existing compiler and pixel demos supply failure-diagnostic and actual execution evidence; this state workflow makes no draw, pixel or visual claim. Evidence:`checkpoint-519-c-shader-state-focused-*`. |
| C ShaderEffect typed uniforms, arrays and block metadata | New `c_api.graphics.shader_effect_uniforms` passes18 controls directly and under both renderer selections with byte-identical runner output and empty stderr. Its dialect-correct shader keeps the six scalar/vector/matrix families and four array families active; the C routes accept distinct typed values, tightly packed three-float vec3 and sixteen-float mat4 elements, and every empty-null form while refusing nonempty null, negative-count and embedded-NUL inputs. A two-member std140 declaration accepts explicit offsets, copies caller storage and clears with zero members; malformed array pairs and a negative block size are refused. Eleven routes raise `effects.h`275->286/290 and total C coverage1066->1077/3205. Existing C++ pixel demos prove rendered uniform behavior; this workflow applies state without drawing. Evidence:`checkpoint-520-c-shader-uniforms-focused-*`. |
| C ShaderEffect texture families, replacement and retained ownership | New `c_api.graphics.shader_effect_textures` passes20 controls directly and under both renderer selections with byte-identical runner output and empty stderr. Real 2D, cube and 1x1x1 volume resources establish all three positive binding families, while every setter refuses a negative unit and the volume setter plus the other families reject a live wrong-kind handle. Successive same-unit bindings release the displaced 2D and cube resources. The final Texture3D survives root Effect destruction through its canonical pass and releases only after all retained collection/technique/pass views do. Three ShaderEffect routes plus Texture3D create/destroy raise `effects.h`286->289/290, `texture_volume.h`2->4/16 and total C coverage1077->1082/3205. No sampling, draw, pixel or visual claim is made. Evidence:`checkpoint-521-c-shader-textures-focused-*`. |
| C ContentManager stock-Effect loading and owned lifetime | The existing `c_api.content.content_manager` now loads a generated authored BasicEffect CNJ and retains all prior texture/font/cache/root/draw behavior. Exact diffuse/emissive colors, specular power, alpha and vertex-color state prove that loading produced the authored concrete effect rather than a default substitute. The handle applies normally and preserves every value after ContentManager.Unload and manager destruction before its independent release. All53 controls pass under both renderer selections with byte-identical output and empty stderr. The last route raises `effects.h`289->290/290 and total C coverage1082->1083/3205. Existing texture/font back-buffer pixels are programmatically checked; no screenshot or visual inspection is claimed. Evidence:`checkpoint-521-c-effect-load-focused-*`. |
| Complete C Effect route audit | Complete521 runs every one of the521 catalog demos under SOFTWARE and OPENGLES3. All17 IDs added since complete504 appear in both reports:16 pass and `c_api.graphics.pbr_effect_material_state` retains expected ISSUE-070 on each renderer, while every earlier status/knownIssue pair is unchanged. The audit now semantically groups all290 `effects.h` routes across annotations, typed values, reflection collections, core ownership, every stock effect, ShaderEffect state/uniforms/textures, actual pixel demos and ContentManager loading. This closes the header's route audit, not the broader XNA member/behavior audit. Evidence:`checkpoint-521-complete-*`. |
| C dynamic index-buffer transfer and event lifetime | New `c_api.graphics.index_buffer` passes18 identical controls under both renderers with empty stderr. A dynamic four-entry16-bit buffer reports its complete applied state, accepts a full upload from a nonzero caller-array window and atomically refuses a short shifted read destination before an exact copy preserves its prefix. A two-entry caller window then patches the buffer at byte offset four; complete read-back proves both earlier indices survive while the selected final entries change. Its owned ContentLost registration stays silent during ordinary transfers on current non-device-losing renderers, releases once and refuses the stale handle. Five routes raise C coverage701->706/3205 and `index_resources.h`3->8/10. The two remaining fixed type-name routes are trivial/uncredited, giving every header route a semantic disposition. All59 focused C/API, C++ IndexBuffer and CLI tests pass in isolated X11. No drawing, actual device loss or visual inspection is claimed. Evidence:`checkpoint-496-c-index-buffer-focused-*`. |
| C graphics-state presets, live round trips and restoration | New `c_api.graphics.device_states` passes19 identical controls under both renderers with empty stderr. It compares every public field from all5 blend,3 depth/stencil,4 rasterizer and7 sampler presets against the canonical value, then saves the live device's blend, depth, rasterizer and pixel/vertex sampler-slot3 states. Distinctive complete values round-trip field for field; setting each sampler stage leaves the corresponding slot in the other stage unchanged. Five independent restore calls run before one complete read-back proves every saved field is restored. Ten routes raise C coverage706->716/3205 and `graphics_state.h`2->12/14. Its two remaining routes are assigned to the existing pixel-validated SpriteBatch workflow. All74 focused C/API, C++ state and CLI tests pass. No drawing or visual inspection is claimed. Evidence:`checkpoint-497-c-device-states-focused-*`. |
| C graphics-device identity, ownership and scalar/geometric state | The existing `c_api.graphics.device_states` grows19->22 controls and retains identical output with empty stderr under both renderers. The callback proves the game device is live, Normal, HiDef and tied to a current adapter; direct disposal is NOT_SUPPORTED and leaves it live. It then saves, applies and exactly reads a distinctive scissor rectangle, blend factor, multisample mask and reference stencil alongside the prior complete object and sampler states, and the final aggregate read proves every borrowed value was restored. Thirteen routes raise C coverage732->745/3205 and `graphics_device.h`16->29/84. All83 focused C/API, C++ graphics-device/state and CLI tests pass in49.86s. Nothing is drawn and no visual inspection is claimed. Evidence:`checkpoint-498-c-device-values-focused-*`. |
| C SpriteBatch explicit states and transformed default effect | The existing `c_api.graphics.sprite_batch` grows from43to61 controls without a new ID. An explicit Opaque/PointClamp/None/CullNone interval writes a half-alpha red source as exact raw RGB128 over the clear colour. A second interval uses `CNA_INVALID_HANDLE` for the default effect and a nonidentity translation; exact read-back proves its opaque green command leaves the untranslated pixel clear and appears sixty pixels right. Both renderers produce identical output with empty stderr; shared texture/batch release remains checked and all76 focused C/API, C++ SpriteBatch and CLI tests pass. These two routes raise coverage716->718/3205 and complete `graphics_state.h` at14/14. No screenshot or visual inspection is claimed. Evidence:`checkpoint-497-c-sprite-states-focused-*`. |
| C Viewport values, projection and live device state | New `c_api.graphics.viewport` passes19 identical controls under both renderers with empty stderr. It distinguishes the all-zero XNA value default from parameterized0..1 depth construction; derives aspect, bounds and desktop safe area; preserves depth across a bounds change; performs identity and perspective project/unproject round trips; and proves exact/atomic canonical text copies. A callback saves the live viewport, applies and reads every field of a distinctive finite value, refuses infinity before mutation, then restores and re-reads the original. Thirteen routes establish `graphics_device.h`16/84 and a newly named matrix identity route raises total coverage718->732/3205. All61 focused C/API, C++ Viewport, split-screen and CLI tests pass. ISSUE-064 records the passing default-value documentation mismatch. Nothing is drawn and no visual inspection is claimed. Evidence:`checkpoint-498-c-viewport-focused-*`. |
| C text and IME event/lifecycle contract | New standalone `c_api.input.text_input` passes20 identical controls on SOFTWARE/OPENGLES3 with empty stderr. Authored raises synchronously deliver committed UTF-16 including a surrogate pair, copy multibyte UTF-8 composition with unvalidated byte offsets, and copy candidate views/orientation/unchecked selection. Selective unsubscribe and reset establish all three process-wide registration lifetimes. The demo captures the real backend token, forces a deterministic unbound activation/query/no-op contract, accepts all nine hint identities, refuses the first undefined one, exercises the restored backend token without assuming keyboard visibility, proves reset clears it and restores it before exit. All17 `input_text.h` routes gain semantic credit, raising total C coverage395->412/3210; no physical typing, platform IME or visible keyboard claim. Direct compilation is diagnostic-free, metadata is479/0 and all61 exact input/C-API/CLI tests pass with six declared skips. The standard/full build remains externally blocked; focused runtime uses the last built CNA libraries. Evidence:`checkpoint-479-c-input-text-focused-*`. |
| C raw-joystick values, disconnected snapshots and hot-plug events | New standalone `c_api.input.joystick` passes23 identical controls on SOFTWARE/OPENGLES3 with empty stderr; this Xvfb environment reports zero raw joysticks. Descriptor defaults/equality observe id, type and UTF-8 name. Capability defaults/equality observe every hardware field plus name/GUID and retain unknown charge as -1. An absent id returns exact disconnected capabilities and zero-byte name/GUID size/copy results without touching guards. Two absent ids yield independent owned snapshots; all four count/copy pairs are empty and preserve destinations, the snapshots compare equal and release independently. Connected/disconnected raises synchronously deliver exact ids; selective unsubscribe, reset and post-reset release have separate checks. Twenty-seven actual routes raise coverage412->439/3210 and establish `input_joystick.h`27/30. The three successful enumerated info/name routes and physical array values remain hardware-manual/uncredited. Direct compilation is diagnostic-free, metadata is480/0 and all62 exact input/C-API/CLI tests pass with six declared skips. A watched standard build sees3898 stable CNA inputs but is externally blocked in stale SDL_GPU source definitions absent from its reduced header; focused runtime uses the last built libraries. Evidence:`checkpoint-480-c-input-joystick-focused-*`. |
| C haptic values and closed-device lifecycle | New standalone `c_api.input.haptics` passes24 identical controls on SOFTWARE/OPENGLES3 with empty stderr; this Xvfb environment reports zero haptic devices. Direction equality observes all components. Effect equality observes unused family fields and caller-owned custom samples. Closed capabilities retain -1 unknown limits and equality observes feature bits/name. An absent id yields an owned closed device with empty name and canonical capabilities. Effect support, rumble, create/update/run/status/stop/destroy, global stop, gain, autocenter, pause and resume all return documented safe negative answers; dispose is idempotent and destroy releases. Mouse and absent-joystick query/open paths are coherent. Thirty-three routes raise coverage439->472/3210 and establish `input_haptics.h`33/36. Three successful enumeration id/name routes plus upload/actuation/physical feedback remain hardware-manual. Direct compilation is diagnostic-free, metadata is481/0 and all63 exact input/C-API/CLI tests pass with six declared skips. The external SDL_GPU mismatch still blocks the standard/full checkpoint; focused runtime uses the last built libraries. Evidence:`checkpoint-481-c-input-haptics-focused-*`. |
| Host-driven frame stepping and extended lifecycle hooks | host_frames checks the copied GameFrameHooks table, one variable update per host call, startup order, BeginDraw suppression, default true/null-time arguments, terminal false without replay, and unload on destruction. It now removes every frame hook for a frame (ordinary Update/Draw continue), installs a copied replacement table with a separate context, verifies no startup replay, then restores the original EndRun hook. Both renderers pass 26 child checks (28 with capture), retaining all twenty-four older assertions. Fully allocated incompatible hook objects declare a short size or unsupported version; both are refused, and the immediately following original frame traces prove the installed table survives each refusal. Exact trace ILBUbUbDeUDUrDsXEZ; inspected last-frame captures are RGBA-identical to prior host images. Desktop evidence does not establish browser/WebAssembly scheduling. error_handling already demonstrates two run_one_frame calls and a failing Update callback; that basic route is not absent. error_handling now verifies retained RunOneFrame failure and destruction cleanup separately. `c_api.game.host_failure` now verifies host-run failure completion in a fresh process: second Update failure returns CALLBACK/false, later host calls do nothing, and destruction unloads once. Contrary to the public promise, C exiting/end_run hooks are suppressed (ISSUE-051); both selections produce ILBUDUZ. Its ten child checks have nine passes and exactly one ISSUE-051 failure on both selections; the supplied diagnostic survives and the destroyed handle is invalid. The independent minimal reproducer remains available. Other failing callback locations remain untested. ABI_VERSIONING.md explicitly permits a larger structure after validating its known prefix. The initial host_frames install now uses an allocated prefix with two nonzero unknown tail fields, then wipes that caller storage. With no previous hooks installed, the original startup trace proves actual installation/copying. Both selections retain all 26 checks (28 with captures), with only the initial setup label changed; ordinary-sized replacement/restoration and incompatible-layout controls remain. Four before/after host captures were actually inspected and are RGBA-identical (800x480); focused stderr is empty. |
| Foreign-thread calls, dispatch and thread-affinity failures | Runtime headers offer explicit opt-in dispatch with game-thread pumping and shutdown semantics. foreign_thread_dispatch retains all eighteen original passing controls: default/restored THREAD rejection, owner-served target-step reads/writes and worker-side native failure details (INVALID_ARGUMENT/ARGUMENT, canonical message, System.ArgumentOutOfRangeException, ParamName=value). C11 atomics publish completion, and bounded cleanup disables dispatch before joining. The existing demo now adds no-pump RunFrameEXT/Destroy probes for the documented owner-only exception. Both selections now produce exactly 38 passing checks and two ISSUE-050 failures (all thirty prior checks retained): these forbidden calls queue and cancellation returns INVALID_STATE/STATE. The owner retains its changed target step and destroys the game successfully. Delayed pre-call workers are reported as inconclusive, never credited as successful affinity rejection. The standalone diagnostic tools/repro_c_api_owner_thread.c remains a minimal independent reproducer. No Game loop, audio initialization, rendering or forbidden queued body is executed; creating the game still initializes its renderer. The same demo now cancels a permitted queued setter before execution, requires INVALID_STATE/STATE, checks the unchanged target step, pumps while disabled and then serves a new worker after re-enabling. That worker observes the unchanged value, detecting replay of the cancelled setter. All ten new controls pass on both selections; delayed pre-call refusal cannot pass the actual-cancellation assertion. Automatic Update service now has its own focused example below. The final owner-destruction extension now passes six additional controls on both selections: a permitted setter waits without pumping, the owner still sees its old value before destruction, destruction releases the worker with INVALID_STATE/STATE, bounded cleanup precedes joining, and a stale-handle query preserves its output. All forty original child labels/order remain after removing these six new checks; the total is 44 passes plus the same two ISSUE-050 failures. Both stderr files are empty and the destruction trace is pending=1/result=3/category=3. No read of already-destroyed game state is claimed. Automatic Draw setter service is now separately isolated below; other driving routes and platforms remain gaps. |
| Automatic Update dispatch and callback-scope lifetime | `c_api.game.automatic_dispatch` starts a host-driven variable-step game before launching an ordinary property worker, suppresses every Draw and never calls the explicit pump. Fifteen child checks pass under SOFTWARE/OPENGLES3 with empty stderr: read/write/read changes 166667 to 333333 ticks, the owner Update observes that value, all Draw callbacks stay suppressed, a separately served device borrow succeeds after startup and its identifier is INVALID_HANDLE on return to the host. The worker never reuses/releases that callback-scoped handle; its value is retained solely for the expiry check. Bounded service and dispatch shutdown precede joining. The same demo now preserves those fifteen checks and adds six passing Draw controls on both renderer selections. A single setter starts inside BeginDraw after Update returns; the owner still sees 333333 and an unfinished worker, then the very first Draw sees 500000 with no intervening Update or explicit pump. Bounded dispatch shutdown precedes joining, and SUCCESS distinguishes service from cancellation. Both stderr files are empty; exact trace pending=1/before=333333/observed=500000/updates_between=0/result=0. Total 21 child checks, parent JSON count 0. No screenshot/visual, latency guarantee, Draw-served device borrow, browser scheduling or arbitrary borrowed-resource claim. The Update phase still proves its separate callback-scoped borrow and suppression controls; alternating frames alone would not prove the isolated Draw step. |
| C storage callbacks, stream ownership and EOF | save_game already observes a selector callback before return and a complete byte-for-byte save round trip; the C ABI intentionally combines CNA's synchronously completed Begin/End pairs into one route, so an asynchronous C example is not warranted. Source review of storage.h, CnaCApiStorage.cpp and generated Storage.inc distinguishes device -> container -> stream release guards, ordinary seeking, partial EOF counts and stale handles. A draft extension keeps the same save format and original assertions, adds refused parent destruction while children are live, seeks to the existing name tail, reads beyond EOF into guarded storage, rewinds and checks closed-handle output preservation. Separate native probes now preserve all23old checks plus8new:30pass and one position-after-EOF failure on both requested renderer environments (SUCCESS/-1instead of24). A direct FileStream probe reports-1instead of4after short/empty reads while data, guards and rewind/reread controls pass, isolating the transitive sharp-runtime defect (ISSUE-056). The extension is now applied: focused467 retains all23old child controls plus the8named additions, with knownIssue056 metadata and a passing CTest expected-failure contract. No Game, renderer execution or visual credit for this storage-only program. The read-only extension now adds seven positive controls while retaining every previous line/status (38children,37pass/one056): Read+Truncate is refused before changing the file; a valid Read-only stream reports read/seek capabilities and no write capability, rejects a one-byte write with NOT_SUPPORTED, keeps position0/length24 and reads every original byte before closing. Both requested selections agree, stderr empty; four actual routes gain metadata. Capability getters serve this permission example rather than independent getter demos. Share modes remain explicitly ignored by the canonical implementation; successful truncation/extension, write-only access, share enforcement and device-change notifications remain separate decisions. |
| C media collection retention, lifecycle, queue views, bounds and events | c_api.media.media_player now passes55identical child checks on both selections with empty stderr. Filtering the two event additions yields every prior53-line label in order. While both process-wide registrations are live, explicit active-song/media-state raises return only after incrementing their corresponding counters exactly once. After both registration handles release successfully, the same raises succeed without invoking either handler. The existing collection Dispose/idempotence/shared-Song and queue copying/view/Clear/bounds/index controls remain intact; ISSUE-059 records the active-output documentation mismatch. This does not call MediaPlayer::Play with a queue-owned Song; ISSUE-058 remains outside smoke. queue_get_count and unsubscribe were already materially exercised and are now credited with both raise routes in metadata. Failed-add, live-view Clear, callback order/reentrancy, foreign-thread delivery and game-destruction cleanup remain separate. Focused and complete event evidence:checkpoint-474-c-media-events-*; complete bounds evidence:checkpoint-474-c-media-bounds-*. |
| C explicit Song duration and disposal | The same C demo now passes six additional controls on both selections while retaining all55old labels in order (61total, empty stderr). A caller-owned Song over the existing two-second rising.wav stores the deliberately distinct explicit duration750ms as exactly7,500,000ticks. The elapsed fallback is false at7,499,999and true at7,500,000; after setting duration to zero, ten elapsed seconds still report false. IsDisposed starts false; Dispose succeeds twice, sets it true and leaves zero Duration readable until the handle is destroyed. The helper Song is never played or queued, so this demonstrates caller metadata and lifecycle rather than probing, acoustic timing or queue propagation. Focused and complete evidence:checkpoint-474-c-song-duration-*. |
| C Song path identity, text and hashing | The same C demo now passes eight additional controls on both selections while retaining all61old labels in order (69total, empty stderr). Three caller-owned Songs use the plain-path constructor: two distinct handles and display names share rising.wav, while an empty-name control uses falling.wav. Handle-text size/copy returns the exact stored rising path; the two rising Songs compare equal despite their names and handles, the falling Song compares unequal, and the equal pair has equal hash codes. The empty display name is stored literally. All three release independently and are never played or queued. A differing-path hash is not required to differ because collisions remain valid. Focused and complete evidence:checkpoint-474-c-song-identity-*. |
| C Song metadata defaults and mutable play count | The existing unplayed, caller-owned plain-path Song now adds two controls on both selections while retaining all69old labels in order (71total, empty stderr). It reports IsProtected=false, IsRated=false, Rating=0, TrackNumber=0 and PlayCount=0, coherently demonstrating the absence of scanned library tags. Setting PlayCount to42 and reading42 back demonstrates the one mutable counter. These six getters/setter routes belong to one state record; trivial type-name formatting does not warrant separate checks. No tagged-library mutation, persistence or playback-count automation is claimed. Focused and complete evidence:checkpoint-474-c-song-metadata-*. |
| C process-wide MediaPlayer settings | After playback is stopped, the same C demo adds four controls on both selections while retaining all71old labels in order (75total, empty stderr). Mute, repeat and shuffle each set true, read true, restore false and read false. Volume accepts -0.25 and reads clamped0, accepts1.25 and reads clamped1, then restores1. These eight C getters/setters directly demonstrate state and documented clamping without claiming audible loudness, shuffled order, repeated playback or persistence beyond the process. Focused and complete evidence:checkpoint-474-c-player-settings-*. |
| C collection start index and previous navigation | The same C demo now passes three additional controls on both selections while retaining all75old labels in order (78total, empty stderr). With the original two-song collection still live and playback stopped, play_songs_from rebuilds the queue and starts Playing at index1; move_previous selects index0; the already demonstrated move_next returns to index1 and Stop restores the exact state expected by later ownership checks. The collection remains caller-owned. No queue-owned Song is passed to Play, preserving the ISSUE-058 boundary, and immediate calls make no event-delivery or audible claim. Focused and complete evidence:checkpoint-474-c-player-transport-*. |
| C visualization value, live data and disabled behavior | The same C demo now passes four additional controls on both selections while retaining all78old labels in order (82total, empty stderr). `cna_visualization_data_init` writes version1 and two zeroed256-float arrays. The process-wide flag sets true and reads true before playback; after one second of the existing live song, both frequency energy and waveform peak are nonzero. After Stop, disabling reads false and `get_visualization_data` replaces deliberately poisoned arrays with zeros. The flag is therefore restored before later lifecycle work. This demonstrates the C data/state path, not audible output, FFT-bin accuracy, exact amplitude, paused capture or renderer pixels. Focused and complete evidence:checkpoint-474-c-player-visualization-*. |
| C caller-owned single-Song Play and queue copy | After the old stopped queue is cleared and its view released, the same C demo creates an independent Song and now passes six additions on both selections while retaining all82old labels in order (88total, empty stderr). `play_song` starts Playing with one active queue entry; the caller handle and published queue copy both retain the name. Destroying the input while playback continues leaves the copy readable; Stop preserves the published copy, which releases independently from its view. No queue-owned Song is passed back to Play, preserving the ASan-proven ISSUE-058 boundary. No audible output or event-timing claim. Focused and complete evidence:checkpoint-474-c-player-single-song-*. |
| C early MediaPlayer resource release and restart | The same C demo now passes one additional control on both selections while retaining all88older labels in order (89total, empty stderr). During live playback, `cna_media_player_program_exit_ext` succeeds; replaying the same independent caller-owned Song immediately returns to Playing with one active queue entry, while the previously published owned copy remains readable. Source confirms ProgramExit destroys initialized native media resources and clears the initialization flag; the subsequent Play initializes again. No queue-owned Song reaches Play, and no audible continuity, event timing or state change caused by ProgramExit is claimed. Focused and complete evidence:checkpoint-474-c-player-program-exit-*. |
| C desktop MediaSource enumeration | The same C demo now passes three additional controls on both selections while retaining all89older labels in order (92total, empty stderr). The C snapshot reports exactly one source, with `LOCAL_DEVICE` type and exact `Local Device` display bytes. The index equal to Count returns INVALID_ARGUMENT before changing the preseeded type, byte count, copied count or destination. CNA's desktop implementation synthesizes this one entry; this is not hardware discovery, hot-plug evidence or a Windows Media Connect claim. Four actual source count/type/name routes gain metadata credit. The eight remaining `media.h` routes are generated size/copy pairs for fixed reflection type names on VisualizationData, MediaSource, Song and SongCollection; CNA's pure-C smoke already verifies their generic text-buffer contract, and independent demo controls would be trivial. Focused and complete evidence:checkpoint-474-c-media-sources-*. The accepted full runtime was built and focused-tested at clean CNA aa3f4e0; its post-build capture observed concurrent CMake-only CNA edits and final observation was clean74194f4. All37selected binaries stayed hash-identical, so this is immutable binary/runtime evidence rather than an atomic external-source snapshot. An earlier drifted run is archived as refused evidence. |
| C value transforms versus handles | vectors_and_matrices already demonstrates plain C vectors/matrices, row-vector scale/rotation/translation and inversion. It now verifies TransformNormal applies the supplied linear part without translation or normalization. This does not claim automatic inverse-transpose correction under nonuniform scaling. Successful scalar output aliasing in Normalize does not establish all routes' aliasing contracts. |
| C vector array source/destination ranges | The same example now transforms a source subrange into a different destination subrange, checks both positions and untouched sentinels, and refuses independently invalid source/destination ranges before any output byte changes (18 child checks overall on both renderer selections). Extra allocated guard elements lie beyond the declared counts, making these negative checks safe even if the routine wrongly accepted that exact range. Empty/null ranges, overlapping storage and other vector/quaternion array families remain uncredited. Current implementation iterates forward; the header does not promise overlapping-range safety. |

| C sensor reading and event-argument values | New deterministic standalone `c_api.devices.sensor_values` passes34 identical controls under SOFTWARE/OPENGLES3 with empty stderr. It constructs accelerometer, gyroscope, attitude, compass and fused-motion readings, verifies versioned defaults and authored fields, and demonstrates that equality/hashing observe timestamps and nested orientation fields omitted from canonical text. Text checks distinguish each documented subset, one short buffer remains untouched, and every exact type name is copied. The legacy accelerometer event payload preserves its component-first constructor and timestamp-aware equality; calibration's empty payload is represented by its type name. All50 pure value/event-description routes establish `sensors.h`50/144 and current-inventory C coverage472->522/3205. The denominator fell by five solely because concurrent sibling-CNA graphics work removed declarations. Direct compilation is clean, metadata is482/0 and all64 exact input/device/C-API/CLI tests pass with six declared skips. No game, live acquisition or physical sensor is claimed. The standard/full checkpoint remains externally blocked; focused runtime uses the last built libraries. Evidence:`checkpoint-482-c-sensor-values-focused-*`. |
| C sensor acquisition, synthetic readings and event lifetime | New `c_api.devices.sensor_lifecycle` reports real host availability and uses documented test switches/backends to reach supported paths deterministically. Accelerometer platform units convert to g and deliver CurrentValueChanged before legacy ReadingChanged; selective unsubscribe preserves the first. Gyroscope retains angular rate. Compass/Motion publish authored nested readings and context-only calibration. All four families cover creation/state, validity/current data, interval, Start/Stop, registration release, repeated-Dispose refusal and handle expiry; backend replacement is refused while started. Both renderers agree on40 controls with empty stderr:38 pass, while the two disposed Accelerometer/Gyroscope CurrentValue checks fail under ISSUE-062 because the shared getter omits a disposal guard. Sixty-six routes raise coverage522->588/3205 and `sensors.h`50->116/144. Metadata is483/0 and all65 exact input/device/C-API/CLI CTests pass with six skips and one known-issue expected failure. No physical measurement or callback-thread behavior is claimed. Evidence:`checkpoint-483-c-sensor-lifecycle-focused-*`. |
| Remaining C `sensors.h` route decisions | Every one of144 routes now has a semantic disposition. The two demos demonstrate116. Eight device-handle type-name routes are four generic size/copy pairs returning fixed canonical names regardless of sensor state; the value demo already establishes sensor type-name buffer semantics, so these remain trivial/uncredited. The other twenty are explicitly test-only Accelerometer/Gyroscope internals: subsystem-hold and connected-id probes, started-instance register/unregister/explicit dispatch, event-watch failure injection, swallowed-exception count/message and a disposal cleanup hook. CNA's pure-C tests use them to verify its dispatcher implementation; reproducing internal test plumbing is not a coherent application concept and does not justify demo credit. Physical sensor acquisition remains a manual platform path, while the supported observable behavior is demonstrated through the public synthetic backends. |
| C read-only host and display information | New `c_api.devices.host_info` passes12 controls under both renderer selections with empty stderr. It identifies the environment and device extension, reads a coherent power state/percentage/remaining-time snapshot, requires positive logical CPU and RAM capacity, copies every preferred language/country string with exact UTF-8 lengths and refuses all four index-at-count routes before writing. Display scale and safe area accept their documented nonnegative windowless/windowed forms: SOFTWARE reports zero scale/area while OPENGLES3 reports scale1 and800x480. Fourteen routes establish `devices.h`14/62 and raise C coverage588->602/3205. Metadata is484/0 and all66 exact input/device/C-API/CLI CTests pass in182.31s with six skips and one known-issue expected failure. Values describe this host and no portability, power transition, locale mutation, rendering or visual inspection is claimed. Evidence:`checkpoint-484-c-host-info-focused-*`. |
| C vibration request delivery and platform restoration | New `c_api.devices.vibration` reads the real capability and exact device-name bytes without actuating it, then passes18 identical controls under both renderers with empty stderr using CNA's public in-memory backend. It proves versioned zero log state; exact and short-buffer-safe names; full-intensity Start; inclusive zero/five-second bounds and adjacent refusals before dispatch; scalar and independent dual-motor clamping including NaN-to-zero; exact motor separation; idle Stop; atomic backend replacement/log reset; and final platform restoration. All nine vibration routes raise C coverage602->611/3205 and `devices.h`14->23/62. Metadata is485/0 and all67 exact input/device/C-API/CLI CTests pass in115.45s with six skips and one known-issue expected failure. No physical actuation, haptic sensation, request timing or platform motor success is claimed. Evidence:`checkpoint-485-c-vibration-focused-*`. |
| C message-box and file-dialog contracts | New `c_api.devices.dialogs` reads both real availability flags without opening UI, then passes20 identical controls under both renderers with empty stderr using CNA's immediate in-memory services. Message boxes cover versioned log reset, simple severity, button count/chosen index, close-without-choice, pre-dispatch refusal and restoration. File dialogs cover a versioned filter, two-path open, one-path save/folder answers, synchronous borrowed views copied inside the callback, canonical empty/null cancellation, invalid callbacks/filter arrays/version and restoration. All ten routes raise C coverage611->621/3205 and `devices.h`23->33/62. Metadata is486/0 and all68 exact input/device/C-API/CLI CTests pass in117.99s with six skips and one known-issue expected failure. No real window, manual response, external filesystem access or callback-thread behavior is claimed. Evidence:`checkpoint-486-c-dialogs-focused-*`. |
| C in-memory system-tray model | New `c_api.devices.system_tray` reads real tray availability without creating a host icon, then passes16 identical controls under both renderers with empty stderr against a per-handle in-memory tray. It demonstrates owned creation/destruction, tooltip and contiguous menu entries, checked/enabled state, label mutation, the canonical ignored upper-bound mutators/false reads, exact callback context selection, silent no-handler activation, invalid click bounds and stale-handle refusal. Eleven routes raise C coverage621->632/3205 and `devices.h`33->44/62. Metadata is487/0 and all69 exact input/device/C-API/CLI CTests pass in198.53s with six skips and one known-issue expected failure. The twelfth tray route, real `cna_system_tray_create`, remains manual because successful execution creates external host UI; no icon appearance, native click delivery or callback thread is claimed. Evidence:`checkpoint-487-c-system-tray-focused-*`. |
| C camera enumeration and synthetic frame ownership | New `c_api.devices.camera_frames` enumerates the host descriptors/names and upper bound without opening hardware, then passes23 identical controls under both renderers with empty stderr. A synthetic camera round-trips Opening/Denied/Lost, refuses unreachable or undefined states, validates frame inputs, publishes an exact2x2 RGBA frame, copies every channel into a caller-owned texture, returns false for mismatched/no-frame acquisition, clears back to Opening and leaves the acquired texture readable after camera destruction. Fourteen routes raise C coverage632->646/3205 and `devices.h`44->58/62. Metadata is488/0 and all70 exact input/device/C-API/CLI CTests pass in221.19s with six skips and one known-issue expected failure. ISSUE-063 records the passing header mismatch that still says create/clear produce Closed even though implementation, CNA smoke and runtime use Opening and refuse Closed. Real camera creation remains manual; no permission prompt, capture, preview, pixel display or visual inspection is claimed. Evidence:`checkpoint-488-c-camera-frames-focused-*`. |

| Staged C vertex values and resource data | New `c_api.graphics.vertex_values` and `c_api.graphics.vertex_buffer_data` keep CNA's public element records, declaration arrays, data windows, binding descriptor and ContentLost callback visible. Their strict C99 builds and direct SOFTWARE/OPENGLES3 runs pass byte-identically with empty stderr:7 and36 controls respectively. The second workflow covers all15 meaningful previously uncredited `vertex_resources.h` routes; four fixed reflection-name routes remain trivial/uncredited. Evidence:`checkpoint-526-c-vertex-values-focused-*-direct.*`, `checkpoint-527-c-vertex-buffer-data-focused-*-direct.*`. Project-built runner and generated coverage evidence await a coherent sibling CNA checkout. |
| Staged C sound sources and instance state | New `c_api.audio.sound_effect_sources` passes22 direct controls for exact sample conversion, looped PCM ownership, encoded-memory/private-file creation, name buffers, disposal, master volume restoration and one-shot Play. New `c_api.audio.sound_effect_instance_state` passes19 controls in five repeated SOFTWARE runs and OPENGLES3 for defaults, mixing properties, transport, latched loop refusal, natural completion, replay and Stop. Output is byte-identical and stderr empty. ISSUE-072 records the header's pitch-clamping description; runtime correctly rejects out-of-range pitch. Evidence:`checkpoint-528-c-sound-effect-sources-focused-*-direct.*`, `checkpoint-529-c-sound-instance-state-focused-*-direct.*`. Project-built runner and generated coverage evidence await a coherent sibling CNA checkout. |
| Staged C spatial audio, streaming and microphone state | New `c_api.audio.spatial_state` preserves one visible ISSUE-073 failure: a refused live-3D direct-pan setter changes the public pan snapshot; both direct selections otherwise pass21/22 with identical output and empty stderr. `c_api.audio.dynamic_streaming` passes21/21 for sample conversion, copied/aligned PCM, float submission, live format refusal, queue/update, three BufferNeeded requests and unsubscribe. `c_api.audio.microphone_availability` passes20/20 against SDL dummy's real capture device for enumeration, exact/short names, duration, live start/data/stop, conversion, BufferReady, pump, unsubscribe and upper-bound refusals. Current-source metadata covers66/74 `audio.h` routes; the remaining eight are four fixed reflection-name pairs. Evidence:`checkpoint-530-c-spatial-state-focused-*-direct.*`, `checkpoint-532-c-dynamic-streaming-focused-*-direct.*`, `checkpoint-533-c-microphone-availability-focused-*-direct.*`. No audible quality, physical microphone or callback-latency claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C MediaQueue reflection buffer contract | The existing `c_api.media.media_player` adds exact size, zero-capacity sizing, atomic short refusal and exact unterminated copy controls for the last two `media_player.h` routes. Both direct selections pass98 controls with empty stderr and identical check outcomes; only the already printed live playback position differs. Current-source metadata is41/41 for the header. Evidence:`checkpoint-531-c-media-queue-type-name-focused-*-direct.*`. Project-built runner and generated coverage evidence await a coherent sibling CNA checkout. |
| C packed-vector formats and binary16 | New renderer-independent `c_api.graphics.packed_vectors` passes nine byte-identical controls with empty stderr under both selected environments. All17 formats produce exact bits and canonical values; equality observes selected width; invalid high bits and consumed nonfinite integer components fail atomically; unused components are ignored; binary16 covers exact conversion, reduced precision, NaN, signed infinities and negative zero. All7 `packed_vectors.h` routes are credited. Evidence:`checkpoint-534-c-packed-vectors-focused-*-direct.*`. No graphics device, vertex fetch, endianness portability or image-quality claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C color construction, representations and arithmetic | New renderer-independent `c_api.math.color_values` passes15 byte-identical controls with empty stderr. All eight constructor routes establish exact alpha defaults, clamping and XNA byte rounding. Packed/vector forms, equality/hash, selected named constants, exact debug/canonical text and atomic short writes are observable. Lerp, scalar multiplication, float/integer premultiplication and native vector packing cover truncation, clamping, NaN and infinity behavior. All24 `color.h` routes are credited. Evidence:`checkpoint-537-c-color-values-focused-*-direct.*`. No game, graphics device, colour-space, blending or display-quality claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C quaternion values, rotations and interpolation | New renderer-independent `c_api.math.quaternion_values` passes15 byte-identical controls with empty stderr. Construction, identity versus the zero value, equality/hash, length, returned/in-place conjugation and normalization, zero normalization, exact/atomic text and component arithmetic are explicit. Axis-angle, roll and matrix construction agree; multiplication and XNA concatenation show opposite operand order; inverse/division, dot and distinct quarter-step Lerp/Slerp paths complete all28 `quaternion.h` routes. Evidence:`checkpoint-538-c-quaternion-values-focused-*-direct.*`. No game, graphics device, transformed geometry or renderer claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C scalar math helpers | New renderer-independent `c_api.math.scalar_helpers` passes11 byte-identical controls with empty stderr. It demonstrates all15 scalar routes in `math.h`: exact constants, affine/curve interpolation, SmoothStep clamping, float/integer bounds, NaN propagation, distance/min/max, angle conversions and wrapping, the strict framework-epsilon boundary, and bounded MSAA power selection with atomic negative refusal. Evidence:`checkpoint-539-c-scalar-helpers-focused-*-direct.*`. The first draft's overly tight degree round-trip tolerance was a demo defect and was corrected to established single-precision tolerance. No graphics device or actual MSAA support is claimed; project-built evidence awaits a coherent sibling CNA checkout. |
| C integer Point values and arithmetic | New renderer-independent `c_api.math.point_values` passes seven byte-identical controls with empty stderr. Default/Zero and authored construction, equality/hash, exact and atomic text, ordinary component arithmetic, defined two's-complement add/subtract/multiply wraparound, and atomic zero-divisor/signed-overflow division refusal demonstrate all12 Point routes in `math.h`. Evidence:`checkpoint-540-c-point-values-focused-*-direct.*`. No floating-vector or rectangle behavior is implied; project-built evidence awaits a coherent sibling CNA checkout. |
| C integer Rectangle values and geometry | New renderer-independent `c_api.math.rectangle_values` passes12 byte-identical controls with empty stderr. Empty/authored construction, edges/location/center and wraparound; half-open coordinate/Point/rectangle containment; sequential location/offset/inflate mutation; equality/hash; exact/atomic text; strict positive-area overlap; and exact intersection/union demonstrate all25 Rectangle routes. Together the scalar, Point and Rectangle workflows close `math.h` at52/52. Evidence:`checkpoint-541-c-rectangle-values-focused-*-direct.*`. No clipping, rasterization or pixel-coverage claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C Vector2 values, interpolation and transforms | New renderer-independent `c_api.math.vector2_values` passes15 byte-identical controls with empty stderr. Constructors/constants, equality/hash, length and returned/in-place normalization, exact/atomic text, component/scalar arithmetic with IEEE zero division, metrics, interpolation, component bounds, reflection, distinct matrix point/normal and quaternion transforms, all three bounded array routes, and atomic invalid-window refusal demonstrate all41 Vector2 routes in `vectors.h`. Evidence:`checkpoint-542-c-vector2-values-focused-*-direct.*`. No rendered geometry or SIMD-performance claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C Vector3 values, directions and transforms | New renderer-independent `c_api.math.vector3_values` passes17 byte-identical controls with empty stderr. All50 Vector3 routes are explicit: constructors, basis and six named XNA directions with negative-Z Forward, equality/hash, length/normalization including zero, exact/atomic text, component/scalar arithmetic, dot/cross/distance, interpolation/bounds/reflection, distinct 3D point/normal/quaternion transforms, all three bounded array forms and atomic invalid-window refusal. Evidence:`checkpoint-543-c-vector3-values-focused-*-direct.*`. No rendered geometry or SIMD-performance claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C Vector4 values and homogeneous transforms | New renderer-independent `c_api.math.vector4_values` passes17 byte-identical controls with empty stderr. All46 Vector4 routes are explicit: constructor extensions and four unit axes, equality/hash, length/normalization including zero, exact/atomic text, component/scalar arithmetic, metrics, interpolation/bounds, Vector2/3 inputs with implicit W one, authored Vector4 W scaling matrix translation, quaternion W preservation, both bounded array forms and atomic invalid-window refusal. Together checkpoints542-544 close `vectors.h` at137/137. Evidence:`checkpoint-544-c-vector4-values-focused-*-direct.*`. No projection, clipping, rendered geometry or SIMD-performance claim is made; project-built evidence awaits a coherent sibling CNA checkout. |
| C Matrix values, properties and operations | New renderer-independent `c_api.math.matrix_values` passes18 byte-identical controls with empty stderr. It distinguishes zero initialization from Identity, maps all16 public fields, establishes the signed directional rows and translation row, and covers determinant, equality/hash, successful and singular decomposition, exact/atomic text, component arithmetic, row-vector composition order, transpose, inverse and quaternion transform. All35 non-factory `matrix.h` routes are explicit. Evidence:`checkpoint-545-c-matrix-values-focused-*-direct.*`. No game, graphics device or rendered geometry is claimed; project-built evidence remains pending. |
| C Matrix construction families | New renderer-independent `c_api.math.matrix_factories` passes13 byte-identical controls with empty stderr. All22 `cna_matrix_create_*` routes are explicit: ordinary and coincident/fallback billboards, constrained orientation, four equivalent Z-rotation paths plus direct X/Y rotation, paired world/look-at frames, centered/off-center orthographic and perspective projections, atomic invalid projection ranges, three scale forms, offset-plane shadow/reflection and two translation forms. Together checkpoints545-546 close `matrix.h` at57/57. Evidence:`checkpoint-546-c-matrix-factories-focused-*-direct.*`. No clipping, device projection, rendering or visual result is claimed; project-built evidence remains pending. |
| C Plane construction, classification and transforms | New renderer-independent `c_api.math.plane_values` passes nine byte-identical controls with empty stderr. All20 Plane routes are explicit: five construction forms, three distinct dot products, equality/hash, returned and in-place normalization, box/sphere/frustum classification, exact/atomic text, matrix translation and quaternion rotation. Evidence:`checkpoint-547-c-plane-values-focused-*-direct.*`. The frustum is a caller-authored analytic value; no game, device, culling pipeline or rendered geometry is claimed, and project-built evidence remains pending. |
| C Ray values and analytic intersections | New renderer-independent `c_api.math.ray_values` passes seven byte-identical controls with empty stderr. All11 Ray routes are explicit: zero/authored construction, identity/hash, equal nearest distances against a unit box and sphere, forward-hit/away-miss plane semantics, inside/miss frustum semantics, and exact/atomic legacy double-brace text. Evidence:`checkpoint-548-c-ray-values-focused-*-direct.*`. No physics engine, acceleration structure or rendered geometry is claimed; project-built evidence remains pending. |
| C BoundingBox values, containment and intersections | New renderer-independent `c_api.math.bounding_box_values` passes11 byte-identical controls with empty stderr. All20 BoundingBox routes are explicit: zero/min-max construction, all three containment results across four operand kinds, atomic ordered-corner copying, hit/miss ray distance, frustum/sphere/box/plane intersections, identity/hash, exact/atomic legacy text, point/sphere/merge factories and atomic empty-point refusal. Evidence:`checkpoint-549-c-bounding-box-values-focused-*-direct.*`. No collision response, broad phase or rendering is claimed; project-built evidence remains pending. |
| C BoundingSphere values, containment and intersections | New renderer-independent `c_api.math.bounding_sphere_values` passes12 byte-identical controls with empty stderr. All21 BoundingSphere routes are explicit: construction, anisotropic transform with largest-axis radius, all containment classes, identity/hash, enclosing box/frustum/point/merge factories, atomic empty-point refusal, box/frustum/tangent-sphere/ray/plane intersections and exact/atomic text. The first draft incorrectly assumed the documented enclosing frustum sphere was the exact centered minimum; CNA's Ritter-style result encloses all eight corners and the demo now checks that promised behavior. Evidence:`checkpoint-550-c-bounding-sphere-values-focused-*-direct.*`. No collision response, broad phase or rendering is claimed; project-built evidence remains pending. |
| C BoundingFrustum values and analytic culling | New renderer-independent `c_api.math.bounding_frustum_values` passes13 byte-identical controls with empty stderr. All22 BoundingFrustum routes are explicit: retained matrix, six exact identity clipping planes, all containment classes, atomic XNA-ordered corners, four volume/plane intersections, ray inside/miss behavior and the documented atomic `NOT_SUPPORTED` boundary-origin case, identity/hash and exact/atomic six-plane text. Checkpoints547-551 close `geometry.h` at94/94. Evidence:`checkpoint-551-c-bounding-frustum-values-focused-*-direct.*`. No graphics device, rendered culling or visibility-performance claim is made; project-built evidence remains pending. |
| C CurveKey fixed values and ordering | New renderer-independent `c_api.math.curve_key_values` passes nine byte-identical controls with empty stderr. All17 key routes are explicit: the fixed five-field layout through a C99-compatible alignment probe, constructor defaults/full values, every getter and mutable property, atomic invalid-continuity refusal, clone independence, position-only comparison and all-field equality/hash behavior. Evidence:`checkpoint-552-c-curve-key-values-focused-*-direct.*`. The first draft's C11 `_Alignof` was a demo portability defect and was replaced; project-built evidence remains pending. |
| C CurveKeyCollection ordering and ownership | New renderer-independent `c_api.math.curve_key_collection` passes11 byte-identical controls with empty stderr. All14 collection routes are explicit: owned empty/mutable creation, sorted insertion and replacement, full-key membership/indexing, atomic short and exact offset copies, invalid-key preservation, independent deep clone, clear, both removal forms, invalid-index preservation and stale-handle refusal. Checkpoints552-553 establish31/45 `curve.h` routes. Evidence:`checkpoint-553-c-curve-key-collection-focused-*-direct.*`. Curve-owned borrowed views and evaluation remain separate; project-built evidence remains pending. |
| C Curve evaluation, tangent modes and retained views | New renderer-independent `c_api.math.curve_evaluation` passes15 byte-identical controls with empty stderr. All14 curve routes are explicit: empty/single/multi-key constant state, the retained mutable key view, interpolation, four whole/per-key tangent forms, every loop mode and readback, atomic invalid modes, independent cloning and view retention after curve-handle destruction. Equal endpoint slopes cover ordinary Linear extrapolation; distinct endpoint slopes and Step semantics remain visible in the existing ISSUE-036 and ISSUE-002 C++ demos rather than becoming duplicate known-failure checks. Checkpoints552-554 close `curve.h` at45/45. Evidence:`checkpoint-554-c-curve-evaluation-focused-*-direct.*`. Project-built evidence remains pending. |
| C logger severity, filtering and sink ownership | New renderer-independent `c_api.core.logger` passes10 byte-identical controls with empty stderr. All19 logger routes are explicit: fixed severity/category identities, exact formatted borrowed callback text, seven severity helpers, seven conditional helpers, the general condition/category route, minimum filtering, malformed input refusal and restoration of the process-global sink and level. This establishes `core_ext.h` at19/57 without claiming hidden scaffold calls as evidence. Evidence:`checkpoint-555-c-core-logger-focused-*-direct.*`. Callback concurrency and default-stderr bytes remain separate; project-built evidence remains pending. |
| C target platform and assembly title | New renderer-independent `c_api.platform.host_identity` passes10 byte-identical controls with empty stderr. Eight stable `core_ext.h` routes correlate target, Apple/mobile and desktop identities; exactly copy the unterminated platform name; atomically reject short destinations; and own, validate and restore the process assembly title. Embedded NUL returns the shared text contract's `ENCODING` and preserves state, while the route-specific header lists only `INVALID_ARGUMENT`; passing documentation ISSUE-074 records the mismatch. Stable coverage reaches27/57; graphics backend/renderer-selection routes remain deferred during external retirement work. Evidence:`checkpoint-556-c-platform-host-identity-focused-*-direct.*`. Project-built evidence remains pending. |
| C TitleLocation and whole-file TitleContainer reading | New `c_api.game.title_content` passes11 byte-identical controls with empty stderr under both Xvfb selections. Three newly credited routes move the process root from the executable directory to deployed assets, size and atomically read the complete256-byte checker PNG, distinguish missing-file IO and restore the original root; the existing path-copy route proves exact/atomic unterminated text. The first sandboxed attempt lacked a usable X display and is not runtime evidence. Evidence:`checkpoint-557-c-title-content-focused-*-direct.*`. No incremental stream or arbitrary-file claim is made; project-built evidence remains pending. |
| C game runtime controls, explicit ticks and event registrations | New `c_api.game.runtime_controls` passes14 byte-identical controls with empty stderr under both Xvfb selections. All18 remaining `runtime.h` routes are explicit: verified window-title mutation, mouse/fixed-step/sleep/run flags, target FPS/millisecond views, reset and framework pump without a frame, one-draw suppression across two Ticks, and attach/remove/destroy behavior for owned Disposed registrations. The first draft confused owner destruction with release of the detached registration handle; correcting that was a demo ownership fix. Checkpoints557-558 close `runtime.h` at45/45 and the title getter/copy pair establishes `runtime_window.h`2/20. Evidence:`checkpoint-558-c-runtime-controls-focused-*-direct.*`. Wall-clock pacing, focus events, pixels and visual inspection remain unclaimed; project-built evidence remains pending. |
| C GameWindow state, events and native descriptor | New `c_api.game.window_state` passes19 byte-identical controls with empty stderr under both Xvfb selections. All18 remaining window routes are explicit: restored resizing/border policies, bounds/orientation, exact/atomic screen and reflection names, minimize/restore requests with documented platform refusal, a bracketed client resize with synchronous event ownership, and legacy/versioned native identities whose borrowed system-specific fields are classified without dereference. Together with checkpoint558's title pair this closes `runtime_window.h` at20/20. Evidence:`checkpoint-559-c-window-state-focused-*-direct.*`. Appearance, manual window interaction and pointer lifetime beyond game destruction remain unclaimed; project-built evidence remains pending. |
| C component members, lifecycle and event ownership | New `c_api.game.component_members` passes23 byte-identical controls with empty stderr under both Xvfb selections. Fourteen newly credited routes cover owner identity, enabled/visible and update/draw ordering defaults and readback, update-order comparison direction, explicit initialize/update/draw behavior independent of scheduling flags, idempotent disposal with drawable unload, and one-shot event detachment. The first draft confused DrawOrder with the UpdateOrder used by comparison; correcting the expected +/-10 to +/-4 was a demo defect. The existing components workflow also now credits its meaningful drawable classification and callback-scoped device borrow. Evidence:`checkpoint-560-c-component-members-focused-*-direct.*`. No frame scheduling, rendered pixels or borrowed-device lifetime beyond the callback is newly claimed; project-built evidence remains pending. |
| C component collection and named runtime services | New `c_api.game.component_collection` passes19 byte-identical controls with empty stderr under both Xvfb selections. Eight newly credited routes prove indexed insertion/removal and bounds, membership/index/lookup agreement, canonical minus-one absence, component reuse after removal, one removed event per cleared item, named service presence and idempotent removal. `runtime_components.h` consequently has a disposition for every route at35/39 demonstrated. Its four remaining routes are ordinary size/copy helpers for fixed component and collection reflection names; CNA's pure-C smoke already asserts the exact strings and generic buffer contract, so they remain intentionally trivial and uncredited. Evidence:`checkpoint-561-c-component-collection-focused-*-direct.*`. The demo removes a service only at the end because the C surface deliberately has no registration counterpart; project-built evidence remains pending. |
| C graphics-manager information and preferences | New `c_api.game.graphics_preferences` passes21 byte-identical controls with empty stderr under both Xvfb selections. Twenty-four newly credited routes cover validated deep value copying of nested device information, one-manager ownership, a callback-scoped device borrow and all ten manager preference pairs. The controls distinguish recorded preferences from adapter negotiation, including an accepted negative width, a combined orientation bit set and atomic invalid identities. Evidence:`checkpoint-562-c-graphics-preferences-focused-*-direct.*`. No preference application, selected adapter or visual result is claimed here; project-built evidence remains pending. |
| C graphics-manager events, mutable preparation and lifecycle | New `c_api.game.graphics_manager_lifecycle` passes24 byte-identical controls with empty stderr under both Xvfb selections. Nine newly credited routes attach every data-free event plus both views of PreparingDeviceSettings, prove that a valid mutable width reaches the active device and an invalid versioned mutation is ignored, then exercise apply/recreate, a draw bracket, idempotent disposal/service removal and one-shot destruction. The first draft incorrectly expected an event mutation to overwrite the stored manager preference; correcting that was a demo defect. `runtime_graphics_manager.h` now has a disposition for every route at33/38 demonstrated. Four fixed reflection-name helpers are trivial/uncredited. Full-screen toggling remains platform/manual: Xvfb returned success but SDL asynchronously timed out and reverted, so the attempted run is not deterministic evidence and does not establish a CNA issue. Evidence:`checkpoint-563-c-graphics-manager-lifecycle-focused-*-direct.*`. No mode-switch, presentation timing or visual claim is made; project-built evidence remains pending. |
| C content paths, manifest and reader usage | New `c_api.content.manager_inspection` passes18 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. Seventeen newly credited routes cover exact/atomic resolved paths, normalized cache keys, idempotent built-in registration, service/device association, unordered manifest traversal, native and XNB entry details, exact bounds and the sorted reader-usage summary. Evidence:`checkpoint-564-c-content-manager-inspection-focused-*-direct.*`. The manifest is the deployed tree's point-in-time snapshot; no asset load or rendered result is claimed, and project-built evidence remains pending. |
| C additional content loaders and foreign-object caching | New `c_api.content.additional_loaders` passes15 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. Four newly credited routes load an original WAV with positive duration, decode a deterministic generated six-face DDS cube to Color, and register/load a caller-owned CNJ object with bounded JSON, duplicate-registration refusal, cache identity, missing-type IO and post-Unload re-invocation. The first draft expected the DDS's source DXT1 identity after content decoding; correcting that was a demo defect. A removed resource-manager check ran against the older checkpoint library, which predates current CNA's explicit placeholder and still fell back to loose files; this is stale-binary behavior, not current-source evidence or a CNA issue. Focused1007 later credits the successful resource-manager constructor in `c_api.content.cnb_loader_registry`; only embedded-resource loading remains intentionally unsupported. `content.h` is now32/32 demonstrated. Evidence:`checkpoint-565-c-content-additional-loaders-focused-*-direct.*` and `/tmp/checkpoint-1007-c-route-revalidation/*`. No audible or rendered-quality claim is made; project-built evidence remains pending. |
| C storage selectors, listings, resizing and disposal | New storage-only `c_api.storage.members` passes35 byte-identical controls with empty stderr in both renderer-labelled environments. Twenty-five newly credited routes cover exact/atomic root and display names, all selector variants and synchronous completion, capacity, process registration ownership, container state/owner, unordered directory and filtered file listing, explicit share selection, flush, successful truncate/zero-extension and idempotent disposal with one event. The first sandboxed attempts could not create the standard user storage root and provide no runtime evidence. `storage.h` now has a disposition for every route at47/49 demonstrated; its fixed container reflection-name pair is trivial/uncredited. Actual device-change delivery is manual/external, and file-share enforcement is not claimed because CNA currently ignores the bits. Evidence:`checkpoint-566-c-storage-members-focused-*-direct.*`. No game, renderer execution or visual claim is made; project-built evidence remains pending. |
| C ContentReader identity, values, limits and ownership | New storage-only `c_api.content.reader_values` passes33 byte-identical controls with empty stderr in both renderer-labelled environments. All first20 `content_readers.h` routes are explicit: standalone manager identity, exact/atomic asset-name copying, version/platform state, empty type-reader and shared-resource tables, a null object tag, canonical Vector2/3/4, Quaternion, Matrix, Color and BoundingSphere layouts, collection/decoded-size guards, capacity-before-consumption, exact bytes, truncation and exclusive stream borrowing through one-shot releases. Evidence:`checkpoint-567-c-content-reader-values-focused-*-direct.*`. The stream is synthetic and no renderer is initialized; compiled pipeline production, nonempty object graphs and project-built evidence remain separate. |
| C content type-reader registry, placeholders and callbacks | New storage-only `c_api.content.type_reader_registry` passes37 byte-identical controls with empty stderr in both renderer-labelled environments. Fifteen routes cover an isolated registry clear, the idempotent empty known-unsupported hook, missing lookup, explicit placeholder identity/refusal, exact/atomic target names, version acceptance and initialization. A caller-owned factory demonstrates malformed and duplicate refusal, fresh instances, callback-scoped reader use plus post-return invalidation, failure propagation, instance survival after unregistration, exact cleanup and name reuse. `content_readers.h` reaches35/62 demonstrated. Evidence:`checkpoint-568-c-content-type-reader-registry-focused-*-direct.*`. No built-in reader decoding, renderer execution or project-built evidence is claimed. |
| C reflective content-reader builders and field mapping | New storage-only `c_api.content.reflective_reader` passes40 byte-identical controls with empty stderr in both renderer-labelled environments. Eleven routes cover exact/atomic reflective and enum canonical names, malformed declaration refusals, first-registration snapshot behavior, all14 inline `CNA_ContentFieldKind` identities, enum registration, custom callback reader scope, registration survival after builder destruction and a distinct shared/reference-shaped reader over caller-owned storage. `content_readers.h` reaches46/62 demonstrated. Evidence:`checkpoint-569-c-reflective-reader-focused-*-direct.*`. Synthetic payloads invoke the readers directly; no pipeline-produced XNB, renderer execution or project-built evidence is claimed. |
| C object-dictionary inspection and value transfer | New `c_api.content.object_dictionary` passes47 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. The final16 routes load a deterministic generated XNB and cover exact/atomic logical runtime names and sorted keys, all15 public entry-kind identities across every fixed scalar, a string, packed Vector3 array, shared caller-created object and deliberately unknown UInt32 diagnostic type, plus wrong-kind/shape refusal, cache reuse, independent handles and stale-handle rejection. The first run failed only because the staged fixture had not been copied into the old build tree; an exact deployment copy fixed it, so no CNA issue is recorded. `content_readers.h` is62/62 demonstrated. Evidence:`checkpoint-570-c-object-dictionary-focused-*-direct.*`. Decoded values are validated; no rendered output, content-compiler provenance or project-built evidence is claimed. |
| C Video asset construction, metadata and ownership | New `c_api.media.video_asset` passes26 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. The first19 `video.h` routes probe the generated160x120/15fps WMV, correlate individual and aggregate metadata, verify positive duration/device association and exact/atomic path/reflection text, then cover metadata-only missing-file construction, version refusal, duration correction, deferred track indices, the URI extension's literal-path behavior and independent one-shot ownership. Evidence:`checkpoint-571-c-video-asset-focused-*-direct.*`. `video.h` reaches19/42 demonstrated. No playback, decoded frame, audible output, pixels or project-built evidence is claimed. |
| C VideoPlayer transport, decoded frames and lifetime | New `c_api.media.video_player` passes31 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. The final23 `video.h` routes cover stopped defaults, configuration/volume clamping, exact/atomic type text, active Video identity, a real versioned frame, next-call invalidation of borrowed texture handles, stable paused position, Resume, an observed end-to-start loop, Stop/reset, strictly monotonic generation across replay and disposed refusal. Evidence:`checkpoint-572-c-video-player-focused-*-direct.*`. `video.h` is42/42 demonstrated. The run validates state and texture identity, not pixels, audible quality, visual appearance or a project-built runner. |
| C XACT engine, renderer value, globals and disposal | New `c_api.audio.xact_engine` passes17 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. Seventeen `xact.h` routes cover both settings constructors, CNA's one renderer through three exact text views plus atomic refusal/bounds/equality/hash, the mutable and clamped authored SpeedOfSound global versus cue-scoped Intensity, explicit Update, live-state query and one synchronous disposal registration with independent release. Evidence:`checkpoint-573-c-xact-engine-focused-*-direct.*`. `xact.h` reaches17/62 demonstrated. No audible output, look-ahead scheduling effect or project-built runner is claimed. |
| C XACT wave/sound banks, categories and child ownership | New `c_api.audio.xact_banks` passes35 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. Twenty-eight more `xact.h` routes cover resident/actual-streaming bank creation and readiness, sound-bank lookup, initial/live use, exact category names/equality/hashes, finite volume validation, selective Pause/Resume/Stop observed through simultaneous Music/SFX cues, fire-and-forget plus positioned playback, child-before-parent release and one disposal event per bank. The corrected workflow removed a redundant missing-streaming probe whose canonical success also logs an expected audio error. Evidence:`checkpoint-574-c-xact-banks-focused-*-direct.*`. `xact.h` reaches45/62 demonstrated. No category gain, spatial speaker output or project-built runner is claimed. |
| C XACT caller-owned Cue behavior and lifetime | New `c_api.audio.xact_cue` passes33 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. The final nine meaningful `xact.h` routes cover exact/atomic authored names, independent and clamped Intensity variables, Apply3D's public distance/angle/Doppler outputs, Pause/Resume/Immediate Stop snapshots, a distinct generated AsAuthored release phase and one disposal event whose registration releases after its sender. Evidence:`checkpoint-575-c-xact-cue-focused-*-direct.*`. All62 routes have dispositions at54 demonstrated; eight fixed engine/bank/Cue type-name helpers remain trivial/uncredited. No acoustic position, gain, fade shape, audible quality or project-built runner is claimed. |
| C network QoS and fixed session-property values | New renderer-free `c_api.net.session_properties` passes20 byte-identical controls with empty stderr in both renderer-labelled environments. The first22 `net.h` routes cover unavailable/measured/versioned QualityOfService, an absent per-thread join diagnostic after an ordinary failure, all eight optional property slots, indexed replacement/bounds/search, every structural refusal, exact atomic shifted copying, live ordered enumeration, reset/end behavior and child-before-parent destruction. Evidence:`checkpoint-576-c-net-properties-focused-*-direct.*`. `net.h` reaches22/51 demonstrated. No game, session, socket, real join/throughput or project-built runner is claimed. |
| C in-memory packet-buffer values and ownership | New renderer-free `c_api.net.packet_buffers` passes20 byte-identical controls with empty stderr in both renderer-labelled environments. The final29 `net.h` routes write every public Color, vector, quaternion, matrix and scalar value into one ordered132-byte payload, prove exact/atomic copying, seek and overwrite without truncation, then copy and restore every value through an independent reader. Position-independent copies, seeking, end-of-input, empty reset, invalid data and one-shot ownership are explicit. The first run incorrectly required an undersized compound Color read to preserve its position; the header promises I/O exhaustion but not transactional position, so the assertion was a demo defect. Evidence:`checkpoint-577-c-net-packets-focused-*-direct.*`. `net.h` is51/51 demonstrated. No game, session, socket, wire transport, peer interoperability, network byte order or project-built runner is claimed. |
| C detached NetworkGamer and NetworkMachine values | New renderer-free `c_api.net.gamer_machine_values` passes17 byte-identical controls with empty stderr in both renderer-labelled environments. All26 non-event `net_gamers.h` routes cover detached neutral role/voice/ready/talking/id/timing/session state, exact transport-owned field mutation, noncanonical boolean and remote-ready refusal, empty-machine bounds/removal, copy-on-set, independent copy-out and one-shot ownership. Evidence:`checkpoint-578-c-net-gamer-values-focused-*-direct.*`. `net_gamers.h` reaches26/33 demonstrated. Populated machine borrowing, local-gamer ready publication, successful session removal and a project-built runner remain separate. |
| C network-session event payload values | New renderer-free `c_api.net.session_event_values` passes15 byte-identical controls with empty stderr in both renderer-labelled environments. The final seven `net_gamers.h` routes cover canonical and malformed transition prefixes, exact joined/left/host gamer identities, atomic stale-handle refusal, all four session-end reasons, zero reserved storage and canonical/invalid leaderboard leaving flags with an optional gamer. Evidence:`checkpoint-579-c-net-event-values-focused-*-direct.*`. `net_gamers.h` is33/33 demonstrated. Payload construction does not establish live event dispatch or a project-built runner. |
| C discovered-session values and collection snapshots | New renderer-free `c_api.net.available_sessions` passes23 byte-identical controls with empty stderr in both renderer-labelled environments. The first21 `net_sessions.h` routes cover exact roster/slot/type/port values, atomic host/address text, measured QoS, copied session properties, equality that excludes QoS/properties but includes connection identity, copied collection inputs, bounds, readable idempotent disposal and an entry that outlives its collection. Evidence:`checkpoint-580-c-net-available-focused-*-direct.*`. `net_sessions.h` reaches21/104 demonstrated. The inputs are synthetic; no discovery, join, socket, address reachability or project-built runner is claimed. |
| C Local session state, policy, roster and lifetime | New renderer-free `c_api.net.local_session_state` passes30 byte-identical controls with empty stderr in both renderer-labelled environments. Thirty-seven more `net_sessions.h` routes cover explicit-gamer construction, all four roster counts, local/host/find-by-id borrowed views, child-before-parent ownership, host/join policy, slot limits, simulated conditions, independent property replacement, exact/atomic type text, traffic and process counts, ready/reset behavior, queued lobby/game transitions and idempotent disposal. The supporting signed-in-gamer constructor/destructor are also newly demonstrated. Evidence:`checkpoint-581-c-net-local-state-focused-*-direct.*`. `net_sessions.h` reaches58/104 demonstrated. No socket, remote machine, measured throughput, cross-machine propagation or project-built runner is claimed. |
| C Local session event subscriptions and scoped payloads | New renderer-free `c_api.net.session_events` passes19 byte-identical controls with empty stderr in both renderer-labelled environments. Fourteen more `net_sessions.h` routes cover owned registration for every event family, existing/late GamerJoined, remote GamerLeft, queued GameStarted/GameEnded, injected HostChanged/SessionEnded, exact versioned payloads, callback-scoped gamer invalidation and one-shot unsubscribe. Evidence:`checkpoint-582-c-net-events-focused-*-direct.*`. `net_sessions.h` reaches72/104 demonstrated. The three leaderboard-write and invite registrations stay silent under this Local workflow, so no service-event, socket, remote-machine or project-built delivery is claimed. |
| C LocalNetworkGamer identity, packet queues and sends | New renderer-free `c_api.net.local_gamer_packets` runs28 byte-identical controls with empty stderr in both renderer-labelled environments:27 pass and one retains CNA-DEMOS-ISSUE-075. All16 LocalNetworkGamer routes in `net_sessions.h` cover standalone/session identity, borrowed signed-in text, voice/party validation, explicit queue injection, plain/offset/PacketReader receive, scoped senders, clearing and every byte/range/recipient/PacketWriter send form. The failure shows that offset receive zeroes caller bytes outside the requested range despite placing the payload correctly. Two supporting signed-gamer text routes are also newly demonstrated. Evidence:`checkpoint-583-c-local-packets-focused-*-direct.*`. `net_sessions.h` reaches88/104 demonstrated. No socket/wire delivery, voice audio, party-service interaction or project-built runner is claimed. |
| C Local session construction and local-gamer admission | New renderer-free `c_api.net.session_construction` passes28 byte-identical controls with empty stderr in both renderer-labelled environments. Six more `net_sessions.h` routes cover the published-gamer basic and property constructors, copied private-slot/property state, one successful plus one duplicate local-gamer admission, and all three Begin/End async adapters with exact once-before-return callbacks and owned results. Instance/action counts return to zero, while two supporting signed-in collection routes are newly demonstrated. Evidence:`checkpoint-584-c-session-construction-focused-*-direct.*`. `net_sessions.h` reaches94/104 and staged metadata reaches2148/3205. No socket, service, concurrency, remote peer or project-built runner is claimed. |
| C session discovery, join and offline invitation semantics | New renderer-free `c_api.net.session_discovery_join` passes32 byte-identical controls with empty stderr in both renderer-labelled environments. The final ten `net_sessions.h` routes cover owned empty LocalWithLeaderboards results across all implicit/explicit sync/async finds, exact callback counts, authenticated-online-description refusal, successful synthetic Local sync/async Join with joining-peer authority, and all four no-service invitation refusals without callbacks. Evidence:`checkpoint-585-c-session-discovery-join-focused-*-direct.*`. `net_sessions.h` is104/104 and staged metadata reaches2158/3205. No socket, authenticated directory, accepted invite, address reachability, remote peer or project-built runner is claimed. |
| C Gamer identity, profiles and service lookup refusals | New renderer-free `c_api.gamer.profile_identity` passes19 byte-identical controls with empty stderr in both renderer-labelled environments. The first25 audited `gamer_services.h` routes cover exact/atomic Gamer names and text, UTF-8 mutation, live state, a 64-bit tag, independent sync/async profile snapshots, numeric and text/picture profile values, stale-handle refusal, and explicit unsupported gamertag/partner-token service operations without callbacks. Evidence:`checkpoint-586-c-gamer-profile-focused-*-direct.*`. `gamer_services.h` reaches31/239 and staged metadata reaches2183/3205. No online account lookup, token, profile picture, game, renderer or project-built runner is claimed. |
| C signed-in gamer state and process collection | New renderer-free `c_api.gamer.signed_in_state` passes29 byte-identical controls with empty stderr in both renderer-labelled environments. Nineteen more `gamer_services.h` routes cover guest/service/player identity, party state, versioned invite and presence values, free-text presence, canonical privileges, offline friendship/friend-list results, positional versus player-index collection lookup, membership, owned borrowed views and published-source retention. Evidence:`checkpoint-587-c-signed-in-state-focused-*-direct.*`. `gamer_services.h` reaches50/239 and staged metadata reaches2202/3205. The passing demo records CNA-DEMOS-ISSUE-076 for the absent player-index output contract mismatch. No service friendship, presence publication, microphone, game, renderer or project-built runner is claimed. |
| C friend values and mutable gamer collections | New renderer-free `c_api.gamer.friend_collection` passes20 byte-identical controls with empty stderr in both renderer-labelled environments. Nineteen more `gamer_services.h` routes cover distinct friend status/request snapshots, exact atomic presence text, retained ordered indexing and bounds, IndexOf/Contains, atomic shifted CopyTo, full/reset cursor traversal, remove/no-op/add/clear mutations, disposal state and one-shot ownership. Evidence:`checkpoint-588-c-friend-collection-focused-*-direct.*`. `gamer_services.h` reaches69/239 and staged metadata reaches2221/3205. No live account friend graph, service state, game, renderer or project-built runner is claimed. |
| C achievement values and mutable collections | New renderer-free `c_api.gamer.achievement_collection` runs30 byte-identical controls with empty stderr in both renderer-labelled environments:29 pass and one retains CNA-DEMOS-ISSUE-077. All29 value/collection routes in `gamer_services.h` cover versioned numeric fields, exact atomic text, empty optional catalog fields, picture refusal, value equality, copied collection ownership, owned lookups, membership/bounds, atomic shifted copying and every public mutation. The failed control exposes a true read-only property on a successfully mutable collection; passing CNA-DEMOS-ISSUE-078 records unknown-key `INTERNAL` versus documented `INVALID_ARGUMENT`. Evidence:`checkpoint-589-c-achievement-collection-focused-*-direct.*`. `gamer_services.h` reaches98/239 and staged metadata reaches2250/3205. No catalog-backed picture/how-to text, gamer service, game, renderer or project-built runner is claimed. |
| C signed-in achievement award, read and persistence | New renderer-free `c_api.gamer.signed_in_achievements` passes28 byte-identical controls with empty stderr in fresh SOFTWARE and OPENGLES3-labelled stores, plus an identical repeat over populated state. The final four signed-in achievement routes cover complete offline catalog reads, sync/async awards and reads, exactly-once callback adapters, catalog metadata, idempotent first timestamps and disk persistence across fresh gamer objects. Evidence:`checkpoint-590-c-signed-in-achievements-focused-*-direct.*`. `gamer_services.h` reaches102/239 and staged metadata reaches2254/3205. EarnedOnline remains with existing C++ CNA-DEMOS-ISSUE-017; no online service, game, renderer, notification appearance or project-built runner is claimed. |
| C headless Guide keyboard and message-box dialogs | New renderer-free `c_api.gamer.guide_dialogs` passes23 byte-identical controls with empty stderr in both renderer-labelled environments. All19 non-rendering dialog routes cover pending/callback timing, overlap and argument refusals, exact atomic password/title/description text, masked display text, cancellation state, repeated empty End, message-box focus and choice, invalid-click preservation, repeated End and active/completed reset semantics. Evidence:`checkpoint-591-c-guide-dialogs-focused-*-direct.*`. `gamer_services.h` reaches121/239 and staged metadata reaches2273/3205. Rendering, real input, visible appearance, game and project-built evidence remain separate. |
| C Guide state, dispatcher component and registration ownership | New `c_api.gamer.guide_dispatcher` passes22 byte-identical controls with empty stderr under both Xvfb/dummy-audio selections. Twenty more `gamer_services.h` routes cover pre/post initialization, sync/async pumps, exact opaque and component-installed window identities, notification and screen-saver mutation, update-latched trial state, the canonical GamerServicesComponent, duplicate-init refusal, retained-identity count and three one-shot registrations that correctly stay silent over an isolated empty profile store. Evidence:`checkpoint-592-c-guide-dispatcher-focused-*-direct.*`. `gamer_services.h` reaches141/239 and staged metadata reaches2293/3205. Event delivery, visible overlays, user interaction, pixels and project-built evidence remain separate. |
| C Guide dialog rendering and visible appearance | New `c_api.gamer.guide_rendering` draws a pending masked-password keyboard and, after exact cancellation/reset, a focused three-button alert through the two Guide rendering extensions. Both SOFTWARE and OPENGLES3 report the same substantial changed-pixel bounds and counts for each overlay;17 one-frame and24 two-frame controls pass with empty stderr. Four800x480 captures were inspected and show readable labels, six password masks, the alert icon and middle-button focus. Evidence:`checkpoint-593-c-guide-rendering-focused-*-direct.*`. `gamer_services.h` reaches143/239 and staged metadata reaches2295/3205. Real keyboard, mouse and controller interaction and project-built execution remain separate. |
| C local sign-in overlay and offline social contracts | `c_api.gamer.guide_social_offline` automatically renders and pixel-checks the one-pane local-profile picker, closes it through Escape simulation, then sends valid text, recipient and target handles through every service-backed Guide screen. Twelve routes retain documented no-service refusals and none leaves a partial overlay. Focused928 distinguishes their structured causes: Friends retains `GamerServicesNotAvailableException`, the phone-only invitation overload retains `System.NotSupportedException`, and Marketplace retains `GamerPrivilegeException` with its distinct INVALID_STATE/STATE result. Focused929 adds the live-overlay collision: a second valid ShowSignIn retains `GuideAlreadyVisibleException` and leaves the first screen visible. Ten27-control runs per renderer pass with stable per-renderer output and empty stderr. Evidence:`checkpoint-594-c-guide-social-offline-focused-*-direct.*` and `/tmp/checkpoint-928-guide-final-*` through `/tmp/checkpoint-929-guide-*`. `gamer_services.h` remains157/239; all239 routes have semantic dispositions. Real service screens and user input remain service/manual; `cna_guide_delay_notifications` is intentionally uncredited because delivered invitations have no public C fixture or readback. |
| C GameDefaults and tagged PropertyDictionary values | New renderer-free `c_api.gamer.game_defaults_properties` passes24 byte-identical controls with empty stderr in both renderer-labelled environments. All30 GameDefaults/property routes cover exact canonical defaults from the initializer and an offline gamer, atomic version refusal, empty/count/read-only state, all eight C-writable tagged values, kind discovery, exact typed reads, copied/atomic text, stream wrong-kind output reset, ascending exact key copies, replacement across kinds, invalid outcome and getter refusals, removal, clear and stale ownership. Evidence:`checkpoint-595-c-game-defaults-properties-focused-*-direct.*`. `gamer_services.h` reaches187/239 and staged metadata reaches2339/3205. A positive stream remains service-only because the public C factory has no stream setter; no game, renderer, service, stream payload or project-built runner is claimed. |
| C leaderboard entry values, callbacks and columns | New renderer-free `c_api.gamer.leaderboard_entries` passes23 byte-identical controls with empty stderr in both renderer-labelled environments. All eight entry routes cover exact versioned gamer/ranking/rating state, borrowed gamer identity, an anonymous value, mutable live columns retained beyond their entry handle, equality that includes gamer/rating/ranking but excludes columns, a once-per-write detachable rating hook and stale ownership. Evidence:`checkpoint-596-c-leaderboard-entries-focused-*-direct.*`. `gamer_services.h` reaches195/239 and staged metadata reaches2347/3205. CNA-DEMOS-ISSUE-079 records that absent gamer output is cleared despite the header promising preservation. No game, renderer, service, persisted leaderboard or project-built runner is claimed. |
| C offline leaderboard identities, reads and paging | New renderer-free `c_api.gamer.leaderboard_readers` passes30 byte-identical controls with empty stderr across separate isolated stores in both renderer-labelled environments. The remaining15 leaderboard routes cover all four exact canonical identities, every sync/callback explicit/pivot/restricted read shape, exact versioned empty state and identity, entry-bound refusal, successful sync/callback page-up toward zero, empty-final-page page-down refusal and stale ownership. Evidence:`checkpoint-597-c-leaderboard-readers-focused-*-direct.*`. `gamer_services.h` reaches210/239 and staged metadata reaches2362/3205. Positive entry lookup/page-down needs persisted writer/service rows which public C cannot construct; no game, renderer, service, persisted row or project-built runner is claimed. |
| C avatar expressions and descriptions | New renderer-free `c_api.gamer.avatar_descriptions` passes26 byte-identical controls with empty stderr in both renderer-labelled environments. Nine routes cover neutral expression initialization, exact1021-byte foreign payload ownership and atomic copies, valid/unmeasured semantics, unconstrained plus female/male random measured avatars, byte-codec round-trip, exactly-once offline gamer lookup and one owned per-instance Changed registration that correctly stays silent. Evidence:`checkpoint-598-c-avatar-descriptions-focused-*-direct.*`. `gamer_services.h` reaches219/239 and staged metadata reaches2371/3205. Service-driven change delivery, game, renderer, drawing and project-built execution remain separate. |
| C avatar animation timelines, bones and expressions | New renderer-free `c_api.gamer.avatar_animations` passes17 byte-identical controls with empty stderr in both renderer-labelled environments. All seven animation routes cover all31 preset lengths and71-bone counts, a moving Wave elbow, clamped and looping time, canonical-boolean refusal, Celebration's keyed face, atomic structure/bone bounds and one-shot ownership. Evidence:`checkpoint-599-c-avatar-animations-focused-*-direct.*`. `gamer_services.h` reaches226/239 and staged metadata reaches2378/3205. CNA-DEMOS-ISSUE-080 records the C header's obsolete zero-timeline statement. No game, renderer, composed skeleton, pixels or project-built execution is claimed. |
| C avatar renderer, skeleton and both draw inputs | New `c_api.gamer.avatar_renderer` passes43 controls with byte-identical normalized output and empty stderr on SOFTWARE and OPENGLES3. All11 renderer routes cover description retention through Ready, versioned state, exact/default/optional transforms, exact lighting and atomic non-finite refusal, the canonical71-parent hierarchy, finite bind poses, human-scale composed height, bounds and one-shot ownership. Both `draw_animation` and caller-supplied `draw_bones` cover substantial two-dimensional back-buffer regions; both inspected800x480 screenshots show a complete waving avatar. Evidence:`checkpoint-600-c-avatar-renderer-focused-*-direct.*`. `gamer_services.h` reaches237/239 and staged metadata reaches2389/3205; the remaining Guide-delay and physical-headset observations are intentionally service/manual, giving all239 routes semantic dispositions. No service avatar, physical microphone or project-built execution is claimed. |
| C media-library construction and music-collection lifetime | New `c_api.media.library_collections` passes34 byte-identical controls with empty stderr in isolated SOFTWARE- and OPENGLES3-labelled Xvfb environments. Thirty-three `media_library.h` routes cover default/explicit Local Device construction, exact/atomic source text, five live music collections, nonnegative counts, four atomic upper bounds, idempotent library disposal, child-retained lifetime, canonical collection disposal-to-empty and stale ownership. Evidence:`checkpoint-601-c-media-library-collections-focused-*-direct.*`. `media_library.h` reaches33/148 and staged metadata reaches2422/3205. Positive entity relationships, album art, pictures and save/import behavior remain for controlled fixtures; no host content, window, audio, image or project-built execution is claimed. |
| C tagged media-library values and relationships | New `c_api.media.library_music_values` passes33 byte-identical controls with empty stderr on SOFTWARE and OPENGLES3 over self-created private Linux/XDG roots. Forty-two more `media_library.h` routes cover exact/atomic Album/Artist/Genre/Playlist names, same-title album identity by artist, equality/hash, every music group relationship and Song back-reference, reversed M3U order, exact duration aggregation, missing-art state, shared disposal and retained releases after the top-level library is gone. Evidence:`checkpoint-602-c-media-library-values-focused-*-direct.*`. `media_library.h` reaches75/148 and staged metadata reaches2464/3205. Album image bytes and the picture/save subsystem remain; no playback, audible quality, host media or project-built execution is claimed. |
| C media-library album art and thumbnails | New `c_api.media.library_album_art` passes15 byte-identical controls with empty stderr on SOFTWARE and OPENGLES3 over two private tagged albums. Four routes return the exact1666-byte original cover, preserve both short destinations, refuse both bare-album size requests atomically and return a distinct parsed128x64 PNG thumbnail. Evidence:`checkpoint-603-c-media-library-album-art-focused-*-direct.*`. `media_library.h` reaches79/148 and staged metadata reaches2468/3205. No rendering, screenshot, visual inspection or project-built execution is claimed. |
| C picture-library hierarchy, values and imports | New `c_api.media.library_pictures` passes57 byte-identical controls with empty stderr on SOFTWARE and OPENGLES3 over a controlled two-level Linux/XDG tree. All43 meaningful picture routes cover exact atomic values and tokens, parent/child collections, equality/hashes, byte-exact image and generated-thumbnail transfer, token lookup, buffer save, full storage-stream import, live membership, shared disposal, collection emptying and child-retained lifetime. Evidence:`checkpoint-604-c-media-library-pictures-focused-*-direct.*`. `media_library.h` reaches122/148 and staged metadata reaches2511/3205. The other26 fixed reflection type-name routes are intentionally trivial/uncredited, giving the header complete semantic dispositions. No rendering, screenshot, visual inspection, host media or project-built execution is claimed. |
| C CNB format, checksum and compression foundations | New renderer-free `c_api.content.cnb_foundations` passes25 byte-identical controls with empty stderr in both renderer-labelled direct environments. The first26 `cnb.h` routes cover exact wire layout/magic, chunk and asset identities, logical-name validation, versioned limits, atomic overflow guards, the published CRC-32C check vector across selected/split/portable paths, exact stored bytes, unavailable codec refusals and a real shrinking Zstandard round trip. Evidence:`checkpoint-605-c-cnb-foundations-focused-*-direct.*`. `cnb.h` reaches26/272 and staged metadata reaches2537/3205. No game, renderer, filesystem, compiled asset or project-built execution is claimed. |
| C CNB bounded primitive byte I/O and ownership | New renderer-free `c_api.content.cnb_byte_io` passes27 byte-identical controls with empty stderr in both renderer-labelled direct environments. Thirty-eight `cnb.h` routes cover copied writer prefixes, all primitive encodings, string/count/raw/zero appends, exact and short-buffer-safe copies, ownership transfer, copied reader input/context, cursor bounds, retained string copies, exact exhaustion, EOF/schema failure, stale handles and valid/malformed UTF-8. Evidence:`checkpoint-606-c-cnb-byte-io-focused-*-direct.*`. `cnb.h` reaches64/272 and staged metadata reaches2575/3205. No game, renderer, filesystem, complete document or project-built execution is claimed. |
| C CNB document/writer round trip and ownership | New renderer-free `c_api.content.cnb_document` passes51 byte-identical controls with empty stderr in both renderer-labelled direct environments. All40 document/writer routes cover copied schema chunks, metadata, replaced references, parse-compatible limits, optional compression, deterministic and atomic builds, file I/O, complete memory/file parsing, table/lookup/mandatory rules, exact metadata/reference transfers, borrowed-reader lifetime, corrupt/truncated refusal and stale ownership. Evidence:`checkpoint-607-c-cnb-document-focused-*-direct.*`. `cnb.h` reaches104/272 and staged metadata reaches2615/3205. No game, renderer, decoded asset or project-built execution is claimed. |
| C CNB texture formats, representations and neutral data | New renderer-free `c_api.content.cnb_texture_data` passes26 byte-identical controls with empty stderr in both renderer-labelled direct environments. Twenty `cnb.h` routes cover frozen identities, exact names, classifications, unit/level sizes, all27 two-way SurfaceFormat mappings, copied Rgba8 creation, a three-mip volume, ordered Rgba8/BC7 representations, atomic level transfers, synchronous preference selection, bounds and stale ownership. Evidence:`checkpoint-608-c-cnb-texture-data-focused-*-direct.*`. `cnb.h` reaches124/272 and staged metadata reaches2635/3205. No document, graphics device, GPU texture, renderer, pixels or project-built execution is claimed. |
| C CNB standalone and embedded texture codecs | New renderer-free `c_api.content.cnb_texture_codecs` passes22 byte-identical controls with empty stderr in both renderer-labelled direct environments. The final eight texture routes round-trip multi-mip 2D, face-major cube and depth-aware volume data through complete documents; enforce exact/atomic output, asset/shape/schema boundaries and wrong-decoder refusal; and restore an embedded SpriteFont atlas without treating its document as a standalone texture. Evidence:`checkpoint-609-c-cnb-texture-codecs-focused-*-direct.*`. `cnb.h` reaches132/272 and staged metadata reaches2643/3205. No graphics device, GPU texture, renderer, filesystem, displayed pixels or project-built execution is claimed. |
| C CNB model bones, parts, bytes and materials | New renderer-free `c_api.content.cnb_model_core` passes26 byte-identical controls with empty stderr in both renderer-labelled direct environments. Thirty `cnb.h` routes cover graph flags/counts, indexed bone ownership, exact atomic names, versioned part replacement, copied vertex/index bytes, full material values, eight distinct texture-name slots, seven distinct coordinate/transform/sampler slots, bounds and stale ownership. Evidence:`checkpoint-610-c-cnb-model-core-focused-*-direct.*`. `cnb.h` reaches162/272 and staged metadata reaches2673/3205. No document codec, ContentManager, graphics device, renderer or draw call is claimed. |
| C CNB model morph targets and weight timeline | New renderer-free `c_api.content.cnb_model_morph` passes20 byte-identical controls with empty stderr in both renderer-labelled direct environments. All12 morph routes cover absence, attachment, two indexed targets, distinct copied delta streams, default weights, a cubic key's three streams, atomic copies, derived counts, selector bounds, flag replacement with retained streams and idempotent whole-graph clear. Evidence:`checkpoint-611-c-cnb-model-morph-focused-*-direct.*`. `cnb.h` reaches174/272 and staged metadata reaches2685/3205. No document codec, mesh deformation, animation player, graphics device, renderer or draw call is claimed. |
| C CNB model mesh, skeleton, animation, light and codec | New renderer-free `c_api.content.cnb_model_scene` passes28 byte-identical controls with empty stderr in both renderer-labelled direct environments. The remaining20 neutral model/codec routes cover an attached mesh and ordered part list, a two-joint skeleton's hierarchy and three matrix sets, a copied two-key scene animation, a directional light, exact/atomic transfers, idempotent skeleton removal and a complete Model document encode/parse/decode round trip. Evidence:`checkpoint-612-c-cnb-model-scene-focused-*-direct.*`. `cnb.h` reaches194/272 and staged metadata reaches2705/3205, completing all62 neutral model-data/model-codec routes. No CNJ file, ContentManager, graphics device, renderer, animation playback or draw call is claimed. |
| C CNJ model compiler result and dependency ownership | New renderer-free `c_api.content.cnb_model_from_cnj` passes18 byte-identical controls with empty stderr in both renderer-labelled direct environments. All nine result routes compile a real private CNJ/sidecar fixture, expose exact ordered absorbed files and sorted external assets with atomic text copies and Count bounds, transfer the derived graph exactly once, retain the lists after transfer and keep the model alive after result destruction. Stale access and an absent CNJ fail without partial handles. Evidence:`checkpoint-613-c-cnb-model-from-cnj-focused-*-direct.*`. `cnb.h` reaches203/272 and staged metadata reaches2714/3205. No ContentManager, graphics device, renderer, draw call or project-built execution is claimed. |
| C CNB SpriteFont data, atlas ownership and codec | New renderer-free `c_api.content.cnb_sprite_font_data` passes24 byte-identical controls with empty stderr in both renderer-labelled direct environments. All11 SpriteFont routes cover default and authored metrics, two complete shared-family glyph rows, derived counts, invalid-row rollback, Count bounds, copied atlas lifetime, strict character ordering, atomic and deterministic complete encoding, decoding and stale ownership. Evidence:`checkpoint-614-c-cnb-sprite-font-data-focused-*-direct.*`. `cnb.h` reaches214/272 and staged metadata reaches2725/3205. No ContentManager, graphics device, renderer, text layout, draw call or project-built execution is claimed. |
| C CNB audio identities and neutral SoundEffect codec | New renderer-free `c_api.content.cnb_sound_effect_data` passes17 byte-identical controls with empty stderr in both renderer-labelled direct environments. All nine audio/SoundEffect routes cover frozen identities, exact fixed-frame sizes and atomic names, copied PCM8 ownership, versioned shape and loop, atomic sample/document transfers, deterministic schema-two encoding, decoding, stale access, inconsistent payload refusal and a reserved format without a codec. Evidence:`checkpoint-615-c-cnb-sound-effect-data-focused-*-direct.*`. `cnb.h` reaches223/272 and staged metadata reaches2734/3205. No SoundEffect runtime object, audio device, playback, audible output or project-built execution is claimed. |
| C CNB Song and Video streaming metadata | New renderer-free `c_api.content.cnb_media_metadata` passes20 byte-identical controls with empty stderr in both renderer-labelled direct environments. All ten media-schema routes cover deterministic complete Song/Video documents, exact versioned values, atomic display/stream text, equivalence with each document's single XREF, valid empty Song metadata, invalid reference/frame/rate/soundtrack refusals, wrong-asset decoding and stale ownership. Evidence:`checkpoint-616-c-cnb-media-metadata-focused-*-direct.*`. `cnb.h` reaches233/272 and staged metadata reaches2744/3205. No media file, ContentManager, decoder, audio/video device, playback, rendering or project-built execution is claimed. |
| C CNB Curve, AnimationClip and shared keyframe bytes | New renderer-free `c_api.content.cnb_curve_animation` passes29 byte-identical controls with empty stderr in both renderer-labelled direct environments. All11 routes round-trip an ordinary Curve and an independently owned two-track scene-node clip, cover target space, track bounds and atomic key copies, and establish the single48-byte keyframe representation through its public writer/reader. Finite and unrepresentable `TimeSpan` seconds are handled on both sides. Evidence:`checkpoint-617-c-cnb-curve-animation-focused-*-direct.*`. `cnb.h` reaches244/272 and staged metadata reaches2755/3205. No model, animation player, graphics device, renderer, drawing or project-built execution is claimed. |
| C CNB loader registry, callback scope and copied loaders | New standalone `c_api.content.cnb_loader_registry` passes21 byte-identical controls with empty stderr under SOFTWARE and OPENGLES3 Xvfb. All11 routes cover empty state, exact atomic names, idempotence, a real FNV collision, custom document resolution/invocation, callback-scoped handle expiry, numeric lookup, loader lifetime after removal, built-in registration, the C++ object boundary and stale ownership. Evidence:`checkpoint-618-c-cnb-loader-registry-focused-*-direct.*`. `cnb.h` reaches255/272 and staged metadata reaches2766/3205. A caller-owned graphics device exists only to construct the required ContentManager; no pixels, drawing or project-built execution is claimed. |
| C CNB image, DDS cube and WAV import front ends | New renderer-free `c_api.content.cnb_importers` passes23 byte-identical controls with empty stderr in both renderer-labelled environments. All five routes cover exact default/keyed BMP pixels, DXT1-to-RGBA8 cube decompression, PCM8 sample and `smpl`-loop preservation, file/memory equivalence, caller-buffer ownership and atomic refusal of invalid options plus missing, malformed, non-cube, compressed and truncated sources. Evidence:`checkpoint-619-c-cnb-importers-focused-*-direct.*`. `cnb.h` reaches260/272 and staged metadata reaches2771/3205. No ContentManager, graphics/audio device, rendering, playback, audible/visual quality or project-built execution is claimed. |
| C generic CNJ-to-CNB compiler result and dependencies | New renderer-free `c_api.content.cnb_compile_cnj` passes22 byte-identical controls with empty stderr in both renderer-labelled environments. The final12 `cnb.h` routes cover exact Model identity/name, deterministic complete bytes, CNJ-first absorbed paths, sorted external assets, atomic transfers and Count bounds, independent copied-byte lifetime, real document/model decode, stale ownership and missing/unsupported-document refusal. Evidence:`checkpoint-620-c-cnb-compile-cnj-focused-*-direct.*`. `cnb.h` reaches272/272 and staged metadata reaches2783/3205, completing the public header's semantic route audit. No ContentManager, graphics device, renderer, animation playback, draw call, visual result or project-built execution is claimed. |
| C ModelBone values, retained hierarchy and collection views | New renderer-free `c_api.graphics.model_bones` passes25 byte-identical controls with empty stderr in both renderer-labelled environments. The first17 `models.h` routes cover default/copied values, exact transforms, live ordered child mutation, parent/index/name/membership lookup, ownership beyond original handles, Count bounds, cycle/self-parenting refusal and stale access. The first malformed-UTF-8 expectation was a demo bug: the public string boundary correctly returns `ENCODING`. Evidence:`checkpoint-621-c-model-bones-focused-*-direct.*`. `models.h` reaches17/216 and staged metadata reaches2800/3205. No Model, mesh, ContentManager, graphics device, renderer, animation player, draw call, visual result or project-built execution is claimed. |
| C standalone ModelMeshPart values, samplers and collections | New renderer-free `c_api.graphics.model_mesh_parts` passes23 byte-identical controls with empty stderr in both renderer-labelled environments. Twenty-one more `models.h` routes cover complete signed scalar/tag ranges, every primitive topology, all seven independent material/specular sampler slots, invalid-state atomicity, ordered retained snapshots, shared live mutation, stable ownership and stale access. Evidence:`checkpoint-622-c-model-mesh-parts-focused-*-direct.*`. `models.h` reaches38/216 and staged metadata reaches2821/3205. No effect, buffer, Model, mesh, graphics device, renderer, draw call, visual result or project-built execution is claimed. |
| C ModelMeshPart resources, ModelMesh values, collections and drawing | New `c_api.graphics.model_meshes` passes33 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. Thirty-two more `models.h` routes cover exact effect/vertex/index-buffer retention, premature-destruction refusal, copied names and bounds, tags, parents, live part/effect collections, duplicate effect ordering/removal, retained mesh snapshots, lookup/bounds, stable aliases and actual indexed drawing. Exact center-pixel probes prove the authored green triangle and the black empty-mesh control. Construction leaves a pre-attached part effect absent from `Effects`; CNA-DEMOS-ISSUE-081 records the confirmed native and C-adapter defect, while detach/reattach proves the live setter path. Evidence:`checkpoint-623-c-model-meshes-focused-*-direct.*`. `models.h` reaches70/216 and staged metadata reaches2853/3205. No ContentManager, imported asset, top-level Model, screenshot, visual-quality assessment or project-built execution is claimed. |
| C top-level Model ownership, transforms, diagnostics and drawing | New `c_api.graphics.models` passes39 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. Twenty-seven more `models.h` routes cover all three constructors, empty and retained collections, explicit root/mesh parent selection, tags, one-shot owned-resource callbacks, atomic local/absolute transform transfers, every glTF report/diagnostic scalar and owned text channel, Count bounds and destruction. Exact framebuffer probes prove `Model.Draw` composes the child's absolute transform with its world matrix: the green center moves to(700,120) and the old(400,240) center remains black. The first probe expected only the world transform; source review confirmed the parent-bone composition, so this was a demo error rather than a CNA defect. Evidence:`checkpoint-624-c-models-focused-*-direct.*`. `models.h` reaches97/216 and staged metadata reaches2880/3205. No ContentManager, imported glTF, animation player, screenshot, visual-quality assessment or project-built execution is claimed. |
| C morph-target ownership, tracks, CPU blend and part upload | New `c_api.graphics.morph_targets` passes29 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. All26 morph-related `models.h` routes cover deeply copied base bytes, position/normal/tangent deltas, current weights, cubic/step/linear track ownership and evaluation, exact normalized CPU blending, flat-normal triangle metadata, atomic bounds/refusals and retained ModelMeshPart attachment. Direct VertexBuffer readback proves weights(0.5,0.25) upload the independently predicted position(0.5,0,0.5). Evidence:`checkpoint-625-c-morph-targets-focused-*-direct.*`. `models.h` reaches123/216 and staged metadata reaches2906/3205. No mesh, Model, ContentManager, animation player, draw call, screenshot, visual result or project-built execution is claimed. |
| C SkinnedModelEXT skeleton, clips, moves and part ownership | New `c_api.graphics.skinned_models` passes28 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. Twenty-six more `models.h` routes cover deep-copied skeleton/bind arrays and clips, sorted lookup, atomic track copies, loop/clamp transform evaluation, invalid hierarchy retention, move construction/assignment and matching-skeleton part transfer. A same-device part proves ownership of its VertexBuffer, IndexBuffer and optional Texture2D; exact-name replacement releases the old set, transfers the donor set and leaves removal fully releasable. Evidence:`checkpoint-626-c-skinned-models-focused-*-direct.*`. `models.h` reaches149/216 and staged metadata reaches2932/3205. No skinning shader, texture sampling, draw call, screenshot, visual result or project-built execution is claimed. |
| C SkinningData values and AnimationPlayer transforms | New renderer-free `c_api.graphics.skinning_data_player` passes28 byte-identical controls with empty stderr in both renderer-labelled direct environments. Thirty-two more `models.h` routes cover deeply copied hierarchy/bind/root-prefix state, sorted clips, exact atomic transfers, clip target space, declared rig-root metadata, initial bind pose, named playback, half-time local/world/skin composition, absolute clamp and loop, invalid-update atomicity and retained player lifetime after source release. Evidence:`checkpoint-627-c-skinning-data-player-focused-*-direct.*`. `models.h` reaches181/216 and staged metadata reaches2964/3205. No Model, ContentManager, graphics device, renderer, draw call, screenshot, visual result or project-built execution is claimed. |
| C ModelAnimationsEXT collection and direct Model posing | New `c_api.graphics.model_animations` passes29 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. Eleven more `models.h` routes cover empty and deep-copied clip collections, sorted exact metadata, duplicate and atomic bounds refusal, target-space mutation, joint-palette safety, real scene-node posing of a two-bone Model, preservation of an untracked child, hierarchy composition, time clamp/non-finite behavior and upload of a composed two-bone bind palette to one real retained SkinnedEffect. Evidence:`checkpoint-628-c-model-animations-focused-*-direct.*`. `models.h` reaches192/216 and staged metadata reaches2975/3205. No vertex/index buffer, texture, draw call, screenshot, visual result or project-built execution is claimed. |
| C Model cameras, skins, bounds and infinite projection | New `c_api.graphics.model_scene_metadata` passes28 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. Sixteen more `models.h` routes cover complete copied camera state and atomic text, explicit skin-to-model-mesh mapping, retained and absent skeleton identities, fresh skeleton handles, list clearing, exact root-placed aggregate bounds and infinite-far perspective construction and refusal paths. The initial zero-aspect refusal was a demo error corrected after source review showed native XNA parity. Evidence:`checkpoint-629-c-model-scene-metadata-focused-*-direct.*`. `models.h` reaches208/216 and staged metadata reaches2991/3205. No material, buffer, texture, draw call, screenshot, visual result or project-built execution is claimed. |
| C content-loaded Model variants and processor tags | New `c_api.content.model_loading` passes30 controls with byte-identical stdout and empty stderr under SOFTWARE and OPENGLES3 Xvfb. The final eight `models.h` routes cover missing/empty and real glTF Model loads, exact ordered material-variant names, atomic short copies, valid/default/invalid selection state and exact red/blue/gold body pixels with unaffected black wheels. Private hand-written XNB Models establish dictionary-versus-foreign tag discrimination, retained dictionary lifetime after Model destruction, exact BoundingSphere data and caller ownership of a shared reflective tag object. Evidence:`checkpoint-630-c-model-loading-focused-*-direct.*`. `models.h` reaches216/216 and staged metadata reaches2999/3205, completing the header's semantic route audit. Pixel readback was programmatic; no screenshot, visual inspection or project-built execution is claimed. |

Remaining audit scope: other XNA public members, overload families and behaviors, CNA-specific
headers, renderer-specific execution, and manual/service scenarios. Neither these tables
nor the existing type decisions constitute a complete public API audit.

### Complete634 generated checkpoint

The focused member work through VertexElement is now included in a complete project build and both
full smoke suites. Generated coverage is627 demos,297/329 types,1417/2326 XNA members and
2999/3205 C routes. SOFTWARE reports556 pass/33 expected fail/35 skip/3 manual; OPENGLES3 reports
588/33/3/3, both with zero unexpected passes. This replaces temporary projections through
checkpoint633 with generated runtime-backed data.

The checkpoint also changes two audit dispositions. `ResourceContentManager` remains an expected
ISSUE-012 failure but is now process-isolated because prior process-global XNB reader registration
could mask its cold-start failure. RenderTargetCube attachment/sampling is now a passing EasyGL
behavior after CNA `a6e8104d2`, so ISSUE-021 no longer contributes a runtime marker. The next focused
member audit continued in focused635 and focused636. The stock vertex workflow projects member
coverage to1437/2326; the SpriteFont behavior audit adds no generated XNA-member credit because its
five inventory members were already represented, while directly exercising all four CNA inspection
extensions. Continue with the SpriteBatch lifecycle/queued-resource behaviors classified above.

### Complete647 generated checkpoint

Focused635 through646 are now included in a complete project build and both full smoke suites.
Generated coverage is627 demos,297/329 types,1519/2326 XNA members and2999/3205 C routes.
SOFTWARE reports555 pass/34 expected fail/35 skip/3 manual and OPENGLES3 reports587/34/3/3, both
with zero unexpected passes. All640 enabled CTests pass and three declared tests remain disabled.
Compared with complete634, only `graphics.sprite_batch.begin_end_rules` changes state: its new
Dispose accounting check exposes the documented ISSUE-082 and therefore becomes an expected
failure on both renderers. The other extended workflows retain their prior states.

The complete evidence replaces the temporary member projections through focused646. The next
selected coherent gap is the public ContentReader scalar/math read family in an existing custom
reader workflow. Collection enumerators and routine Dispose/IsDisposed repetition remain subject
to semantic disposition rather than automatic demo expansion. Evidence:
`build/linux-full/reports/checkpoint-647-complete-*`. No screenshot, listening test or manual
visual inspection was performed.

### Remaining XNA member dispositions after focused716

The generated inventory has exactly212 uncredited non-field entries after focused716. They are
grouped below by why another metadata credit or demo would be misleading. This closes the current
generated-gap review; it does not claim that every overload, error path, renderer, service,
hardware device or visual behavior has been exercised.

| Namespace | Uncredited | Semantic disposition |
|---|---:|---|
| `Microsoft.Xna.Framework` | 8 | Both `CurveKeyCollection` enumerator spellings are already exercised through real C++ range iteration. `Game` and `GameComponent` finalizers map to native destructor/lifetime behavior already demonstrated. `Game::ShowMissingRequirementMessage` is a protected default-false hook, while the three `GameWindow::On*` entries are protected empty platform hooks; calling subclasses solely to name those defaults would add no user-visible behavior. |
| `.Audio` | 7 | Four finalizers are native destruction machinery already covered by owned-bank, engine, wave-bank and instance lifetimes. The three exception constructors represent missing hardware, missing microphone and instance-limit failure paths. The first two remain environmental/manual; CNA declares the instance-limit type but its production XACT path does not throw it. Directly constructing exceptions would demonstrate no audio behavior. |
| `.Content` | 7 | These are constructors/properties on four nominal serializer attribute wrappers. Focused687 covers the configurable `ContentSerializerAttribute` state and real XML descriptor behavior, including collection names, ignored members, runtime types and versions. Wrapper-only value checks would inflate the catalog without another serialization concept. |
| `.GamerServices` | 77 | Achievement/PropertyDictionary collection aliases and GamerCollection enumerator machinery are already represented by their real C++ collection workflows. Avatar finalizers are destructor plumbing. Avatar `Changed` and dispatcher `InstallingTitleUpdate` have no offline production trigger. Friend states, gamertag/partner tokens, social Guide overlays, invites, party/headset state and friend lookup require service identities, another user or hardware. Event-argument and exception constructors are framework-created/error payload plumbing. Keep these service/manual or intentionally uncredited. |
| `.Graphics` | 56 | Collection/nested enumerators and explicit `IVertexType`/`IEffectLights` aliases are represented by real range iteration and stock-type/effect use. Protected `OnApply` hooks are reached through actual effect passes; finalizers are native lifetimes. `DeviceLost` and device exceptions need a real platform loss, and `EffectMaterial` needs a compiled-FX material environment. `ModelMesh::Tag` is an arbitrary caller payload already dispositioned with the model collection audit. |
| `.Input.Touch` | 6 | These are the nested/non-generic CLR enumerator surface. Real `TouchCollection` range iteration, lookup, copy and mutation are already covered; no distinct CNA C++ entry point exists. |
| `.Media` | 14 | Seven collection types retain only generic/non-generic CLR `GetEnumerator` spellings. Their actual album, artist, genre, picture, picture-album, playlist and song ranges are traversed in populated library workflows. |
| `.Net` | 34 | Event-argument constructors and `NetworkSessionProperties` CLR aliases are already represented by real events and the fixed-shape C++ collection contract. Voice/talking/mute, guest identity, invitations and leaderboard callbacks require hardware or service participants. The real refused join covers `NetworkSessionJoinException::JoinError`; explicit construction/CLR serialization adds no network behavior. |
| `.Storage` | 3 | `StorageContainer::Finalize` is native lifetime plumbing. `StorageDevice::DeviceChanged` has no CNA production raiser, and the disconnected-device exception requires removable-device loss. These remain platform/manual rather than synthetic event or exception construction. |

Thus the212-entry generated difference is a reviewed mixture of already demonstrated semantics,
CLR aliases, protected/native machinery and truthful platform/service/manual work; it is not a list
of212 missing coherent demos. Fields/constants remain a separate inventory, and comprehensive CNA
behavioral coverage is still not declared.

### Complete717 generated runtime checkpoint

The consolidated dispositions above are backed by full current-catalog execution. SOFTWARE/X11
reports558 pass,36 expected fail,35 skip and3 manual; OPENGLES3/X11 reports590/36/3/3. Both cover
all632 IDs with zero unexpected failures or unexpected passes. All645 enabled CTests pass and three
manual tests remain disabled. Generated coverage is301/329 types,2114/2326 non-field members,
157/753 fields and2999/3205 C routes.

Three failures found while assembling the checkpoint were demo assumptions and are corrected: a
windowless native system may lack desktop-global pointer/cursor control, fixed-step timing can
legitimately issue multiple Updates before a draw, and exact explicit Tick accounting must avoid
fixed-step catch-up. Repeated focused runs and the final complete suites validate the corrections.
Evidence:`build/linux-full/reports/checkpoint-717-complete-*`. This checkpoint used incrementally
rebuilt cna-demos executables against the existing production libraries; `HANDOFF.md` records the
exact hashes and the required next full build. No screenshot, listening test, physical input,
service session or manual visual-quality inspection was performed.

### Remaining XNA type dispositions after focused718

The generated type inventory had28 entries without a bare-type metadata credit after Complete717.
`GraphicsDeviceStatus` was the one genuine metadata omission: `graphics.device.owned_lifecycle`
already compares it with `Normal` after construction and both before and after Reset. Focused718
credits that demonstrated enum type without adding another control or demo. The remaining27 types
are reviewed below; they are not a queue of27 synthetic constructors or enumerator demos.

| Namespace | Types | Semantic disposition |
|---|---:|---|
| `Microsoft.Xna.Framework.Audio` | 3 | `NoAudioHardwareException` and `NoMicrophoneConnectedException` require missing host facilities; the runner already handles the former and the manual microphone workflow truthfully skips without a device. CNA declares `InstancePlayLimitException`, but the production XACT capacity path does not throw it. Direct exception construction would demonstrate no audio behavior. |
| `.Content` | 4 | The collection-item-name, ignore, runtime-type and type-version attributes are nominal serializer wrappers. Focused687 already demonstrates the configurable base attribute plus real XML descriptor collection names, ignored members, runtime types and versions. Wrapper-only construction would add no serialization behavior. |
| `.GamerServices` | 9 | `FriendCollection`/`FriendGamer`, privilege/title-update errors and invite payloads require service identities or framework-created events. `GamerCollection` and its nested enumerator are CLR collection machinery already represented by real local/session gamer ranges. `NetworkException` is a base payload, while `NetworkNotAvailableException` remains an environment refusal caught by existing code but not triggered in the successful local workflows. Synthetic construction would not exercise the service paths. |
| `.Graphics` | 8 | Four nested Model collection enumerators are CLR aliases for ranges already traversed in loaded-model workflows. `DeviceLostException`, `DeviceNotResetException` and `NoSuitableGraphicsDeviceException` require real device/platform failure. `EffectMaterial` requires a compiled-FX material environment unavailable in this build; the C adapter's separate concrete-type demo does not establish the C++ content path. |
| `.Input.Touch` | 1 | `TouchCollection.Enumerator` is the nested CLR spelling for the real range iteration already used by the touch workflow. |
| `.Net` | 1 | `WriteLeaderboardsEventArgs` is service-created callback data and needs a persisted online leaderboard write. |
| `.Storage` | 1 | `StorageDeviceNotConnectedException` requires removable-device loss; directly constructing it would not demonstrate storage behavior. |

Generated coverage is therefore302/329 types, with all27 uncredited types explicitly classified.
This completes the current type-name audit while retaining the truthful environment, service and
compiled-effect gaps. It does not establish comprehensive public behavior coverage.

### Complete719 generated full-build checkpoint

The complete documented build and validation now run against clean CNA `0c83dcc9dc`. Metadata is
632/0, all645 enabled CTests pass in296.77s and three manual tests remain disabled. SOFTWARE/X11
reports558 pass/36 expected fail/35 skip/3 manual; OPENGLES3/X11 reports590/36/3/3. Both cover all
632 IDs with zero unexpected failures or unexpected passes. Generated coverage is302/329 types,
2114/2326 non-field members,157/753 fields and2999/3205 C routes.

Evidence:`build/linux-full/reports/checkpoint-719-complete-*`. The exact binary and report hashes
are recorded in `HANDOFF.md`. No screenshot, listening test, physical input, service session or
manual visual-quality inspection was performed.

### Focused720 GraphicsDevice Present-region behavior

The existing owned-device lifecycle now exercises the three-argument Present overload as a real
renderer contract. Null/null/zero takes the ordinary path. A source rectangle extending beyond the
top-left is validated, clipped and accepted by SOFTWARE; OPENGLES3 refuses the same valid request
with a `NotSupportedException` that names the renderer and source rectangle. A source rectangle
entirely outside the back buffer is an `ArgumentException` under both. A valid destination
rectangle plus foreign window handle is accepted only by the documented HEADLESS contract and is
explicitly refused by both displaying renderers, so neither may silently present the whole frame.

SOFTWARE and OPENGLES3/X11 each pass11 controls, and the exact SOFTWARE CTest passes. This adds
meaningful overload, validation and renderer-refusal evidence while leaving generated member
coverage at2114/2326 because `GraphicsDevice::Present` was already credited. No presented window
pixels, screenshot or manual visual-quality inspection was performed. Evidence:
`/tmp/checkpoint-720-present-regions-{SOFTWARE,OPENGLES3}.json`.

### Focused721 Texture2D binary32 storage and sampling

New `graphics.texture2d.floating_formats` uses the dedicated float, Vector2 and Vector4 SetData/
GetData overloads rather than treating every format as Color bytes. All three values round-trip
exactly, including Vector4 channels above one and below zero. A Vector2 element cannot be written to
the narrower Single format, and the rejected write preserves the prior value. Point-sampled draws
show Single expanding absent G/B/A channels to one, Vector2 expanding absent B/A, and a Color
back buffer clamping Vector4(2.0,0.5,-1.0,1.0) only at final output.

Both primary renderers pass8/8 controls with byte-identical sampled colors; the focused SOFTWARE
CTest passes. Temporary generated coverage stays302/329 types and2114/2326 non-field members while
fields move157->158/753 through the newly named `SurfaceFormat::Vector2`. Evidence:
`/tmp/checkpoint-721-floating-formats-*`. Pixel readback was programmatic; no screenshot or manual
visual inspection was performed.

### Focused722 Texture2D binary16 storage and sampling

The existing `graphics.texture2d.floating_formats` now pairs its binary32 row with HalfSingle,
HalfVector2 and HalfVector4 textures. Exact packed half values survive SetData/GetData, including
the HalfVector4 channels above one and below zero. Point sampling establishes the same missing-
channel expansion as the binary32 formats, while the Color back buffer clamps the out-of-range
four-channel value to `(255,128,0,255)`.

SOFTWARE and OPENGLES3/X11 each pass all15 controls with byte-identical sampled colors, and the
exact SOFTWARE CTest passes. Metadata remains633/0 and all five validator self-tests pass.
Temporary coverage remains633 demos,302/329 types,2114/2326 non-field members and2999/3205 C
routes; fields move158->161/753 through the three newly named half-float SurfaceFormat values.
Evidence:`/tmp/checkpoint-722-half-formats-{SOFTWARE,OPENGLES3}.json` and
`/tmp/checkpoint-722-half-formats-build.log`. Pixel readback was programmatic; no screenshot or
manual visual inspection was performed.

### Focused723 Texture2D normalized integer storage and sampling

New `graphics.texture2d.normalized_formats` keeps all six remaining classic normalized integer
layouts in one coherent texture workflow. NormalizedByte2/4, Rgba1010102, Rg32, Rgba64 and Alpha8
all retain their exact packed SetData/GetData values. Point-sampled output separately establishes
signed negative-channel clamping at the Color target, two- and four-channel expansion, coarse
two-bit alpha, wide unsigned-normalized channels and Alpha8's zero-RGB expansion.

SOFTWARE and OPENGLES3/X11 each pass all10 controls. Their only sampled difference is the expected
halfway signed-alpha rounding of129 versus128, within the declared two-byte tolerance. The exact
SOFTWARE CTest passes, metadata is634/0 and all five validator self-tests pass. Temporary coverage
is634 demos,302/329 types,2114/2326 non-field members,167/753 fields and2999/3205 C routes; all six
new field credits are SurfaceFormat values exercised by actual textures. Evidence:
`/tmp/checkpoint-723-normalized-formats-*`. Pixel readback was programmatic; no screenshot or
manual visual inspection was performed.

### Complete724 runtime checkpoint

The complete documented build and outer-Xvfb validation run against clean sibling CNA
`a37bb07bad`. Metadata is634/0, all647 enabled CTests pass in184.31s and three manual tests remain
disabled. SOFTWARE/X11 is560 pass/36 expected fail/35 skip/3 manual; OPENGLES3/X11 is592/36/3/3.
Both renderer reports contain every catalog ID with zero unexpected failures or unexpected passes.

All632 Complete719 status/knownIssue pairs remain exact on both renderers; the two added Texture2D
IDs pass on both. Generated runtime-backed coverage is302/329 types,2114/2326 non-field members,
167/753 fields and2999/3205 C routes. Evidence:
`build/linux-full/reports/checkpoint-724-complete-*`. No screenshot, listening test, physical
input, service session or manual visual-quality inspection was performed.

### Focused725 Texture2D transfer rejection atomicity

The existing `graphics.texture2d.set_data_rectangle` now covers three coherent failure families in
addition to its real painting workflow. A partly out-of-bounds rectangle is rejected in both
directions before either the intersecting edge texel or a sentinel destination changes. A valid
4x4 region with a logical count of15 is rejected atomically despite physically sufficient caller
storage. A separate disposed texture rejects SetData and GetData with ObjectDisposedException and
leaves the read destination intact.

SOFTWARE and OPENGLES3/X11 each pass all7 controls, and the exact SOFTWARE CTest passes. Metadata
remains634/0, all five validator self-tests pass and generated coverage remains302/329 types,
2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-725-texture2d-rejections-*`. No screenshot or manual visual inspection was
performed.

### Focused726 Texture2D classic stream boundaries

The existing `graphics.texture2d.from_stream` now exercises the third classic image container
named by the public API. A compact 2x2 palette GIF decodes to its exact two transparent and two
opaque RGBA texels, rather than merely returning plausible dimensions. A second stream containing
only the recognized GIF header is deterministically rejected with InvalidOperationException.
Existing PNG original/fit/center-crop behavior remains intact; `save_as_image` retains the separate
PNG/JPEG round trips and DDS keeps its named extension workflow.

SOFTWARE and OPENGLES3/X11 each pass all6 controls, and the exact SOFTWARE CTest passes. Metadata
remains634/0, all five validator self-tests pass and generated coverage remains302/329 types,
2114/2326 non-field members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-726-from-stream-*`. Pixel validation used public GetData; no screenshot or manual
visual inspection was performed.

### Focused727 SpriteBatch source-rectangle pixels

The existing `graphics.sprite_batch.source_rectangle` now uses PointClamp and reads the first
framebuffer. One aggregate check proves that four 32x32 source rectangles select the four authored
atlas colors when enlarged to 128x128. A second proves that a known transparent texel in every
selection leaves DarkSlateGray visible. A third proves that `std::nullopt` draws all four frames in
their original unscaled order.

After correcting an initial compile failure before accepting any report, the rebuilt demo passes
all3 controls on SOFTWARE and OPENGLES3/X11, and the exact SOFTWARE CTest passes. Metadata is634/0,
all five validator self-tests pass and coverage remains302/329 types,2114/2326 non-field members,
167/753 fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-727-source-rectangle-*`. Pixel
validation used public back-buffer readback; no screenshot or manual visual inspection was
performed.

### Complete735 runtime checkpoint

The full documented configure/build and outer-Xvfb validation absorb focused725 through734 against
clean sibling CNA `d23e139de`. Metadata is635/0, all648 enabled CTests pass in156.99s, and both
complete renderer smokes execute every catalog ID with zero unexpected failures or unexpected
passes. SOFTWARE/X11 is561 pass/36 expected fail/35 skip/3 manual; OPENGLES3/X11 is593/36/3/3.

All634 Complete724 status/knownIssue pairs remain exact on both renderers. The sole added ID,
`graphics.sprite_batch.cross_device_texture`, passes on both. Normalized stderr also remains exact.
Generated runtime-backed coverage is635 demos,302/329 types,2114/2326 non-field members,167/753
fields and2999/3205 C routes. Evidence:`build/linux-full/reports/checkpoint-735-complete-*`. No
screenshot, listening test, physical input, service session or manual visual-quality inspection
was performed.

### Focused736 SpriteFont layer depth in the shared queue

`graphics.sprite_batch.layer_depth_sorting` now proves a behavior that sprite-only sorting could
not establish: nonzero DrawString depth places every generated glyph in the same queue as ordinary
sprites. Reversed submission order on two opaque panels makes depth0.25 text visible over a
depth0.5 sprite and makes depth0.75 text disappear behind a depth0.5 sprite under BackToFront.

Both primary renderers pass5/5 and the exact SOFTWARE CTest passes. Metadata remains635/0 and
temporary generated coverage remains302/329 types,2114/2326 non-field members,167/753 fields and
2999/3205 C routes because DrawString was already credited. Evidence:
`/tmp/checkpoint-736-layer-depth-*` and `/tmp/checkpoint-736-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed.

### Focused737 SpriteFont transformed layout pixels

`graphics.sprite_font.transformed_text` replaces unasserted presentation variants with three
compiled-font framebuffer contracts: scalar/Vector2 full-layout bounds,90-degree centre-origin
rotation, and exact horizontal/vertical/combined reflection of asymmetric `LJ` occupancy masks.
The latter distinguishes whole-string layout mirroring from independently flipped glyphs.

Both primary renderers pass3/3 and the exact SOFTWARE CTest passes. Metadata remains635/0 and
temporary generated coverage remains302/329 types,2114/2326 non-field members,167/753 fields and
2999/3205 C routes because the involved members were already credited. Evidence:
`/tmp/checkpoint-737-transformed-text-*` and `/tmp/checkpoint-737-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Focused738 SpriteFont basic multiline pixels

`graphics.sprite_font.draw_string` now has runtime evidence for the basic compiled-font picture.
The exact `Line ` occupancy mask repeats at each LineSpacing interval across CRLF and LF, while a
second aggregate check finds real opaque glyph ink under all four displayed tints. The latter is
scene validation rather than a new tint-semantics credit.

Both primary renderers pass3/3 and the exact SOFTWARE CTest passes. Metadata remains635/0 and
temporary generated coverage remains302/329 types,2114/2326 non-field members,167/753 fields and
2999/3205 C routes. Evidence:`/tmp/checkpoint-738-draw-string-*` and
`/tmp/checkpoint-738-coverage.md`. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. No CNA issue was found.

### Focused739 SpriteBatch equal depth and depth-buffer interaction

The sorting demo now closes both pending table entries. A three-item equal-depth batch produces
XNA's observable unstable reversal, while paired FrontToBack panels prove layerDepth also reaches
the depth buffer: None ends blue by painter order and Default retains nearer red.

Both primary renderers pass7/7 and the exact SOFTWARE CTest passes. Metadata remains635/0 and
temporary generated coverage remains302/329 types,2114/2326 non-field members,167/753 fields and
2999/3205 C routes because the APIs were already credited. Evidence:
`/tmp/checkpoint-739-layer-depth-*` and `/tmp/checkpoint-739-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Focused740 SpriteBatch stock effect and custom-effect disposition

New `graphics.sprite_batch.stock_effect` proves the portable effect-argument behavior omitted by
the prior custom-only scene. AlphaTestEffect Projection and BasicEffect World/Projection place two
sprites in opposite right model-space quadrants; exact black controls distinguish this from the
ordinary SpriteBatch pixel projection. Both Deferred and Immediate paths are present.

SOFTWARE and OPENGLES3/X11 pass2/2, and the exact SOFTWARE CTest passes. Metadata is636/0; all five
validator self-tests and four checkpoint-comparison tests pass. Temporary coverage remains302/329
types,2114/2326 non-field members,167/753 fields and2999/3205 C routes because the APIs were
already credited. The compiled custom-effect branch is dispositioned rather than padded: both
primary compiled-effect build options are OFF, the existing C API fixture checks refusal, and the
GLSL demos already cover executable source effects. Evidence:`/tmp/checkpoint-740-stock-effect-*`
and `/tmp/checkpoint-740-coverage.md`. Pixel readback was programmatic; no screenshot or manual
visual inspection was performed. No CNA issue was found.

### Focused741 ShaderEffect cube binding replacement

`graphics.texture_cube.sampler_ownership` now rebinds a distinct all-white cube to unit1 after the
existing survivor has drawn its six authored faces. The replacement overwrites only half of the
red +X card, so exact adjacent red/white samples prove a new object replaces the already occupied
binding rather than inheriting residual upload or sampler state.

OPENGLES3/X11 passes6/6; SOFTWARE retains its declared CustomEffects skip and its exact CTest is
reported skipped. Metadata remains636/0, the five validator self-tests and four checkpoint tests
pass, and temporary coverage remains302/329 types,2114/2326 members,167/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-741-cube-rebinding-*` and `/tmp/checkpoint-741-coverage.md`. Pixel
readback was programmatic; no screenshot or manual visual inspection was performed. No CNA issue
was found.

### Focused742 ShaderEffect cube-face orientation

The cube sampler's +X face now contains four distinct authored corner texels around a red centre.
Four exact-centre direction vectors follow GLSL's +X mapping and produce
White/Yellow/Cyan/Magenta in uploaded top-left through bottom-right order. The unchanged red centre
and white rebound block keep face selection and binding replacement independently observable.

OPENGLES3/X11 passes7/7; SOFTWARE retains its CustomEffects skip and exact skipped CTest. Metadata
remains636/0, the five validator and four checkpoint tests pass, and temporary coverage remains
302/329 types,2114/2326 members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-742-cube-orientation-*` and `/tmp/checkpoint-742-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Focused743 ShaderEffect cube mip filtering

The cube sampler now carries authored Lime mip1 and Blue mip2 on every face and accepts an explicit
LOD uniform. At LOD1.25, unit1 `LinearMipPoint` selects exact Lime while `PointMipLinear` yields the
predicted RGB(0,191,64) 3:1 blend. This is a stable public sampler-state observation independent of
implicit derivatives; all prior base-face, corner-orientation and rebinding controls remain.

OPENGLES3/X11 passes8/8; SOFTWARE retains its CustomEffects skip and exact skipped CTest. Metadata
remains636/0, the five validator and four checkpoint tests pass, and temporary coverage remains
302/329 types,2114/2326 members,167/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-743-cube-mips-*` and `/tmp/checkpoint-743-coverage.md`. Pixel readback was
programmatic; no screenshot or manual visual inspection was performed. No CNA issue was found.

### Complete744 SpriteBatch, SpriteFont and cube checkpoint

The full636-demo rebuild and runtime checkpoint was made against an isolated clean CNA
`d23e139de` worktree because the live sibling checkout had unrelated external changes. All649
enabled CTests pass, with three manual tests disabled. Complete SOFTWARE and OPENGLES3/Xvfb smokes
execute every ID: SOFTWARE is562 pass/36 expected fail/35 skip/3 manual and GL is594/36/3/3.
Every Complete735 common status, knownIssue value and failed-assertion multiplicity remains exact;
the only new ID is the passing stock-effect SpriteBatch scene. Normalized stderr is byte-identical
on both renderers, and the validator/checkpoint self-tests pass5/5 and4/4.

Generated runtime-backed coverage is636 demos,302/329 types,2114/2326 non-field members,169/753
fields and2999/3205 C routes. The two new enum credits are the cube demo's intentional
`LinearMipPoint` and `PointMipLinear` filters. Full validation exposed and corrected a demo timing
assumption in C error handling and parallel races among four tests that mutate shared desktop
state; neither was a CNA defect. Evidence:`build/linux-full/reports/checkpoint-744-complete-*`.
Pixel checks were programmatic; no screenshot, listening test, physical input, external service
session or manual visual-quality inspection was performed.

### Focused745 Texture3D coordinate and mip filtering

The existing volume sampler now has ten GL controls. Its moved4x4x2 Color chain preserves the
original compilation, ownership, slice-centre, point/linear Z and W-address checks. Four exact U/V
texel centres expose authored White/Yellow/Cyan/Magenta slice-zero orientation. Uniform Lime mip1
and Blue mip2 at explicit LOD1.25 separately prove `LinearMipPoint` selection and
`PointMipLinear` RGB(0,191,64) cross-level interpolation. This closes the documented varying-X/Y
and mip-filter gaps in the same coherent scene.

OPENGLES3/Xvfb passes10/10; SOFTWARE retains its CustomEffects skip and exact skipped CTest. The
incremental build is warning-free at `-j3`, metadata remains636/0, both tool self-test suites pass
and coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-745-texture3d-*`. Pixel readback was programmatic; no screenshot or manual visual
inspection was performed. Non-Color sampling, active-binding moves, cross-device assignment and
device/context loss remain outside this focused claim.

### Focused746 Texture3D raw transfers and active binding

`graphics.texture3d.volume_data` grows from16 to20 controls on each renderer. A2x1x2 Color volume
round-trips arbitrary four-byte trivially-copyable words through offset caller windows, preserving
both guards and X-fastest/slice-major order. Eight-byte values are rejected in both directions
before resource or output mutation. Binding the volume to slot2 rejects an upload; move construction
transfers the active binding to the new wrapper, which still rejects replacement until unbound and
then returns the complete original payload.

SOFTWARE and OPENGLES3/Xvfb pass20/20 with empty stderr, and the exact SOFTWARE CTest passes. The
incremental build is warning-free at `-j3`, metadata remains636/0, tool self-tests pass5/5 and4/4,
and coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Packed and
half-float conversion variants are intentionally dispositioned through the exhaustive Texture2D
format workflows because Texture3D uses the same packed-value serialization paths. Evidence:
`/tmp/checkpoint-746-texture3d-*`. No screenshot or manual visual inspection was performed. No CNA
issue was found.

### Focused747 Texture3D device boundaries and cross-device assignment

`graphics.texture3d.volume_data` grows from20 to25 checks on both renderers. An occupied1x1x1
target bound on the primary device receives a bound2x1x2 source from a secondary HiDef device.
After move assignment it reports the secondary owner, retains exact Red/Green/Blue/Yellow payload,
and refuses upload until the secondary binding is cleared. This proves target replacement, source
binding transfer and cross-device ownership together. Separate constructor controls reject DXT1
volume storage, width257 above the HiDef ceiling and even1x1x1 Color on a Reach device.

SOFTWARE and OPENGLES3/Xvfb pass25/25; the exact SOFTWARE CTest passes and focused stderr is empty.
The `-j3` incremental build is warning-free, metadata remains636/0, tool self-tests pass5/5 and4/4,
and coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Together
with focused745-746, the Texture3D table is semantically closed: repeated box-coordinate guards,
packed/half format variants and reflection/renderer hooks are dispositioned rather than inflated
into duplicate scenes. Evidence:`/tmp/checkpoint-747-texture3d-*`. No screenshot or manual visual
inspection was performed. No CNA issue was found.

### Focused748 TextureCube cross-device lifecycle and EasyGL teardown

`graphics.texture_cube.face_transfers` now executes12 controls. Four additions establish
cross-device copy assignment into an occupied/bound target, old-slot detachment, shared later face
edits, an active slot following move construction, exact retained payload and device-owned wrapper
disposal with atomic refused readback. All four additions pass on SOFTWARE and OPENGLES3.

SOFTWARE passes the complete12/12 workflow. OPENGLES3 passes the ten data/lifecycle assertions but
fails the two existing atlas checks because secondary-device teardown leaves the primary frame
entirely black. A diagnostic Texture3D screenshot after its independent two-device workflow is also
entirely black despite25 passing logical controls, so CNA-DEMOS-ISSUE-086 records the shared EasyGL
lifecycle defect. The build is warning-free at `-j3`; metadata remains636/0, self-tests pass5/5 and
4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-748-texture-cube-final-*`, `/tmp/checkpoint-748-failed-gl.png`
and `/tmp/checkpoint-748-texture3d-gl.png`. The black diagnostic captures were inspected; no
physical-GPU or other-renderer result is claimed.

### Focused749 Texture2D cross-device copy assignment

`graphics.texture2d.copy_ownership` grows from six to ten checks. Copy assignment replaces an
occupied and bound primary wrapper with a2x2 secondary-device source, updates its owner/bounds/data
and clears the old slot. A later partial source edit is visible through the target, while the
target's simple full Color upload detaches only that writing wrapper. Disposing and copy-assigning
the target again revives it as a live, registered, shared secondary wrapper with exact source data.

SOFTWARE and OPENGLES3/Xvfb pass10/10 with empty stderr; the exact SOFTWARE CTest passes. The final
serial incremental build is warning-free and stays below the three-job ceiling; metadata remains
636/0, self-tests pass5/5 and4/4, and coverage remains302/329 types,2114/2326 members,169/753
fields and2999/3205 C routes. The initial `-j3` build succeeded but one concurrent content staging
scavenger emitted a transient warning; immediate serial repetition was clean. Evidence:
`/tmp/checkpoint-749-texture2d-*` and `/tmp/checkpoint-749-coverage.md`. Primary framebuffer checks
were programmatic; cross-device data was read back only. No screenshot or manual visual inspection
was performed, and no CNA issue was found.

### Focused750 RenderTarget2D bound move identity

`graphics.render_target.render_target2d` grows from three to nine controls. A bound 32x16 target
refuses both Dispose and move assignment without changing its live binding, either move operand's
shape/properties or an authored Gold pixel. Move construction from an active 128x128 source then
retargets the device binding to the new wrapper, and move assignment retargets it again to the
unbound destination while transferring the complete target shape. Clearing Orange through that
transferred binding and reading the exact pixel after unbinding proves the renderer follows the
updated public identity.

SOFTWARE and OPENGLES3/Xvfb pass9/9 with empty focused stderr, and the exact SOFTWARE CTest passes.
The final incremental build is warning-free at `-j3`; metadata remains636/0, self-tests pass5/5
and4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205
C routes. Evidence:`/tmp/checkpoint-750-render-target-*` and
`/tmp/checkpoint-750-coverage.md`. Pixel validation used programmatic read-back; no screenshot or
manual visual inspection was performed. No CNA issue was found.

### Focused751 RenderTargetCube bound disposal

`graphics.render_target.render_target_cube` grows from seven to nine controls. Its existing
NegativeZ binding is made active and cleared Gold before public Dispose is attempted. The call is
refused while the target stays live and `GetRenderTargets` retains the exact cube pointer and
NegativeZ face. After unbinding, one exact Gold texel proves the face storage also survived. The
ordinary six-face rendering, preservation and environment-map sampling checks remain intact.

SOFTWARE and OPENGLES3/Xvfb pass9/9, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, self-tests pass5/5 and4/4, and generated
coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-751-render-target-cube-*` and `/tmp/checkpoint-751-coverage.md`. Pixel validation
used programmatic read-back; no screenshot or manual visual inspection was performed. No CNA issue
was found.

### Focused752 RenderTarget2D bound destruction and address reuse

`graphics.render_target.render_target2d` grows from nine to thirteen controls. Destroying an
optional-backed target while bound removes its dangling public binding, while `Present` correctly
remains refused until the caller explicitly unbinds. That transition restores the complete saved
back-buffer viewport. Reconstructing a target in the same optional storage then yields a genuine
new binding with the expected24x12 viewport; a CornflowerBlue clear through it reads back exactly
after unbinding. The prior explicit Dispose and move-identity controls remain intact.

SOFTWARE and OPENGLES3/Xvfb pass13/13, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, self-tests pass5/5 and4/4, and generated
coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-752-render-target-*` and `/tmp/checkpoint-752-coverage.md`. Pixel validation used
programmatic read-back; no screenshot or manual visual inspection was performed. No CNA issue was
found.

### Focused753 ContentManager disposal dispatch and refusal parity

Two existing ContentManager workflows close the documented lifetime gap without adding an ID.
`read_asset_ownership` grows from sixteen to eighteen controls: disposal through `IDisposable`
reaches a derived protected `Dispose(true)` hook, repeated public disposal reaches the override
again while base cleanup stays idempotent, and the established disposed ReadAsset call retains its
exact ObjectDisposedException/no-side-effect result. Seventeen controls pass; only the unchanged
default-recorder assertion fails as CNA-DEMOS-ISSUE-011.

`several_managers` grows from two to three controls and becomes the exact runtime reproduction for
the ContentManager part of CNA-DEMOS-ISSUE-010: disposed `Load<T>` throws std::runtime_error rather
than ObjectDisposedException before any asset read. SOFTWARE and OPENGLES3/Xvfb reproduce both
known issues with the same assertion texts; both exact SOFTWARE CTests pass as expected failures.
The warning-free incremental build uses `-j3`; metadata remains636/0, known-issue demos increase
from38 to39, self-tests pass5/5 and4/4, and generated coverage remains302/329 types,2114/2326
members,169/753 fields and2999/3205 C routes. Evidence:`/tmp/checkpoint-753-*` and
`/tmp/checkpoint-753-coverage.md`. No screenshot or manual inspection was performed, and no new
CNA issue was found.

### Focused754 CNB loader registration precedence and lifetime

`content.custom.cnb_loader` grows from twelve to fourteen controls and becomes `ownProcess` because
custom registrations are process-global. Two different factories are registered under the same
custom ID and canonical name. The registry retains the first factory's owned state, immediately
releases the ignored replacement capture and decodes Moon rather than the replacement marker. Its
first capture remains live after a manager is destroyed and another is created; every manager
still has an independent asset cache. All previous extension, repair and malformed-schema checks
remain intact.

SOFTWARE and OPENGLES3/Xvfb pass14/14, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, self-tests pass5/5 and4/4, and generated
coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-754-cnb-loader-*` and `/tmp/checkpoint-754-coverage.md`. This is in-process binary
content behavior without rendering or screenshots; no manual inspection was performed and no CNA
issue was found.

### Focused755 CNB loader registration guards

`content.custom.cnb_loader` grows from fourteen to seventeen controls. An empty factory, CNA's
built-in Curve identifier, the first reserved identifier and a valid custom identifier paired with
a different canonical name are all refused with `std::invalid_argument`. The installed first
factory capture remains live after every refusal, and the later Moon load through that factory
proves the valid process-global registration remains usable rather than merely allocated.

SOFTWARE and OPENGLES3/Xvfb pass17/17, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, validator/checkpoint self-tests pass5/5 and
4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-755-cnb-loader-*` and `/tmp/checkpoint-755-coverage.md`. This is
in-process binary content behavior without rendering or screenshots; no manual inspection was
performed and no CNA issue was found.

### Focused756 CNJ envelope dispatch and retry

`content.custom.cnj_loader` grows from eleven to fifteen controls. Private documents carrying an
unknown type name or future `cnjVersion` are refused with contextual `ContentLoadException`
messages before the registered factory runs. Rewriting each exact file with a supported envelope
and retrying it through the same manager produces the authored Enemy and increments the factory
once, proving failed loads do not publish cache entries.

SOFTWARE and OPENGLES3/Xvfb pass15/15, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, validator/checkpoint self-tests pass5/5 and
4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-756-cnj-loader-*` and `/tmp/checkpoint-756-coverage.md`. This is
content parsing without rendering or screenshots; no manual inspection was performed and no CNA
issue was found.

### Focused757 CNJ registration guards

`content.custom.cnj_loader` grows from fifteen to eighteen controls. Empty type names and empty
factories are rejected with `std::invalid_argument` before claiming the game-owned C++ type. A
custom CNJ registration for the built-in Texture2D type is rejected with `std::logic_error` before
its unreachable callback runs. The subsequent valid Enemy registration and repaired-path loads
still succeed, proving refused calls leave manager state usable.

SOFTWARE and OPENGLES3/Xvfb pass18/18, and the exact SOFTWARE CTest passes. The warning-free
incremental build uses `-j3`; metadata remains636/0, validator/checkpoint self-tests pass5/5 and
4/4, and generated coverage remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C
routes. Evidence:`/tmp/checkpoint-757-cnj-loader-*` and `/tmp/checkpoint-757-coverage.md`. This is
content registration/parsing without rendering or screenshots; no manual inspection was performed
and no CNA issue was found.

### Focused758 ReadAsset callback and partial-failure ownership

`content.content_manager.read_asset_ownership` grows from eighteen to twenty-one controls. Its
regenerated XNB has two typed tile fields resolved by the custom reader to one disposable identity.
The supplied callback receives both occurrences while the caller deduplicates them into one
ownership slot. Removing the final null-field byte proves a later parse failure preserves that
already delegated resource for explicit cleanup. A callback-thrown sentinel propagates while the
short-read stream and unclaimed object unwind.

SOFTWARE and OPENGLES3/Xvfb each pass20/21; only the unchanged default-recorder assertion fails as
CNA-DEMOS-ISSUE-011, and the expected-failure SOFTWARE CTest passes. The ownership asset
regenerates byte-identically. The warning-free incremental build uses `-j3`; metadata remains636/0,
known-issue demos remain39, validator/checkpoint self-tests pass5/5 and4/4, and generated coverage
remains302/329 types,2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-758-read-asset-*` and `/tmp/checkpoint-758-coverage.md`. This is content parsing and
lifetime validation without rendering or screenshots; no manual inspection was performed and no
new CNA issue was found.

### Focused759 XNB shared-resource disposal recording

`content.xnb.shared_resources` grows from fifteen to seventeen controls. Its existing Item now
implements `IDisposable`, and a derived manager exposes protected ReadAsset with a supplied
recorder. The callback receives the typed inline Item, proving it is installed and functional, but
omits both Items materialized from the type-erased shared-resource table. All established
alias/fixup/cache/failure/lifetime controls remain intact. CNA-DEMOS-ISSUE-087 records the defect.

SOFTWARE and OPENGLES3/Xvfb each pass16/17, and the expected-failure SOFTWARE CTest passes. The
warning-free incremental build uses `-j3`; metadata remains636/0, known-issue demos increase39->40,
validator/checkpoint self-tests pass5/5 and4/4, and generated coverage remains302/329 types,
2114/2326 members,169/753 fields and2999/3205 C routes. Evidence:
`/tmp/checkpoint-759-shared-resources-*` and `/tmp/checkpoint-759-coverage.md`. This is content
parsing and lifetime validation without rendering or screenshots; no manual inspection was
performed.

### Complete disposition of the remaining public C ABI routes

Focused823 compared the641-demo catalog after focused822 with the exact3205 declarations in the
clean `d23e139de` CNA headers. Its original difference was127 routes. Focused1007 repeats the exact
inventory against the current693-demo catalog and the same pinned headers, and corrects one
classification: `cna_content_manager_create_resource` was already executed meaningfully by the CNB
loader-registry workflow. The remaining126 routes retain explicit decisions below; only executed,
meaningful behavior is credited, so generated coverage is3079/3205.

| Class | Count | Exact disposition |
|---|---:|---|
| Fixed type-name reflection | 84 | Fourteen headers contain size/copy pairs returning constant canonical names: `audio.h`8, `graphics.h`2, `graphics_device.h`2, `index_resources.h`2, `media.h`8, `media_library.h`26, `runtime_components.h`4, `runtime_graphics_manager.h`4, `sensors.h`8, `storage.h`2, `texture.h`2, `texture_volume.h`4, `vertex_resources.h`4 and `xact.h`8. Representative value and handle workflows already prove exact sizing, zero-capacity queries, atomic short-buffer refusal and unterminated exact copies. Add one to an existing workflow only when its text is material to that concept; independent controls for all pairs would be count inflation. |
| Explicit test-only hooks | 21 | `core_ext.h` contributes `cna_graphics_renderer_reset_selection_for_tests_ext`. `sensors.h` contributes ten Accelerometer and ten Gyroscope hooks for subsystem-held/connected probes, started-instance registration, forced dispatch, event-watch failure injection, swallowed-exception inspection and disposal cleanup. They support CNA's own unit tests and are not application behavior. |
| Already meaningfully demonstrated | 1 | `cna_content_manager_create_resource` successfully creates the resource-backed manager used by `c_api.content.cnb_loader_registry`, which passes it through repeated custom and built-in loader invocations, observes a live callback-scoped manager and destroys the owner. Embedded-resource loads remain an explicitly unsupported behavior, but that does not make the successful constructor a placeholder. Focused1007 adds the previously missing metadata credit. |
| Manual/platform/service or externally observable production behavior | 21 | These remain real API, but the current automated environment cannot establish their positive behavior honestly. Exact decisions follow below. |

| Header and routes | Decision |
|---|---|
| `devices.h`: `cna_url_launcher_open_ext`, `cna_system_tray_create`, `cna_camera_create` | Success crosses into an external application, native tray UI or physical camera/permission flow. Existing demos cover availability plus in-memory/synthetic tray and camera behavior. Keep real creation/opening manual. |
| `gamer_services.h`: `cna_signed_in_gamer_is_headset`, `cna_guide_delay_notifications` | Headset classification needs a service gamer plus hardware state. Notification delay affects invitations delivered by the online service. Offline calls cannot distinguish the behaviors. |
| `graphics_device.h`: `cna_graphics_device_set_context_recovery_enabled`, `cna_graphics_device_set_string_marker_ext` | Recovery needs an actual renderer-context loss. A string marker is consumed by an external graphics debugger and has no public readback. Calling either without the observer would prove only that the call returned. |
| `input.h`: `cna_gamepad_get_state_with_dead_zone` | A disconnected neutral controller produces the same state for None, IndependentAxes and Circular. The pure analog route already proves all three algorithms; the live overload remains a physical-controller check. |
| `input_devices.h`: the `get_info_at`/name-size/name-copy triples for Sensors and TouchDevices | Bounds and empty-host behavior are already exercised. Positive descriptor evidence requires a host exposing those device classes; no deterministic public fixture exists. |
| `input_joystick.h`: `cna_joysticks_get_info_at` and its name-size/name-copy pair | Positive enumeration and identity need attached joystick hardware. The existing workflow covers descriptor values, disconnected raw state and deterministic connection events without pretending to enumerate a host device. |
| `input_haptics.h`: `cna_haptics_get_id_at` and its name-size/name-copy pair | Positive enumeration and identity need an attached haptic device. The existing workflow covers value types and safe closed-device behavior. |
| `runtime_graphics_manager.h`: `cna_graphics_device_manager_toggle_full_screen` | A real asynchronous fullscreen transition under the available Xvfb desktop timed out and reverted during the earlier audit. Preserve it as platform/manual instead of crediting a setter call without a completed transition. |

This closes the route-inventory decision pass, not CNA behavioral coverage. Hardware success,
service delivery, platform transitions, external-tool observation and many overload/error/renderer
combinations remain valid manual or future-environment work.

### PbrEffect image-based-lighting sampling

New `graphics.pbr_effect.image_based_lighting` supplies a complete `ImageBasedLightEXT` through the
public effect API and observes the rendered result. An incomplete bundle leaves a yellow flat
ambient term active. A complete bundle stores the exact three texture identities, mip count and
intensity, then replaces a deliberately red ambient term with green diffuse irradiance. Reducing
intensity from1 to0.25 changes the exact linear-output green centre from245 to61. With metallic1
and a scale-one/bias-zero BRDF texel, roughness0 samples the red first mip of the prefiltered cube
while roughness1 samples its blue last mip.

OPENGLES3/Xvfb passes all8 controls in three runs with identical reported centre values. The
captured800x480 image was inspected and shows five clean panels matching the programmatic checks.
SOFTWARE remains a declared capability skip because CNA's documented fallback does not sample PBR
lighting. This covers actual rigid PBR environment consumption, not environment preprocessing,
physical-GPU output, HDR values or the parallel skinned shader path.

### SkinnedPbrEffect image-based-lighting sampling

The existing `graphics.model.skinned_pbr` now keeps its original imported-type and bent orange
material controls, then freezes that exact50-degree two-joint pose while installing the same public
environment bundle on every imported `SkinnedPbrEffect`. The bundle round-trips its green
irradiance, black prefiltered-specular cube, BRDF LUT and intensity2. With all direct lights disabled
and linear output selected, the same bent-top pixel changes from `{133,96,54}` to exact green
`{0,107,0}`. This establishes that the skinned PBR shader consumes the environment without losing
its animation pose.

Two OPENGLES3/Xvfb runs pass5/5 with identical output. The captured final frame was inspected and
shows the expected green articulated column. SOFTWARE is a declared fallback skip. This does not
duplicate the rigid demo's mip/intensity matrix or claim physical-GPU behavior.

### PbrEffect dielectric specular factors and maps

New `graphics.pbr_effect.specular_material` uses controlled image-based light to remove diffuse and
directional-light ambiguity. Black irradiance, a white prefiltered cube, a scale-one/bias-zero BRDF
texel and linear output make the quad centre equal to the dielectric Fresnel result times intensity
four. The default IOR1.5 produces neutral `{41,41,41}`; IOR2 produces `{113,113,113}`; factor0
removes the result; a red colour factor produces `{41,0,0}`; alpha128 in the scalar strength map
produces `{20,20,20}`; and a linear blue colour map produces `{0,0,41}`. The demo also verifies all
five factor/map properties round-trip together before rendering.

Two OPENGLES3/Xvfb runs pass8/8 with identical values. The inspected capture visibly separates all
six cases. SOFTWARE remains a declared PBR fallback skip. This proves the public
`KHR_materials_ior`/`KHR_materials_specular` effect path, not glTF import, HDR output, alternate UV
sets/transforms or sRGB decoding; those remain separate behavior decisions.

### PbrEffect dual-UV selection and texture transforms

New `graphics.pbr_effect.texture_coordinates` supplies the renderer's canonical60-byte rigid PBR
record: position, normal, tangent, UV0, UV1 and packed opaque-white colour. Its first state controls
prove that all five texture slots default to UV0 with identity transforms and that changing the
base-color selector and full scale/rotation/offset transform leaves the emissive slot unchanged.

Two point-sampled2x1 maps make the rendered behavior exact. Base colour samples red through UV0
and green through UV1; emissive samples black through UV0 and blue through UV1. For each slot an
authored scale0.5, counter-clockwise90-degree rotation and offset(0.875,0.125) maps UV0(.25,.25)
to UV(.75,.25), reproducing the second texel without changing the other slot. Two OPENGLES3/Xvfb
runs pass8/8 identically. The inspected capture shows red/green/green above black/blue/blue.
SOFTWARE remains a declared fallback skip. This establishes renderer consumption of independent
UV selectors and `KHR_texture_transform` state; sRGB transfer behavior remains the next separate
audit batch.

### PbrEffect texture and output colour spaces

New `graphics.pbr_effect.color_spaces` turns all four colour-space switches into sampled evidence.
An authored gray128 texel remains exact128 with decoding disabled and becomes exact linear55 with
sRGB decoding enabled through each of the base-color, emissive and specular-color paths. The
specular pair uses black irradiance, a white prefiltered cube, scale-one/bias-zero BRDF lookup,
IOR2 and intensity9, making the isolated dielectric result equal to the map sample. The output pair
keeps texture decoding disabled and changes linear128 to sRGB188 while preserving alpha96, proving
that final encoding affects RGB only. Default-enabled and disabled round-trip state is also checked.

Two OPENGLES3/Xvfb runs pass10/10 with identical values. The inspected capture shows three matching
128/55 pairs and the final128/188 pair. SOFTWARE remains a declared fallback skip. This proves the
transfer controls in the rigid shader on an 8-bit target; it does not claim HDR, physical-GPU or
parallel skinned-shader output.

### PbrEffect normal scale and occlusion strength

The existing `graphics.pbr_effect.texture_maps` now makes the two remaining scalar map controls
observable. Its authored normal bumps yield green189 on the light-facing side and62 on the opposite
side at scale one; scale zero makes the same samples132/132, matching the flat basis. Ambient-only
occlusion yields194 in an open texel and82 in a groove at strength one; strength zero makes both
exact194. Default-one and independent fractional state round-trips precede the draw.

Two OPENGLES3/Xvfb runs pass8/8 identically. The inspected capture shows the expected relief and
grooves beside their uniform zero-factor counterparts. SOFTWARE remains a declared fallback skip.
No new ID was needed because the controls refine the exact maps already demonstrated here.

### PbrEffect specular-map UV selection and transforms

The existing `graphics.pbr_effect.specular_material` now supplies CNA's canonical dual-UV rigid
vertex and samples both `KHR_materials_specular` maps through their independent selectors and
transforms. The scalar map's UV0 alpha128 gives neutral20; UV1 alpha64 gives exact10, and the full
scale/rotate/offset transform maps UV0 to the same10 texel. The colour map's UV0 blue gives
`{0,0,41}`; UV1 bright green gives `{0,41,0}`, and its separate transform reaches that same green.
The two aggregate state controls also cover selector/transform defaults and exact mutations.

Two corrected OPENGLES3/Xvfb runs pass12/12 identically, and the ten-panel capture was inspected.
The first draft's named `Color::Green` was XNA's128-channel web colour, so its observed20 was right;
changing the fixture to explicit255 restored the intended full-green41 oracle. This was a demo-data
error, not a CNA defect. SOFTWARE remains a declared fallback skip. Together with the base/emissive
coordinate demo, all seven public PBR texture slots now have sampled selector/transform evidence.

### PbrEffect vertex colour, alpha coverage and double-sided shading

New `graphics.pbr_effect.material_surface` uses the canonical60-byte record to exercise the final
glTF material-surface controls. With white ambient and base colour, a blue `COLOR_0` yields white
when disabled and exact blue when enabled. Its alpha128 reaches `BlendState::NonPremultiplied` and
produces blue128 over black. Mask mode discards red alpha64 below cutoff0.5, retains red alpha192,
and retains alpha64 after the cutoff moves to0.2. A real back face under oblique rear lighting stays
black with one-sided shading and becomes exact green249 when double-sided state reverses the whole
basis. Defaults and combined property mutations also round-trip.

Two final OPENGLES3/Xvfb runs pass10/10 identically, and the capture was inspected. Early test
iterations used a zero view/light half-vector and then the wrong winding assumption for XNA's
viewport convention; correcting the demo oracle resolved both without a CNA issue. SOFTWARE remains
a declared fallback skip. The demo proves rigid PBR behavior; the equivalent skinned fields share
the same shader terms and already have actual animated/IBL coverage, so duplicate panels require a
separate reason rather than automatic demo-count growth.

### PbrEffect and SkinnedPbrEffect public-surface disposition

The pinned d23e139 headers expose113 public declarations on `PbrEffect` and118 on
`SkinnedPbrEffect`, including duplicate field/accessor forms and the public renderer bridge. Every
declaration now has a semantic disposition:

| Public area | Disposition |
|---|---|
| Construction, type identity, `Clone`, matrices and `Apply`/draw lifecycle | Covered by the two C state workflows and the actual rigid/skinned draws. The rigid clone's four lost surface fields remain CNA-DEMOS-ISSUE-070; the skinned clone preserves material, palette and texture ownership. |
| Diffuse/alpha, ambient/directional lights, required lighting, fog and default lighting | State/default/error behavior is covered in C; directionals and ambient are sampled in several PBR scenes. Material `Alpha` and fog still need pixel evidence. The three public `DirectionalLight` fields duplicate the property accessors and do not merit separate panels. |
| Seven maps, all material factors, map strengths, UV selectors/transforms and colour spaces | Every property and invalid selector boundary has state coverage; all seven slots and every factor family now have rigid sampled evidence. |
| `SetOwned*` for all seven maps | The C texture-slot workflow exercises the same C++ owned setters: every slot retains an exact resource across source destruction/clone and releases it after the final reference clears. Repeating the convenience spelling in C++ would not add behavior. |
| Alpha mode/cutoff, double-sided and vertex colour | Rigid sampled coverage is complete. Shared shader terms are credited to the skinned sibling; broad identity-bone duplication is intentionally omitted. |
| Skinned weights, `MaxBones`, bone set/get and palette ownership | Covered at all1/2/4 modes and the complete72-entry boundary in C, with actual two-joint animated deformation in C++. |
| Image-based light | Rigid and skinned programs both have actual irradiance/BRDF observations; invalid fallback and stored bundle state are covered. |
| Directional shadows and punctual light | Detailed receiver semantics exist on `SkinnedEffect`, but neither separate PBR renderer program is executed by those scenes. Add compact rigid-plus-skinned branch evidence rather than duplicate the entire receiver matrix. |
| `GetTypeName`, protected `OnApply`, `FillGpuDrawParams` | Type names are generic reflection. Apply and the public-but-internal renderer bridge are transitively exercised by every draw and do not justify direct internal-structure demos. |

This closes the disposition pass, not the PBR family. The next meaningful batch is rigid-plus-skinned
directional-shadow and punctual-light reception, followed by material-alpha and fog pixels folded
into existing scenes where that remains clear.

### Rigid and skinned PBR shadow-receiver branches

New `graphics.pbr_effect.shadow_receivers` supplies branch evidence for both PBR renderer programs.
Its rigid row translates through World and its skinned row through one full-weight bone, putting the
same quad at Z=-3. Each row compares unshadowed and authored-map reception for a directional light
and a point light. The1x1 depth map and every face of the1x1 distance cube store64/255, below the
receiver's normalized depth/distance0.5, so direct light is removed while ambient and emission
remain. Rigid and skinned values match exactly: directional lit `{167,125,74}`, point lit
`{255,207,115}`, and shadowed floor `{20,28,26}`.

Two OPENGLES3/Xvfb runs pass all six controls identically and the capture was inspected. SOFTWARE
retains the declared EasyGL capability skip. The existing `SkinnedEffect` receiver scenes remain the
detailed evidence for PCF radii, bias, detachment, cascades, spot cones and range boundaries; this
scene establishes that the distinct rigid/skinned PBR programs consume the already-audited receiver
model. Material `Alpha` and fog are now the PBR family's remaining sampled gaps.

### PbrEffect material alpha

The existing `graphics.pbr_effect.material_surface` now makes the material-level `Alpha` multiplier
observable alongside its vertex-alpha coverage. With straight-alpha compositing over black,
`Alpha=0.5` changes an opaque blue vertex to exact blue128. Keeping the material factor and changing
the vertex to alpha0.5 produces exact blue64, proving the two factors multiply before compositing.
Default-one and an authored0.25 value also round-trip in the existing aggregate state controls.

Two OPENGLES3/Xvfb runs pass all twelve controls with identical values and the expanded capture was
inspected. SOFTWARE retains its declared PBR capability skip. No new ID was needed because material
alpha belongs to the surface-mode workflow. Sampled fog through both PBR programs is now the sole
remaining behavioral gap from the public-surface disposition.

### Rigid and skinned PBR fog depth

New `graphics.pbr_effect.fog_depth` closes the last sampled gap from the PBR public-surface audit.
One rigid World transform and one full-weight bone independently place the same red quad at view
depths2,4 and6. Disabled fog leaves all six pixels exact red. Enabling a blue fog colour over the
2..6 interval yields exact red, `{128,0,128}` and exact blue in each row, proving both PBR programs
consume post-transform view depth and linearly interpolate the complete authored interval.

Two OPENGLES3/Xvfb runs pass all ten controls identically and the capture was inspected. SOFTWARE
retains its declared EasyGL capability skip. Together with the state/ownership workflows, rigid
material scenes, animated skinned/IBL model and receiver branch scene, every public PBR declaration
now has either meaningful behavior evidence or an explicit intentional disposition on d23e139.
ISSUE-070 and the documented renderer limitations remain visible; no comprehensive CNA-wide claim
is implied.

### Complete859 generated runtime checkpoint

Focused845-858 are now included in a complete project build and both full smoke suites. Generated
coverage is655 demos,302/329 types,2114/2326 non-field members,169/753 fields and3078/3205 C
routes. SOFTWARE reports563 pass/46 expected fail/43 skip/3 manual; OPENGLES3 reports602/47/3/3,
both across every catalog ID with no unexpected result. All668 enabled CTests pass and three manual
tests remain disabled. Complete844 comparisons add only the reviewed Model/animation IDs, check
growth and ISSUE-031/092-096 witnesses.

This runtime checkpoint does not turn the declaration-level member disposition into comprehensive
behavior coverage. The next concrete recorded gap is `CurveKeyCollection` duplicate-position
insertion and indexed replacement ordering/atomicity; it belongs in the existing focused workflow
rather than a new demo ID.

### Focused860 CurveKeyCollection duplicate and replacement boundaries

The existing `math.curve.key_collection` workflow now covers stable insertion of three keys at one
position, exact-position replacement between duplicate neighbours, sorted reinsertion after a
position change and atomic refusal of both invalid index directions. This closes the recorded
duplicate and ordinary replacement behavior gap without inventing another demo ID.

The nearby-position boundary exposes CNA-DEMOS-ISSUE-097. Replacing position zero with
`0.75 * MachineEpsilonFloat` while the next key is at `0.5 * MachineEpsilonFloat` assigns in place
and leaves the collection descending. The implementation's epsilon shortcut therefore contradicts
the public promise that indexed replacement preserves position ordering. Both renderer-labelled
runs pass20/21 with byte-identical output and empty stderr; the exact expected-failure CTest passes.
Temporary generated coverage is numerically unchanged because every named member already had
declaration credit. Evidence:`/tmp/checkpoint-860-curve-key-collection-*` and
`/tmp/checkpoint-860-coverage.md`. No rendering or visual inspection was involved.

### Focused861 XACT ReleaseTime RPC state transition

The explicit later candidate left by Focused816 is now closed in the existing
`audio.xact.cue_states` workflow. A generated settings bank binds a three-point volume RPC to the
built-in `ReleaseTime` variable, and a simple cue references it without carrying any authored fade.
Its public state enters `Stopping` after `Stop(AsAuthored)` and reaches `Stopped` after the RPC's
150ms domain while the engine is pumped. This distinguishes the RPC-only release branch from both
immediate stop and the already demonstrated500ms authored cue fade.

Both renderer-labelled runs pass23/23 with empty stderr, and all ten SOFTWARE XACT CTests pass.
Temporary generated coverage is numerically unchanged because the involved Cue members already
had declaration credit. Evidence:`/tmp/checkpoint-861-xact-release-rpc-*` and
`/tmp/checkpoint-861-coverage.md`. This is public transport-state evidence only; volume, envelope
shape, samples and audible output remain outside the claim.

### Focused862 morph-target caller-managed bounds

The existing `graphics.model.morph_targets` workflow closes the recorded runtime morph-bound
update gap. Public blended vertex bytes place the taller target outside the imported base-pose
sphere. `SetMorphWeightsEXT` updates weights without changing that caller-managed bound; assigning
`BoundingSphere::CreateFromPoints` over every stretched position to
`ModelMesh::BoundingSphere` immediately expands `Model::BoundingSphereEXT` around them.

The first oracle asked strict `BoundingSphere::Contains` to accept every generated boundary point.
Float rounding classified one point as `Disjoint` even though its printed distance equalled the
radius. The final geometric radius-plus-`1e-5` test matches CNA's own `CreateFromPoints` enclosure
tests and the already documented numerical-boundary disposition, so this was a demo correction and
not a CNA issue. Both Xvfb renderer runs pass9/9 with byte-identical output and empty stderr; the
exact SOFTWARE CTest also passes. Temporary coverage remains numerically unchanged because both
named properties already had declaration credit. Evidence:`/tmp/checkpoint-862-*`. No screenshot
or manual visual inspection was performed.

### Focused863 same-root content compiler lease

The existing `content.custom.multi_artifact_publication` workflow now has seventeen child controls.
A bounded gate holds one `RunContentCompiler` invocation inside its importer, after the public
coordinator has acquired the destination lease. A concurrent in-process invocation for the same
output root returns failure without reaching the writer or publishing either CNB, the deployment
file or the ownership manifest. The owner publishes the complete set after release, and a third
unchanged invocation skips without another writer call.

SOFTWARE and OPENGLES3 renderer-labelled runs pass with the expected publication, active-lease and
graph-conflict diagnostics and byte-identical normalized stdout/stderr. The exact SOFTWARE CTest
passes five consecutive repetitions. Temporary generated coverage remains655 demos,302/329 types,
2114/2326 members,169/753 fields and3078/3205 C routes. This is meaningful same-process contention
evidence through the public compiler entry point; it does not claim independent-process behavior
or crash consistency. Evidence:`/tmp/checkpoint-863-*`. No screenshot or visual inspection was
performed.

### Focused864 concurrent CNB loader registration

The isolated `content.custom.cnb_loader` workflow now covers the process-wide registry's documented
thread-safe accessor contract through the public templated entry point. Eight threads released by
one gate register distinct owned factory states for the same custom ID and canonical name. All
calls complete, equivalent duplicates preserve exactly one capture, and a generated CNB carrying
payload77 is decoded exactly once by that retained winner.

Both renderer-labelled runs pass25/25 with byte-identical normalized stdout and empty stderr. The
exact SOFTWARE CTest passes twenty consecutive repetitions. This adds concurrent first-wins
behavior to the existing serial replacement/lifetime and collision checks without adding an ID.
Temporary generated coverage remains655 demos,302/329 types,2114/2326 members,169/753 fields and
3078/3205 C routes. Evidence:`/tmp/checkpoint-864-*`. No renderer, screenshot or visual inspection
was used.

### Focused865 nested source-root capability propagation

The existing `content.custom.nested_builds` workflow grows from twenty to twenty-three child
controls. Its stage now names an original Texture2D CNJ whose sidecar is `@art/checker.png`.
Without a parent grant, the nested BuildAsset reports the inner `CNA.CnjImporter` unknown-alias
failure through the outer stage processor. Granting the parent a disjoint textures root propagates
the capability automatically and produces one exact checker Texture2D CNB. The merged source
dependency retains alias `art`, while the parent records the same child name and Texture2D type in
its XREF and runtime reference.

The first diagnostic predicate omitted CNA's word `alias`; correcting it was a demo oracle repair,
not a CNA issue. Both renderer-labelled runs pass23/23 with byte-identical normalized stdout/stderr,
and the exact SOFTWARE CTest passes five consecutive repetitions. Temporary generated coverage
remains655 demos,302/329 types,2114/2326 members,169/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-865-*`. This is CPU-side compilation and byte inspection; no renderer, screenshot
or visual inspection was used.

### Focused866 runtime model-mesh reparenting

The existing `graphics.model.bones` workflow now reassigns the loaded Forearm mesh through public
`ModelMesh::ParentBone`. Moving it from the authored Forearm parent to bone zero immediately changes
`Model::BoundingSphereEXT`; assigning a null parent produces the same aggregate bound, matching
`Model::Draw`'s documented bone-zero fallback. Restoring the authored parent restores the exact
original aggregate sphere. Together with Focused862's caller-managed morph-bound update, this
closes the recorded runtime mesh parent/bound mutation row without adding a catalog ID.

SOFTWARE and OPENGLES3 Xvfb runs pass 15/15 with byte-identical stdout and empty stderr, and the
exact SOFTWARE CTest passes. The `-j3` incremental build, metadata 655/0 and both tool self-test
suites pass; temporary coverage remains 302/329 types, 2114/2326 members, 169/753 fields and
3078/3205 C routes. Evidence: `/tmp/checkpoint-866-*`. These are numeric public-state assertions;
no screenshot or manual visual inspection was performed.

### Focused867 conservative full-skin-palette bounds

The existing `graphics.model.skin_weight_import` workflow now reads every public vertex of its four
placements and proves that joint four has no nonzero influence. With that joint at identity the
aggregate model-bound radius is 1.037; moving only the unused joint to X=8 grows it to 4.857. All
existing draw checks still pass at their analytically predicted positions, demonstrating the public
contract's deliberate safe overbound across the complete active palette rather than a stale bound
or accidental vertex motion.

Both Xvfb renderer runs pass 24/24 with byte-identical stdout and the same four expected importer
warnings on stderr; the exact SOFTWARE CTest passes. The `-j3` incremental build, metadata 655/0
and both tool self-test suites pass; temporary coverage remains 302/329 types, 2114/2326 members,
169/753 fields and 3078/3205 C routes. Evidence: `/tmp/checkpoint-867-*`. No capture or manual
visual inspection was performed in this batch; Focused839 retains the scene's inspected pixel
evidence.

### Focused868 external source-root symlink containment

The existing `content.custom.source_root_capabilities` workflow now creates a real file symlink
inside its granted `art` root that targets the different primary source root. A CNJ reference to
that apparently contained alias path is refused during import after canonical resolution, with the
exact source, logical asset, importer and outside-root diagnostic. The valid recipe still rebuilds
afterward, separating containment refusal from persistent pipeline damage.

Both displayless renderer-labelled runs pass 11/11 with byte-identical path-normalized stdout and
empty stderr; the exact SOFTWARE CTest passes. The `-j3` incremental build, metadata 655/0 and both
tool self-test suites pass; temporary coverage remains 302/329 types, 2114/2326 members, 169/753
fields and 3078/3205 C routes. Evidence: `/tmp/checkpoint-868-*`. The probe ran on Linux; metadata
explicitly states that platforms unable to create symlinks report that one control as unavailable.
No renderer, screenshot or visual inspection was used.

### Focused869 configured external-root fingerprint stability

The existing `content.custom.incremental_compiler` now resolves its shared multiplier through the
`shared` root declared in the conventional `.cna-content.json`. Its real manifest records the
alias-relative dependency, and changing those external bytes rebuilds both dependent score nodes.
After all earlier invalidation and source-retirement controls, remapping `shared` from `shared-a` to
`shared-b` with identical bytes enters no importer, processor or writer, preserves both outputs and
keeps the manifest byte-identical. The workflow now has twenty-one internal controls.

Both renderer-labelled standalone runs pass with byte-identical path-normalized stdout/stderr, and
the exact SOFTWARE CTest passes. The three stderr lines are the deliberate invalid-import and
guarded-clean diagnostics. The `-j3` incremental build, metadata 655/0 and both tool self-test
suites pass; temporary coverage remains 302/329 types, 2114/2326 members, 169/753 fields and
3078/3205 C routes. Evidence: `/tmp/checkpoint-869-*`. No renderer, screenshot or visual inspection
was used.

### Focused870 pre-built registry service-option refusal

The existing `content.custom.incremental_compiler` now first calls the public pre-built-registry
`RunContentCompiler` overload with `--fx-compiler`. It returns command-line status 2, names the
`ContentPipelineRegistryFactory` overload that can apply the option, and enters no game importer,
processor or writer. The same process then completes every established factory-based build,
fingerprint and cleanup control, bringing the workflow to twenty-two internal checks.

Both renderer-labelled standalone runs pass with byte-identical path-normalized stdout/stderr, and
the exact SOFTWARE CTest passes. Stderr gains the expected one-line service-option refusal before
the existing invalid-import and guarded-clean diagnostics. The `-j3` incremental build, metadata
655/0 and both tool self-test suites pass; temporary coverage remains 302/329 types, 2114/2326
members, 169/753 fields and 3078/3205 C routes. Evidence: `/tmp/checkpoint-870-*`. No renderer,
screenshot or visual inspection was used.

### Complete871 generated runtime checkpoint

Focused860-870 are now included in a warning-free complete project build, all668 enabled CTests and
both 655-ID smoke suites. SOFTWARE reports562 pass/47 expected fail/43 skip/3 manual; OPENGLES3
reports601/48/3/3, with no unexpected result. Complete859 comparison finds the six intended
in-process check-count increases and only the reviewed CurveKeyCollection ISSUE-097 failed assertion
and status change. The standalone compiler/publication controls also pass through their complete
CTest entries. Generated coverage remains302/329 types,2114/2326 non-field members,169/753 fields
and3078/3205 C routes. Evidence:`build/linux-full/reports/checkpoint-871-complete-*`.

This checkpoint confirms the recorded behavior and dispositions; it does not turn declaration
coverage into a comprehensive claim. Continue with the next explicit public-behavior gap.

### Focused872 concurrent disjoint registry factories

The existing `content.custom.incremental_compiler` adds three controls around two synchronized
public registry-factory invocations. Both calls demonstrably overlap inside one shared factory,
receive different `--fx-compiler` values, return distinct registries that CNA freezes, persist
different writer versions in their real manifests and publish the exact independent values10 and21
under disjoint roots. The complete workflow now passes25/25 under both displayless renderer labels
with byte-identical path-normalized stdout/stderr; the exact SOFTWARE CTest passes twenty repeated
runs. The warning-free `-j3` build, metadata655/0 and both self-test suites pass. Coverage remains
302/329 types,2114/2326 non-field members,169/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-872-*`. No dynamic-plugin, shared-root, independent-process, built-in-service or
crash-consistency claim is made; no renderer, screenshot or visual inspection was used.

### Focused873 independent-process output lease

The existing `content.custom.multi_artifact_publication` adds three controls around a real child
process. Its importer writes a ready handshake only after the public compiler owns the destination
lease, then blocks before its writer. A parent-process compiler call for that same root is refused
without entering its writer or observing any published artifact. After a release handshake, the
child publishes both CNBs, the support file and manifest; the parent then acquires the released
lease and skips the unchanged node. The workflow now passes20/20 in serial SOFTWARE and OPENGLES3
runs with byte-identical path-normalized output, and its exact SOFTWARE CTest passes twenty
repetitions. An initial simultaneous launch of both renderer-labelled parents also passes but
reproduces CNA-DEMOS-ISSUE-057's staging-scavenger missing-owner warning in one process; serial
evidence has only the expected eight diagnostic lines. The warning-free `-j3` build, metadata655/0
and both self-tests pass. Coverage remains302/329 types,2114/2326 non-field members,169/753 fields
and3078/3205 C routes. Evidence:`/tmp/checkpoint-873-*`. No crash-consistency, renderer, screenshot
or visual claim is made.

### Focused874 stale output-lease recovery

The same multi-artifact workflow adds a deliberate child-process `_Exit(86)` from its importer,
after lease and staging acquisition but before its writer. The parent observes the crash marker, a
regular persistent output lock and no published artifact. A following public compiler call claims
the unlocked stale lease and publishes both CNBs, deployment file and manifest; the demo removes
the child's isolated staging root. All22 controls pass under both displayless labels with byte-
identical path-normalized output, and the exact SOFTWARE CTest passes twenty repetitions. The
warning-free `-j3` build, metadata655/0 and both self-tests pass; coverage remains302/329 types,
2114/2326 non-field members,169/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-874-*`. This is pre-write process-exit recovery, not a claim about interruption
during multi-file publication; no renderer, screenshot or visual inspection was used.

### Focused875 mid-publication crash disposition

The public content compiler exposes no callback or barrier between its sequential per-file atomic
replacements. The deterministic blocked-deployment control already proves the observable contract:
earlier CNBs may be new while a later support destination and the last successful manifest remain
old, and retry publishes the complete current set. A real process kill inside that narrow interval
needs CNA-internal fault injection to avoid a scheduling race, so it is intentionally not added as
a demo. The documentation-only `-j3` build, metadata655/0, both self-tests and byte-identical
coverage regeneration pass. Evidence:`/tmp/checkpoint-875-*`. No runtime or visual claim was added.

### Focused876 nested picture-album tree

The isolated save-picture workflow adds an ordinary two-level `Trips/Day One` folder tree and lets
a fresh public `MediaLibrary` discover it. Exact parent identities, one local picture per album,
Picture-to-Album back-references, decoded640x320 dimensions and caller-owned streams containing the
original PNG bytes all pass. The workflow now executes27 controls under each renderer label:26 pass
and only the established ISSUE-041 short-read assertion fails. Normalized output is byte-identical,
stderr is empty and the exact known-issue SOFTWARE CTest passes twenty repetitions. The clean `-j3`
build, metadata655/0, both self-tests and byte-identical coverage regeneration pass; counts remain
302/329 types,2114/2326 non-field members,169/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-876-*`. Image bytes and dimensions were inspected programmatically; no screenshot
or visual inspection was used.

### Focused877 same-name picture replacement

`media.media_library.save_picture` now repeats the original name with the valid128x64 thumbnail
bytes. The physical file count remains fixed, while the live library appends a distinct snapshot
view. Both Picture objects share path-based equality and token identity but retain their respective
640x320 and128x64 creation-time dimensions. A fresh library resolves that token to one128x64
Picture whose caller-owned stream reproduces the replacement bytes. This closes the explicit
same-name-save gap without a new ID. The workflow executes30 controls per renderer label:29 pass
and only established ISSUE-041 fails. The exact known-issue SOFTWARE CTest passes twenty
repetitions. The clean incremental `-j3` build, metadata655/0, both tool self-tests and byte-
identical coverage regeneration pass; counts remain302/329 types,2114/2326 non-field members,
169/753 fields and3078/3205 C routes. Evidence:`/tmp/checkpoint-877-*`. No screenshot or visual
inspection was used.

### Focused878 already-small Picture thumbnail

The fresh128x64 replacement Picture now supplies two `GetThumbnail` calls. Their stream pointers are
distinct and caller-owned, while both byte sequences exactly match the already-small PNG. This
proves the no-upscale/no-reencode branch and per-call ownership without another ID. The workflow
executes31 controls under each renderer label:30 pass and only established ISSUE-041 fails; stdout
and stderr are byte-identical. The exact known-issue SOFTWARE CTest passes20 consecutive runs. The
first full `-j3` build succeeds with a tracked ISSUE-057 staging-race diagnostic, and its immediate
unchanged retry is clean. Metadata validates655/0, both validator self-test suites pass, and the
byte-identical coverage regeneration remains302/329 types,2114/2326 members,169/753 fields and
3078/3205 C routes. Evidence:`/tmp/checkpoint-878-*`. No new CNA defect, screenshot or visual
inspection is claimed.

### Focused879 corrupt Picture thumbnail fallback

The fresh replacement Picture's public path token now supplies a deterministic external mutation:
the workflow overwrites the indexed file with twelve invalid image bytes, then proves
`GetThumbnail` returns a caller-owned stream over those exact current bytes when decoding fails.
This closes the corrupt-source fallback without another ID or a decoded-image claim. The first
fixture failure wrote an assumed filename; source review showed SavedPictureStore appends a sniffed
extension, so the corrected test uses the public token and records no CNA defect. Both renderer
labels execute32 controls:31 pass and only established ISSUE-041 fails, with byte-identical stdout
and stderr. The exact known-issue SOFTWARE CTest passes20 consecutive runs. The clean incremental
`-j3` build, metadata655/0, both validator self-test suites and byte-identical coverage regeneration
pass; counts remain302/329 types,2114/2326 members,169/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-879-*`. No screenshot or visual inspection was used.

### Focused880 SavePicture stream-boundary disposition

Pinned CNA `d23e139de` allocates the public stream's total Length, makes one Read from its current
position and ignores the returned count. Nonzero position and premature EOF therefore reduce to
the same ISSUE-041 mechanism already demonstrated by the valid short-read fixture; another failing
assertion would add no distinct behavior. SharpRuntime `f2aecfefe` makes Length pure virtual and
defines it as total bytes, so unknown length has no supported sentinel; a throwing implementation
is a propagated capability refusal. Later CNA `a83673fc5` loops to Length/EOF and shrinks to the
received bytes, but that source-reviewed fix remains outside the pinned runtime baseline. No demo,
metadata or coverage credit changes. The warning-free incremental `-j3` build, metadata655/0, both
validator self-test suites and byte-identical coverage regeneration pass; counts remain302/329
types,2114/2326 members,169/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-880-*`. No runtime, screenshot or visual validation was used.

### Focused881 disposed Picture/PictureAlbum refusal

The save-picture workflow now corrects its former expectation that disposed views remain readable.
XNA calls `ThrowIfDisposed` from the ordinary Picture and PictureAlbum public surface; pinned CNA
only sets each IsDisposed flag. After a passing repeated-disposal and sibling-isolation control, the
demo invokes twelve Picture properties/streams/value operations and nine parallel PictureAlbum
operations. Both renderer labels observe0/12 and0/9 exact ObjectDisposedException refusals, so two
grouped ISSUE-098 assertions fail alongside established ISSUE-041 and31/34 controls pass. Metadata
names ISSUE-098 as the primary expected failure. The exact known-issue SOFTWARE CTest passes20
consecutive runs. The warning-free full incremental `-j3` build, metadata655/0, both validator
self-test suites and byte-identical coverage regeneration pass; counts remain302/329 types,
2114/2326 members,169/753 fields and3078/3205 C routes. Evidence:`/tmp/checkpoint-881-*`. No
screenshot or visual inspection was used.

### Focused882 Album art selection and fallbacks

The album-art standalone grows8->12 controls without another ID. Its formerly bare album receives
distinct mixed-case cover and folder images. Exact GetAlbumArt bytes establish case-insensitive
matching and cover-before-folder priority. Two distinct GetThumbnail streams preserve the selected
already-small64x64 PNG exactly, and SavePicture publicly decodes it to64x64. After external
corruption of the selected cover, HasArt remains true while GetAlbumArt and GetThumbnail both return
the exact current twelve bytes. This closes file-based small/corrupt thumbnail behavior; embedded
art remains a separate branch.

Both renderer-labelled direct runs pass12/12 with byte-identical stdout and stderr; stderr contains
the same ten reviewed TitleContainer INFO lines. The exact SOFTWARE CTest passes20 consecutive
runs. The warning-free full incremental `-j3` build, metadata655/0, both validator self-test suites
and byte-identical coverage regeneration pass; counts remain302/329 types,2114/2326 members,
169/753 fields and3078/3205 C routes. Evidence:`/tmp/checkpoint-882-*`. No new CNA defect,
playback, screenshot or visual inspection is claimed.

### Focused883 embedded Album art

The original music-fixture generator now emits a deterministic tagged FLAC whose unique album
carries the existing128x128 gradient PNG in a PICTURE block. A second generation pass preserves
its exact hash and leaves all older tracks unchanged. The album-art workflow grows12->14 controls:
without folder art, GetAlbumArt extracts the exact embedded PNG, two distinct GetThumbnail streams
retain those bytes, and SavePicture decodes them as128x128. A fresh scan after adding a folder cover
proves that file takes precedence over the member track's embedded image.

Both renderer-labelled direct runs pass14/14 with byte-identical stdout and stderr; stderr contains
the same fourteen reviewed TitleContainer INFO lines. The exact SOFTWARE CTest passes20 consecutive
runs. The warning-free full incremental `-j3` build, metadata655/0, both validator self-test suites
and byte-identical coverage regeneration pass; counts remain302/329 types,2114/2326 members,
169/753 fields and3078/3205 C routes. Evidence:`/tmp/checkpoint-883-*`. No new CNA defect,
playback, screenshot or visual inspection is claimed.

### Focused884 disposed music entity refusal

The existing `music_index` workflow now distinguishes successful Dispose flagging from the public
post-disposal contract. It checks 13 ordinary Album operations and eight each on Artist, Genre and
Playlist, including properties, value operations and Album's two image methods. Pinned CNA throws
ObjectDisposedException for none of the 37 operations, so four grouped failures extend ISSUE-098
from Picture/PictureAlbum to all four music entity views. The two renderer-labelled runs are
byte-identical and retain 29 passing relationship, duration, identity, collection and library
controls. This is direct behavior evidence; post-destruction borrowed pointers remain excluded.
Evidence: `/tmp/checkpoint-884-*`.

### Focused885 disposed music collection and library refusal

Windows XNA 4.0 IL confirms that Count, Item and GetEnumerator on each music collection guard
against disposal. It also guards all nine MediaLibrary properties, GetPictureFromToken and both
SavePicture overloads, while its Dispose leaves previously obtained children alive. The existing
`music_index` workflow now separates those contracts. Retained children stay valid, but each
disposed collection produces 0/2 exact refusals: Count reads zero and Item takes the wrong bounds
exception path. The disposed library produces 0/12 refusals and even completes both isolated
one-byte saves. Alongside the four entity groups, both renderer labels now report 29 passing
controls and ten grouped ISSUE-098 failures with byte-identical streams. The CLR GetEnumerator
member is source-dispositioned because CNA exposes only its separate CNAEXT range adapters.
Evidence: `/tmp/checkpoint-885-*`; reference assembly SHA-256
`38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.

### Focused886 remaining media disposal guards

The Windows XNA 4.0 reference IL closes the remaining public media disposal-guard question for
Song, PictureCollection and PictureAlbumCollection. `media.song.properties` retains its seven
metadata/identity controls, proves repeated disposal and peer isolation, then calls all sixteen
ordinary Song operations. Pinned CNA reports 0/16 exact refusals, so the stable demo becomes an
expected ISSUE-098 failure. The save-picture workflow replaces its stale zero-count expectation
with retained-child lifetime evidence and two Count/Item groups; both picture collections report
0/2. It now has 31 passing controls, four grouped ISSUE-098 failures and its separate ISSUE-041
failure. Both workflows are byte-identical across renderer labels; Song stderr is empty and the
picture workflow retains its two reviewed INFO lines. CLR GetEnumerator routes are source-
dispositioned because CNA exposes separate CNAEXT ranges. Evidence: `/tmp/checkpoint-886-*`.

### Focused887 public playlist import boundaries

The new isolated `media.media_library.playlist_import` turns internal parser behavior into a
public MediaLibrary workflow. Four root-level M3U/M3U8 fixtures cover deterministic discovery and
names, comments, CRLF/whitespace handling, nested relative audio paths, order, duplicate shared
Song identity, missing-file skipping, absolute-path refusal, canonical symlink containment and a
UTF-8 filename under uppercase `.M3U8`. A PLS file and nested M3U remain undiscovered by design.
Both direct renderer-labelled runs pass9/9 with byte-identical streams, and the exact SOFTWARE
CTest passes20 repetitions. Generated member totals do not change because all named members were
already credited; the new demo supplies behavior evidence instead. Evidence: `/tmp/checkpoint-887-*`.

### Focused888 album-art filename precedence

The stable album-art workflow now covers all distinct branches of CNA's fixed13-name candidate
order in19 controls. Existing mixed-case evidence proves `cover > folder`; new fresh-library scans
prove `cover.jpg > cover.jpeg > cover.png`, then `front.jpeg > album.jpg > albumart.png` as each
winner is removed. The final check establishes the lowest recognized fallback. The remaining
same-family extension aliases use the identical case-folded candidate loop and do not justify
duplicate controls. Both renderer-labelled direct runs are byte-identical and the exact SOFTWARE
CTest passes20 repetitions. Evidence: `/tmp/checkpoint-888-*`.

### Focused889 public music format index

The new `media.media_library.format_index` turns the extension filter and four tag readers into one
public graph. Its generated Ogg, OGA, MP3, FLAC and Opus assets plus an existing WAV produce six
Songs with exact names, tracks, group identities, rating and fallback metadata. Unsupported AAC,
M4A and BIN spellings remain absent despite containing valid audio bytes; nested discovery and a
self-referential symlink leave exactly six records. The fixture generator is byte-reproducible in
two runs and does not alter prior assets. Both renderer-labelled runs pass9/9 byte-identically and
the exact SOFTWARE CTest passes20 repetitions. Evidence: `/tmp/checkpoint-889-*`.

### Focused890 public MP3 APIC album art

The stable album-art workflow now covers CNA's second embedded-image parser. A generated ID3v2.4
MP3 carries the exact64x64 checker PNG as APIC front art; public HasArt, GetAlbumArt, two independent
GetThumbnail streams and SavePicture preserve and decode it. Two generator runs reproduce the MP3
hash without changing older assets. Both direct renderer-labelled runs pass20/20 byte-identically
and the exact SOFTWARE CTest passes20 repetitions. Evidence: `/tmp/checkpoint-890-*`.

### Focused891 partial tags and linked music directory

The stable format-index workflow now produces eight public Songs in11 controls. Its generated
title-only Vorbis fixture retains that title while artist and album fall back independently from
the nested path, with no invented genre or track. A child directory symlink leading outside the
private Music root is followed once and preserves a real tagged FLAC; the canonical visited set
still terminates the existing self-cycle. Malformed/truncated tag layouts stay dispositioned as
internal parser-bounds tests because their public effect is the same field-wise fallback already
demonstrated. Both renderer-labelled runs pass11/11 byte-identically and the exact SOFTWARE CTest
passes20 repetitions. Evidence: `/tmp/checkpoint-891-*`.

### Complete892 generated runtime checkpoint

The complete checkpoint absorbs focused872-891 and gives both post-Complete871 MediaLibrary IDs
full-suite evidence. Metadata is657/0; all670 enabled CTests pass with three manual entries disabled.
SOFTWARE reports561 pass/49 expected fail/44 skip/3 manual and OPENGLES3 reports601/50/3/3, with
zero unexpected results. The exact comparison adds only the two passing IDs, the reviewed ISSUE-098
status/metadata changes and their fifteen grouped failed assertions; SOFTWARE additionally takes
the declared global-pointer environment skip. Generated coverage remains302/329 types,2114/2326
members,169/753 fields and3078/3205 C routes, with no absent IDs. Evidence:
`build/linux-full/reports/checkpoint-892-complete-*`.

### Focused919 full diagnostics frame and trace recording

The new `diagnostics.profiler.trace_recording` workflow runs under an actual
`CNA_DIAGNOSTICS=FULL` build. Its14 controls use one `FrameScope`, a frame counter, nested
manual/macro zones and direct/macro markers; correlate the five recorded events with the provider
snapshot; verify parent/category/payload data and zero drops; and round-trip both the binary and
Chrome Trace exporters. The ordinary OFF build has an explicit capability skip. Eleven FULL runs
pass, the final ten with byte-identical stdout and empty stderr, and the complete three-demo
diagnostics CTest group passes. This closes the central frame/zone/marker/recording/export path;
resource metadata and registered frame-source collection remain meaningful public behaviors.

### Complete920 generated runtime checkpoint

The complete pinned-CNA build and outer-Xvfb validation absorb focused913-919. Metadata is660/0;
all673 enabled CTests pass, with three manual entries disabled. SOFTWARE reports560 pass/52
expected fail/45 skip/3 manual and OPENGLES3 reports600/53/4/3, with all660 IDs present and zero
unexpected results. Against Complete912, the trace ID is the only addition and takes its declared
diagnostics-OFF skip; existing statuses and failed-assertion multiplicities are unchanged. Generated
coverage remains302/329 types,2114/2326 members,674/753 fields and3078/3205 C routes. Evidence:
`build/linux-full/reports/checkpoint-920-complete-*`.

### Focused921 diagnostics resource metadata and provider cursor

The new `diagnostics.profiler.resource_metadata` workflow validates the public resource contract in
an actual FULL build. Owned descriptor strings and every dimension/format/accuracy field survive
caller mutation; Update keeps the stable ID while replacing metadata and aggregate bytes; moves
transfer exactly one registration; repeated Reset restores the baseline. A retained provider cursor
returns the ordered Core create/destroy events, `ResolveName` supplies their canonical names, and the
parallel bounded recording contains exactly those two events without drops. The initial failed run
was a demo error caused by asking `ReadEvents(0, 8)` for the oldest process events; capturing the
current cursor before the lifecycle fixed it. Eleven corrected FULL runs pass, ten byte-identically
with empty stderr, and all four diagnostics CTests pass. The ordinary OFF build declares a skip.
Registered frame-source collection remains the next meaningful diagnostics behavior.

### Focused922 diagnostics frame sources and direct metric handles

The new `diagnostics.profiler.frame_sources` workflow covers the optional subsystem extension seam.
It rejects a null source; proves move-only registration and retained shared ownership; publishes an
estimated nanosecond gauge plus exact frame counter only at manual frame boundaries; distinguishes
per-frame reset from accumulation; and proves idempotent unregistration stops collection and releases
the source. A throwing callback increments `sourceCollectionFailures` while frame publication still
completes. The counters workflow now separately exposes explicit CounterHandle/GaugeHandle IDs,
metadata, signed Add and diagnostics-OFF inertness. All five FULL diagnostics CTests pass and ten
repeated source runs are byte-identical with empty stderr. The ordinary build passes counters and
declares the source skip. Bounded history/drop and malformed-use reporting remain under audit.

### Focused923 diagnostics bounds and malformed-use containment

The new `diagnostics.profiler.bounded_history` workflow converts the remaining public capacity
signals into one consumer scenario. A drained history overflow reports three cursor losses and exact
bounds/retained values; an undrained producer burst reports seven drops; a moved bounded recording
keeps the newest three of five and reports two drops. Repeated manual-zone end preserves the valid
zone while emitting/counting one Malformed record, duplicate frame transitions count two errors while
one frame completes, and242 scopes prove the newest240-frame retention window. `modes` now checks
provider InterfaceVersion1. All six FULL diagnostics CTests and eleven focused runs pass; the last
ten runs are byte-identical with empty stderr. The ordinary diagnostics audit is dispositioned:
representative enum labels already establish storage, while maximum-depth, malformed-binary and
multi-thread saturation stay in CNA unit/stress tests.

### Focused924 canonical renderer-selection exception details

The stable `c_api.platform.renderer_selection` workflow now observes the canonical exception
identity carried by two existing production refusals. An unknown renderer spelling preserves
INVALID_ARGUMENT/ARGUMENT together with `System.ArgumentException` and `ParamName=name`; after a
real graphics device latches the selected fallback, another selection mutation preserves
INVALID_STATE/STATE together with `System.InvalidOperationException` and an empty parameter name.
The four public exception-detail count/copy routes are exercised directly, with exact byte-sized
copies and caller-added terminators. This complements the existing
`System.ArgumentOutOfRangeException(value)` evidence without constructing an exception solely for
coverage.

The warning-free incremental build uses `-j3`; metadata remains663/0. SOFTWARE and OPENGLES3/Xvfb
each pass20/20 in ten repetitions, all twenty stdout files are byte-identical with SHA-256
`f4d21e6243e71683408004f4593e8ab64f31f766a9f2a938a54359c0fa28a19f`, and every stderr file is
empty. The exact SOFTWARE CTest, metadata validation and validator/checkpoint self-tests pass.
Generated coverage remains302/329 types,2114/2326 members,674/753 fields and3078/3205 C routes.
Evidence:`/tmp/checkpoint-924-renderer-*`. No pixel or manual visual-quality claim is made.

### Focused925 ContentLoadException identity through the C boundary

The stable `c_api.content.content_manager` workflow now inspects its existing post-Unload missing-
asset failure. The native load path throws
`Microsoft.Xna.Framework.Content.ContentLoadException`, and the public C contract explicitly says
the exception-detail query returns that canonical name so a managed binding can rethrow it. The C
route still returns IO with category IO and a nonempty message, but the type query returns zero
bytes. CNA-DEMOS-ISSUE-103 records that the adapter catches the exception and calls `IoError`
before the common barrier can record its dynamic type. Source review finds the same pattern in the
other typed content loaders, while runtime scope remains this Texture2D path.

Both renderers retain all53 older passing controls and fail only the new exact identity check among
54. Ten SOFTWARE and ten OPENGLES3/Xvfb runs have byte-identical stdout SHA-256
`960cf6f99b002990d67441e387e954a8eeb6a79249356308e9183aabc1edb348` and empty stderr; each prints
`missing load exception type: <none>`. The exact known-issue SOFTWARE CTest passes ten repetitions,
the warning-free build uses `-j3`, metadata is663/0 and both validator suites pass. Generated
surface totals remain302/329 types,2114/2326 members,674/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-925-content-*` and `/tmp/checkpoint-925-content-exception-source.txt`. No CNA source
was edited and no screenshot or manual visual-quality claim was made.

### Complete926 generated runtime checkpoint

The complete pinned-CNA build and outer-Xvfb validation absorb focused921-925. Metadata is663/0;
all676 enabled CTests pass, with three manual entries disabled. SOFTWARE reports560 pass/53
expected fail/47 skip/3 manual and OPENGLES3 reports599/54/7/3, with all663 IDs present and zero
unexpected results. The exact Complete920 comparison adds the three declared diagnostics-OFF skips
and the reviewed ISSUE-103 failure only; failed-assertion multiplicity moves91->92 and95->96.
SOFTWARE's keyboard/mouse workflow moves from its valid no-desktop-pointer skip to the full42-check
pass because this Xvfb instance exposes the global pointer and cursor paths. Generated coverage
remains302/329 types,2114/2326 members,674/753 fields and3078/3205 C routes, with no absent IDs.
Evidence: `build/linux-full/reports/checkpoint-926-complete-*`.

### Focused927 ObjectDisposedException identity through the C boundary

The stable `c_api.net.local_session_state` workflow now proves that its real post-disposal `Update`
retains the native `System.ObjectDisposedException` alongside INVALID_STATE/category STATE. This is
a distinct binding decision: the handle is permanently dead rather than temporarily unable to
advance. The demo calls the public error-info and exception-type size/copy routes directly with the
exact byte capacity and a caller terminator.

Ten SOFTWARE and ten OPENGLES3-labelled renderer-free runs pass31/31 with byte-identical stdout
SHA-256 `8a5e3e94fdab552781048034533b77ae0088ca12eb2543d671ee6eec2d58330b`
and empty stderr. The exact SOFTWARE CTest passes ten repetitions; the warning-free build uses
`-j3`, metadata is663/0 and both validator suites pass. Surface totals remain302/329 types,
2114/2326 members,674/753 fields and3078/3205 C routes. Evidence:`/tmp/checkpoint-927-net-*` and
`/tmp/checkpoint-927-net-exception-source.txt`. No CNA defect appeared.

### Focused928 Guide service, platform and privilege exception identities

The stable `c_api.gamer.guide_social_offline` workflow grows24->26 controls and demonstrates why a
binding needs the canonical type in addition to the C result. Friends reports NOT_SUPPORTED for an
unavailable account service and retains `GamerServicesNotAvailableException`; the Windows Phone-
only invitation overload reports the same result but retains `System.NotSupportedException`;
Marketplace reports INVALID_STATE and retains `GamerPrivilegeException`. All three error categories
and exact type bytes are checked through the public diagnostic routes. The last type moves generated
coverage302->303/329 and GamerServices43->44/52; other surface totals are unchanged.

Ten final SOFTWARE/Xvfb and ten OPENGLES3/Xvfb runs pass26/26 with stable per-renderer stdout hashes
`a3106c9033bccd3596aac18f3fc485c6b707a0a2ed5309dc4be83640fcfebb21` and
`56324b2f9bcce01bd7890b2dfa7d56d78997ef94360045d24b967b8cc9c67f7e`; stderr is empty. The exact
SOFTWARE CTest, metadata validation and both validator suites pass. Evidence:
`/tmp/checkpoint-928-guide-final-*` and `/tmp/checkpoint-928-guide-exception-source.txt`. No CNA
defect appeared.

### Focused929 GuideAlreadyVisible identity and overlay preservation

The stable `c_api.gamer.guide_social_offline` workflow grows26->27 controls. While its real local
sign-in screen is visible, another valid ShowSignIn returns INVALID_STATE/category STATE with exact
`GuideAlreadyVisibleException`; a following public visibility read proves the refusal does not
replace or close the original overlay. This establishes a temporary modal collision separately from
Focused928's unavailable service, unsupported platform and missing privilege decisions.

Ten SOFTWARE/Xvfb and ten OPENGLES3/Xvfb runs pass27/27 with stable per-renderer stdout hashes
`8828db2de49322037917e77cb09772c635c4a017a881da9559bc1640e3895bef` and
`d011e09ac17589c56a7fd470d492aa3b5bc0fb60b8e257a18453f46901517852`; stderr is empty. The exact
SOFTWARE CTest, metadata validation and both validator suites pass. Generated totals remain303/329
types,2114/2326 members,674/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-929-guide-*` and `/tmp/checkpoint-929-guide-visible-source.txt`. No CNA defect
appeared.

### Focused930 authored ContentReader BoundingSphere

The stable `content.xnb.custom_type_reader` workflow extends its Level state block with a meaningful
encounter volume. The game-owned reader calls `ContentReader::ReadBoundingSphere` directly and the
demo checks the exact authored center `(6.5,-7.5,8.5)` and radius `9.25`; the deterministic asset
generator writes the canonical center-XYZ/radius float layout. This supplies direct public-reader
evidence instead of inferring it from CNA's built-in Model reader, without adding a variant ID.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass11/11 with byte-identical stdout SHA-256
`4b468c20346030f7073fecbf22998e30a3cddcf49b4af4266d63c2402c25e31b` and empty stderr. The exact
SOFTWARE CTest passes ten repetitions; the warning-free incremental build uses `-j3`, metadata is
663/0 and both validator suites pass. Generated totals remain303/329 types,2114/2326 members,
674/753 fields and3078/3205 routes because this FNA-compatible helper is outside the documented
XNA inventory. Evidence:`/tmp/checkpoint-930-content-sphere-*`. No CNA defect appeared.

### Focused931 exact ContentReader byte blobs

The stable `content.content_manager.read_asset_ownership` workflow now uses
`ContentReader::ReadBytesExactOrThrow` for its real variable-length tile resource. The normal
eight-byte path preserves its aliased payload. A second original XNB fixture declares nine bytes
but physically contains eight; loading it through the same game-owned reader raises
`EndOfStreamException` naming that reader before constructing or recording a disposable resource.
This demonstrates the helper's useful security boundary through actual `ContentManager::ReadAsset`
rather than a synthetic direct call.

Ten SOFTWARE and ten OPENGLES3-labelled runs retain exactly one failure, the existing ISSUE-011
default-recorder assertion, among22 controls. All stdout files are byte-identical with SHA-256
`c51873402c61efa4028feafe84bc33e6012825ed89f342aaf1422d648ec9fe3c`; stderr is empty. The exact
known-issue SOFTWARE CTest passes ten repetitions and the warning-free incremental build uses
`-j3`. Generated totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes;
this helper is outside the XNA inventory. Evidence:`/tmp/checkpoint-931-content-exact-*`. No new
CNA defect appeared.

### Focused932 ContentReader count and decoded-byte limits

The game-owned readers in `content.content_manager.read_asset_ownership` now apply
`CheckCollectionElementCount` before their Level row loop and `CheckDecodedByteSize` before tile
allocation. Two original XNB fixtures declare10,000,001 rows and268,435,457 decoded bytes. Both
actual `ContentManager::ReadAsset` paths raise contextual `ContentLoadException` before iteration,
allocation or disposable recording, while the valid zero-row/eight-byte path retains its original
ownership behavior.

Ten SOFTWARE and ten OPENGLES3-labelled runs retain exactly the established ISSUE-011 failure among
24 controls (23 pass/1 expected fail), with byte-identical stdout SHA-256
`c51873402c61efa4028feafe84bc33e6012825ed89f342aaf1422d648ec9fe3c` and empty stderr. The exact
known-issue SOFTWARE CTest passes ten repetitions; the warning-free build uses `-j3`. Generated
totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes because both
helpers are CNA-specific. Evidence:`/tmp/checkpoint-932-content-limits-*`. No new CNA defect
appeared.

### Complete933 generated runtime checkpoint

The full pinned-CNA build and runtime validation absorb focused927-932. Metadata is663/0; all676
enabled CTests pass (629 ordinary passes and47 declared skips), with three manual tests disabled.
SOFTWARE reports560 pass/53 expected fail/47 skip/3 manual and OPENGLES3 reports599/54/7/3, both
with all663 IDs and zero unexpected results. Exact comparison with Complete926 changes only the
intended check counts: `content.xnb.custom_type_reader`10->11 and
`content.content_manager.read_asset_ownership`21->24. Statuses, knownIssue identities and failed-
assertion messages/multiplicities remain unchanged at92 SOFTWARE and96 OPENGLES3. Generated
coverage is303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes. Evidence:
`build/linux-full/reports/checkpoint-933-complete-*`. No screenshot, listening, physical-input,
external-service or manual visual-quality validation was performed.

### Focused934 native C++ renderer metadata

New `core.renderer.metadata` supplies direct public C++ evidence for `GraphicsRendererType`,
`GraphicsBackendCategory` and `GraphicsBackendMaturity`. Nine controls enumerate and classify all
ten renderer identities, round-trip canonical names, accept mixed case, preserve an output on
failed parsing, name every category/maturity value including the currently unused historical
states, verify unknown-value fallbacks and correlate current-build helpers with the general
queries. This is useful launcher and diagnostic behavior independent of the already audited C
adapter.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass9/9 with byte-identical stdout and empty
stderr. The warning-free incremental build uses `-j3`, metadata is664/0 and the exact SOFTWARE
CTest passes. Generated XNA/C totals remain303/329 types,2114/2326 members,674/753 fields and
3078/3205 routes. Complete933 remains the full runtime baseline, so its reports honestly leave this
new ID absent. Evidence:`/tmp/checkpoint-934-renderer-metadata-*`. No device, renderer execution,
pixels or manual visual result is claimed; no CNA defect appeared.

### Focused935 native renderer fallback and latch

The stable `platform.renderer_selection.set_preferred` standalone closes the ordinary native
`GraphicsRendererSelection` behavior that its earlier preferred-renderer path did not reach. It
checks fallback's disabled default and automatic toggle, installs an explicit SOFTWARE chain,
selects absent WEBGL2 through the case-insensitive string overload, preserves WEBGL2 as selected
while a real device activates SOFTWARE, and reads one nonempty `NotCompiledIn` history record.
Preferred, chain and automatic mutations all raise `InvalidOperationException` after the latch.

Ten SOFTWARE/Xvfb and ten OPENGLES3/Xvfb runs pass with stable per-renderer stdout hashes
`a5eb362d699b4405bb6c41622b97e3772b654d3b2f30cb87bc2a7cec7904f652` and
`05c747212f7c845a7e7e27be7138562e1862422890c8e6689c36c6c54c15cedb`; their intentional
warning/info stderr is also byte-stable. The warning-free `-j3` build, exact SOFTWARE CTest,
metadata and validator suites pass. Counts stay unchanged. Evidence:
`/tmp/checkpoint-935-renderer-fallback-*`. Test-only reset and the graphics-module handshake are
implementation/test machinery. No pixel or visual result is claimed, and no CNA defect appeared.

### Focused936 native host-platform identity

New renderer-free `core.platform.host_identity` provides direct C++ evidence for the public
`TargetPlatform` and `DesktopOS` families rather than relying only on the C adapter. Four controls
tie mobile to Android/iOS, Apple to iOS/macOS, the target enum to its stable diagnostic name and,
on the validated Desktop/Linux build, the desktop OS to Linux. On a non-desktop target the same
source instead requires the documented `CNAException` from the desktop-only query.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass4/4 with byte-identical stdout SHA-256
`0539f7336f101f817f8b5e4ea5262d111123caefbb0d3e4242377d4d45e8c475` and empty stderr. The
`-j3` updated object, metadata665/0, exact SOFTWARE CTest and validator suites pass. XNA/C totals
remain unchanged; Complete933 correctly leaves both post-checkpoint IDs absent. Evidence:
`/tmp/checkpoint-936-host-identity-*`. No renderer/window behavior or visual result is claimed, and
no CNA defect appeared.

### Focused937 assembly title and real window default

New isolated `platform.window.assembly_title` covers the process-wide assembly-title API without
polluting the shared runner. A namespace-scope `AssemblyTitleAttributeEXT` installs the first value,
an explicit `SetAssemblyTitleEXT` call replaces it before Game construction,
`GetAssemblyTitleEXT` returns the replacement and a real OPENGLES3/X11 GameWindow begins with that
exact title.

Ten OPENGLES3/Xvfb runs pass with stable stdout SHA-256
`47a978c8d138130681cefbceea59d5d8dd428e7e2a82ad29fe1f9f7c771c0082`. Ten SOFTWARE/Xvfb runs
take the declared no-window skip with stable stdout SHA-256
`d5c3657edd163f1e505730cd235e73fcf99c52c55511b8b8e4889e25597aa4a2`; source review corrected
the first draft's assumption because SOFTWARE intentionally exposes no platform window. The
`-j3` build, metadata666/0, exact SOFTWARE skip CTest and validator suites pass. Generated XNA/C
totals remain unchanged. Evidence:`/tmp/checkpoint-937-assembly-title-*`. No pixel or visual result
is claimed, and no CNA defect appeared.

### Complete938 generated runtime checkpoint

The complete pinned-CNA validation absorbs the native core batches through focused937. Metadata is
666/0; all679 enabled CTests pass (631 ordinary passes and48 declared skips), with three manual
tests disabled. SOFTWARE reports562 pass/53 expected fail/48 skip/3 manual and OPENGLES3 reports
602/54/7/3. Both execute every ID with zero unexpected results. Against Complete933, the three new
IDs take exactly their intended statuses; all663 existing statuses, knownIssue identities and
failed-assertion messages/multiplicities remain unchanged at92 SOFTWARE and96 OPENGLES3. Generated
coverage remains303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes. Evidence:
`build/linux-full/reports/checkpoint-938-complete-*`. No screenshot, listening, physical-input,
external-service or manual visual-quality validation was performed.

### Focused939 native Logger general and experimental paths

The stable `core.logger.minimum_level` demo grows four to seven checks. It distinguishes WARN's
three severe messages, TRACE's six production severities and EXPERIMENT's complete seven-level
stream. It also sends a true and false conditional message through the general `Logger::Log`
overload and checks the delivered DEBUG level, explicit INPUT category and exact message, then does
the same for `ExperimentIf` with its EXPERIMENT level and default APPLICATION category.

This closes the meaningful ordinary Logger remainder alongside the existing custom sink, reset,
minimum-level and `WarnIf` evidence. `FatalIf`, `ErrorIf`, `InfoIf`, `DebugIf` and `TraceIf` are the
same severity-specific forwarding shape already represented by `WarnIf` and `ExperimentIf`; adding
one assertion per spelling would inflate controls without demonstrating distinct behavior. The C
adapter remains exhaustively covered separately.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass7/7 with byte-identical stdout SHA-256
`7406560520b74fb8046818acf498bf5030ec9d98be52e70e39ba9bf030e6f878` and empty stderr. The exact
SOFTWARE CTest passes, the warning-free build uses at most three jobs and metadata remains666/0.
Generated totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes because
the Logger family is CNA-specific. Complete938 remains the complete-runtime baseline. Evidence:
`/tmp/checkpoint-939-logger-*`. No CNA defect appeared.

### Focused940 native platform capability matrix

New `core.platform.capability_matrix` adds the667th stable ID. Five controls demonstrate the
ordinary platform capability decision surface: the Game-owned platform is also the installed
ambient platform and has a name; the Game caches one `PlatformCapabilities` object; all31
`PlatformCapability` values are unique, in declaration order and have stable names; every focused
`IPlatform::Supports` answer agrees with the cached aggregate; and a default aggregate reports all
optional capabilities unsupported.

This establishes how an application discovers and caches platform support without pretending that
one Xvfb host exercises every advertised feature. Clipboard, text input, devices, power, dialogs,
tray, camera, windows and renderer services keep their own behavioral evidence and environment or
manual limits. The matrix does not justify one new demo per Boolean field.

Ten SOFTWARE/Xvfb and ten OPENGLES3/Xvfb runs pass5/5 with byte-identical stdout SHA-256
`77da04600192540017570b48c1f22bbb62fa42569818142b8b8de660bcfa604f` and empty stderr. The changed
object compiles without warnings in a build using at most three jobs, metadata is667/0 and the exact
SOFTWARE CTest passes.
Generated totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes because
this family is CNA-specific. Complete938 remains the full baseline and correctly leaves the new ID
absent. Evidence:`/tmp/checkpoint-940-platform-*`. No CNA defect appeared.

### Focused941 native renderer capability profile

The existing `graphics.device.capability_report` demo grows three to nine controls. Its live
`RendererCapabilityProfile` reference is stable and bears the active renderer name. Every one of
the28 defined `RendererFeature` values has a unique stable name, a description and a non-Unknown
classification; profile `Supports` and both device query forms agree on the meaning of Supported.
All eight defined `RendererLimit` values are uniquely named, known and identical through profile
and device access. All27 public `SurfaceFormat` values expose bounded known/supported masks,
supported remains a subset of known, and the render-target usage agrees with the direct query.

An empty profile plus retired feature/limit and out-of-range format identities return explicit
unknown state. The three `DisplayColorSpace` values are probed without state change and each setter
must agree with the advertised result before restoring the original. The generated English report
is the cached profile's own report, includes its renderer/features/limits/formats and has explicit
additional limitations. This closes the ordinary native structured-profile consumer contract; the
separate legacy capability demo already enumerates every coarse `GraphicsCapability`, while actual
feature pixels and refusals stay with their focused renderer demos.

Ten SOFTWARE/Xvfb and ten OPENGLES3/Xvfb runs pass9/9 with stable renderer-specific stdout hashes
`106bd6671659437351ca4e7906a405e818111d0f71a9a1406b12d24a8bada0f3` and
`d06b74bed37872baf2ad8a7491b41d0346d86dac25e015c238ad326b0213f964`; stderr is empty. The changed
object compiles without warnings with at most three jobs, metadata remains667/0 and the exact
SOFTWARE CTest passes. Generated totals remain303/329 types,2114/2326 members,674/753 fields and
3078/3205 C routes because this family is CNA-specific. Complete938 retains the prior three-check
result for this stable ID. Evidence:`/tmp/checkpoint-941-capability-final-*`. No CNA defect appeared.

### Focused942 portable shader diagnostic parser

New renderer-free `graphics.shader_effect.diagnostic_parser` adds the668th stable ID. An authored
`ShaderDiagnosticEXT` owns and returns every value after caller strings change. A six-line compiler
log then covers `file:line:column`, numeric source-id/line, `source:line(column)` and
`file(line,column)` locations; VS/FS markers persist for continuation lines; information, warning
and default-error severities remain distinct; and unavailable locations stay zero. Every record
owns its common source label and trimmed text after the source log is overwritten. Whitespace-only
logs produce no synthetic record.

The value constructor refuses invalid severity, stage, negative location, column-without-line and
empty-message inputs, while the parser refuses an invalid fallback stage. A
`ShaderCompilationExceptionEXT` owns all six diagnostics after the source vector is cleared,
retains the structured fifth location and produces a stable first-record summary ending in the
remaining count; an empty diagnostic list is refused. This closes the renderer-independent public
diagnostic contract. The existing `graphics.shader_effect.compiler_diagnostics` workflow still
provides the distinct live GLSL compiler, renderer-owned log and retained valid-program evidence.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass9/9 with byte-identical stdout SHA-256
`55befe4d294f126a9c9352edd678eeae9da9468974bba22baeee0e2cc99a8484` and empty stderr. The changed
object compiles without warnings with at most three jobs, metadata is668/0 and the exact SOFTWARE
CTest passes. Generated totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C
routes because this family is CNA-specific. Complete938 correctly leaves this new ID absent.
Evidence:`/tmp/checkpoint-942-diagnostic-*`. No CNA defect appeared.

### Focused943 native shader language and stage matrix

The existing `graphics.device.capability_report` demo grows nine to12 controls. It queries all eight
executable `ShaderLanguageEXT` values against both live stages, while Unknown, Count and negative
language/stage identities must return false. A bounded non-Unknown `ShaderDialectEXT` on a renderer
that executes supplied source must map by public ordinal to a supported Vertex/Fragment language
pair. The separate descriptor-layout helper returns true for exactly SPIR-V and WGSL and false for
the other executable languages plus both sentinels. This complements actual GLSL selection and
execution in the portable-package and compiler workflows.

SOFTWARE reports dialect0/Unknown and zero complete pairs; OPENGLES3 reports dialect2/GLSL ES and
one complete pair. Ten SOFTWARE/Xvfb and ten OPENGLES3/Xvfb runs pass12/12 with stable stdout
hashes `99d2415292cf0d4b20aa61c1d2d20be25c670e7f9833c26fec529d5ab478c7d5` and
`98dab0b7978bca7d09d4db7cf1a921e62c02d555781f95ca81b5cb2c59514566`; stderr is empty. The changed
object compiles without warnings with at most three jobs, metadata remains668/0 and the exact
SOFTWARE CTest passes. Generated totals remain303/329 types,2114/2326 members,674/753 fields and
3078/3205 C routes because this surface is CNA-specific. Complete938 contains the older
three-control form of this stable ID. Evidence:`/tmp/checkpoint-943-shader-*`. No CNA defect
appeared.

### Focused944 native unsupported-3D policy and STUB defect

The stable `graphics.device.supports_capability` workflow now names
`CNA::Unsupported3DGraphicsCallBehavior` and both GraphicsDevice policy members directly. Four
portable controls prove the fresh `Throw` default, the immediate `WarnAndStub` round trip, an
unchanged `ThreeD` capability answer, continued invalid-dimension refusal and restoration of the
original policy. A renderer reporting no 3D receives three further controls: `Throw` must refuse a
valid Texture3D, while `WarnAndStub` must provide a usable zero-reading volume null object and emit
only one RENDER warning across two creations.

SOFTWARE and OPENGLES3 each pass all five applicable controls in ten runs, with stable stdout
SHA-256 `e0cef59accec93d66bbc0938db50c781d849da30b493dab8df410103ec17ecd2` and
`de1d45804b6b0eabfa8d56e97ecce3dc37143eee6e74b6d389ce7d84775355c7`; stderr is empty. STUB
repeats a stable5/7 result ten times: the default refusal works, but its inherited null
`CreateTexture3D` result makes the first transfer throw and emits no policy warning. The demo names
renderer-specific CNA-DEMOS-ISSUE-104 rather than hiding either failed assertion. The warning-free
incremental build uses at most three jobs and the exact SOFTWARE CTest passes.

Generated XNA/C totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes;
the regenerated CNA-extension appendix now credits the enum and both policy members. Complete938
retains this stable ID's older capability-only result. Evidence:
`/tmp/checkpoint-944-unsupported3d/*`. No other unsupported factory/draw behavior, pixel result or
visual quality is claimed.

### Focused945 native vertex-type registry

The stable `graphics.vertex_declaration.custom_vertex` workflow grows ten to14 controls and now
demonstrates the C++ reflection bridge used by the public System::Type vertex-buffer constructors.
It proves that CNA's stock `VertexPositionColor` resolves without application setup; an
unregistered application type returns null from `TryResolve` and throws `ArgumentException` from
`Resolve`; registering a default-constructible `IVertexType` twice is harmless and preserves the
authored20-byte declaration; and both `VertexBuffer` and `DynamicVertexBuffer` System::Type
constructors consume that registered declaration with their requested counts and usage.

The registered polymorphic object is used only to report its declaration; the existing plain
`GlowVertex` remains the actual GPU stream and retains the exact pixel control. This keeps the
public API visible without suggesting that an `IVertexType` object's C++ vtable bytes belong in a
vertex upload. Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass14/14 with byte-identical stdout
SHA-256 `3c15ae72d23e0b03dbddfacdb632a2e17cee50924ff79781ca77fbd0204826ec` and empty stderr. The
warning-free incremental build uses at most three jobs and the exact SOFTWARE CTest passes.

Generated XNA/C totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes;
the CNA-extension appendix now credits `VertexTypeRegistryEXT` and all three public operations.
Complete938 retains the older ten-control form of this stable ID. Evidence:
`/tmp/checkpoint-945-vertex-registry/*`. No CNA defect or manual visual-quality result appeared.

### Focused946 native project graphics profile

The stable `game.graphics_device_manager.graphics_profile` workflow grows two to five controls and
now makes CNA's process-wide project declaration visible beside the existing manager/device
behavior. It proves that cna-demos' declared HiDef profile seeds a newly constructed
GraphicsDeviceManager, a later local `ProjectGraphicsProfileEXT(Reach)` declaration changes the
future global default without rewriting that existing manager, and the direct setter restores the
original HiDef declaration before device creation. The demo then retains its ordinary explicit
Reach manager request and HiDef-only back-buffer-read refusal.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass5/5 with byte-identical stdout SHA-256
`2f0774014669ea982fbf3d464175c90e7a003a123a06a3bb6d3768b0ad1e78b4` and empty stderr. The
warning-free incremental build uses at most three jobs, metadata remains668/0 and the exact
SOFTWARE CTest passes. Generated XNA/C totals remain303/329 types,2114/2326 members,674/753 fields
and3078/3205 routes; the CNA-extension appendix now credits the project-profile declaration,
getter and setter. Complete938 retains the older two-control form of this stable ID. Evidence:
`/tmp/checkpoint-946-project-profile/*`. No CNA defect or visual-quality result appeared.

### Focused947 native XNB object dictionary

The new `content.xnb.object_dictionary` loads the committed heterogeneous
`dictionaries/member_audit.xnb` through native `ContentManager::Load<std::map<std::string,
std::any>>` and boxes that actual reader product as `ObjectDictionaryEXT`. Ten controls demonstrate
both logical-type constructors, all sixteen values and exact membership, typed scalar/XNA/array
lookups, raw `operator[]` type preservation, the complete value map, and distinct missing-key and
wrong-type exceptions. A game-defined reference entry is read through
`ReflectiveTypeReaderBuilder::Field` plus `RegisterShared`; the existing C demo retains the separate
handle, size/copy and malformed-input contract.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass10/10 with byte-identical stdout SHA-256
`cae714c255e241dffcc09357a400e1ceff040afcafba4603e9ac9b07925feac9` and empty stderr. The
warning-free incremental build uses at most three jobs, metadata is669/0, the exact SOFTWARE CTest
passes, and both validator suites pass. Generated XNA/C totals remain303/329 types,2114/2326
members,674/753 fields and3078/3205 routes; the CNA-extension appendix gains the seven
ObjectDictionary operations and two reflective-builder operations. Complete938 correctly marks
the new ID absent. Evidence:`/tmp/checkpoint-947-object-dictionary/*`. No CNA defect or manual
visual-quality result appeared.

### Complete948 generated runtime checkpoint

The authoritative `linux-full` validation at cna-demos commit
`9a97c46aa4a47c36f543b6daa70f4c51f356948a` remains pinned to CNA
`d23e139de9e5c15511b6be745de290896a69ee37` and uses at most three jobs. Metadata validates669/0;
all682 enabled CTests pass (634 ordinary passes plus48 declared skips), with three manual tests
disabled. Full SOFTWARE reports565 pass,53 expected fail,48 skip and3 manual; OPENGLES3/Xvfb
reports605/54/7/3. Both cover every catalog ID with zero unexpected failures or unexpected passes.

Compared with Complete938, the three new IDs pass on both renderers and the five reviewed stable
IDs have only their intended check-count growth. Every one of the666 prior statuses, known-issue
identities and failed-assertion messages/multiplicities is unchanged; failure evidence remains92
SOFTWARE and96 OPENGLES3 assertions. Stderr normalized for temporary-directory IDs and X protocol
serial/resource numbers is byte-identical. Generated totals remain303/329 XNA types,2114/2326
members,674/753 fields and3078/3205 C routes. Evidence is archived under
`build/linux-full/reports/checkpoint-948-complete-*`. No screenshot, listening test, physical input,
external service session or manual visual-quality inspection was performed.

### Focused949 native imported PCM processing

The new renderer-free `content.sound_effect.imported_pcm` demonstrates the public build-time seam
between source-oriented WAV data and CNB processing. It verifies the experimental API marker, safe
`ImportedSound` defaults and every encoding's byte width, narrowing classification and diagnostic
name. A committed16-bit WAV then preserves exact rate/channel/frame/loop/sample data before
AsAuthored and WholeSoundWhenUnset apply distinct loop policy without rewriting PCM. Constructed
24-bit,32-bit and float source values prove exact rounding, saturation, finite clamp and NaN-to-
silence rules; inconsistent byte counts and an unknown encoding both raise ContentLoadException.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass8/8 with byte-identical stdout SHA-256
`cab9003311b65e7fed42e08931670a59c58973ec23517e29cf9a43c10a57ed80` and empty stderr. The
warning-free incremental build uses at most three jobs, metadata is670/0, the exact SOFTWARE CTest
passes and both validator suites pass. Generated XNA/C totals remain303/329 types,2114/2326
members,674/753 fields and3078/3205 routes; the CNA-extension appendix gains nine names.
Complete948 correctly marks the new ID absent. Evidence:`/tmp/checkpoint-949-imported-pcm/*`. No
audio device, playback, listening test or CNA defect is claimed.

### Focused950 processed sound and foreign-carrier disposition

The stable `content.sound_effect.imported_pcm` grows eight to 13 controls and now covers the
container-neutral value produced by the XNA-compatible sound processor. `ProcessedSoundEffect`
defaults safely; both supported encoding identities expose exact byte/bit widths and names; and
unknown helpers return zero plus an explicit diagnostic. The real 16-bit imported WAV becomes an
exact processed value with the XNA whole-source loop, then `ToCnbSoundEffect` changes only the
container format identity. An authored 8-bit value remains byte-for-byte 8-bit through both steps,
while a 24-bit value is refused at this XNA boundary even though the earlier general CNB path
demonstrates its explicit narrowing policy.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass 13/13 with unchanged byte-identical
stdout SHA-256 `cab9003311b65e7fed42e08931670a59c58973ec23517e29cf9a43c10a57ed80` and empty stderr. The
warning-free incremental build uses at most three jobs, metadata remains 670/0, the exact SOFTWARE
CTest passes and both validator suites pass. Generated XNA/C totals remain unchanged; eight
processed-sound names enter the CNA-extension appendix. Evidence:
`/tmp/checkpoint-950-processed-sound/*`. No audio device, playback or CNA defect is involved.

`CNA::Content::ForeignContentObjectEXT` is intentionally not given an independent demo or metadata
credit. Its complete public surface is one default-null, opaque, non-owning `void*` field with no
operation to exercise. The behavior that justifies the carrier already has stronger end-to-end C
evidence: `c_api.content.reflective_reader` proves value/shared reader shapes and exact caller
object identity; `c_api.content.object_dictionary` proves cached reuse and typed foreign lookup;
and `c_api.content.model_loading` proves a Model returns the exact pointer and destruction neither
frees nor mutates it. Direct aggregate construction would only restate the field initializer.

### Focused951 SoundEffect pipeline components

The new renderer-free `content.sound_effect.pipeline_components` makes the public built-in
SoundEffect components visible rather than relying only on generated assets. Fourteen controls
prove importer, processor and writer identities and routes, the fixed no-parameter policy, exact
CNB writer schema/codec identity, registry wiring, a real deterministic WAV-to-CNB build, ordered
diagnostics and dependency shape, exact PCM plus whole-source loop behavior, contextual failure,
and the named compressed adapter's callback and two decoder refusals. The compressed success case
deliberately gives a copied WAV payload to a caller-supplied decoder under an `.mp3` name; it tests
the adapter contract and makes no claim about real MP3 decoding.

The fifteenth control exposes `CNA-DEMOS-ISSUE-105`. A sole importer whose
`SelectedByNameOnly()` returns true is nevertheless selected by an unnamed convention lookup.
Source review isolates the cause: `ContentPipelineRegistry::ResolveImporter` removes named-only
candidates only in its multiple-candidate branch, then its generic resolver accepts the sole
candidate. Explicit-name resolution and the subsequent build both work as documented.

Ten SOFTWARE and ten OPENGLES3/Xvfb processes reproduce 14 passes and that one expected failure
with byte-identical stdout SHA-256
`252bfb6aeb6e22c2040fd11c2515be8b006542b7b8ae332ef44b266762967e66` and empty stderr. The
warning-free build uses `-j3`; metadata is 671/0; the exact known-issue CTest and both validator
suites pass. Generated XNA/C totals remain 303/329 types, 2114/2326 members, 674/753 fields and
3078/3205 routes; 21 public component names enter the CNA-extension appendix. Complete948 marks
both post-baseline IDs absent. Evidence: `/tmp/checkpoint-951-sound-pipeline/*`. No audio playback,
listening test or CNA source change is involved.

### Focused952 CNB SoundEffect codec

The new renderer-free `content.sound_effect.cnb_codec` closes every public declaration in
`CnbSoundEffectCodec.hpp` with 11 controls. It proves safe `CnbSoundEffectData` defaults and the
frozen format/schema/chunk constants; enumerates the schema version, mono/stereo frame width and
diagnostic name for all six assigned values plus an unknown value; preserves PCM16 bytes and widens
the full representative PCM8 range exactly; and refuses adaptation of a reserved codec.

Deterministic encode/decode round trips preserve complete metadata, header size, payload and sound
fields for both PCM widths. A hand-built schema-1 PCM16 file remains readable, while schema-1 PCM8,
reserved flags, an inconsistent payload, a wrong asset identity, missing data and a duplicate
header are refused. The encoder separately rejects reserved format, both sample-rate boundaries,
invalid channels, payload length and loop bounds.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass 11/11 with byte-identical stdout SHA-256
`d815f7d414ba936100a8757f6f81a0b3adcac56d029beaeb1f0a0827a2972fba` and empty stderr. The
warning-free build uses `-j3`; metadata is 672/0; the exact SOFTWARE CTest and both validator suites
pass. Generated XNA/C totals remain 303/329 types, 2114/2326 members, 674/753 fields and 3078/3205
routes; 14 codec names enter the CNA-extension appendix. Complete948 marks all three post-baseline
IDs absent. Evidence: `/tmp/checkpoint-952-cnb-sound-codec/*`. No audio device, playback or CNA
defect is involved.

### Focused953 Song pipeline components

The new renderer-free `content.song.pipeline_components` closes the ordinary public
`SongContentPipeline.hpp` workflow in 16 controls. It proves stable type and parameter names,
`ImportedSongSource` defaults, the ordinary and named WAV importer descriptors, processor types and
four distinct validation failures, exact writer schema identity, and combined registration where
WAV remains a SoundEffect by convention while WAV-as-Song resolves explicitly.

A committed Ogg source travels through the real pipeline without decoding or embedding its media
bytes. A caller probe supplies duration; the build emits deterministic CNB metadata, one primary
dependency, one runtime XREF and one deployment file. Parameter overrides move the deployed stream
and replace the display name and duration together. A named real WAV takes the same external Song
shape. Direct processing proves authored compatibility-container metadata is the fallback, while a
default importer records unknown duration as zero and an empty source fails at the Import stage.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass 16/16 with byte-identical stdout SHA-256
`a58bef89ae57c252c44a168e2bef786649025e3e9c2bd7465d0a54dff862cfc4` and empty stderr. The
warning-free build uses `-j3`; metadata is 673/0; the exact SOFTWARE CTest and both validator suites
pass. Generated XNA/C totals remain 303/329 types, 2114/2326 members, 674/753 fields and 3078/3205
routes; 28 component names enter the CNA-extension appendix. Complete948 marks all four post-
baseline IDs absent. Evidence: `/tmp/checkpoint-953-song-pipeline/*`. No media decoder, audio
device, playback or CNA defect is claimed.

### Focused954 CNB media codec

The new renderer-free `content.media.cnb_codec` closes every public declaration in
`CnbMediaCodec.hpp` with eight grouped controls. Shared schema/chunk constants and both data
structures have exact safe defaults. Song and Video encode deterministically, retain canonical
type/content metadata, write exactly one untyped external-media XREF and decode every field.

Song encoding refuses empty, escaping, absolute and native-separator stream names plus duration
outside the signed runtime range. Its decoder independently enforces asset/schema identity, one
header, one untyped XREF and reserved flags. Video accepts dimensions 1 and 65536, soundtrack
values 0-2 and duration `INT32_MAX`; its encoder refuses unsafe stream, duration, dimension, finite-
positive-rate and soundtrack violations. Raw valid-container fixtures prove the decoder repeats
the metadata and structural checks rather than trusting producers. Generic malformed CNB layout
remains the existing `CnbDocument` audit's responsibility.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass 8/8 with byte-identical stdout SHA-256
`16b157f17349c7cb652c73481e98412f009598019b80830bcf4168870bc1dcae` and empty stderr. The
warning-free build uses `-j3`; metadata is 674/0; the exact SOFTWARE CTest and both validator suites
pass. Generated XNA/C totals remain 303/329 types, 2114/2326 members, 674/753 fields and 3078/3205
routes; 12 codec names enter the CNA-extension appendix. Complete948 marks all five post-baseline
IDs absent. Evidence: `/tmp/checkpoint-954-cnb-media-codec/*`. No media decoder, playback or CNA
defect is involved.

### Focused955 Video pipeline components

The new `content.video.pipeline_components` closes the ordinary public
`VideoContentPipeline.hpp` workflow in 15 controls. Stable value and parameter names, imported and
probed defaults, every importer extension, both processor metadata modes, exact validation faults,
writer schema identity and registry wiring are visible directly.

A real generated MKV remains an opaque external stream while a public callback supplies its known
160x90, 10 fps metadata. The build is byte-deterministic and separately reports the source
dependency, runtime XREF and deployment file. Parameters override every probed field and the stream
destination. A `nullopt` probe produces a Process-stage diagnostic without complete frame
parameters, then succeeds when they are supplied; registration without any probe uses the same
required triple. An empty file is refused at Import.

Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass 15/15 with byte-identical stdout SHA-256
`f65ad9a43bc7964b89a3705d4c736432be69b7af35f6d59bb9b02f1219bfe072` and empty stderr. The
warning-free build uses `-j3`; metadata is 675/0; the exact SOFTWARE CTest and both validator suites
pass. Generated XNA/C totals remain 303/329 types, 2114/2326 members, 674/753 fields and 3078/3205
routes; 27 component names enter the CNA-extension appendix. Complete948 marks all six post-
baseline IDs absent. Evidence: `/tmp/checkpoint-955-video-pipeline/*`. No decoder, playback, visual
inspection or CNA defect is involved.

### Complete956 generated runtime checkpoint

The authoritative `linux-full` validation at cna-demos commit
`89e31fc93b956947639f2754a87fc8032dc29bbe` remains pinned to CNA
`d23e139de9e5c15511b6be745de290896a69ee37` and uses at most three jobs. Metadata validates675/0;
all688 enabled CTests pass (640 ordinary passes plus48 declared skips, 320.03s), with three manual
tests disabled. Full SOFTWARE reports570 pass,54 expected fail,48 skip and3 manual;
OPENGLES3/Xvfb reports610/55/7/3. Both cover every catalog ID with zero unexpected failures or
unexpected passes.

Compared with Complete948, the five ordinary new IDs pass on both renderers and
`content.sound_effect.pipeline_components` has only the reviewed CNA-DEMOS-ISSUE-105 failure. All
669 prior statuses, known-issue identities and failed-assertion messages/multiplicities are
unchanged; failure evidence moves92->93 on SOFTWARE and96->97 on OPENGLES3. Stderr normalized for
temporary-directory IDs and X protocol serial/resource numbers is byte-identical. Generated totals
remain303/329 XNA types,2114/2326 members,674/753 fields and3078/3205 C routes. Evidence is
archived under `build/linux-full/reports/checkpoint-956-complete-*`. No screenshot, listening test,
physical input, external service session or manual visual-quality inspection was performed.

### Focused957 content build configuration

The new renderer-free `content.custom.build_configuration` closes the public
`ContentBuildConfiguration.hpp` parsing contract in 11 controls. It proves the format version and
conventional filename, safe minimal defaults, deterministic asset/root order, every component and
logical-name override, project and per-asset container choices, and exact bool/i64/u64/f64/string
parameter variants.

Grouped diagnostics retain the supplied configuration filename while refusing malformed JSON,
wrong format/version, unknown or duplicate fields, unsafe source/logical paths, empty component
overrides, unknown containers, mismatched/lossy/non-finite parameter values and invalid or repeated
source-root grants. The existing incremental-compiler demo retains stronger evidence that parsed
configuration affects only the intended build node and its fingerprint.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass11/11 with byte-identical stdout SHA-256
`ab027c37510300a73e561bf7ff3a0cce751e73a57f8fb6816397357c42ec23d0` and empty stderr. The
changed object compiles without warnings with at most three jobs; metadata is676/0; the exact
SOFTWARE CTest passes20 repetitions and both validator suites pass. Generated totals remain303/329
XNA types,2114/2326 members,674/753 fields and3078/3205 C routes; 13 public names enter the CNA-
extension appendix. Complete956 marks the new ID absent. Evidence:
`/tmp/checkpoint-957-build-configuration/*`. No filesystem resolution, compilation or CNA defect is
claimed by this parser-focused demo.

### Focused958 content build manifest

The new renderer-free `content.custom.build_manifest` closes the public
`ContentBuildManifest.hpp` helper surface in 14 controls. A representative successful in-memory
result becomes one root-relative node owning primary and additional compiled outputs plus a
deployed support file, all with exact byte digests. Direct fingerprint refresh exposes separate
source-byte, dependency-set/byte, parameter, writer-schema, output-definition and deployment-
definition domains. Effective refresh separately requires the content-build dependency result;
changing it leaves direct state fixed, and the combined helper reproduces the stored result without
mutating its input.

The direct manifest lifecycle covers ordered insertion, replacement, lookup, deterministic
serialization/current-version parsing and clear. All fields round-trip without a physical source
root leaking into JSON. Set refuses escaping source identity, missing primary ownership,
undeclared output schema, deployment without a byte-hashed dependency and a non-lowercase digest;
Parse refuses malformed or obsolete persisted state. Existing incremental-compiler,
dependency-graph and multi-artifact demos retain stronger publication, skip, graph, cleanup and
concurrency evidence.

Ten SOFTWARE-labelled and ten OPENGLES3-labelled runs pass14/14 with byte-identical stdout SHA-256
`a608fd41c05f45a4ac13d2dc806bdb0a1180b8d4d60d8a47b7544864cadf638f` and empty stderr. The
changed object compiles without warnings with at most three jobs; metadata is677/0; the exact
SOFTWARE CTest passes20 repetitions and both validator suites pass. Generated totals remain303/329
XNA types,2114/2326 members,674/753 fields and3078/3205 C routes; 17 previously unlisted public
names enter the CNA-extension appendix. Complete956 marks both newer IDs absent. Evidence:
`/tmp/checkpoint-958-build-manifest/*`. No CNA defect appeared.

### Complete981 generated runtime checkpoint

The authoritative `linux-full` validation at cna-demos commit
`638e98c332dca53367bd95365c87d2d7cbb6083b` remains pinned to CNA
`d23e139de9e5c15511b6be745de290896a69ee37` and uses at most three jobs. Metadata validates683/0;
all696 enabled CTests pass (648 ordinary passes plus48 declared skips, 335.74s), with three manual
tests disabled. Full SOFTWARE reports578 pass,54 expected fail,48 skip and3 manual;
OPENGLES3/Xvfb reports618/55/7/3. Both cover every catalog ID with zero unexpected failures or
unexpected passes.

Compared with Complete956, eight new IDs pass on both renderers and five established IDs have only
their reviewed check-count increases. All675 prior statuses, known-issue identities and failed-
assertion messages/multiplicities are unchanged; failed assertions remain93 on SOFTWARE and97 on
OPENGLES3. Normalized stderr differs only by the expected compiler diagnostic for the newly covered
mutually exclusive FX service options. Generated totals remain303/329 XNA types,2114/2326 members,
674/753 fields and3078/3205 C routes. The complete public `CNA/Content` header tree has semantic
declaration-level dispositions. The then-uncredited `cna_content_manager_create_resource` route was
provisionally treated as an unsupported placeholder; Focused1007 later corrects that classification
because the existing loader-registry workflow already performs successful construction, use and
destruction. Evidence is archived under `build/linux-full/reports/checkpoint-981-complete-*`. No
screenshot, listening
test, physical input, external service session or manual visual-quality inspection was performed.

### Inspector public-header audit

Focused982 enables the optional module in `linux-full` and adds three coherent workflows for the
public `Protocol.hpp`, `Agent.hpp` and `Client.hpp` surface. Focused983 extends those same stable
IDs with the remaining ordinary backpressure and negotiation boundaries and completes the semantic
declaration-level disposition.

| Public family | Current disposition |
|---|---|
| Protocol constants, masks and carrier types | Exact version and maximum payload/event/preview bounds are checked. Every public field has populated encoded/decoded state, including ordered metadata, snapshot loss/accuracy/resource state, resolved events and preview bytes. Capability and snapshot-part helpers test present and absent bits. Every structured error code retains its wire identity; Unavailable, Pending, Ready and Failed have distinct response behavior. |
| `ProtocolCodec` | The fixed 24-byte header and all ten typed `Encode`/`Decode` payload pairs run directly. Grouped refusals cover header size/magic/type, packet type/length/trailing bytes, event/preview/poll bounds, pending tickets and encoded string length. Arbitrary hostile-input fuzzing remains CNA unit-test scope. |
| Agent configuration and lifecycle | Every configuration field has direct default or applied behavior. An injected provider and the process provider both run; explicit/generated tokens, endpoint getters, idempotent stop and the non-loopback authorization guard are direct. Authentication fails before provider access. Process capabilities track the compiled mode, and a STATS provider omits FULL event/zone service. Two admitted requests followed by Busy prove the rate limit without disconnecting. Deleted copy operations and destructors are ordinary C++ ownership declarations and need no independent runtime demo. |
| Client configuration and lifecycle | Every configuration field has direct default or applied behavior. A real synchronous loopback client connects, exposes its negotiated session, captures a selective snapshot, reads cursor-based events, handles unavailable previews, pings, disconnects safely and reconnects. Disconnected and unnegotiated-capability requests refuse without provider access. |
| Preview provider | A public provider receives exact bounded input, returns Pending twice and then a bounded Ready PNG through `PollPreview`. A one-ticket ceiling, completed-ticket retirement, cooldown, invalid resource identity and oversized provider output are contained while the connection remains usable. |

The warning-free `-j3` build validates686/0 metadata. Ten SOFTWARE and ten OPENGLES3-labelled runs
of the three IDs pass20/20,10/10 and18/18 with the same combined stdout SHA-256
`1623907d20ef5df25856552529e900eacb7b7de6a5f1564db283c3196dd25cda` and empty stderr; the exact
SOFTWARE CTest set passes ten repetitions and both validator self-test suites pass. Generated
XNA/C totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes; 54 names
enter the CNA-extension appendix. Complete981 marks the three new IDs absent. Evidence:
`/tmp/checkpoint-982-inspector-*`. A sandboxed listener refusal is environmental; authoritative
network runs occurred outside the sandbox. No browser UI, remote connection, render output or CNA
defect is claimed.

Focused983's warning-free incremental build keeps686/0 metadata. Ten SOFTWARE and ten OPENGLES3-
labelled runs pass23/23,13/13 and20/20 with the same combined stdout SHA-256
`1623907d20ef5df25856552529e900eacb7b7de6a5f1564db283c3196dd25cda` and empty
stderr. The exact SOFTWARE CTest set passes ten repetitions and both validator suites pass.
Generated XNA/C totals remain unchanged; explicit carrier/configuration metadata brings the
Inspector extension inventory to127 names. Complete981 correctly marks the three IDs absent.
Evidence:`/tmp/checkpoint-983-inspector-*`. The public header audit is complete for ordinary
consumers. Timeout and rate-window timing stress, hostile-input fuzzing, and actual remote exposure
remain CNA unit, stress or manual security-test scope.

### Block-compression build-time audit

Focused984 adds the renderer-free standalone `content.texture_processor.block_compression` for the
separately linked `CNA::ContentPipelineBuild` module and dispositions every public declaration in
`BlockCompression.hpp`.

| Public family | Current disposition |
|---|---|
| Format and sizing | Bc1/Bc2/Bc3 retain stable enum identities and Dxt1/Dxt3/Dxt5 names. Exact 8/16-byte block sizes and rounded 1x1, 4x4, 5x5, 13x7 and 16x16 grids are checked. |
| Alpha policy | Opaque, binary cutout and partial transparency are distinguished. BC1 switches between transparent and opaque modes at a caller-selected cutoff; a fully transparent block selects index three for all texels. BC2 preserves the exact sixteen four-bit alpha values in row-major order. |
| Encoding behavior | All three formats produce deterministic declared-size output for an awkward 13x7 image. A 1x1 solid image matches its 4x4 edge-replicated form. Zero and eight refinement rounds retain deterministic layout. Exact visual reconstruction quality remains CNA encoder-test and runtime texture-demo scope. |
| Input boundaries | Zero width, zero height and a pixel span that does not exactly match width times height times four are refused with `std::invalid_argument`. Overflow and allocation-failure stress remain unit/fault-injection scope. |
| Texture-pipeline adapter | Focused985 maps CNB Bc1/Bc2/Bc3 requests to byte-identical direct encoder output with the caller's options. Rgba8 and unsupported Bc3Srgb requests are refused. This closes the one-function `TextureCompressionPipeline.hpp` surface. |

The changed source builds without warnings with at most three jobs and metadata is687/0. Ten
SOFTWARE-labelled and ten OPENGLES3-labelled direct runs pass12/12 with byte-identical stdout
SHA-256 `12e12168ec7e5a58bc7bb21811ae1cdd1dfb0762a70c5b4b644b5a0ee71b19bb` and empty stderr. The exact
SOFTWARE CTest passes ten repetitions and both validator self-test suites pass. Generated XNA/C
totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes; ten names enter the
CNA-extension appendix. Complete981 marks this and the three Inspector IDs absent. Evidence:
`/tmp/checkpoint-984-block-compression-focused/*`. No graphics device, runtime sampling,
screenshot, visual inspection or CNA defect is claimed. The adjacent public
`TextureCompressionPipeline.hpp` adapter should extend this same standalone.

Focused985 extends that stable ID from12 to14 controls. Ten SOFTWARE-labelled and ten OPENGLES3-
labelled direct runs pass14/14 with byte-identical stdout SHA-256
`f07c28a07541778c22d377dda1c769f54564e581e567791ac30250cd5e1ed8d0` and empty stderr. The
warning-free corrected source builds with at most three jobs, metadata remains687/0, the exact
SOFTWARE CTest passes ten repetitions and both validator suites pass. Generated totals remain
unchanged and one adapter name enters the CNA-extension appendix. Evidence:
`/tmp/checkpoint-985-texture-compression-adapter/*`. Audit the build-time media decoder next.

### Build-time media decoder audit

Focused986 adds `content.media.build_time_decoder`, a renderer-free standalone over the optional
build-time FFmpeg boundary. It dispositions every public declaration in
`BuildTimeMediaDecoder.hpp`.

| Public family | Current disposition |
|---|---|
| Availability and carriers | The available build reports an empty refusal and a non-`none` fingerprint identity. `DecodedAudio` and `ProbedVideo` defaults are checked field-completely. The unavailable implementation remains a build configuration path and returns the documented skip. |
| Direct probes | The authored WMV reports 160x120 at 15 fps, positive bit rate, about three seconds and an audio stream. The authored Ogg reports about four seconds. Missing media and an audio-only file take distinct direct refusal paths. |
| Direct audio decode | Any-format Ogg decoding preserves mono 22050-Hz shape as complete PCM16 frames. The MPEG-only route accepts a real MP3 and resamples to 44100 Hz. Both MPEG-only and WindowsMedia-only routes reject readable Ogg input. |
| Pipeline adapters | Video metadata converts ticks to milliseconds and song duration returns milliseconds; both contain unreadable input as `nullopt`. The compressed-sound adapter returns owned signed PCM16 at 44100 Hz with exact frame accounting and no invented loop, and likewise contains failure. |
| Windows Media encode | One second of generated mono PCM becomes a nonempty WMA/ASF file and decodes through the WindowsMedia-only route with the requested shape. Empty input is refused. Encoded bytes and lossy waveform quality are FFmpeg-version-specific and intentionally not asserted. |

The warning-free corrected source builds with at most three jobs and metadata is688/0. Ten
SOFTWARE-labelled and ten OPENGLES3-labelled direct runs pass12/12 with byte-identical stdout
SHA-256 `92b193d68f4003a716c26cf7c2cf5269a0d76ec42051d003b14c53802345a69c` and empty stderr. The exact
SOFTWARE CTest passes ten repetitions and both validator self-test suites pass. Generated XNA/C
totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes; 24 public names
enter the CNA-extension appendix. Complete981 marks this fifth post-baseline ID absent. Evidence:
`/tmp/checkpoint-986-build-time-media/*`. No audio/video device, playback, listening, screenshot or
visual inspection is claimed. The initial path-composition compiler error belonged to the demo and
was corrected; no CNA defect appeared.

### DirectX file-reader audit

Focused987 adds the renderer-free `content.model.directx_file_reader` standalone and dispositions
every public declaration in `DirectXFileReader.hpp`.

| Public family | Current disposition |
|---|---|
| Text object model | The generated twin-panel model parses as one named Frame with ordered transform and child-frame objects. Case-insensitive `TypeIs`, names and exact flattened mesh numbers are checked. A second source omits its inline template while preserving strings, references and a nested object's numbers. |
| Binary object model | An in-memory `bin` token stream declares an inline template containing primitive and array tokens, then parses one named Vector with exact float-list values. Binary 64-bit-float input is refused rather than misread. |
| Errors | BadFile, BadFileType, BadFileVersion and ParseError retain their exact D3DX code names and owned details. Empty, foreign, unknown-version, compressed and truncated documents take the expected distinct category. |
| Resource ceilings | All default byte/depth/object/number/string limits are exact. Individually tightening each limit refuses representative input before excessive allocation or traversal. Mutation fuzzing and allocation fault injection remain CNA unit/stress-test scope. |

The warning-free corrected source builds with at most three jobs and metadata is689/0. Ten
SOFTWARE-labelled and ten OPENGLES3-labelled direct runs pass9/9 with byte-identical stdout SHA-256
`5367924f0bbba25aa3dbd8633e6098b6c89dd3bba2745c4efc148227254271da` and empty stderr. The exact
SOFTWARE CTest passes ten repetitions, both validator suites pass and isolated generator output
matches both checked-in fixture copies. Generated XNA/C totals remain303/329 types,2114/2326
members,674/753 fields and3078/3205 routes; 24 names enter the CNA-extension appendix. Complete981
marks this sixth post-baseline ID absent. Evidence:`/tmp/checkpoint-987-directx-reader/*`. The first
run's wrong expected number count was a demo defect and was corrected; no CNA defect appeared.

### FBX file-reader audit

Focused988 adds the renderer-free `content.model.fbx_file_reader` standalone and dispositions every
public declaration in `FbxFileReader.hpp`.

| Public family | Current disposition |
|---|---|
| Text document | A deterministic FBX 7400 document preserves top-level and child order, exact-case lookup, scalar/quoted/bare-text/array property alternatives and typed fallback behavior. `Numbers()` flattens scalar and array values while skipping strings. |
| Binary document | A version-7400 record preserves scalar, string and uncompressed double-array properties. A second double array carries fixed zlib bytes and decompresses to its exact values, exercising the optional compressed-array path in this build. Wide version-7500 record layout remains represented by CNA's own reader tests; it is a storage-width variant without a separate consumer concept. |
| Errors | CannotInitialize, NotFbx, ParseError and Unsupported retain exact identity and owned detail. Empty input, a recognized DirectX file, unrelated bytes and malformed FBX distinguish the first three runtime categories. Unsupported is the no-zlib build disposition; this build has zlib and directly proves successful decompression. |
| Resource ceilings | All byte/depth/node/array/decompressed-byte/string defaults are exact. Tightening each one independently refuses representative text or binary input before excessive work. Mutation fuzzing and allocation fault injection remain CNA unit/stress-test scope. |

The warning-free source builds with at most three jobs and metadata is690/0. Ten SOFTWARE-labelled
and ten OPENGLES3-labelled direct runs pass7/7 with byte-identical stdout SHA-256
`ddb318091576bea132de8deab22c749bc42d6a095c4abc3d67e9eaadf52c902a` and empty stderr. The exact
SOFTWARE CTest passes ten repetitions and both validator self-test suites pass. Generated XNA/C
totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes; 27 names enter the
CNA-extension appendix. Complete981 marks this seventh post-baseline ID absent. Evidence:
`/tmp/checkpoint-988-fbx-reader/*`. No CNA defect appeared.

### XNA model source-pipeline audit

Focused989 adds the renderer-free `content.model.xna_source_pipeline` standalone and dispositions
both declarations in `XnaModelSourceContentPipeline.hpp` through real registry and build behavior.

| Public family | Current disposition |
|---|---|
| Registration contract | One call adds exactly `CNA.XImporter`, `CNA.FbxImporter`, `CNA.XnaModelProcessor`, `MaterialProcessor` and the name-only `TextureProcessor` adapter. The stable imported node-graph identity, extension lists, output types and common default processor are checked directly. |
| Intentional writer gaps | Both CNB and XNB registry inventories retain a nonempty reason for the absence of a standalone MaterialContent writer: materials are intermediate model content. |
| Actual source routes | Deterministic `.x` and text FBX triangles run through `ContentPipeline::Build`; each result retains its selected importer and the shared model processor, then produces nonempty Model CNB and XNB containers with the expected signatures and reader identity. |
| Semantic convergence | Both CNB outputs decode as schema-2 models. The `.x` graph has its expected implicit unnamed root and the FBX mesh is the root, while vertex declaration/bytes, index bytes, bound, draw range and material agree exactly. Parser encodings and resource ceilings remain owned by Focused987/988. |
| Direct XNA model bridge | The actual XImporter and ModelProcessor run through public bridge contexts over the same frozen coordinator. `ToCanonicalModel` produces the exact schema-2 graph emitted by the registered route after normalizing the documented nonserialized null-name bit. Two parts sharing a vertex buffer, index buffer and material retain the same three resource indices; missing vertex data and an unsupported material each produce the documented PipelineException. |

The warning-free source builds with at most three jobs and metadata is691/0. Ten SOFTWARE-labelled
and ten OPENGLES3-labelled direct runs pass8/8 with byte-identical stdout SHA-256
`9f324c6110296e4a1d8d479070c1b0f376a7b36cb98e571d52638c6b25c336ca` and empty stderr. The exact
SOFTWARE CTest passes ten repetitions and both validator self-test suites pass. Generated XNA/C
totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 routes; 24 names enter the
CNA-extension appendix. Complete981 marks this eighth post-baseline ID absent. Evidence:
`/tmp/checkpoint-989-xna-source/*`. The first direct run's byte-identity assumption belonged to the
demo; inspecting both schema-2 graphs established the legitimate source-root distinction and no
CNA defect appeared.

### Complete990 generated runtime checkpoint

The authoritative `linux-full` validation at cna-demos commit
`94bc60e9ac75a9d6cc56093f5db6d2c5133c737c` remains pinned to CNA
`d23e139de9e5c15511b6be745de290896a69ee37` and uses at most three jobs. Metadata validates691/0;
all704 enabled CTests pass (656 ordinary passes plus48 declared skips, 296.32s), with three manual
tests disabled. Full SOFTWARE reports586 pass,54 expected fail,48 skip and3 manual;
OPENGLES3/Xvfb reports626/55/7/3. Both cover every catalog ID with zero unexpected failures or
unexpected passes.

Compared with Complete981, eight new IDs pass on both renderers. All683 prior statuses, known-issue
identities and failed-assertion messages/multiplicities are unchanged; failed assertions remain93
on SOFTWARE and97 on OPENGLES3. Normalized stderr is identical after replacing only generated
temporary-directory IDs and X protocol serial/resource numbers. Generated totals remain303/329 XNA
types,2114/2326 members,674/753 fields and3078/3205 C routes; no catalog ID is absent. Evidence is
archived under `build/linux-full/reports/checkpoint-990-complete-*`. No screenshot, listening test,
physical input, external service session or manual visual-quality inspection was performed.

### XNA pipeline bridge metadata and scanner audit

Focused991 extends the stable known-issue `content.custom.xna_pipeline_components` demo without a
new ID. Thirteen new controls directly cover the four conversion helpers, all importer/processor
metadata views, the attribute-free registration helper and the entire public
`PipelineComponentScanner` result/update surface. The real importer, nested conversion, writer,
dependency, environment and strictness workflow remains intact, so these are connected to the
same registry that subsequently performs actual builds.

SOFTWARE-labelled and OPENGLES3-labelled runs each produce33 passes and exactly the two existing
ISSUE-047/048 failures. Ten repetitions per renderer label have one normalized stdout SHA-256,
`ab7718b8c6a6a3acb983f170f06931d09b4e2c681b8bf7d6a7b6eba2db7b0434`, and empty stderr. The exact
known-issue SOFTWARE CTest passes ten repetitions. The warning-free `-j3` build, metadata691/0,
four CLI/catalog tests and validator/checkpoint self-tests5/5 and4/4 pass. Runtime inventory totals
remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes; the build-time API is
outside that inventory and its25 newly direct names appear in the extension appendix. Evidence:
`/tmp/checkpoint-991-xna-bridge-*`. No screenshot, GPU, genuine-XNA oracle or new CNA defect is
claimed.

### Direct XNA model conversion audit

Focused992 extends `content.model.xna_source_pipeline` from eight to twelve passing controls and
closes the one public `XnaModelBridge.hpp` function. It deliberately obtains `ModelContent` from
the real public XNA importer and processor over CNA bridge contexts instead of hand-authoring a
minimal graph. Its canonical projection agrees exactly with the already validated end-to-end CNB
route, and separate shared-resource and refusal models cover the function-specific contract.

The warning-free `-j3` build, metadata691/0, ten exact SOFTWARE CTests, ten direct SOFTWARE and ten
direct OPENGLES3-labelled runs, and validator/checkpoint self-tests5/5 and4/4 pass. Direct stdout is
byte-identical with SHA-256 `572b1f47c9ec41ddf98102ad4c1bdf7de1076b2f0a275718e46c6ef9279d9b52`;
stderr is empty. Generated runtime totals remain303/329 types,2114/2326 members,674/753 fields and
3078/3205 C routes; this build-time helper enters the extension appendix. Evidence:
`/tmp/checkpoint-992-xna-model-*`. No renderer, screenshot, genuine-XNA oracle or new CNA defect is
claimed.

### XNA bridge contexts and logging audit

Focused993 extends `content.custom.xna_pipeline_components` to34 passing controls while retaining
only its two established ISSUE-047/048 failures. A real coordinator build constructs the public
importer/processor contexts and both logger variants, forwards every observable environment
property and records exactly one Info, Important and Warning message with stage/component context.
The warning additionally proves help-link formatting. Existing route conversion, dependency and
additional-output branches continue to exercise the remaining context operations; nested builds
already cover the protected forwarding implementations, and the context aliases are behaviorless.

The warning-free retry build uses at most three jobs. Metadata remains691/0; ten exact SOFTWARE
CTests, ten direct runs under each renderer label, four CLI/catalog tests and validator/checkpoint
self-tests5/5 and4/4 pass. Direct outputs share normalized SHA-256
`adc4bbe3750291decde800329d5a00d47ff296013fbc9484606dcd9b828f3b3b`, with empty stderr. Runtime
inventory totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes;
31 direct public names enter the build-time extension appendix. The first build reproduced open
ISSUE-057 and the unchanged retry was clean. Evidence:`/tmp/checkpoint-993-*`. No renderer, GPU,
genuine-XNA oracle or new CNA defect is claimed.

### Direct XNA XNB-output registration audit

Focused994 extends `content.custom.xml_xnb_pipeline` without a new ID. A fresh compiler plus one
custom Vital writer is registered directly rather than only through the XML convenience helper.
The resulting registry has exactly one XNB route for every sorted compiler type; an existing
System.Int32 route is retained. Repeating registration preserves the exact writer objects. A
resolved Vital writer exposes its option-bound component identity and XnaObject schema, writes a
compressed XNB and restores value63 through the game reader. A null compiler is refused before
registry mutation.

The environment-specific control reveals CNA-DEMOS-ISSUE-111: despite requesting Windows Phone and
HiDef through the writer's environment overload, the XNB header says Windows/Reach. Source review
confirms only outputDirectory comes from the environment while target platform/profile come from
registration options, contrary to the public contract. Ten serial runs per renderer label report
26 passes and exactly the ISSUE-090/111 failures. Normalized stdout SHA-256 is
`f043f692ac5ad687634516a2ecdbdb7ed2cf3d4fa142c8946f7cf5b0e6e8e695`; normalized stderr SHA-256
is `d4de1e8a60e84e94cd4430995e7b9d160df8c9e3010e6bd39aa0fdc4cec57dad`. Ten exact SOFTWARE
CTests, metadata691/0, four CLI/catalog tests and self-tests5/5 plus4/4 pass. Generated runtime
totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes; the direct
registration name joins the build-time appendix. One earlier parallel run repeated open ISSUE-057;
all serial runs were clean. Evidence:`/tmp/checkpoint-994-authoritative-*`. No GPU, visual or
genuine-XNA behavior is claimed.

### XNA XML source registration extension audit

Focused995 adds five passing controls to `content.custom.xml_xnb_pipeline`. The first direct
registration exposes one XmlImporter and exactly one pass-through processor/XNB writer per compiler
type. Registering a second compiler preserves that importer and every prior component while adding
only its `XmlExtensionSource` routes; the importer's dynamic output list follows the documented
first-compiler then appended-new-type order. Repeating the same registration preserves every
component object. A private XML fixture then travels through the extended importer, pass-through
processor and writer and restores rank42 through its game reader. Null is rejected with an untouched
registry.

The warning-free `-j3` build, metadata691/0, ten exact SOFTWARE CTests, ten serial direct runs per
renderer label, four CLI/catalog tests and self-tests5/5 plus4/4 pass. Direct runs report31 passes
and only established ISSUE-090/111, with normalized stdout SHA-256
`2be8ac174e57aa7e88203f9a6fc49adcb3064b25343b3cffc24b3e8e71b944d7` and stderr SHA-256
`df194fd20910f4cf1206a8a06e8a4429fa6194cb7ada323069f3b791395e6d89`. Runtime totals remain
303/329 types,2114/2326 members,674/753 fields and3078/3205 C routes; metadata API names are
unchanged. Evidence:`/tmp/checkpoint-995-authoritative-*`. No GPU, visual, external compiler or
genuine-XNA behavior is claimed. This closes the one-function `XnaXmlSourceContentPipeline.hpp`
contract.

### Effect compiler service and source-pipeline audit

Focused996 adds the renderer-free `content.effect.source_pipeline` standalone and dispositions the
public `EffectCompilerService.hpp` and `EffectSourceContentPipeline.hpp` surfaces through one real
authored route. Exact profile names and strict parsing, structured diagnostic parsing/rendering,
compiler identity, include scanning and the explicitly missing-executable external factory are
checked directly. A private source includes two files transitively; the registered importer records
all three canonical dependencies and the processor sends the controlled service exact source and
include paths, sorted defines, HiDef and disabled debug output. That service returns the committed
genuine Effect Framework binary, which reaches the registered XNB writer with the expected runtime
asset identity and staged import/process messages.

The same route refuses null and malformed parameters, an unavailable service before Compile, a
structured compiler failure, success with an invalid signature, Reach output containing a shader-
model-three token, Windows Phone before Compile and the documented CNB absence. Source review
corrected two demo assumptions: the writer identity is `CNA.XnbEffectWriter`, and CNB writer
selection occurs after processing, so the controlled compiler is called before the absence is
reported. The warning-free `-j3` build, metadata692/0, ten exact SOFTWARE CTests, ten direct runs
under each renderer label and self-tests5/5 plus4/4 pass. Every direct run reports19/19 with stdout
SHA-256 `b40f196da937673d2cf0521e15097d7557906ff4d0995f25bb85256a348e3b2b` and empty stderr.
Generated runtime totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C
routes;34 public names enter the extension appendix. Evidence:
`/tmp/checkpoint-996-effect-source/*`. No new CNA defect, external compiler execution, GPU, visual,
runtime Effect load or genuine-XNA behavior is claimed.

### XMA encoder service and AudioContent consumer audit

Focused997 adds the renderer-free `content.audio.xma_encoder_service` standalone and dispositions
every public declaration in `XmaEncoderService.hpp`. Direct controls cover all quality names,
complete request/result defaults, all identity forms, the unavailable service and its fixed
sentence, a separately diagnosed missing executable, and exact process-wide null/set/get behavior.
Three real `AudioContent::ConvertFormat(Xma, ...)` calls forward the same 800-byte PCM16 source,
channel/rate/bit shape and 400-frame loop with Low, Medium and Best respectively. The XNA-facing
object then adopts the controlled payload, complete 52-byte XMA2WAVEFORMATEX and backend loop
values. A failed service retains its identity and both diagnostics; the default service retains the
canonical unavailability phrase.

The external factory relaunches the demo itself as a controlled encoder-contract process through
the launcher field. Its argument template proves input/output, quality, loop-start and loop-length
substitution. The child reads CNA's canonical input WAV and writes XMA2-shaped RIFF test bytes; the
service parses its format/data chunks and retains the child's diagnostic. Independent invalid-RIFF
and exit7 modes exercise both backend failures. This stub is deliberately not an XMA encoder. The
first run's two-temporary iterator range was a demo bug and was corrected before the range reached
CNA. The warning-free `-j3` build, metadata693/0, ten exact SOFTWARE CTests, ten direct runs under
each renderer label and self-tests5/5 plus4/4 pass. Every direct run reports17/17 with stdout
SHA-256 `4da0dfd66eea16f8b2f66d52df96ef20f3fac98a36719371e380486d9f436c24` and empty stderr.
Generated runtime totals remain303/329 types,2114/2326 members,674/753 fields and3078/3205 C
routes;34 CNA names enter the extension appendix. Evidence:`/tmp/checkpoint-997-xma-service/*`.
No new CNA defect, genuine XMA encoding, Xbox/audio playback, listening test or genuine-XNA
behavior is claimed.

### Complete998 generated runtime checkpoint

The authoritative `linux-full` checkpoint at cna-demos
`ff2012a1def3cc9b3b5ec9eddda8c36357fbad0b` remains pinned to isolated CNA
`d23e139de9e5c15511b6be745de290896a69ee37` and uses at most three jobs. Its build is warning-free,
metadata validates693/0, and all706 enabled CTests pass in286.68s; three manual tests remain
disabled. Full SOFTWARE reports588 pass,54 expected fail,48 skip and3 manual;
OPENGLES3/Xvfb reports628/55/7/3. Both reports contain all693 catalog IDs with zero unexpected
failures or unexpected passes.

Against Complete990, the two new service IDs pass on both renderers and all691 prior status values
are unchanged. Focused994 added the reviewed ISSUE-111 assertion to an existing known-issue demo,
so failed-assertion evidence moves93->94 on SOFTWARE and97->98 on OPENGLES3; no other assertion
changes. After normalizing generated temporary-directory IDs and X protocol serial/resource values
and removing Focused995's two expected new asset-load messages, stderr is byte-identical to
Complete990 on each renderer. Validator self-tests pass5/5 and comparator self-tests pass4/4.

The initial `tools/validate.sh` process lost its PTY and was externally terminated during the final
OPENGLES3 stage after build, metadata, CTest and SOFTWARE had completed. The exact documented
OPENGLES3/Xvfb command was rerun over the unchanged binary and exited0, so the evidence is a
complete set of stages rather than one uninterrupted wrapper execution. Immutable artifacts are
under `build/linux-full/reports/checkpoint-998-complete-*`.

Generated totals remain303/329 XNA types,2114/2326 non-field members,674/753 fields and3078/3205
public C routes, with no absent catalog IDs. This checkpoint absorbs Focused991-997 and records a
semantic disposition for every public header in build-time `CNA/Content/Pipeline`. The next audit
returns to the generated XNA gaps, beginning with the eight residual members in root
`Microsoft.Xna.Framework`.

### Focused999 residual root, Audio, Content and GamerServices member audit

The generated gap list after Complete998 begins with eight root members. Source review against CNA
d23e139de confirms that both `CurveKeyCollection` enumerator spellings are CLR aliases for C++
ranges already traversed, and that `Game`/`GameComponent` finalizers map to demonstrated native
destruction and explicit disposal. `Game::ShowMissingRequirementMessage` is a protected, uncalled
default-false hook. `GameWindow::OnActivated`, `OnDeactivated` and `OnPaint` are protected empty
methods without a production caller. These dispositions remain intentionally uncredited.

Audio's seven gaps remain four native-lifetime finalizers and three exception constructors. A
fresh-process invalid SDL audio-driver probe reaches the real `NoAudioHardwareException` path and
is reported as a declared requirement skip. Missing microphone hardware remains manual, and CNA's
production XACT capacity path still never throws `InstancePlayLimitException`. Content's seven
gaps are the nominal collection-name, ignore, runtime-type and type-version attribute wrappers;
the serializer consumes the already demonstrated public C++ descriptors rather than these wrapper
objects. Synthetic construction would increase the generated count without new semantics.

Of77 GamerServices gaps, three are meaningful in the existing offline privileges concept.
`SignedInGamer::PartySize` returns zero for a local player outside a party, and both
`SignedInGamer::IsFriend` and `SignedInGamer::GetFriends` throw `GamerPrivilegeException` before
service access. The other74 divide as follows:

| Residual family | Disposition |
|---|---|
| AchievementCollection, GamerCollection and PropertyDictionary CLR aliases | Their useful C++ indexing, mutation and range behavior is already exercised; the CLR-mangled names have no separate C++ entry point. |
| AvatarAnimation/AvatarRenderer finalizers | Explicit disposal and native destruction are already demonstrated. |
| AvatarDescription.Changed | CNA polls cached local/service avatars, but there is no public offline avatar-mutation route; the real delivery path remains service/editor dependent. |
| FriendCollection lifecycle and FriendGamer state | These require an online service snapshot. Calling public `CreateInternal` only to manufacture a collection would not demonstrate the XNA retrieval path. |
| Gamer lookups/tokens and Guide social calls | Gamertag lookup, partner tokens, friends, messages, invites, parties, cards, reviews, marketplace and players require configured service identities or interactive Guide UI. `DelayNotifications` only affects service invitation prompts. |
| Event/exception constructors and InstallingTitleUpdate | Event payloads are framework-created, direct exception construction adds no failed operation, and CNA has no production raiser for InstallingTitleUpdate. |
| SignedInGamer.IsHeadset | Requires an enumerated microphone and remains in the manual microphone workflow. |

The warning-free `-j3` build and metadata693/0 pass. Ten serial direct runs per renderer label pass
4/4 with identical stdout SHA-256
`bf7bf411eda2d695c5764241de18e3780e644302a0939991ed0d1d5346793ca2` and empty stderr. The exact
SOFTWARE CTest, all15 CLI tests and validator/comparator self-tests5/5 plus4/4 pass. Generated
coverage moves2114->2117/2326 non-field members; types remain303/329, fields674/753 and C routes
3078/3205. An initial sandboxed GL launch was blocked at the X11 socket before demo entry; the
unsandboxed OPENGLES3/Xvfb repetitions are the recorded runtime result. Evidence:
`/tmp/checkpoint-999-gamer-members/*`. No online service, real friend/party, microphone, avatar-
change delivery, screenshot or manual inspection is claimed. Continue with the56 residual Graphics
members.

### Focused1000 residual Graphics, Input.Touch and Media member audit

The residual Graphics review adds only two behaviors. `graphics.effect.custom_apply_lifecycle`
constructs `EffectMaterial` from a base Effect, verifies the independent fallback reflection graph
and keeps applying the clone after source disposal. `graphics.graphics_adapter.display_modes`
reaches the production `NoSuitableGraphicsDeviceException` path by latching SOFTWARE with one real
device and requesting incompatible HEADLESS selection for a second device. The remaining54 names
are protected hooks already reached indirectly, CLR iterator machinery, native finalizers,
physical-loss-only behavior, an already represented arbitrary Tag carrier or explicit-interface
aliases. They remain uncredited.

The Input.Touch review finds that its five nested enumerator members are substantive CNA APIs,
rather than mere CLR aliases. `input.touch.touch_values` now checks pre-start refusal, sequence,
copied cursor independence, Reset, typed and erased Current, Dispose and snapshot ownership. The
coverage matcher accepts an owner-qualified nested name so this evidence cannot accidentally credit
the four unrelated Graphics `Enumerator` types. Only TouchCollection's non-generic CLR
`GetEnumerator` remains intentionally uncredited. Media's14 residual collection enumerator names
have no separate C++ methods; the already exercised count/index/lifetime behavior and representative
public ranges remain their semantic disposition.

The warning-free `-j3` build, metadata693/0 and nine focused SOFTWARE CTests pass. Ten serial runs
of each changed workflow pass on both renderer labels with stable stdout and empty stderr. Generated
coverage moves303->306/329 types and2117->2124/2326 members; fields remain674/753 and C routes
3078/3205. Evidence:`/tmp/checkpoint-1000-graphics-members/*`. No physical device loss, compiled-FX
material asset, screenshot, visual inspection or manual media interaction is claimed. Audit the34
residual Net members next.

### Focused1001 residual Net member audit

The public `NetworkSessionProperties::GetEnumerator` is substantive even though ten adjacent
inventory names are CLR aliases. The existing fixed-shape demo now checks invalid Current before
and after traversal, all eight values, Reset and visibility of an in-place slot write. A real local
session gamer also reaches `SendPartyInvites` and proves the exact no-other-party-member refusal;
successful delivery remains service-only. The actual System Link exception in `join_refused` now
passes through `GetObjectData`, which writes the base message/inner fields before `joinError` and
retains the runtime values.

The other31 Net entries remain six framework-created event payload constructors, the join-exception
constructor, five voice/guest members needing hardware or a service identity for positive behavior,
four accepted-invitation paths, five service leaderboard event/payload members and ten explicit CLR
collection aliases. Existing workflows already establish their reachable event and collection
semantics; synthetic construction or constant-false getters would add no useful coverage.

The warning-free `-j3` build and metadata693/0 pass. All three focused SOFTWARE CTests pass, with
the network test repeated outside the sandbox. Ten direct runs of every changed workflow under both
renderer labels have matching per-workflow stdout and empty stderr. The known-issue join reaches the
new passing serialization assertion before only ISSUE-032/033 remain. Coverage moves2124->2127/2326
members; types remain306/329, fields674/753 and C routes3078/3205. Evidence:
`/tmp/checkpoint-1001-net-members/*`. No service invitation, leaderboard session, guest identity,
voice hardware, screenshot, listening or manual claim is made. Audit the three residual Storage
members next.

### Focused1002 residual Storage member audit

`StorageDeviceNotConnectedException` has a deterministic production path in the current Linux
environment. The existing selector demo temporarily supplies a regular file as `XDG_DATA_HOME`,
causing the actual storage-root creation in `SetAppNameEXT` to throw the documented exception with
its filesystem cause. It restores the prior environment, removes the blocker and completes the
ordinary connected selector and space checks. Own-process execution isolates both mutations.

The other two names stay uncredited. `StorageDevice::DeviceChanged` has no production raiser on the
current platforms, and invoking the public event would manufacture delivery. `StorageContainer`
Finalize is CLR machinery already represented by explicit Dispose, its one-shot Disposing event,
idempotence and native destruction. The warning-free `-j3` build, metadata693/0, two focused
SOFTWARE CTests and ten direct runs per renderer label pass. The direct workflow reports4/4,
normalized stdout SHA-256 `59e35519566a0823b08414c5a37cb1699b2f99e70c947b2626bbad2b8604ff64`
and empty stderr. Coverage moves306->307/329 types and2127->2128/2326 members; fields remain674/753
and C routes3078/3205. Evidence:`/tmp/checkpoint-1002-storage-members/*`. This closes the semantic
namespace pass over all generated XNA member gaps while leaving198 intentionally uncredited names.
Run the complete693-demo checkpoint next, then continue the deeper behavior/C-route audit.

### Complete1003 post-member-audit checkpoint

The full checkpoint at cna-demos `5017b7a` and isolated CNA
`d23e139de9e5c15511b6be745de290896a69ee37` passes a warning-free three-job build, metadata693/0
and all706 enabled CTests in271.90s; three manual tests remain disabled. Full SOFTWARE results are
588 pass/54 expected fail/48 skip/3 manual, while OPENGLES3/Xvfb reports628/55/7/3. Every one of
the693 catalog IDs is present and neither renderer has an unexpected result.

The first complete CTest exposed a demo-only timing flaw in the repeated-Play assertion. The old
fixed observation left about250ms for the dummy-audio callback under load. The corrected workflow
first proves a half-speed four-second voice is Playing at its two-second midpoint, repeats Play,
then allows three seconds for the original progress to finish; a restarted voice would need about
four. Fifteen focused runs under three concurrent processes, both direct renderer labels and the
repeated full CTest pass. No CNA issue is recorded.

Against Complete998 all693 statuses and all failed-assertion messages/multiplicities are unchanged
(94 SOFTWARE,98 OPENGLES3). Passing check totals move3085->3099 and3315->3328 through the reviewed
Focused999-1002 additions and the audio precondition. Validator self-tests pass5/5 and comparator
self-tests4/4. Evidence:`build/linux-full/reports/checkpoint-1003-complete-*`. Generated totals are
307/329 XNA types,2128/2326 members,674/753 fields and3078/3205 C routes. Continue with explicit
behavior gaps, then revalidate Focused823's127 intentionally uncredited C routes.

### Focused1004 ShaderEffect pre-Apply uniform delivery

`graphics.shader_effect.uniform_palettes` now uploads its scalar Matrix/Vector4 and Matrix/Vector3
array values before the first Apply. The exact Red/Lime first-panel pixels prove those public
setters make their effect program current, copy the caller buffers and retain the values across the
later Apply. Later panels preserve the ordinary apply-then-update order.

The warning-free three-job build and metadata693/0 pass. The SOFTWARE CTest keeps its declared
CustomEffects skip; ten OPENGLES3 and ten OPENGL33 direct runs pass6/6 with identical stdout SHA-256
`9cd702a22644957a8d309aada617979e0a0afd346ef29355cd8df30ac4b2faf8` and empty stderr. CLI tests
pass15/15 and validator/comparator self-tests pass5/5 and4/4. Evidence:
`/tmp/checkpoint-1004-shader-preapply/*`. Counts remain307/329 XNA types,2128/2326 members,674/753
fields and3078/3205 C routes. No non-GL renderer behavior, invalid count, missing uniform,
screenshot or manual inspection is claimed. Audit ShaderEffect source accessor ownership next.

### Focused1005 ShaderEffect source ownership

The existing compiler-diagnostics workflow now creates its retained program from temporary
`ShaderCodeEXT` values, destroys them, overwrites the original caller strings and verifies exact
vertex/fragment text through both `ShaderEffect` properties and virtual `Effect` accessors. The
program then retains its established diagnostic isolation and exact cyan draw after a replacement
compile fails. This closes the source-accessor behavior row without a getter-only demo.

The warning-free three-job build, metadata693/0, CLI15/15 and self-tests5/5 plus4/4 pass. SOFTWARE
keeps its declared CustomEffects skip. Ten OPENGLES3 and ten OPENGL33 runs pass8/8 with stable
per-dialect stdout and deliberate compiler stderr. Generated coverage remains307/329 XNA types,
2128/2326 members,674/753 fields and3078/3205 C routes; the extension appendix gains the four
source-accessor names. Evidence:`/tmp/checkpoint-1005-shader-sources/*`. No non-GL renderer,
screenshot or manual visual claim is made. Audit bound RenderTargetCube destruction next.

### Focused1006 RenderTargetCube bound destruction

`graphics.render_target.render_target_cube` retains its nine disposal, face-rendering,
preservation and environment-map controls and adds four for the distinct native-destruction path.
Destroying an optional-backed cube while PositiveY is active removes its dangling public binding,
but Present remains refused until an explicit null-target transition restores the saved back-buffer
viewport. Reconstructing the cube in the same optional storage produces a genuine NegativeX
binding with a 24x24 viewport; a CornflowerBlue clear reads back exactly from that face after
unbinding. This closes the resource-lifetime table's cube-specific gap without a new ID.

The warning-free `-j3` build and metadata693/0 pass, as does the exact SOFTWARE CTest. Ten direct
runs per renderer pass13/13 with stable per-renderer stdout and empty stderr. SOFTWARE stdout
SHA-256 is `6048b86ccae610fa6c935f1d5fa4d47b1fbd5bf70f2087110ceb0752e5c3413e`; OPENGLES3 is
`9d1e709a4970ca65066d8c5b34e255ee5ec0c3e4ef94174bb5f3df1a328e3c2a`. Their existing reflected-
pixel diagnostic differs by renderer, while every assertion passes. Generated coverage remains
307/329 XNA types,2128/2326 members,674/753 fields and3078/3205 C routes. Evidence:
`/tmp/checkpoint-1006-render-target-cube/*`. Pixel validation used programmatic readback; no
screenshot or manual visual inspection was performed. No CNA issue was found. Revalidate the127
intentionally uncredited C routes against the pinned public headers next.

### Focused1007 C route disposition revalidation

An exact catalog/header comparison against pinned CNA
`d23e139de9e5c15511b6be745de290896a69ee37` still finds3205 declarations, but the prior127-route
classification contained one documentation/metadata error. `c_api.content.cnb_loader_registry`
already calls `cna_content_manager_create_resource`, requires successful construction, passes the
live manager through three custom-loader invocations plus one built-in invocation, observes the
callback-scoped view and destroys the owner. The lack of a native embedded-resource store affects
loads through that manager; it does not make the successful constructor a placeholder. Naming the
route in the existing metadata closes `content.h` at32/32 and moves C coverage3078->3079/3205.

The remaining126 routes divide exactly into84 constant type-name helpers,21 explicit test-only
hooks and21 production routes whose positive behavior needs hardware, service traffic, host UI,
real context loss/fullscreen transition or an external graphics debugger. The prior per-header
decisions remain valid for every one of them. The warning-free `-j3` build and metadata693/0 pass,
as does the exact SOFTWARE CTest. Ten SOFTWARE and ten OPENGLES3/Xvfb direct executions retain the
existing21/21 controls with byte-identical stdout SHA-256
`653383602c657feb42e575fb3d6131e35c2b817f0ab3701c71a91050374873f1` and empty stderr. Evidence:
`/tmp/checkpoint-1007-c-route-revalidation/*`. No embedded-resource load, hardware, service,
screenshot or manual result is claimed. Continue the deeper behavior audit with
`ShaderEffect::DeclareUniformBlockEXT`, including actual layout/pixel evidence if the pinned GL
backends expose a deterministic path.

### Focused1008 ShaderEffect uniform-block disposition revalidation

Pinned CNA d23 confirms that `ShaderEffect::DeclareUniformBlockEXT` forwards to the effect renderer,
whose interface implementation is a documented no-op for renderers with loose uniforms. The public
contract names the retired IGL/Vulkan backend; the only current override is WebGPU's owned staging
block and name-to-offset map. `linux-full` builds OPENGLES3, OPENGL33, SDL_GPU, SOFTWARE, HEADLESS
and STUB, none of which overrides the no-op. A call in an existing GL scene would therefore prove
only that a no-op returns, not uniform-block delivery.

The route remains intentionally renderer-specific and uncredited. Actual coverage requires a
WebGPU/browser run with explicit offsets and pixel output; zero-count clearing and replacement
belong to that same workflow. The warning-free `-j3` no-op build, metadata693/0, validator
self-tests5/5 and checkpoint-comparison tests4/4 pass. No demo, metadata or coverage count changes.
The exact source/cache audit is `/tmp/checkpoint-1008-uniform-block/source-audit.txt`, SHA-256
`d65b908e8a45fc1471c382ea40831e3122127261609219cee2c955cc2cd01c04`. No runtime or visual claim
is made. Audit active-open `OcclusionQuery` disposal next; both tested renderer implementations
expose a native release path and the existing visibility scene can make subsequent target reuse
observable.

### Focused1009 OcclusionQuery active disposal

`graphics.occlusion_query.visible_pixels` now disposes a separate query while its native Begin/End
pair is open. It immediately verifies the public disposed state and released renderer, then its
existing hidden and visible queries both complete. This makes native target reuse observable: a
stale active slot would prevent the later query pair from producing its established zero/visible
results. The coherent lifecycle scene grows from11 to12 controls without a new ID.

The warning-free `-j3` build, metadata693/0 and exact SOFTWARE CTest pass. Ten direct runs per
renderer pass12/12 with stable stdout and empty stderr. SOFTWARE reports its precise4422 visible
samples with SHA-256 `cc3685ab121347b234e16e5db25f7c45dbc463449600ef438df1b87fa9d2401f`;
OPENGLES3 reports its expected boolean1 with SHA-256
`8030af18db95b55dc322303da4abf37e7928fcaa0fc931783e1bf15a2eff215e`. Counts remain307/329 XNA
types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1009-occlusion-query/*`. Pixel results come from query counters; no screenshot or
manual visual inspection was performed. No CNA issue was found. Audit the next reachable explicit
behavioral gap, prioritizing an existing coherent graphics-resource workflow.

### Focused1010 RenderTarget2D cross-device move and secondary-context defect

`graphics.render_target.render_target2d` closes its cross-device move gap with an occupied target on
the primary device and an actively bound source on a live secondary device. Move assignment changes
the owner and complete 20x10/Depth24Stencil8/PreserveContents shape, detaches the old primary texture
slot, transfers the secondary public target binding, preserves an authored LimeGreen texel and
accepts a later Orange write after unbinding. These three move controls pass on both tested
renderers, growing the coherent lifecycle scene from13 to17 total controls.

The first attempted post-move Clear failed on OPENGLES3, and a pre-move control isolated it from the
move: a target created and bound through the secondary device already reads transparent black after
Clear, while SOFTWARE reads exact LimeGreen. Source review shows that Clear leases its device's
renderer context but `GraphicsDevice::SetRenderTargets` and EasyGL's binding path do not.
CNA-DEMOS-ISSUE-112 records the genuine defect; the demo retains exactly that pre-move expected
failure for OPENGLES3.

The warning-free `-j3` build, metadata693/0 and exact SOFTWARE CTest pass. Ten SOFTWARE runs pass
17/17; ten OPENGLES3/Xvfb runs reproduce16 passes plus exactly ISSUE-112. Stdout is stable with
SHA-256 `3488cbe6c4875f248908b547fd1f7d71d7f475cdf71d3bdb424c2e1e40778929` and
`6cc0d5a4fde3dbab0fbffa6df1510d055c18d9763729e5eb2ab2e388b8cbf8ee`; stderr is empty. The batch
runner classifies the GL result as one expected failure and no unexpected pass. Counts remain
307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1010-render-target-cross-device/*`. Data checks used programmatic readback; no
screenshot or manual visual inspection was performed. Audit invalid-count and missing-name
ShaderEffect uniform behavior next without duplicating already covered setter families.

### Focused1011 ShaderEffect no-op uniform boundaries

The first panel of `graphics.shader_effect.uniform_palettes` now follows its valid pre-Apply scalar,
vec3-array and mat4-array uploads with zero-count vec3/mat4 calls backed by destructive zero buffers,
plus a zero-valued `SetUniformVec4` under a name absent from the linked program. The existing exact
Red/Lime pixels prove these representative vector/matrix empty updates and missing-name lookup leave
the valid state intact. The remaining panels retain ordinary replacement and composed tint evidence.

Signed negative C++ counts are raw pointer lengths outside the declared contract and are not invoked;
the safe C wrapper already accepts documented empty null sequences and refuses negative/nonempty-null
requests across all four array families. The warning-free `-j3` build, metadata693/0 and exact
SOFTWARE capability-skip CTest pass. Ten OPENGLES3 and ten OPENGL33/Xvfb runs pass6/6 with identical
stdout SHA-256 `9cd702a22644957a8d309aada617979e0a0afd346ef29355cd8df30ac4b2faf8` and empty stderr. Counts
remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1011-shader-noop-uniforms/*`. Pixel checks were programmatic; no screenshot or
manual visual inspection was performed. No CNA issue was found. Audit the next explicit reachable
member/behavior gap outside the completed ShaderEffect family.

### Focused1012 distinct active OcclusionQuery refusal

`graphics.occlusion_query.visible_pixels` now opens an owner query and attempts `Begin` on a second
object before ending the owner. The public header promises `InvalidOperationException`; SOFTWARE
and EasyGL both return normally. The contender is disposed before the owner ends, and the existing
hidden/open queries subsequently complete, isolating the defect without poisoning native state.
CNA-DEMOS-ISSUE-113 records the mismatch.

Source review finds only per-object `isInBeginEndPair_` state in the wrapper and a void renderer
Begin contract. Software detects the occupied slot but cannot report its false activation result;
EasyGL attempts the native begin before its owner map preserves the first query. The warning-free
`-j3` build and metadata693/0 pass; the exact SOFTWARE CTest accepts the marked failure. Ten direct
runs per renderer execute13 controls with twelve passes and exactly ISSUE-113, stable stdout and
empty stderr. SOFTWARE SHA-256 is `bef68001cfccc181ebac7885a8a94ba60cbeb08c69fc5a49df7bdf3b75caa189`;
OPENGLES3 is `0cce5baf79513f1ad419aa37fca901bd99ab7221cb2ffbdc15b16a2272407fb4`.
The GL batch run reports one expected failure and no unexpected pass. Counts remain307/329 XNA
types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1012-occlusion-distinct-begin/*`. No screenshot or manual visual inspection was
performed. Audit the next deterministic explicit lifetime/behavior gap.

### Focused1013 bound VertexDeclaration disposal

`graphics.vertex_declaration.custom_vertex` now supplements its direct user-memory draw with a real
buffer using the same20-byte application layout. It binds the buffer, disposes the caller's
declaration and observes that CNA's buffer-owned wrapper shares the disposed resource identity while
the buffer remains live and present in `GetVertexBuffers`. This is the lifetime-safe C++ mapping of
FNA/XNA's retained declaration reference.

The first draw uses that still-active buffer and receives an independent white-center/black-exterior
readback. The demo then clears the frame and repeats the original explicit-declaration
`DrawUserPrimitives` path with its own readback, so neither result can hide a no-op in the other.
The warning-free `-j3` build, metadata693/0, exact SOFTWARE CTest and self-tests5/5 plus4/4 pass.
Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass16/16 with byte-identical stdout SHA-256
`3c15ae72d23e0b03dbddfacdb632a2e17cee50924ff79781ca77fbd0204826ec` and empty stderr. Counts
remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1013-vertex-declaration-active-dispose/*`; source-audit SHA-256
`3b57be6da20550cd67b0d9a9e45dad808bed9e9d74e15db26907e2d0f64d30db`. No screenshot or manual
visual inspection was performed, and no CNA issue was found. Run a complete checkpoint before the
next behavior batch.

### Complete1014 behavior checkpoint

The complete documented validation at cna-demos `0fd7d71` and pinned CNA `d23e139de` uses at most
three jobs. The build is warning-free, metadata is693/0 and all706 enabled CTests pass in366.19s;
three manual tests remain disabled. SOFTWARE reports587 pass/55 expected fail/48 skip/3 manual and
OPENGLES3/Xvfb reports626/57/7/3. Both contain all693 unique catalog IDs with no unexpected result.

Complete1003 comparison isolates only the reviewed Focused1004-1013 changes: ISSUE-113 on both
renderers, ISSUE-112 on OPENGLES3 and the intended positive control increases in five existing
demos. Each new issue contributes exactly its documented assertion. Normalized stderr remains
byte-identical. Validator/comparator self-tests pass5/5 and4/4. Generated totals are307/329 XNA
types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`build/linux-full/reports/checkpoint-1014-complete-*`; result SHA-256 values are
`c8581dc7f4dd4404cb4be6a00dd450840c1c4ab06ba1641fe135f85c6bbd2b80` and
`dc069865df7b395d0697f5f944bc6ee81a8d27e99b6bde878a033936a7a63453`. No screenshot, listening
test, physical input, external service session or manual visual inspection was performed. Continue
the deterministic behavior audit outside the completed graphics-lifecycle slices.

### Focused1015 SkinnedEffect texture ownership and disposal

`graphics.skinned_effect.clone_ownership` grows from ten to fourteen controls without a new ID. A
separate raw-setter probe observes exactly one caller owner, safely clears the alias and proves the
wrapper expires. The real clones then share one owned wrapper after source/caller release. Explicit
Texture2D disposal is visible through both clone pointers without expiring that wrapper, and the
next actual EffectPass Apply throws ObjectDisposedException through TextureCollection rather than
silently sampling a released renderer object. Clearing the two raw properties leaves their shared
owners intact; the established weak-pointer checks still expire only after the final clone release.

The warning-free three-job build, metadata693/0 and exact SOFTWARE CTest pass. Ten SOFTWARE and ten
OPENGLES3/Xvfb runs pass14/14 with byte-identical stdout SHA-256
`5baed11be4ab6fc4df327d6f8dad134c1546cc1cfd319c8b0497f9017a96b5dc` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1015-skinned-ownership/*`. Existing first-frame programmatic pixels remain the draw
proof; no screenshot or manual visual inspection was performed, and no CNA issue was found. The
same audit pass also removes a stale Model-table sentence: `mesh_parts` already covers null part
Effect removal and exact sibling-only drawing.

### Focused1016 SpriteFont atlas value lifetime

`graphics.sprite_font.glyph_constructor` grows from eight to nine controls. Immediately after the
constructor copies the authored Texture2D, the demo explicitly disposes the caller's wrapper. That
wrapper reports disposed while `getTextureEXT()` remains live; exact GetData returns the complete
30x5 atlas and the existing DrawString pixel still proves renderer-backed use. This establishes the
independent wrapper lifetime over shared texture storage rather than relying only on the caller
wrapper leaving scope.

The warning-free three-job build, metadata693/0 and exact SOFTWARE CTest pass. Ten SOFTWARE and ten
OPENGLES3/Xvfb runs pass9/9 with byte-identical stdout SHA-256
`aebc805e6c0f244af7c9aa300bcfa4544816d24de61aeae9fe7b5896525f85a7` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1016-sprite-font-atlas/*`. Pixel validation is programmatic; no screenshot or
manual visual inspection was performed, and no CNA issue was found.

### Focused1017 nonterminal SoundEffect loop release

`audio.sound_effect.loop_region` replaces its exact-end region with a nonterminal authored layout:
0.25s intro,0.25s loop and0.75s tail. It stays Playing beyond the whole1.25s buffer, then pauses,
receives `Stop(false)` and resumes. At0.5s after resume a correct release must be in the tail even
if it started at the final loop sample. Pinned CNA is already Stopped, exposing ISSUE-114; its mixer
stores the loop end as the track's complete `maxFrame` and release only sets remaining loops to zero.
The other six controls pass, including paused-state/property retention, bounded natural stop,
replay/immediate Stop and all invalid loop coordinates.

The initial expanded fixture also failed its old one-sample-overrun probe because adding the tail
made that coordinate valid. Moving the probe beyond the new whole buffer corrected this demo bug
before marking the genuine failure. The warning-free three-job build, metadata693/0 and exact
SOFTWARE expected-failure CTest pass. Ten SOFTWARE and ten OPENGLES3-labelled dummy-audio runs
retain six passes plus exactly ISSUE-114, byte-identical stdout SHA-256
`3803ae6a2baace31e55b69cd6d039f96345ae3cb24eb5b408fe5b550f1b8de7d` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1017-loop-tail-*`. No listening, output-sample or audible-quality claim is made.

### Focused1018 DynamicSoundEffect live format refusals

`audio.dynamic_sound_effect_instance.buffer_formats` grows from23 to25 controls. Two independent
two-second stereo queues remove the short-buffer completion race: a Playing integer stream refuses
float samples, and a Playing float stream refuses integer bytes. Both checks require the original
Playing state and exact pending-buffer count to survive the exception. All stopped transitions,
paused refusals, range preservation, paused clear/refill and duration controls remain intact.

The warning-free three-job build, metadata693/0 and exact SOFTWARE CTest pass. Ten SOFTWARE and ten
OPENGLES3-labelled dummy-audio runs pass25/25 with byte-identical stdout SHA-256
`ff1d67cbc24e68c856001bae8ce22af345b6f54fdaaba8d35a255ab473edeb96` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1018-dynamic-live-format/*`. Partial stereo float-frame output remains unclaimed;
no listening, sample-output or audible-quality observation was made, and no CNA issue was found.

### Focused1019 SkinnedEffect composed alpha

`graphics.skinned_effect.vertex_colors` grows from three to six exact framebuffer controls without
a new ID or metadata credit. The original disabled/enabled packed RGB controls remain. A second
packed vertex set has black RGB and alpha128, making its source contribution zero under AlphaBlend
while its alpha attenuates MidnightBlue. Vertex alpha alone produces (12,12,56); adding material
Alpha=.5 produces (19,19,84); using an additional texture alpha128 produces (22,22,98). These
sequential products distinguish every factor and close the row's explicit texture/vertex-alpha gap.

The warning-free three-job build, metadata693/0, exact SOFTWARE CTest and self-tests5/5 plus4/4
pass. Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass6/6 with byte-identical stdout SHA-256
`46d9df45d185c209b2c71b1997e6ae59f4432732aab2e640bfd811fd9231db25` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1019-skinned-alpha/*`. Pixels were read programmatically; no screenshot or manual
visual inspection was performed, and no CNA issue was found.

### Focused1020 Texture2D transparent-RGB export

`graphics.texture2d.save_as_image` grows from eight to ten controls without a new ID or metadata
credit. Its generated 64x32 source uses magenta RGB throughout, alpha0 on one half and alpha128 on
the other. PNG decoding yields exact transparent black for every alpha-zero texel and retains every
partial-alpha magenta texel. JPEG decodes to opaque black and magenta at interior center samples,
showing the same pre-encode normalization before that format discards alpha. The original full-size
and resized round trips remain unchanged.

The warning-free three-job build, metadata693/0, exact SOFTWARE CTest and self-tests5/5 plus4/4
pass. Ten SOFTWARE and ten OPENGLES3/Xvfb runs pass10/10 with byte-identical stdout SHA-256
`0daa1217e538c59a72ab6bc76c2fc77905554c0b579da43f344cf48c19a2f829` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1020-save-alpha/*`. Public decoded pixels supplied the validation; no screenshot or
manual visual inspection was performed, and no CNA issue was found.

### Focused1021 TextureCube compressed-alpha sampling

`graphics.texture_cube.sampler_ownership` grows from eight to nine GL controls without a new ID or
metadata credit. Every face of a DXT3 cube carries explicit alpha8/15 and solid red; every DXT5 face
selects alpha endpoint64 with solid blue. Sampling +X and applying NonPremultiplied blending over
black yields exact RGBA(136,0,0,192) and (0,0,64,207) on both dialects. This joins the separate exact
block-transfer evidence to actual renderer colour/alpha decoding. A renderer that refuses either
compressed format retains the original Color cube controls and omits this conditional assertion.

The first draft expected backbuffer alpha255. Its exact RGB136/64 already proved correct source
alpha, while the preset's SourceAlpha/InverseSourceAlpha alpha equation predicts192/207; correcting
that oracle removed the sole failure without a CNA issue. The warning-free three-job build,
metadata693/0, exact SOFTWARE CTest skip and self-tests5/5 plus4/4 pass. Ten OPENGLES3 and ten
OPENGL33/Xvfb runs pass9/9 with byte-identical stdout SHA-256
`d1d27c31318a9e9e969b5f902332dfc883e03f425271a2dcedd120dbb0b33d47` and empty stderr. Generated
totals remain307/329 XNA types,2128/2326 members,674/753 fields and3079/3205 C routes. Evidence:
`/tmp/checkpoint-1021-cube-compressed-alpha/*`. Pixels were read programmatically; no screenshot or
manual visual inspection was performed.
