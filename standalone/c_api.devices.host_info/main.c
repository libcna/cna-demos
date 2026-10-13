/* SPDX-License-Identifier: MS-PL */
/*
 * Host information from C: read-only device, power, machine, locale and display queries.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct HostInfoState {
    CnaDemo* demo;
    int validated;
} HostInfoState;

static void identity_contract(CnaDemo* demo)
{
    CNA_DeviceType device_type = UINT32_MAX;
    CNA_Bool available = UINT8_C(9);

    cna_demo_check(
        demo,
        cna_environment_get_device_type(&device_type) == CNA_RESULT_SUCCESS &&
            device_type <= CNA_DEVICE_TYPE_MAXIMUM,
        "Environment reports one defined Device or Emulator identity");
    printf("  device type: %s\n",
           device_type == CNA_DEVICE_TYPE_DEVICE ? "Device" : "Emulator");
    cna_demo_check(
        demo,
        cna_devices_ext_is_available(&available) == CNA_RESULT_SUCCESS && available == CNA_TRUE,
        "this build reports that the extended device layer is available");
}

static void power_contract(CNA_Handle game, CnaDemo* demo)
{
    CNA_PowerState state = UINT32_MAX;
    int32_t percent = -99;
    int32_t seconds = -99;

    cna_demo_check(
        demo,
        cna_power_get_state_ext(game, &state) == CNA_RESULT_SUCCESS &&
            state <= CNA_POWER_STATE_MAXIMUM &&
            cna_power_get_battery_percent_ext(game, &percent) == CNA_RESULT_SUCCESS &&
            percent >= -1 && percent <= 100 &&
            cna_power_get_seconds_remaining_ext(game, &seconds) == CNA_RESULT_SUCCESS &&
            seconds >= -1,
        "host power uses a defined state and documented unknown/range sentinels");
    printf("  power: state %u, percent %d, seconds %d\n",
           (unsigned)state, (int)percent, (int)seconds);
}

static void machine_contract(CNA_Handle game, CnaDemo* demo)
{
    int32_t cores = 0;
    int32_t ram_megabytes = 0;

    cna_demo_check(
        demo,
        cna_system_info_get_logical_cpu_core_count_ext(game, &cores) == CNA_RESULT_SUCCESS &&
            cores >= 1 &&
            cna_system_info_get_system_ram_megabytes_ext(game, &ram_megabytes) ==
                CNA_RESULT_SUCCESS &&
            ram_megabytes >= 1,
        "logical CPU and system RAM queries return positive host capacities");
    printf("  machine: %d logical cores, %d MiB RAM\n", (int)cores, (int)ram_megabytes);
}

static void locale_contract(CNA_Handle game, CnaDemo* demo)
{
    uint64_t count = UINT64_C(0);
    uint64_t index;
    uint64_t bytes = UINT64_C(0);
    char language[128];
    char country[128];
    unsigned char guard[16];
    int entries_ok = 1;

    if (!cna_demo_check(
            demo,
            cna_locale_get_preferred_count_ext(game, &count) == CNA_RESULT_SUCCESS,
            "the host preferred-locale count is available and zero is ordinary")) {
        return;
    }
    printf("  preferred locales: %llu\n", (unsigned long long)count);
    for (index = UINT64_C(0); index < count; ++index) {
        uint64_t language_bytes = UINT64_C(0);
        uint64_t country_bytes = UINT64_C(0);
        memset(language, 0, sizeof(language));
        memset(country, 0, sizeof(country));
        entries_ok = entries_ok &&
            cna_locale_get_language_size_at_ext(game, index, &language_bytes) == CNA_RESULT_SUCCESS &&
            language_bytes < (uint64_t)sizeof(language) &&
            cna_locale_copy_language_at_ext(
                game, index, language, sizeof(language), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == language_bytes && strlen(language) == (size_t)language_bytes &&
            cna_locale_get_country_size_at_ext(game, index, &country_bytes) == CNA_RESULT_SUCCESS &&
            country_bytes < (uint64_t)sizeof(country) &&
            cna_locale_copy_country_at_ext(
                game, index, country, sizeof(country), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == country_bytes && strlen(country) == (size_t)country_bytes;
        if (entries_ok) {
            printf("    #%llu language '%s', country '%s'\n",
                   (unsigned long long)index, language, country);
        }
    }
    cna_demo_check(
        demo,
        entries_ok,
        "every reported locale has exact copied language and country UTF-8 bytes");

    memset(guard, 0x7f, sizeof(guard));
    cna_demo_check(
        demo,
        cna_locale_get_language_size_at_ext(game, count, &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_locale_copy_language_at_ext(
                game, count, (char*)guard, sizeof(guard), &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_locale_get_country_size_at_ext(game, count, &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_locale_copy_country_at_ext(
                game, count, (char*)guard, sizeof(guard), &bytes) == CNA_RESULT_INVALID_ARGUMENT &&
            guard[0] == 0x7fU && guard[sizeof(guard) - 1U] == 0x7fU,
        "the index equal to the locale count is refused by all four routes before writing");
}

static void display_contract(CNA_Handle game, CnaDemo* demo)
{
    float scale = -1.0F;
    CNA_Rectangle area;

    memset(&area, 9, sizeof(area));
    cna_demo_check(
        demo,
        cna_display_info_get_content_scale_ext(game, &scale) == CNA_RESULT_SUCCESS && scale >= 0.0F &&
            cna_display_info_get_safe_area_ext(game, &area) == CNA_RESULT_SUCCESS &&
            area.width >= 0 && area.height >= 0,
        "display scale and safe area use documented nonnegative windowless/windowed answers");
    printf("  display: scale %.3f, safe area %d,%d %dx%d\n",
           (double)scale, (int)area.x, (int)area.y, (int)area.width, (int)area.height);
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    HostInfoState* state = (HostInfoState*)context;
    (void)game_time;
    (void)out_error;
    if (state->validated) {
        return CNA_RESULT_SUCCESS;
    }
    state->validated = 1;
    power_contract(game, state->demo);
    machine_contract(game, state->demo);
    locale_contract(game, state->demo);
    display_contract(game, state->demo);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    HostInfoState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.devices.host_info", argc, argv, 0);
    identity_contract(&demo);
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
    cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                          "run one frame for owner-thread host queries");
    cna_demo_check(&demo, state.validated == 1,
                   "the update callback completed the read-only host audit once");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
