#!/usr/bin/env python3
# SPDX-License-Identifier: MS-PL
"""Static regression guard for unattended hardware/desktop side-effect gates.

This complements runtime smoke tests: CI cannot prove that a motor, microphone or desktop shell
was untouched, so the guard pins the consent branches, ownership cleanup and safe C-ABI seams that
the retirement audit reviewed. It intentionally covers named demonstrations rather than trying to
parse arbitrary C++ control flow.
"""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def text(path: str) -> str:
    return (ROOT / path).read_text()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exe", required=True, type=Path)
    args = parser.parse_args()
    catalog = json.loads(subprocess.check_output([str(args.exe.resolve()), "--catalog-json"], text=True))
    demos = {demo["id"]: demo for demo in catalog["demos"]}
    problems: list[str] = []
    checks = 0

    expected_automation = {
        "audio.microphone.input_level": "smoke-only",
        "devices.vibrate_controller.start_stop": "smoke-only",
        "input.gamepad.vibration": "smoke-only",
        "input.gamepad.trigger_vibration": "smoke-only",
        "input.gamepad.light_bar": "smoke-only",
        "input.gamepad.device_info": "smoke-only",
        "input.haptics.effects": "smoke-only",
        "input.haptics.rumble": "smoke-only",
        "input.mouse.set_position": "smoke-only",
        "input.mouse.relative_mode": "smoke-only",
        "input.mouse.cursors": "smoke-only",
        "input.clipboard.text": "smoke-only",
        "input.clipboard.data_formats": "smoke-only",
        "devices.camera.live_capture": "manual",
        "devices.sensors.live_readings": "manual",
        "devices.desktop.message_box": "manual",
        "devices.desktop.file_dialog": "manual",
        "devices.desktop.system_tray": "manual",
        "devices.desktop.url_launcher": "manual",
        "input.mouse.capture": "manual",
        "input.mouse.global_position_and_warp": "manual",
    }
    for demo_id, automation in expected_automation.items():
        checks += 1
        demo = demos.get(demo_id)
        if demo is None or demo.get("automation") != automation:
            problems.append(f"{demo_id}: expected automation {automation!r}")
        elif not demo.get("restrictions"):
            problems.append(f"{demo_id}: side-effect limitation is not documented")

    required_fragments = {
        "src/framework/Runner.cpp": [
            "options.automated && demo.entry.info.automation == Automation::Manual",
        ],
        "src/app/Cli.cpp": [
            'args.run.automated && std::getenv("SDL_AUDIO_DRIVER") == nullptr',
            'SetEnvironment("SDL_AUDIO_DRIVER", "dummy")',
        ],
        "demos/audio/microphone/input_level.cpp": [
            "BufferReady.Add", "BufferReady.Remove", "std::weak_ptr<CaptureCallbackState>",
            "!Context().Automated()", "if (!ownsCapture_)",
        ],
        "demos/devices/vibrate_controller/start_stop.cpp": [
            "!Context().Automated()", "StopOwnedOutput", "if (!ownsOutput_)",
        ],
        "demos/input/gamepad/vibration.cpp": [
            "!Context().Automated()", "StopOwnedVibration", "if (!ownsVibration_)",
        ],
        "demos/input/gamepad/trigger_vibration.cpp": [
            "!Context().Automated()", "StopOwnedVibration", "if (!ownsVibration_)",
        ],
        "demos/input/gamepad/light_bar.cpp": [
            "!Context().Automated()", "public API has no light-bar getter",
        ],
        "demos/input/gamepad/device_info.cpp": [
            "!Context().Automated()", "originalLed_", "if (changed_",
        ],
        "demos/input/haptics/effects.cpp": [
            "!Context().Automated()", "OpenAndUpload", "Gain is never changed",
        ],
        "demos/input/haptics/rumble.cpp": [
            "!Context().Automated()", "playedRumble_", "device_.Dispose()",
        ],
        "standalone/c_api.input.keyboard_mouse/main.c": [
            "!state->demo->automated", "state->manual_controls_run = 1",
        ],
        "standalone/c_api.input.gamepad/main.c": [
            "(CNA_PlayerIndex)99", "reject an invalid player before platform access",
        ],
        "standalone/c_api.devices.sensor_lifecycle/main.c": [
            "without starting physical acquisition", "cna_compass_set_test_backend_ext",
            "cna_motion_set_test_backend_ext",
        ],
        "standalone/c_api.input.clipboard/main.c": [
            "Read-only, race-tolerant clipboard text inspection",
        ],
    }
    for path, fragments in required_fragments.items():
        source = text(path)
        for fragment in fragments:
            checks += 1
            if fragment not in source:
                problems.append(f"{path}: missing safety evidence {fragment!r}")

    forbidden_fragments = {
        "standalone/c_api.audio.microphone_availability/main.c": [
            "cna_microphone_start_at(game, default_index)",
            "cna_microphone_get_data_at(game, default_index",
            "cna_microphone_set_buffer_duration_ticks_at(\n                game, default_index",
        ],
        "standalone/c_api.input.haptics/main.c": [
            "cna_haptics_open(game, id,", "cna_haptics_open_from_mouse(game",
        ],
        "standalone/c_api.input.clipboard/main.c": [
            "cna_clipboard_set_text(", "cna_devices_clipboard_set_text_ext(",
        ],
        "demos/input/gamepad/light_bar.cpp": [
            "void UnloadContent() override",
        ],
    }
    for path, fragments in forbidden_fragments.items():
        source = text(path)
        for fragment in fragments:
            checks += 1
            if fragment in source:
                problems.append(f"{path}: forbidden unattended side-effect pattern {fragment!r}")

    if problems:
        print(f"{len(problems)} hardware-safety audit problem(s):")
        for problem in problems:
            print(f"  - {problem}")
        return 1
    print(f"OK: {checks} hardware/desktop safety invariants pinned; no known unattended mutation path.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
