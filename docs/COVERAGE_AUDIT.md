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
| NetworkSessionJoinError, NetworkSessionJoinException | candidate: a join refused (full session, no join in progress) -- with the two-process demos |
| GamerLeftEventArgs | demo: `net.network_session.find_and_join`, `net.network_session.host_leaves` |
| HostChangedEventArgs, NetworkSessionEndReason, NetworkSessionEndedEventArgs | demo: `net.network_session.host_leaves` (two processes) |
| WriteLeaderboardsEventArgs | service: CNA raises the `Write*Leaderboard`/`WriteTrueSkill` events only for leaderboard gameplay on the CNA service |

## Microsoft.Xna.Framework.Storage

| Type | Decision |
|---|---|
| StorageDeviceNotConnectedException | environment: a removed storage device |
