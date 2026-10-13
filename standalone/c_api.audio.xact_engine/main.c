/* SPDX-License-Identifier: MS-PL */
/* XACT AudioEngine construction, renderer values, variables and disposal through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static void count_disposal(void* context)
{
    ++*(int*)context;
}

static int copy_renderer_text(
    CNA_Handle engine,
    uint64_t index,
    CNA_Result (*size_route)(CNA_Handle, uint64_t, uint64_t*),
    CNA_Result (*copy_route)(CNA_Handle, uint64_t, char*, uint64_t, uint64_t*),
    char* destination,
    size_t capacity,
    uint64_t* out_bytes)
{
    uint64_t bytes = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    if (size_route(engine, index, &bytes) != CNA_RESULT_SUCCESS || bytes == UINT64_C(0) ||
        bytes >= capacity ||
        copy_route(engine, index, destination, bytes, &copied) != CNA_RESULT_SUCCESS ||
        copied != bytes) {
        return 0;
    }
    destination[bytes] = '\0';
    *out_bytes = bytes;
    return 1;
}

int main(int argc, char** argv)
{
    static const char SettingsPath[] = "assets/xact/demo.xgs";
    CnaDemo demo;
    CNA_GameCreateInfo game_info;
    CNA_AudioCapabilities capabilities;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle engine = CNA_INVALID_HANDLE;
    CNA_Handle explicit_engine = CNA_INVALID_HANDLE;
    CNA_Handle rejected = UINT64_C(99);
    CNA_AudioEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_Bool flag = UINT8_C(77);
    CNA_Bool equal = CNA_FALSE;
    uint64_t renderer_count = UINT64_C(0);
    uint64_t friendly_bytes = UINT64_C(0);
    uint64_t id_bytes = UINT64_C(0);
    uint64_t text_bytes = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    int32_t hash = 0;
    int32_t same_hash = 1;
    float value = -1.0F;
    char friendly[128];
    char renderer_id[128];
    char renderer_text[128];
    char short_buffer[128];
    char before[128];
    int disposal_count = 0;

    cna_demo_init(&demo, "c_api.audio.xact_engine", argc, argv, 1);
    memset(&game_info, 0, sizeof(game_info));
    game_info.struct_size = (uint32_t)sizeof(game_info);
    game_info.struct_version = UINT32_C(1);
    game_info.is_fixed_time_step = CNA_TRUE;
    game_info.target_elapsed_time_ticks = INT64_C(166667);
    if (!cna_demo_check_result(&demo, cna_game_create(&game_info, &game),
                               "create the XACT owner game")) {
        return cna_demo_finish(&demo);
    }

    memset(&capabilities, 0, sizeof(capabilities));
    capabilities.struct_size = (uint32_t)sizeof(capabilities);
    capabilities.struct_version = UINT32_C(1);
    if (cna_audio_get_capabilities(game, &capabilities) != CNA_RESULT_SUCCESS ||
        capabilities.is_playback_available != CNA_TRUE) {
        (void)cna_game_destroy(game);
        return cna_demo_skip("no audio playback device");
    }

    cna_demo_check(
        &demo,
        CNA_AUDIO_ENGINE_CONTENT_VERSION == INT32_C(46) &&
            cna_audio_engine_create(game, view("assets/xact/definitely-missing.xgs"), &rejected) ==
                CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE,
        "the public content version is 46 and a missing settings file is an atomic I/O failure");
    if (!cna_demo_check_result(&demo,
                               cna_audio_engine_create(game, view(SettingsPath), &engine),
                               "open the generated XACT settings")) {
        (void)cna_game_destroy(game);
        return cna_demo_finish(&demo);
    }
    cna_demo_check(
        &demo,
        cna_audio_engine_get_is_disposed(engine, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "a newly opened AudioEngine reports a live owned state");

    cna_demo_check(
        &demo,
        cna_audio_engine_get_renderer_count(engine, &renderer_count) == CNA_RESULT_SUCCESS &&
            renderer_count == UINT64_C(1),
        "the XACT engine exposes CNA's single audio renderer");
    memset(friendly, 0xA5, sizeof(friendly));
    memset(renderer_id, 0xA5, sizeof(renderer_id));
    memset(renderer_text, 0xA5, sizeof(renderer_text));
    cna_demo_check(
        &demo,
        renderer_count == UINT64_C(1) &&
            copy_renderer_text(
                engine, UINT64_C(0), cna_audio_engine_get_renderer_friendly_name_size,
                cna_audio_engine_copy_renderer_friendly_name, friendly, sizeof(friendly),
                &friendly_bytes) &&
            copy_renderer_text(
                engine, UINT64_C(0), cna_audio_engine_get_renderer_id_size,
                cna_audio_engine_copy_renderer_id, renderer_id, sizeof(renderer_id), &id_bytes) &&
            copy_renderer_text(
                engine, UINT64_C(0), cna_audio_engine_get_renderer_text_size,
                cna_audio_engine_copy_renderer_text, renderer_text, sizeof(renderer_text),
                &text_bytes) &&
            text_bytes == friendly_bytes &&
            memcmp(renderer_text, friendly, (size_t)friendly_bytes) == 0,
        "the renderer has nonempty friendly/id text and ToString matches its friendly name");
    memset(short_buffer, 0xA5, sizeof(short_buffer));
    memcpy(before, short_buffer, sizeof(before));
    cna_demo_check(
        &demo,
        friendly_bytes > UINT64_C(0) &&
            cna_audio_engine_copy_renderer_friendly_name(
                engine, UINT64_C(0), short_buffer, friendly_bytes - UINT64_C(1), &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == friendly_bytes && memcmp(short_buffer, before, sizeof(before)) == 0,
        "a short renderer-name destination reports its requirement without a partial write");
    cna_demo_check(
        &demo,
        cna_audio_engine_get_renderer_hash_code(engine, UINT64_C(0), &hash) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_get_renderer_hash_code(engine, UINT64_C(0), &same_hash) ==
                CNA_RESULT_SUCCESS &&
            hash == same_hash &&
            cna_audio_engine_renderers_equal(
                engine, UINT64_C(0), UINT64_C(0), &equal) == CNA_RESULT_SUCCESS &&
            equal == CNA_TRUE,
        "the same renderer value compares equal and produces one stable process hash");
    cna_demo_check(
        &demo,
        cna_audio_engine_get_renderer_friendly_name_size(
            engine, renderer_count, &required) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_audio_engine_copy_renderer_id(
                engine, renderer_count, renderer_id, sizeof(renderer_id), &required) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_audio_engine_get_renderer_hash_code(engine, renderer_count, &hash) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_audio_engine_renderers_equal(
                engine, UINT64_C(0), renderer_count, &equal) == CNA_RESULT_INVALID_ARGUMENT,
        "every renderer-value route rejects the index equal to Count");

    cna_demo_check(
        &demo,
        cna_audio_engine_get_global_variable(engine, view("SpeedOfSound"), &value) ==
                CNA_RESULT_SUCCESS &&
            value == 343.0F &&
            cna_audio_engine_set_global_variable(engine, view("SpeedOfSound"), 500.0F) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_get_global_variable(engine, view("SpeedOfSound"), &value) ==
                CNA_RESULT_SUCCESS &&
            value == 500.0F,
        "the authored SpeedOfSound global changes from 343 to an interior value");
    cna_demo_check(
        &demo,
        cna_audio_engine_set_global_variable(engine, view("SpeedOfSound"), 5000.0F) ==
                CNA_RESULT_SUCCESS &&
            cna_audio_engine_get_global_variable(engine, view("SpeedOfSound"), &value) ==
                CNA_RESULT_SUCCESS &&
            value == 1000.0F &&
            cna_audio_engine_get_global_variable(engine, view("Intensity"), &value) ==
                CNA_RESULT_INVALID_STATE,
        "global writes clamp to the authored range and cannot address cue-scoped variables");
    cna_demo_check_result(&demo, cna_audio_engine_update(engine),
                          "pump the XACT engine's bookkeeping explicitly");

    cna_demo_check(
        &demo,
        cna_audio_engine_create_with_renderer(
            game, view(SettingsPath), INT64_C(2500000),
            view(renderer_id), &explicit_engine) == CNA_RESULT_SUCCESS &&
            explicit_engine != CNA_INVALID_HANDLE &&
            cna_audio_engine_destroy(explicit_engine) == CNA_RESULT_SUCCESS,
        "the look-ahead/renderer constructor accepts the listed renderer and releases independently");
    explicit_engine = CNA_INVALID_HANDLE;

    cna_demo_check_result(
        &demo,
        cna_audio_engine_subscribe_disposing_ext(
            engine, count_disposal, &disposal_count, &registration),
        "subscribe to the engine's synchronous disposal event");
    cna_demo_check(
        &demo,
        cna_audio_engine_destroy(engine) == CNA_RESULT_SUCCESS && disposal_count == 1 &&
            cna_audio_engine_destroy(engine) == CNA_RESULT_INVALID_HANDLE,
        "destroy raises one disposal event and invalidates the owned engine handle");
    engine = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_audio_unsubscribe_ext(registration) == CNA_RESULT_SUCCESS &&
            cna_audio_unsubscribe_ext(registration) == CNA_RESULT_INVALID_HANDLE,
        "the detached disposal registration releases exactly once after its engine");
    registration = CNA_INVALID_HANDLE;

    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the XACT owner game");
    return cna_demo_finish(&demo);
}
