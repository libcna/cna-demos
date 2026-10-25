/* SPDX-License-Identifier: MS-PL */
/*
 * Haptics from C: compare the public effect values, enumerate any host devices, then exercise the
 * complete closed-device contract without claiming physical force feedback.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct HapticsDemoState {
    CnaDemo* demo;
    int validated;
} HapticsDemoState;

static CNA_StringView text_of(const char* value)
{
    CNA_StringView view;
    view.data = value;
    view.byte_length = (uint64_t)strlen(value);
    return view;
}

static CNA_StringView empty_text(void)
{
    CNA_StringView view;
    view.data = NULL;
    view.byte_length = UINT64_C(0);
    return view;
}

static int effect_is_default(const CNA_HapticEffect* effect)
{
    int index;
    if (effect->struct_size != (uint32_t)sizeof(*effect) ||
        effect->struct_version != UINT32_C(1) ||
        effect->type != CNA_HAPTIC_EFFECT_TYPE_CONSTANT || effect->reserved != UINT32_C(0) ||
        effect->direction.type != CNA_HAPTIC_DIRECTION_TYPE_POLAR ||
        effect->direction.values[0] != 0 || effect->direction.values[1] != 0 ||
        effect->direction.values[2] != 0 || effect->length != UINT32_C(0) ||
        effect->delay != UINT16_C(0) || effect->button != UINT16_C(0) ||
        effect->interval != UINT16_C(0) || effect->level != 0 || effect->period != UINT16_C(0) ||
        effect->magnitude != 0 || effect->offset != 0 || effect->phase != UINT16_C(0) ||
        effect->ramp_start != 0 || effect->ramp_end != 0 ||
        effect->large_magnitude != UINT16_C(0) || effect->small_magnitude != UINT16_C(0) ||
        effect->custom_period != UINT16_C(0) || effect->custom_channels != UINT8_C(0) ||
        effect->reserved2 != UINT8_C(0) || effect->attack_length != UINT16_C(0) ||
        effect->attack_level != UINT16_C(0) || effect->fade_length != UINT16_C(0) ||
        effect->fade_level != UINT16_C(0)) {
        return 0;
    }
    for (index = 0; index < 3; ++index) {
        if (effect->right_saturation[index] != UINT16_C(0) ||
            effect->left_saturation[index] != UINT16_C(0) ||
            effect->right_coefficient[index] != 0 || effect->left_coefficient[index] != 0 ||
            effect->deadband[index] != UINT16_C(0) || effect->center[index] != 0) {
            return 0;
        }
    }
    return 1;
}

static int capabilities_are_closed(const CNA_HapticCapabilities* capabilities)
{
    return capabilities->struct_size == (uint32_t)sizeof(*capabilities) &&
           capabilities->struct_version == UINT32_C(1) &&
           capabilities->features == CNA_HAPTIC_FEATURE_NONE && capabilities->axis_count == 0 &&
           capabilities->max_effects == -1 && capabilities->max_effects_playing == -1 &&
           capabilities->is_open == CNA_FALSE && capabilities->rumble_supported == CNA_FALSE &&
           capabilities->reserved[0] == UINT8_C(0) && capabilities->reserved[1] == UINT8_C(0);
}

static void value_contract(CnaDemo* demo)
{
    static const uint16_t samples[4] = {1U, 2U, 3U, 4U};
    static const uint16_t changed_samples[4] = {1U, 2U, 3U, 5U};
    CNA_HapticDirection direction;
    CNA_HapticDirection other_direction;
    CNA_HapticEffect effect;
    CNA_HapticEffect other_effect;
    CNA_HapticCapabilities capabilities;
    CNA_HapticCapabilities other_capabilities;
    CNA_Bool equal = UINT8_C(9);
    CNA_Bool same_equal;
    CNA_Bool changed_equal;
    CNA_Bool samples_equal;
    CNA_Bool name_equal;
    CNA_Result same_result;
    CNA_Result changed_result;
    CNA_Result samples_result;
    CNA_Result name_result;

    memset(&direction, 9, sizeof(direction));
    cna_demo_check(
        demo,
        cna_haptic_direction_init(&direction) == CNA_RESULT_SUCCESS &&
            direction.type == CNA_HAPTIC_DIRECTION_TYPE_POLAR && direction.values[0] == 0 &&
            direction.values[1] == 0 && direction.values[2] == 0,
        "the default haptic direction is a zero polar value");
    other_direction = direction;
    same_result = cna_haptic_direction_equals(&direction, &other_direction, &equal);
    same_equal = equal;
    other_direction.values[2] = 1;
    changed_result = cna_haptic_direction_equals(&direction, &other_direction, &equal);
    changed_equal = equal;
    cna_demo_check(
        demo,
        same_result == CNA_RESULT_SUCCESS && same_equal == CNA_TRUE &&
            changed_result == CNA_RESULT_SUCCESS && changed_equal == CNA_FALSE,
        "direction equality observes all three components, including an unused polar component");

    memset(&effect, 9, sizeof(effect));
    cna_demo_check(
        demo,
        cna_haptic_effect_init(&effect) == CNA_RESULT_SUCCESS && effect_is_default(&effect),
        "the default haptic effect is a fully zeroed versioned constant-force value");
    other_effect = effect;
    same_result = cna_haptic_effect_equals(
        &effect, samples, UINT64_C(4), &other_effect, samples, UINT64_C(4), &equal);
    same_equal = equal;
    other_effect.large_magnitude = UINT16_C(7);
    changed_result = cna_haptic_effect_equals(
        &effect, samples, UINT64_C(4), &other_effect, samples, UINT64_C(4), &equal);
    changed_equal = equal;
    other_effect = effect;
    samples_result = cna_haptic_effect_equals(
        &effect, samples, UINT64_C(4), &other_effect, changed_samples, UINT64_C(4), &equal);
    samples_equal = equal;
    cna_demo_check(
        demo,
        same_result == CNA_RESULT_SUCCESS && same_equal == CNA_TRUE &&
            changed_result == CNA_RESULT_SUCCESS && changed_equal == CNA_FALSE &&
            samples_result == CNA_RESULT_SUCCESS && samples_equal == CNA_FALSE,
        "effect equality observes ignored family fields and caller-owned custom samples");

    memset(&capabilities, 9, sizeof(capabilities));
    cna_demo_check(
        demo,
        cna_haptic_capabilities_init(&capabilities) == CNA_RESULT_SUCCESS &&
            capabilities_are_closed(&capabilities),
        "default haptic capabilities distinguish closed unknown limits from zero capacity");
    other_capabilities = capabilities;
    same_result = cna_haptic_capabilities_equals(
        &capabilities, text_of("wheel"), &other_capabilities, text_of("wheel"), &equal);
    same_equal = equal;
    other_capabilities.features = CNA_HAPTIC_FEATURE_CONSTANT | CNA_HAPTIC_FEATURE_PAUSE;
    changed_result = cna_haptic_capabilities_equals(
        &capabilities, empty_text(), &other_capabilities, empty_text(), &equal);
    changed_equal = equal;
    name_result = cna_haptic_capabilities_equals(
        &capabilities, text_of("wheel"), &capabilities, text_of("stick"), &equal);
    name_equal = equal;
    cna_demo_check(
        demo,
        same_result == CNA_RESULT_SUCCESS && same_equal == CNA_TRUE &&
            changed_result == CNA_RESULT_SUCCESS && changed_equal == CNA_FALSE &&
            name_result == CNA_RESULT_SUCCESS && name_equal == CNA_FALSE,
        "capability equality observes feature bits and the external UTF-8 device name");
}

static void enumerate_haptics(CNA_Handle game, CnaDemo* demo)
{
    uint32_t count = UINT32_C(99);
    uint32_t index;
    uint32_t id = UINT32_C(99);
    uint64_t bytes = UINT64_C(99);
    char name[256];
    int entries_ok = 1;

    if (!cna_demo_check(demo, cna_haptics_get_count(game, &count) == CNA_RESULT_SUCCESS,
                        "the host haptic-device count is available and zero is ordinary")) {
        return;
    }
    printf("  haptic devices: %u\n", (unsigned)count);
    for (index = UINT32_C(0); index < count; ++index) {
        memset(name, 0, sizeof(name));
        entries_ok = entries_ok && cna_haptics_get_id_at(game, index, &id) == CNA_RESULT_SUCCESS &&
                     cna_haptics_get_name_size_at(game, index, &bytes) == CNA_RESULT_SUCCESS &&
                     bytes < (uint64_t)sizeof(name) &&
                     cna_haptics_copy_name_at(game, index, name, sizeof(name), &bytes) ==
                         CNA_RESULT_SUCCESS &&
                     strlen(name) == (size_t)bytes;
        if (entries_ok) {
            printf("    #%u %s\n", (unsigned)id, name);
        }
    }
    cna_demo_check(demo, entries_ok,
                   "every reported haptic device has an exact id and name without being opened");
    cna_demo_check(
        demo,
        cna_haptics_get_id_at(game, count, &id) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_haptics_get_name_size_at(game, count, &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_haptics_copy_name_at(game, count, name, sizeof(name), &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "the index equal to the haptic count is refused by id and both name routes");
}

static void closed_device_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_HapticDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_HapticEffect effect;
    CNA_HapticCapabilities capabilities;
    CNA_Bool open = UINT8_C(9);
    CNA_Bool applied = UINT8_C(9);
    uint64_t bytes = UINT64_C(99);
    unsigned char name_guard[8];
    int32_t effect_id = 12345;

    cna_demo_check(
        demo,
        cna_haptics_open(game, UINT32_C(0xFFFFFFFF), &device) == CNA_RESULT_SUCCESS &&
            device != CNA_INVALID_HANDLE &&
            cna_haptic_device_get_is_open(device, &open) == CNA_RESULT_SUCCESS && open == CNA_FALSE,
        "opening an absent id returns an owned device that reports closed");
    memset(name_guard, 0x7f, sizeof(name_guard));
    cna_demo_check(
        demo,
        cna_haptic_device_get_name_size(device, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_haptic_device_copy_name(device, (char*)name_guard, sizeof(name_guard), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) && name_guard[0] == 0x7fU,
        "a closed haptic device has an empty exact name copy without touching the destination");
    cna_haptic_capabilities_init(&capabilities);
    cna_demo_check(
        demo,
        cna_haptic_device_get_capabilities(device, &capabilities) == CNA_RESULT_SUCCESS &&
            capabilities_are_closed(&capabilities),
        "a closed device returns the canonical closed capabilities");
    cna_haptic_effect_init(&effect);
    cna_demo_check(
        demo,
        cna_haptic_device_get_is_effect_supported(device, &effect, NULL, UINT64_C(0), &applied) ==
                CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE,
        "a closed device reports the authored constant effect unsupported");
    cna_demo_check(
        demo,
        cna_haptic_device_init_rumble(device, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_play_rumble(device, 0.75f, UINT32_C(300), &applied) ==
                CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_stop_rumble(device, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE,
        "closed-device rumble initialization, play and stop answer false without failing");
    cna_demo_check(
        demo,
        cna_haptic_device_create_effect(device, &effect, NULL, UINT64_C(0), &effect_id) ==
                CNA_RESULT_SUCCESS &&
            effect_id == -1,
        "a closed device cannot store the effect and reports id minus one");
    cna_demo_check(
        demo,
        cna_haptic_device_update_effect(device, effect_id, &effect, NULL, UINT64_C(0), &applied) ==
                CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_run_effect(device, effect_id, UINT32_C(1), &applied) ==
                CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_get_effect_status(device, effect_id, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_stop_effect(device, effect_id, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_destroy_effect(device, effect_id) == CNA_RESULT_SUCCESS,
        "closed-device effect update, run, status, stop and destroy are safe negative answers");
    cna_demo_check(
        demo,
        cna_haptic_device_stop_all_effects(device, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_set_gain(device, 80, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_set_autocenter(device, 60, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_pause(device, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE &&
            cna_haptic_device_resume(device, &applied) == CNA_RESULT_SUCCESS &&
            applied == CNA_FALSE,
        "closed-device global stop, gain, autocenter, pause and resume all answer false");
    cna_demo_check(
        demo,
        cna_haptic_device_dispose(device) == CNA_RESULT_SUCCESS &&
            cna_haptic_device_dispose(device) == CNA_RESULT_SUCCESS &&
            cna_haptic_device_get_is_open(device, &open) == CNA_RESULT_SUCCESS && open == CNA_FALSE &&
            cna_haptic_device_destroy(device) == CNA_RESULT_SUCCESS,
        "device disposal is idempotent while handle destruction releases the closed device");
}

static void alternative_capability_routes(CNA_Handle game, CnaDemo* demo)
{
    CNA_HapticDeviceHandle joystick_device = CNA_INVALID_HANDLE;
    CNA_Bool mouse_haptic = UINT8_C(9);
    CNA_Bool joystick_haptic = UINT8_C(9);
    CNA_Bool joystick_open = UINT8_C(9);

    cna_demo_check(
        demo,
        cna_haptics_get_is_mouse_haptic(game, &mouse_haptic) == CNA_RESULT_SUCCESS &&
            (mouse_haptic == CNA_FALSE || mouse_haptic == CNA_TRUE),
        "the mouse-haptic capability is inspected without opening the physical device");
    cna_demo_check(
        demo,
        cna_haptics_get_is_joystick_haptic(game, UINT32_C(0xFFFFFFFF), &joystick_haptic) ==
                CNA_RESULT_SUCCESS &&
            joystick_haptic == CNA_FALSE &&
            cna_haptics_open_from_joystick(game, UINT32_C(0xFFFFFFFF), &joystick_device) ==
                CNA_RESULT_SUCCESS &&
            joystick_device != CNA_INVALID_HANDLE &&
            cna_haptic_device_get_is_open(joystick_device, &joystick_open) == CNA_RESULT_SUCCESS &&
            joystick_open == CNA_FALSE &&
            cna_haptic_device_destroy(joystick_device) == CNA_RESULT_SUCCESS,
        "an absent joystick is non-haptic and still returns an owned closed-device handle");
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    HapticsDemoState* state = (HapticsDemoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    enumerate_haptics(game, state->demo);
    closed_device_contract(game, state->demo);
    alternative_capability_routes(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    HapticsDemoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.input.haptics", argc, argv, 0);
    value_contract(&demo);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run one frame for haptic access");
    cna_demo_check(&demo, state.validated == 1, "the update callback completed the haptics audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
