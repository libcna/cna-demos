/* SPDX-License-Identifier: MS-PL */
/* Sound, cube and caller-defined CNJ/foreign ContentManager loaders through the public C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ForeignObject {
    int marker;
} ForeignObject;

typedef struct State {
    CnaDemo* demo;
    CNA_Handle manager;
    int validated;
    int loader_calls;
    int saw_health;
    ForeignObject object;
} State;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static int contains_bytes(CNA_StringView text, const char* needle)
{
    size_t needle_length = strlen(needle);
    size_t i;
    if (needle_length > text.byte_length) {
        return 0;
    }
    for (i = 0U; i + needle_length <= (size_t)text.byte_length; ++i) {
        if (memcmp(text.data + i, needle, needle_length) == 0) {
            return 1;
        }
    }
    return 0;
}

static CNA_Result load_enemy(void* context, CNA_StringView json, void** out_object)
{
    State* state = (State*)context;
    ++state->loader_calls;
    state->saw_health = contains_bytes(json, "\"health\": 12");
    state->object.marker = 12;
    *out_object = &state->object;
    return CNA_RESULT_SUCCESS;
}

static CNA_Result exercise_loaders(
    CNA_Handle game,
    const CNA_GameTime* game_time,
    void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_ContentManagerCreateInfo create_info;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle sound = CNA_INVALID_HANDLE;
    CNA_Handle cube = CNA_INVALID_HANDLE;
    CNA_TextureCubeInfo cube_info;
    void* object = NULL;
    void* again = NULL;
    int64_t duration = 0;
    (void)game_time;
    (void)out_error;

    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "borrow the game graphics device")) {
        return CNA_RESULT_SUCCESS;
    }
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.root_directory = view("assets");
    if (!cna_demo_check_result(
            state->demo, cna_content_manager_create(device, &create_info, &state->manager),
            "create the file-backed loader manager")) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check(
        state->demo,
        cna_content_manager_load_sound_effect(
            state->manager, view("sounds/tone_440"), &sound) == CNA_RESULT_SUCCESS &&
            sound != CNA_INVALID_HANDLE &&
            cna_sound_effect_get_duration_ticks(sound, &duration) == CNA_RESULT_SUCCESS &&
            duration > 0,
        "the SoundEffect loader resolves the extensionless WAV and exposes a positive duration");
    if (sound != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_sound_effect_destroy(sound),
                              "destroy the independently owned loaded sound");
    }

    memset(&cube_info, 0, sizeof(cube_info));
    cube_info.struct_size = (uint32_t)sizeof(cube_info);
    cube_info.struct_version = UINT32_C(1);
    {
        CNA_Result cube_result = cna_content_manager_load_texture_cube(
            state->manager, view("textures/cube_faces"), &cube);
        CNA_Result info_result = cube_result == CNA_RESULT_SUCCESS
            ? cna_texturecube_get_info(cube, &cube_info) : CNA_RESULT_INVALID_STATE;
        cna_demo_check(
            state->demo,
            (cube_result == CNA_RESULT_SUCCESS && cube != CNA_INVALID_HANDLE &&
             info_result == CNA_RESULT_SUCCESS &&
             cube_info.size == 4U && cube_info.level_count == 1U &&
             cube_info.format == CNA_SURFACE_FORMAT_COLOR) ||
                ((cube_result == CNA_RESULT_NOT_SUPPORTED || cube_result == CNA_RESULT_IO) &&
                 cube == CNA_INVALID_HANDLE),
            "the cube loader either decodes the six-face DDS or reports the renderer capability boundary");
    }
    if (cube != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_texturecube_destroy(cube),
                              "destroy the independently owned loaded cube");
    }

    cna_demo_check(
        state->demo,
        cna_content_manager_register_cnj_loader_ext(
            state->manager, view("CnaDemos.Enemy"), load_enemy, state) == CNA_RESULT_SUCCESS &&
            cna_content_manager_register_cnj_loader_ext(
                state->manager, view("CnaDemos.Enemy"), load_enemy, state) ==
                CNA_RESULT_INVALID_STATE,
        "a caller CNJ loader is copied into one manager and duplicate registration is refused");
    cna_demo_check(
        state->demo,
        cna_content_manager_load_foreign_ext(
            state->manager, view("custom/guard"), &object) == CNA_RESULT_SUCCESS &&
            object == &state->object && state->object.marker == 12 && state->loader_calls == 1 &&
            state->saw_health &&
            cna_content_manager_load_foreign_ext(
                state->manager, view("custom/guard"), &again) == CNA_RESULT_SUCCESS &&
            again == object && state->loader_calls == 1,
        "foreign CNJ loading passes bounded JSON and returns the cached caller object");
    object = (void*)(uintptr_t)1U;
    cna_demo_check(
        state->demo,
        cna_content_manager_load_foreign_ext(
            state->manager, view("custom/elite"), &object) == CNA_RESULT_IO && object == NULL,
        "a CNJ type with no registered caller loader fails without an opaque result");
    cna_demo_check(
        state->demo,
        cna_content_manager_unload(state->manager) == CNA_RESULT_SUCCESS &&
            cna_content_manager_load_foreign_ext(
                state->manager, view("custom/guard"), &object) == CNA_RESULT_SUCCESS &&
            object == &state->object && state->loader_calls == 2,
        "unload drops the foreign cache so the next load invokes the caller again");

    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.content.additional_loaders", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.manager = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = exercise_loaders;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the additional-loader game")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game), "run its content loader callback");
        cna_demo_check(&demo, state.validated == 1, "the additional loader audit ran once");
    }
    if (state.manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_manager_destroy(state.manager),
                              "destroy the file-backed loader manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the additional-loader game");
    }
    return cna_demo_finish(&demo);
}
