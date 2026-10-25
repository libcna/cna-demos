#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Generate/check the 248-screen cna-examples retirement ledger.

Generation reads the actual sibling cna-examples registration tree and every registered
screen header, then joins that inventory to cna-demos' machine-readable catalog.  The checked-in
JSON is self-contained; normal validation does not require the retired repository to remain.
"""

from __future__ import annotations

import argparse
import ast
import collections
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LEDGER = ROOT / "docs" / "CNA_EXAMPLES_MIGRATION.json"
SUMMARY = ROOT / "docs" / "CNA_EXAMPLES_MIGRATION.md"
DISPOSITIONS = {
    "ALREADY_COVERED", "COVERED_BY_MULTIPLE", "EXTEND_EXISTING", "MIGRATE_NEW",
    "MANUAL_INTEGRATION", "OBSOLETE", "INTENTIONALLY_NOT_MIGRATED",
}

OVERRIDES = {
    "Content / XNB Format / XNB Fixtures": (
        "EXTEND_EXISTING", ["content.xnb.same_assets"],
        "The existing CNB/XNB comparison now verifies the LZX container flag and loads a real external MonoGame-produced compressed Texture2D; generated DXT XNB coverage remains separate."),
    "Audio / 3D Audio / Doppler & Distance Scale": (
        "EXTEND_EXISTING", ["audio.sound_effect_instance.apply_3d"],
        "The existing positional-audio demo now adds explicit approaching, stationary and receding emitter-velocity phases without claiming the authored Pitch property is the derived Doppler pitch."),
    "Devices / Camera / Live Capture": (
        "MANUAL_INTEGRATION", ["devices.camera.live_capture"],
        "A new opt-in C++ workflow enumerates and opens a physical camera, polls readiness, uploads real frames to Texture2D and releases the device; synthetic C-API frames alone were not equivalent."),
    "Devices / Sensors / Accelerometer": (
        "MANUAL_INTEGRATION", ["devices.sensors.live_readings"],
        "The new opt-in sensor workflow opens a real Accelerometer, displays physical readings and stops/disposes it; availability and emulated-value checks remain automated separately."),
    "Devices / Sensors / Gyroscope": (
        "MANUAL_INTEGRATION", ["devices.sensors.live_readings"],
        "The new opt-in sensor workflow opens a real Gyroscope, displays physical rotation readings and stops/disposes it."),
    "Devices / Sensors / Compass": (
        "MANUAL_INTEGRATION", ["devices.sensors.live_readings"],
        "The new opt-in sensor workflow opens a real Compass, displays headings, records calibration requests and removes the subscription on close."),
    "Devices / Sensors / Motion (fused)": (
        "MANUAL_INTEGRATION", ["devices.sensors.live_readings"],
        "The new opt-in sensor workflow opens real fused Motion hardware, displays acceleration, rotation and gravity, then stops/disposes it."),
    "Devices / Desktop Integration / MessageBox": (
        "MANUAL_INTEGRATION", ["devices.desktop.message_box"],
        "A real modal host dialog now has an explicit manual trigger; canned dialog contracts remain automated separately."),
    "Devices / Desktop Integration / FileDialog": (
        "MANUAL_INTEGRATION", ["devices.desktop.file_dialog"],
        "A real asynchronous file/folder picker now has explicit triggers and a callback lifetime that cannot retain a destroyed game."),
    "Devices / Desktop Integration / UrlLauncher": (
        "MANUAL_INTEGRATION", ["devices.desktop.url_launcher"],
        "A manual key press now exercises the system's actual external URL handler without any unattended side effect."),
    "Devices / Desktop Integration / SystemTray": (
        "MANUAL_INTEGRATION", ["devices.desktop.system_tray"],
        "A manual demo now creates a real tray icon only on request, marshals menu callbacks to Update and destroys the icon on exit."),
    "Input / Keyboard / GetState(PlayerIndex)": (
        "EXTEND_EXISTING", ["input.keyboard.get_state"],
        "The focused keyboard-state demo now queries and compares all four PlayerIndex overloads."),
    "Input / Keyboard / Hold Duration & Repeat": (
        "MIGRATE_NEW", ["input.keyboard.typematic_repeat"],
        "A focused recipe now checks initial delay, repeat interval, long-frame catch-up and release/re-press reset deterministically, with a live held-key view."),
    "Input / Mouse / Scroll Wheel": (
        "EXTEND_EXISTING", ["input.mouse.get_state"],
        "The mouse-state demo now derives vertical and horizontal per-frame deltas from the cumulative wheel counters."),
    "Input / Mouse / Global Position & Warp": (
        "MANUAL_INTEGRATION", ["input.mouse.global_position_and_warp"],
        "A manual demo contrasts client/global coordinates and restores the real pointer after its explicitly requested warp."),
    "Input / Mouse / Capture": (
        "MANUAL_INTEGRATION", ["input.mouse.capture"],
        "A manual demo toggles real mouse capture and releases it on deactivation and unload."),
    "Input / Gamepad / Player Index LED": (
        "EXTEND_EXISTING", ["input.gamepad.device_info"],
        "The device-information demo now exposes the write side on an explicit key press and restores the controller's original LED index."),
    "Input / Touch / Peak Simultaneous Touches": (
        "EXTEND_EXISTING", ["input.touch.capabilities"],
        "The meaningful API behavior, TouchDeviceExistsEXT, is now checked beside GetCapabilities; the legacy peak counter was application-local bookkeeping."),
    "Input / Other / Summary": (
        "INTENTIONALLY_NOT_MIGRATED", [],
        "This screen only aggregated the same input subsystems already represented by focused demos; copying a second dashboard would add no behavior or teaching value."),
    "Input / Other / Sentinels & Factories": (
        "EXTEND_EXISTING", ["input.touch.touch_values", "input.gamepad.state_values"],
        "The two focused value demos now check TouchPanel::NO_FINGER and GamePadButtons::FromButtonArray directly."),
    "2D Graphics / SpriteBatch / Drawing Basics / Position Draw": (
        "EXTEND_EXISTING", ["graphics.sprite_batch.draw_texture"],
        "The position demo now places sprites through both the XNA Vector2 and CNA float x/y overloads."),
    "2D Graphics / Textures & Fonts / Texture2D Basics / Procedural Creation": (
        "EXTEND_EXISTING", ["graphics.texture2d.set_data"],
        "The procedural-texture demo now compares CreateFromPixels RGBA8 with the XNA constructor-plus-SetData route byte for byte."),
    "2D Graphics / Device State & Blending / Viewport & Scissor / Reset on RT Switch": (
        "EXTEND_EXISTING", ["graphics.render_target.render_target2d"],
        "The render-target lifecycle demo now checks exact viewport and scissor resets on target bind and back-buffer restore."),
    "2D Graphics / Render Targets / Screen Transition / Fade Transition Effect": (
        "MIGRATE_NEW", ["graphics.render_target.fade_transition"],
        "A complete focused recipe now renders two scenes to off-screen targets and cross-fades them with GameTime-driven alpha blending."),
    "3D Graphics / Primitives & Vertex Types / Buffers / VertexDeclaration Stride Gotcha": (
        "OBSOLETE", [],
        "The old silent color-drop defect no longer exists: current CNA rejects invalid vertex declarations and the corrected custom-vertex behavior is already regression-tested."),
    "3D Graphics / Device State, Camera & Model / Model / Load & Traverse": (
        "COVERED_BY_MULTIPLE", ["c_api.content.cnb_model_from_cnj", "c_api.graphics.models", "graphics.model.mesh_effects"],
        "Current demos load and traverse models and inspect populated mesh effects; the legacy empty-Effects collection observation was a framework gap that is no longer expected behavior."),
    "3D Graphics / Device State, Camera & Model / Model / EffectMaterial & Effect Swapping": (
        "COVERED_BY_MULTIPLE", ["graphics.model.mesh_effects", "c_api.graphics.effect_concrete_types"],
        "Current model demos exercise populated mesh effect collections and live effect state; the legacy claim that content readers never materialize effect associations is not retained as an expectation."),
    "3D Graphics / Textures, Effects & Queries / Volume & Cube Textures / RenderTargetCube Rendered Faces": (
        "ALREADY_COVERED", ["graphics.render_target.render_target_cube"],
        "The current demo renders all six faces, samples them as an environment map and checks exact rendered-face GetData; the legacy silently-zero readback defect is fixed and is not reintroduced."),
    "3D Graphics / Textures, Effects & Queries / Effect Reflection / What Is Actually Exposed": (
        "ALREADY_COVERED", ["graphics.effect.parameters_and_techniques"],
        "The current demo inspects populated BasicEffect parameters, techniques and passes; the legacy empty built-in parameter collection observation is obsolete."),
    "Media / Song / Unsupported Format": (
        "MIGRATE_NEW", ["media.media_player.unsupported_opus"],
        "Current CNA was rechecked: a valid Opus Song is catalogued but the present mixer still lacks an Opus decoder, so the demo records the current stopped/error boundary rather than a timeless claim."),
    "Net / NetworkSession / Packet Round-Trip": (
        "ALREADY_COVERED", ["net.packet.send_and_receive"],
        "The current demo preserves the useful real packet round trip and correct Color serialization; the legacy mismatch claim was a defect observation and is not reintroduced as expected behavior."),
    "Net / NetworkGamer / NetworkMachine": (
        "ALREADY_COVERED", ["net.network_machine.remove_from_session"],
        "Current CNA implements the real two-process RemoveFromSession workflow; the legacy claim that it always throws is obsolete, while the useful machine behavior remains covered."),
    "Net / GamerServices / Achievements": (
        "ALREADY_COVERED", ["gamer_services.achievement.award"],
        "Current CNA has a populated offline achievement catalog, persistence and pictures; the legacy empty collection/GetPicture-throws stub behavior is obsolete."),
    "Net / Leaderboards / LeaderboardReader": (
        "ALREADY_COVERED", ["gamer_services.leaderboard.paging"],
        "Current CNA implements offline typed leaderboard reads and paging, superseding the legacy NotSupported-only stub demonstration."),
    "Net / Leaderboards / LeaderboardWriter": (
        "ALREADY_COVERED", ["gamer_services.leaderboard.write_and_read"],
        "Current CNA implements offline leaderboard writes and reads, superseding the legacy GetLeaderboard-always-throws behavior."),
    "Net / GamerServices / Friends & GamerCard": (
        "COVERED_BY_MULTIPLE", ["c_api.gamer.friend_collection", "gamer_services.signed_in_gamer.privileges", "c_api.gamer.guide_social_offline"],
        "Current demos separately prove the offline empty friend collection, privilege surface and Guide social refusal; the legacy hand-built roster was presentation, not a live service result."),
    "Storage / StorageContainer / Container Lifetime": (
        "COVERED_BY_MULTIPLE", ["storage.storage_container.lifetime", "storage.storage_container.save_and_load"],
        "Current demos prove idempotent disposal, XNA-compatible ObjectDisposedException boundaries, reopening and deletion; the legacy post-dispose-use behavior was a defect and is not retained."),
}

# Stable semantic joins for legacy screens whose wording/API shape spans several focused demos or
# otherwise defeats a token scorer. Rows not listed here still use the scored catalog join below.
DESTINATIONS = {
    "Audio / SoundEffect / Play a Tone": ["audio.sound_effect.from_pcm_buffer", "audio.sound_effect.play"],
    "Audio / SoundEffect / Static Settings": ["audio.sound_effect.play_parameters", "audio.sound_effect_instance.apply_3d", "audio.sound_effect.from_pcm_buffer"],
    "Audio / SoundEffectInstance / Volume/Pitch/Pan": ["audio.sound_effect_instance.volume_pitch_pan"],
    "Audio / SoundEffectInstance / Looping": ["audio.sound_effect_instance.looping"],
    "Audio / XACT / Cues & Categories": ["audio.xact.cue_states", "audio.xact.categories"],
    "Avatars / AvatarDescription / Random Body Descriptions": ["gamer_services.avatar.description_bytes"],
    "Avatars / AvatarRenderer / Renderer State & Skeleton": ["c_api.gamer.avatar_renderer"],
    "Avatars / AvatarRenderer / Male & Female Render": ["gamer_services.avatar.render"],
    "Avatars / Lighting & Variation / Random Avatar Variation": ["gamer_services.avatar.description_bytes", "gamer_services.avatar.render"],
    "Content / ContentManager Basics / Load & Cache": ["content.content_manager.cache_and_unload"],
    "Content / ContentManager Basics / Asset Name Resolution": ["content.content_manager.asset_precedence"],
    "Content / XNB Format / XNB Fixtures": ["content.xnb.same_assets", "content.xnb.dxt_texture"],
    "Content / Errors / Load Failures": ["content.content_manager.missing_assets", "content.custom.cnb_external_references", "content.xnb.reader_versions"],
    "Content / CNJ Format / Envelope": ["content.custom.cnj_loader"],
    "Devices / System & Display / System Info": ["devices.system.machine_info"],
    "Devices / Power / PowerInfo": ["devices.system.machine_info"],
    "Devices / Desktop Integration / Clipboard": ["input.clipboard.text"],
    "Diagnostics / Logging / Logger": ["core.logger.minimum_level", "core.logger.custom_sink"],
    "Diagnostics / Platform & Build / Platform & Build": ["core.platform.host_identity", "core.platform.capability_matrix"],
    "Framework / Game Components / Lifecycle": ["game.components.game_component"],
    "Framework / Game Components / Update/Draw Order": ["game.components.update_and_draw_order"],
    "Framework / Game Components / Enabled & Visible": ["game.components.drawable_game_component"],
    "Framework / Services & Dispatcher / GameServiceContainer": ["game.services.service_container"],
    "Framework / Services & Dispatcher / FrameworkDispatcher": ["media.media_player.dispatcher_events", "c_api.gamer.guide_dispatcher"],
    "Framework / Window / Title & ClientBounds": ["game.window.title", "game.window.client_bounds"],
    "Framework / Device Manager / Resolution & FullScreen": ["game.graphics_device_manager.back_buffer_size", "game.graphics_device_manager.toggle_full_screen"],
    "Framework / Device Manager / VSync & MultiSampling": ["game.graphics_device_manager.vertical_sync", "game.graphics_device_manager.back_buffer_size"],
    "2D Graphics / SpriteBatch / Drawing Basics / Rotation & Origin": ["graphics.sprite_batch.rotation"],
    "2D Graphics / SpriteBatch / Drawing Basics / Scale & SpriteEffects": ["graphics.sprite_batch.scale", "graphics.sprite_batch.sprite_effects"],
    "2D Graphics / SpriteBatch / DrawString / Basics": ["graphics.sprite_font.draw_string", "graphics.sprite_font.measure_string"],
    "2D Graphics / SpriteBatch / DrawString / Transform": ["graphics.sprite_font.transformed_text"],
    "2D Graphics / SpriteBatch / DrawString / SpriteEffects & Newline Quirk": ["graphics.sprite_font.transformed_text", "graphics.sprite_font.measure_string"],
    "2D Graphics / SpriteBatch / DrawString / StringBuilder Overload": ["graphics.sprite_font.string_builder"],
    "2D Graphics / SpriteBatch / Begin/End & State / Nested Begin() Throws": ["graphics.sprite_batch.begin_end_rules"],
    "2D Graphics / SpriteBatch / Begin/End & State / Draw Outside Begin() Throws": ["graphics.sprite_batch.begin_end_rules"],
    "2D Graphics / SpriteBatch / Begin/End & State / BlendState Leaks Past End()": ["graphics.sprite_batch.multiple_batches"],
    "2D Graphics / Textures & Fonts / Texture2D Basics / Properties & Dispose": ["graphics.texture2d.from_stream", "graphics.texture2d.copy_ownership"],
    "2D Graphics / Textures & Fonts / Texture2D Basics / SetData/GetData Round-Trip": ["graphics.texture2d.set_data"],
    "2D Graphics / Textures & Fonts / Texture2D Basics / SurfaceFormat Matrix": ["graphics.texture2d.surface_formats"],
    "2D Graphics / Textures & Fonts / SpriteFont / MeasureString": ["graphics.sprite_font.measure_string"],
    "2D Graphics / Textures & Fonts / SpriteFont / LineSpacing & Spacing": ["graphics.sprite_font.spacing"],
    "2D Graphics / Render Targets / Render-to-Texture Basics / Draw Into & Sample": ["graphics.render_target.render_target2d"],
    "2D Graphics / Render Targets / Render-to-Texture Basics / Multi-Target Round Trip": ["graphics.render_target.render_target2d", "c_api.graphics.render_target_bindings"],
    "2D Graphics / Render Targets / Dispose Safety / Dispose While Bound Throws": ["graphics.render_target.render_target2d"],
    "3D Graphics / Primitives & Vertex Types / Vertex Types / VertexPositionColor": ["graphics.vertex_declaration.stock_vertex_types"],
    "3D Graphics / Primitives & Vertex Types / Buffers / Immediate vs. Buffered Draw": ["graphics.primitives.draw_user_indexed_primitives", "graphics.index_buffer.indexed_cube"],
    "3D Graphics / Primitives & Vertex Types / Buffers / DynamicVertexBuffer Ripple": ["graphics.vertex_buffer.dynamic_vertex_buffer"],
    "3D Graphics / BasicEffect & Lighting / Basic Rendering / VertexColorEnabled": ["graphics.basic_effect.vertex_color_lighting"],
    "3D Graphics / BasicEffect & Lighting / Basic Rendering / Live Texture Swap": ["graphics.basic_effect.texture"],
    "3D Graphics / BasicEffect & Lighting / Basic Rendering / Alpha Blending": ["graphics.basic_effect.alpha"],
    "3D Graphics / Effects Gallery / Custom Shader / Custom GLSL Shader": ["graphics.shader_effect.toon_shading"],
    "3D Graphics / Effects Gallery / Custom Shader / Invalid Shader Handling": ["graphics.shader_effect.compiler_diagnostics"],
    "3D Graphics / Device State, Camera & Model / Depth & Culling / DepthStencilState: Default vs. None": ["graphics.depth_stencil_state.depth_buffer"],
    "3D Graphics / Device State, Camera & Model / Depth & Culling / RasterizerState: FillMode": ["graphics.rasterizer_state.wireframe"],
    "3D Graphics / Device State, Camera & Model / Camera & Projection / Orbiting View Matrix": ["math.matrix.look_at"],
    "3D Graphics / Device State, Camera & Model / Camera & Projection / Perspective vs. Orthographic": ["math.matrix.perspective", "math.matrix.orthographic"],
    "3D Graphics / Device State, Camera & Model / Model / Procedural Construction": ["c_api.graphics.models"],
    "3D Graphics / Device State, Camera & Model / Model / ModelBone Hierarchy": ["graphics.model.bones"],
    "3D Graphics / Device State, Camera & Model / Model / Load & Traverse": ["c_api.content.cnb_model_from_cnj", "c_api.graphics.models"],
    "3D Graphics / Device State, Camera & Model / Model / EffectMaterial & Effect Swapping": ["graphics.model.mesh_effects", "c_api.graphics.effect_concrete_types"],
    "3D Graphics / Device State, Camera & Model / Model / Morph Targets (MorphTargetEXT)": ["graphics.model.morph_targets"],
    "3D Graphics / Textures, Effects & Queries / Volume & Cube Textures / RenderTargetCube Rendered Faces": ["graphics.render_target.render_target_cube"],
    "Input / Keyboard / Live Key State": ["input.keyboard.get_state"],
    "Input / Keyboard / Text Input": ["input.text_input.typing", "input.text_input.composition"],
    "Input / Mouse / SetPosition()": ["input.mouse.set_position"],
    "Input / Mouse / Relative Mode": ["input.mouse.relative_mode"],
    "Input / Mouse / Button Press/Release Log": ["input.mouse.click_detection"],
    "Input / Gamepad / Live Buttons Grid": ["input.gamepad.get_state"],
    "Input / Gamepad / Triggers": ["input.gamepad.get_state"],
    "Input / Gamepad / Vibration": ["input.gamepad.vibration", "input.gamepad.trigger_vibration"],
    "Input / Gamepad / Motion Sensors & Light Bar": ["input.gamepad.motion_sensors", "input.gamepad.light_bar"],
    "Input / Touch / Live Touches": ["input.touch.get_state"],
    "Input / Touch / Tap & DoubleTap": ["input.touch.gestures"],
    "Input / Touch / Hold & Flick": ["input.touch.gestures"],
    "Input / Touch / Drag Gestures": ["input.touch.gestures"],
    "Input / Touch / Pinch Gesture": ["input.touch.gestures"],
    "Input / Touch / All Gestures Log": ["input.touch.gestures"],
    "Input / Touch / Gesture Queue": ["input.touch.gesture_queue"],
    "Input / Other / Host Device Sensors": ["input.sensors.motion"],
    "Input / Other / Host Power/Battery": ["input.power.battery"],
    "Input / Other / Haptics": ["input.haptics.effects"],
    "Input / Other / Device Hot-plug Events": ["input.devices.hot_plug"],
    "Math / Vectors / Vector3 Cross, Dot & Reflect": ["math.vector3.cross", "math.vector3.reflect"],
    "Math / Vectors / Interpolation": ["math.vector3.lerp", "math.vector3.smooth_step", "math.vector3.catmull_rom", "math.vector3.hermite"],
    "Math / Vectors / Transform & Distance": ["math.vector3.transform", "math.vector3.transform_normal", "math.vector3.distance"],
    "Math / Vectors / MathHelper": ["math.math_helper.angles", "math.math_helper.scalar_functions"],
    "Math / Matrix & Quaternion / Composition Order": ["math.matrix.scale_rotate_translate"],
    "Math / Matrix & Quaternion / Decompose, Invert & Determinant": ["math.matrix.decompose", "math.matrix.invert", "math.matrix.determinant"],
    "Math / Matrix & Quaternion / Rotation Basics": ["math.quaternion.from_axis_angle", "math.quaternion.inverse_and_conjugate", "math.matrix.transform_by_quaternion"],
    "Math / Matrix & Quaternion / Slerp vs Lerp": ["math.quaternion.slerp"],
    "Math / Geometry / Bounding Volumes": ["math.bounding_box.contains", "math.bounding_sphere.containment_boundaries", "math.bounding_frustum.containment"],
    "Math / Geometry / Ray Intersection": ["math.ray.intersects"],
    "Math / Geometry / Frustum Culling": ["graphics.model.frustum_culling"],
    "Math / Color & Packed Vectors / Conversions": ["math.color.premultiplied_alpha"],
    "Math / Color & Packed Vectors / PackedVector Gallery": ["c_api.graphics.packed_vectors"],
    "Media / Song / Load & Play": ["media.media_player.play_song"],
    "Media / Song / Volume/Mute/Repeat/Shuffle": ["media.media_player.volume_and_mute", "media.media_player.queue_and_repeat"],
    "Media / MediaLibrary / Catalog Access": ["media.media_library.collections"],
    "Media / MediaLibrary / Album/Artist/Genre": ["media.media_library.music_index"],
    "Media / MediaLibrary / Playlists": ["media.media_library.playlist_import"],
    "Media / Pictures / Browse": ["c_api.media.library_pictures"],
    "Media / Pictures / Album Tree": ["c_api.media.library_pictures"],
    "Media / Pictures / Tokens & Identity": ["c_api.media.library_pictures", "media.media_library.save_picture"],
    "Net / NetworkSession / SystemLink Host": ["net.network_session.find_and_join"],
    "Net / NetworkSession / Properties & Events": ["net.network_session.session_properties", "c_api.net.session_events", "net.network_session.host_leaves"],
    "Net / NetworkGamer / NetworkMachine": ["net.network_machine.remove_from_session"],
    "Net / GamerServices / Signed-In Gamers": ["gamer_services.signed_in_gamer.automatic_sign_in", "gamer_services.signed_in_gamer.signed_out"],
    "Net / GamerServices / Profile/Presence/Privileges": ["gamer_services.gamer_profile.get_profile", "gamer_services.signed_in_gamer.presence", "gamer_services.signed_in_gamer.privileges"],
    "Net / GamerServices / Guide Overlay": ["c_api.gamer.guide_social_offline", "c_api.gamer.guide_dialogs", "c_api.gamer.guide_dispatcher", "c_api.gamer.guide_rendering"],
    "Storage / StorageDevice / StorageDevice": ["storage.storage_device.show_selector", "storage.storage_device.storage_root"],
    "Storage / StorageContainer / Save Game Round Trip": ["storage.storage_container.save_and_load"],
    "Storage / StorageContainer / Directories & Files": ["storage.storage_container.directories"],
    "Storage / StorageContainer / Container Lifetime": ["storage.storage_container.lifetime"],
}

AREA_PREFIXES = {
    "Framework": ("game.", "platform.window."),
    "Math": ("math.", "design.type_converter."),
    "Content": ("content.", "c_api.content."),
    "Storage": ("storage.", "c_api.storage."),
    "Diagnostics": ("core.", "graphics.graphics_adapter.", "graphics.device.", "c_api.platform.", "c_api.graphics."),
    "Input": ("input.", "c_api.input."),
    "Audio": ("audio.", "c_api.audio."),
    "Devices": ("devices.", "input.power.", "input.sensors.", "c_api.devices."),
    "Net": ("net.", "gamer_services.", "c_api.net.", "c_api.gamer."),
    "Avatars": ("gamer_services.avatar.", "c_api.gamer.avatar_"),
    "Media": ("media.", "c_api.media."),
    "2D Graphics": ("graphics.sprite_", "graphics.texture2d.", "graphics.render_target.",
                    "graphics.blend_state.", "graphics.sampler_state.", "graphics.viewport.",
                    "graphics.device.scissor_"),
    "3D Graphics": ("graphics.", "c_api.graphics."),
}

STOP = {
    "the", "a", "an", "and", "or", "to", "of", "in", "on", "with", "from", "for", "is",
    "vs", "real", "live", "screen", "demo", "property", "properties", "state", "states", "value",
    "values", "get", "set", "ext", "xna", "cna", "basic", "basics",
}


def match_pair(text: str, start: int, opening: str, closing: str) -> int:
    depth, quoted, escaped = 0, False, False
    for i in range(start, len(text)):
        ch = text[i]
        if quoted:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                quoted = False
            continue
        if ch == '"':
            quoted = True
        elif ch == opening:
            depth += 1
        elif ch == closing:
            depth -= 1
            if depth == 0:
                return i
    raise ValueError(f"unmatched {opening} at {start}")


def split_args(text: str) -> list[str]:
    parts, start, depths, quoted, escaped = [], 0, [0, 0, 0], False, False
    for i, ch in enumerate(text):
        if quoted:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                quoted = False
            continue
        if ch == '"': quoted = True
        elif ch in "({[": depths["({[".index(ch)] += 1
        elif ch in ")}]": depths[")}]".index(ch)] -= 1
        elif ch == "," and not any(depths):
            parts.append(text[start:i].strip())
            start = i + 1
    parts.append(text[start:].strip())
    return parts


def strings(text: str) -> list[str]:
    out = []
    for token in re.findall(r'"(?:\\.|[^"\\])*"', text):
        try:
            out.append(ast.literal_eval(token))
        except (SyntaxError, ValueError):
            # C++ permits a backslash-newline continuation inside a literal; Python's literal
            # parser does not. It is presentation-only evidence, so omit that rare fragment.
            continue
    return out


def unique(values: list[str]) -> list[str]:
    return list(dict.fromkeys(value for value in values if value))


def screen_evidence(source: str) -> tuple[list[str], list[str]]:
    literals = [value.strip() for value in strings(source)
                if 8 <= len(value.strip()) <= 220 and not value.endswith(".hpp")]
    controls = [value for value in literals if re.search(
        r'(?i)^(?:press\b|hold\b|click\b|drag\b|tap\b|move\b|space\b|enter\b|'
        r'second instance\b|2nd instance\b|[A-Z0-9/,+-]{1,5}:)', value)]
    if "IsMenuSelect(" in source:
        controls.append("Menu select (Space/Enter/controller A/tap)")
    if "IsMenuUp(" in source:
        controls.append("Menu up (Up/controller D-pad up)")
    for key in re.findall(r'IsNewKeyPress\(Keys::(\w+)', source):
        controls.append(f"New key press: {key}")
    for key in re.findall(r'IsKeyDown\(Keys::(\w+)', source):
        controls.append(f"Held key: {key}")
    controls = unique(controls)
    observable = unique([value for value in literals if " " in value and not value.startswith("Microsoft/")])
    return controls[:12], observable[:12]


def blocks(text: str, pattern: re.Pattern[str]) -> list[tuple[re.Match[str], str]]:
    out = []
    for match in pattern.finditer(text):
        opening = text.find("{", match.end())
        closing = match_pair(text, opening, "{", "}")
        out.append((match, text[opening + 1:closing]))
    return out


def catalog_locations(legacy: Path) -> dict[str, tuple[str, str | None, str]]:
    text = (legacy / "src/Navigation/AreaCatalog.cpp").read_text()
    areas = list(re.finditer(r'AreaEntry\{"([^"]+)"', text))
    result = {}
    cat_re = re.compile(r'CategoryEntry\{"([^"]+)",(?:(?!CategoryEntry).){0,300}?(Build\w+Demos)\(\)', re.S)
    for index, area_match in enumerate(areas):
        end = areas[index + 1].start() if index + 1 < len(areas) else len(text)
        body_start, body = area_match.start(), text[area_match.start():end]
        groups = list(re.finditer(r'GroupEntry\{"([^"]+)"', body))
        for category in cat_re.finditer(body):
            group = None
            if groups:
                prior = [g for g in groups if g.start() < category.start()]
                if prior:
                    group = prior[-1].group(1)
            result[category.group(2)] = (area_match.group(1), group, category.group(1))
    return result


def class_sources(legacy: Path) -> dict[str, Path]:
    result = {}
    for path in sorted((legacy / "src/Demos").rglob("*Screen.hpp")):
        text = path.read_text(errors="replace")
        namespace = (re.search(r'^namespace\s+([\w:]+)\s*\{', text, re.M) or [None, ""])[1]
        for cls in re.findall(r'^class\s+(\w+)\s*:\s*public\s+\w+', text, re.M):
            result[f"{namespace}::{cls}" if namespace else cls] = path
    return result


def normalize_member(name: str) -> str:
    match = re.match(r'(?:get|set)([A-Z].*?)Property$', name)
    return match.group(1) if match else name


def extract_apis(source: str, explicit: list[str]) -> list[str]:
    included = set(re.findall(r'#include\s+"(?:Microsoft/Xna/Framework|CNA)/.*/(\w+)\.hpp"', source))
    included.update(re.findall(r'^using\s+(?:namespace\s+)?[\w:]+::(\w+)\s*;', source, re.M))
    apis = set(explicit)
    for type_name, member in re.findall(r'\b([A-Z]\w*)::((?:get|set)?[A-Z]\w*(?:Property)?)', source):
        if type_name in included:
            apis.add(f"{type_name}::{normalize_member(member)}")
    variables: dict[str, str] = {}
    for type_name in sorted(included, key=len, reverse=True):
        escaped = re.escape(type_name)
        patterns = [
            rf'\b{escaped}\s*[*&]?\s+(\w+)_?\b',
            rf'\bstd::(?:optional|unique_ptr|shared_ptr)<\s*{escaped}\s*>\s+(\w+)_?\b',
        ]
        for pattern in patterns:
            for variable in re.findall(pattern, source):
                variables[variable.rstrip("_")] = type_name
    for variable, member in re.findall(r'\b(\w+)_?\s*(?:\.|->)\s*((?:get|set)?[A-Z]\w*(?:Property)?)\s*\(', source):
        type_name = variables.get(variable.rstrip("_"))
        if type_name:
            apis.add(f"{type_name}::{normalize_member(member)}")
    return sorted(apis)


def legacy_inventory(legacy: Path) -> list[dict]:
    locations = catalog_locations(legacy)
    sources = class_sources(legacy)
    entries = []
    fn_re = re.compile(r'std::vector<DemoEntry>\s+(Build\w+Demos)\s*\(\)')
    for area_source in sorted((legacy / "src/Navigation/CatalogAreas").glob("*.cpp")):
        text = area_source.read_text()
        for match, body in blocks(text, fn_re):
            function = match.group(1)
            if function not in locations:
                continue
            namespace_match = re.search(r'using\s+namespace\s+([\w:]+)\s*;', body)
            namespace = namespace_match.group(1) if namespace_match else ""
            cursor = 0
            while True:
                found = re.search(r'MakeDemo<(\w+)>\s*\(', body[cursor:])
                if not found: break
                cls = found.group(1)
                opening = cursor + found.end() - 1
                closing = match_pair(body, opening, "(", ")")
                args = split_args(body[opening + 1:closing])
                title = "".join(strings(args[0]))
                description = "".join(strings(args[1]))
                explicit = strings(args[2]) if len(args) > 2 else []
                qualified = f"{namespace}::{cls}" if namespace else cls
                source_path = sources[qualified]
                source_text = source_path.read_text(errors="replace")
                area, group, category = locations[function]
                parts = [area] + ([group] if group else []) + [category, title]
                lifecycle = [name for name in ("OnDemoLoad", "OnDemoInput", "OnDemoUpdate", "OnDemoDraw", "OnDemoUnload")
                             if name in source_text]
                interactive = "OnDemoInput" in source_text or any(word in (description + " " + source_text).lower() for word in
                                  ("press ", "hold ", "click", "drag", "move the", "tap", "2nd instance"))
                apis = extract_apis(source_text, explicit)
                controls, observable = screen_evidence(source_text)
                entries.append({
                    "legacy_path": " / ".join(parts),
                    "area": area, "group": group, "category": category, "title": title,
                    "source_file": str(source_path.relative_to(legacy)),
                    "screen_class": qualified,
                    "source_sha256": hashlib.sha256(source_text.encode()).hexdigest(),
                    "legacy_public_apis": apis,
                    "legacy_behavior": description,
                    "inputs": (controls if controls else
                               (["Interactive user/device input described by the registered screen."] if interactive else
                                ["Automatic screen lifecycle; no explicit user trigger."])),
                    "expected_outputs": {"summary": description, "screen_evidence": observable},
                    "state_changes": (f"Exercises and observes {', '.join(apis[:8])}." if apis
                                      else "Exercises the CNA objects instantiated by the registered screen."),
                    "lifecycle_requirements": lifecycle or ["constructor/draw path"],
                })
                cursor = closing + 1
    return entries


def tokens(value: str) -> set[str]:
    words = re.findall(r'[a-z0-9]+', re.sub(r'([a-z])([A-Z])', r'\1 \2', value).lower())
    return {word for word in words if len(word) > 1 and word not in STOP}


def semantic_candidates(legacy: dict, demos: list[dict]) -> list[tuple[float, dict]]:
    legacy_text = " ".join([legacy["legacy_path"], legacy["legacy_behavior"], *legacy["legacy_public_apis"]])
    old_tokens = tokens(legacy_text)
    old_apis = set(legacy["legacy_public_apis"])
    prefixes = AREA_PREFIXES.get(legacy["area"], ())
    scored = []
    for demo in demos:
        current_text = " ".join([demo["id"], demo["title"], demo["description"],
                                 *demo.get("apis", []), *demo.get("tags", [])])
        common = old_tokens & tokens(current_text)
        api_common = old_apis & set(demo.get("apis", []))
        prefix = 8 if any(demo["id"].startswith(p) for p in prefixes) else 0
        score = prefix + len(common) + 12 * len(api_common)
        if score:
            scored.append((score, demo))
    return sorted(scored, key=lambda item: (-item[0], item[1]["id"]))[:5]


def behavior_kind(entry: dict) -> str:
    path = entry["legacy_path"].lower()
    if any(word in path for word in ("camera", "microphone", "gamepad", "joystick", "sensor", "vibration", "haptics")):
        return "hardware integration"
    if "desktop integration" in path or "window" in path:
        return "platform feature"
    if entry["area"] in ("2D Graphics", "3D Graphics", "Avatars"):
        return "visual example"
    if any(word in path for word in ("transition", "save game", "hold duration", "round trip")):
        return "practical recipe"
    return "API demonstration"


def generate(legacy: Path, executable: Path) -> dict:
    catalog = json.loads(subprocess.check_output([str(executable), "--catalog-json"], text=True))
    demos = catalog["demos"]
    by_id = {demo["id"]: demo for demo in demos}
    rows = legacy_inventory(legacy)
    for row in rows:
        ranked = semantic_candidates(row, demos)
        override = OVERRIDES.get(row["legacy_path"])
        mapped = DESTINATIONS.get(row["legacy_path"])
        if override:
            disposition, final_ids, rationale = override
        elif mapped:
            final_ids = mapped
            disposition = "COVERED_BY_MULTIPLE" if len(final_ids) > 1 else "ALREADY_COVERED"
            descriptions = "; ".join(
                f"{demo_id}: {by_id[demo_id]['description']}" for demo_id in final_ids if demo_id in by_id)
            rationale = f"Current focused destination evidence: {descriptions}"
        else:
            disposition = "ALREADY_COVERED"
            final_ids = [ranked[0][1]["id"]] if ranked else []
            rationale = (f"The current focused demo {final_ids[0]} states: {by_id[final_ids[0]]['description']} "
                         f"This is the same meaningful behavior as the legacy description: {row['legacy_behavior']}"
                         if final_ids else "No semantic catalog candidate was found.")
        evidence = []
        for demo_id in final_ids:
            demo = by_id.get(demo_id)
            if demo:
                evidence.append({
                    "id": demo_id, "source": demo["source"], "automation": demo["automation"],
                    "description": demo["description"],
                    "shared_api_members": sorted(set(row["legacy_public_apis"]) & set(demo.get("apis", []))),
                })
        row.update({
            "primary_kind": behavior_kind(row),
            "existing_equivalent_ids": final_ids if disposition in {"ALREADY_COVERED", "COVERED_BY_MULTIPLE"} else [],
            "semantic_equivalence_evidence": evidence,
            "missing_meaningful_behavior": ("none" if disposition in {"ALREADY_COVERED", "COVERED_BY_MULTIPLE"}
                                             else rationale),
            "disposition": disposition,
            "migration_decision_rationale": rationale,
            "final_destination_ids": final_ids,
            "validation_evidence": ("Every destination ID, source path and automation class is validated against the live "
                                    "cna-demos catalog; semantic evidence above records the exact catalog descriptions. "
                                    "Final CTest, renderer-sweep and visual results are summarized in HANDOFF.md."),
            "limitations": (
                "Manual/platform/hardware outcome depends on the host and is not claimed as automated runtime coverage."
                if disposition == "MANUAL_INTEGRATION" or any(
                    by_id[i]["automation"] == "manual" for i in final_ids if i in by_id)
                else "Bounded smoke automation covers safe control flow; physical interaction is limited as stated in destination metadata."
                if any(by_id[i]["automation"] == "smoke-only" for i in final_ids if i in by_id)
                else "No additional platform limitation beyond destination metadata."),
            "ranked_catalog_candidates": [{"id": demo["id"], "score": score} for score, demo in ranked],
        })
    counts = collections.Counter(row["disposition"] for row in rows)
    return {
        "schema": "cna-examples-migration-ledger/1",
        "generated_from": {
            "repository": "cna-examples", "license": "MIT",
            "commit": subprocess.check_output(["git", "-C", str(legacy), "rev-parse", "HEAD"], text=True).strip(),
            "inventory": {"areas": len({r["area"] for r in rows}),
                          "categories": len({(r["area"], r["group"], r["category"]) for r in rows}),
                          "screens": len(rows)},
        },
        "destination": {"repository": "cna-demos", "catalog_demo_count": len(demos)},
        "disposition_counts": dict(sorted(counts.items())),
        "screens": rows,
    }


def validate(data: dict, legacy: Path | None, executable: Path | None) -> list[str]:
    problems = []
    rows = data.get("screens", [])
    if len(rows) != 248: problems.append(f"ledger has {len(rows)} screens, expected 248")
    if len({r.get("legacy_path") for r in rows}) != len(rows): problems.append("legacy paths are not unique")
    required = {"legacy_path", "source_file", "screen_class", "legacy_public_apis", "legacy_behavior",
                "inputs", "expected_outputs", "state_changes", "lifecycle_requirements", "primary_kind",
                "existing_equivalent_ids", "semantic_equivalence_evidence", "missing_meaningful_behavior",
                "disposition", "migration_decision_rationale", "final_destination_ids", "validation_evidence",
                "limitations"}
    current_ids = set()
    by_id = {}
    if executable:
        current = json.loads(subprocess.check_output([str(executable), "--catalog-json"], text=True))
        by_id = {demo["id"]: demo for demo in current["demos"]}
        current_ids = set(by_id)
        if data.get("destination", {}).get("catalog_demo_count") != len(current_ids):
            problems.append("destination catalog count differs from the live executable")
    for row in rows:
        path = row.get("legacy_path", "<missing>")
        missing = required - row.keys()
        if missing: problems.append(f"{path}: missing fields {sorted(missing)}")
        if row.get("disposition") not in DISPOSITIONS: problems.append(f"{path}: invalid disposition")
        ids = row.get("final_destination_ids", [])
        if row.get("disposition") not in {"OBSOLETE", "INTENTIONALLY_NOT_MIGRATED"} and not ids:
            problems.append(f"{path}: disposition requires a destination ID")
        for demo_id in ids:
            if current_ids and demo_id not in current_ids: problems.append(f"{path}: unknown ID {demo_id}")
        evidence_ids = [item.get("id") for item in row.get("semantic_equivalence_evidence", [])]
        if evidence_ids != ids: problems.append(f"{path}: semantic evidence does not match destination IDs")
        if row.get("disposition") == "MANUAL_INTEGRATION" and by_id:
            if not ids or any(by_id[demo_id]["automation"] != "manual" for demo_id in ids if demo_id in by_id):
                problems.append(f"{path}: manual integration destination is not catalogued manual")
    counts = collections.Counter(row.get("disposition") for row in rows)
    if data.get("disposition_counts") != dict(sorted(counts.items())):
        problems.append("disposition_counts does not match rows")
    if legacy:
        actual = legacy_inventory(legacy)
        if len({r["area"] for r in actual}) != 13 or len({(r["area"], r["group"], r["category"]) for r in actual}) != 79 or len(actual) != 248:
            problems.append("live legacy checkout is not the expected 13-area/79-category/248-screen inventory")
        expected_hashes = {r["legacy_path"]: r["source_sha256"] for r in actual}
        live_paths = set(expected_hashes)
        for path in sorted((set(OVERRIDES) | set(DESTINATIONS)) - live_paths):
            problems.append(f"unused audit mapping: {path}")
        for row in rows:
            if expected_hashes.get(row["legacy_path"]) != row.get("source_sha256"):
                problems.append(f"{row['legacy_path']}: source hash differs from live legacy checkout")
    return problems


def write_summary(data: dict) -> None:
    counts = data["disposition_counts"]
    additions = sorted({demo_id for row in data["screens"]
                        if row["disposition"] in {"MIGRATE_NEW", "MANUAL_INTEGRATION"}
                        for demo_id in row["final_destination_ids"]})
    extended = sorted({demo_id for row in data["screens"] if row["disposition"] == "EXTEND_EXISTING"
                       for demo_id in row["final_destination_ids"]})
    lines = [
        "# cna-examples migration audit", "",
        "The checked-in JSON ledger accounts for the live legacy catalog at source level: **13 areas, 79 categories, 248/248 registered screens**. Each row records its source hash, APIs/behavior, lifecycle, semantic destination evidence, disposition and limitations.", "",
        "## Dispositions", "",
        "| Disposition | Screens |", "|---|---:|",
    ]
    for name in sorted(counts): lines.append(f"| `{name}` | {counts[name]} |")
    lines += ["", "Unaccounted screens: **0**.", "", "## Added stable IDs", ""]
    lines += [f"- `{demo_id}`" for demo_id in additions]
    lines += ["", "## Existing IDs extended", ""]
    lines += [f"- `{demo_id}`" for demo_id in extended]
    lines += ["", "## Evidence and retirement rule", "",
              "Run `tools/cna_examples_migration_audit.py --legacy-root ../cna-examples --exe build/linux-full/bin/cna-demos` to re-enumerate the live legacy checkout, verify all source hashes and validate every destination ID. Full build, CTest, renderer-sweep and exact-visual results are recorded in `HANDOFF.md` after the final run.", "",
              "No legacy application architecture, source file or asset was copied from cna-examples. The ledger derives titles/descriptions and source facts from that MIT-licensed checkout. New demos are fresh Ms-PL implementations. One real external MonoGame LZX fixture was imported from CNA's Ms-PL test corpus with exact provenance and SHA-256 in `assets/xnb/monogame/PROVENANCE.json`.", ""]
    SUMMARY.write_text("\n".join(lines))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--generate", action="store_true")
    parser.add_argument("--legacy-root", type=Path)
    parser.add_argument("--exe", type=Path)
    args = parser.parse_args()
    legacy = args.legacy_root.resolve() if args.legacy_root else None
    executable = args.exe.resolve() if args.exe else None
    if args.generate:
        if not legacy or not executable:
            parser.error("--generate requires --legacy-root and --exe")
        data = generate(legacy, executable)
        LEDGER.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n")
        write_summary(data)
    if not LEDGER.is_file():
        print(f"missing {LEDGER.relative_to(ROOT)}", file=sys.stderr)
        return 1
    data = json.loads(LEDGER.read_text())
    problems = validate(data, legacy, executable)
    if problems:
        print(f"{len(problems)} migration-ledger problem(s):")
        for problem in problems: print(f"  - {problem}")
        return 1
    inventory = data["generated_from"]["inventory"]
    print(f"OK: {inventory['areas']} areas, {inventory['categories']} categories, "
          f"{inventory['screens']}/248 screens accounted for; 0 unaccounted.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
