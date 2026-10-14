/* SPDX-License-Identifier: MS-PL */
/* Guide state and GamerServicesDispatcher/component lifecycle through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct Events {
    int signed_in;
    int signed_out;
    int title_update;
} Events;

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static void on_gamer_event(void* context, const CNA_SignedInGamerEventInfo* info)
{
    int* count = (int*)context;
    if (info != NULL && info->struct_version == UINT32_C(1)) {
        ++*count;
    }
}

static void on_title_update(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    static const char AppName[] = "cna-demos-c-api-guide-dispatcher";
    static const char WindowTitle[] = "C Guide dispatcher";
    CnaDemo demo;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_GameComponentHandle component = CNA_INVALID_HANDLE;
    CNA_Handle signed_in_registration = CNA_INVALID_HANDLE;
    CNA_Handle signed_out_registration = CNA_INVALID_HANDLE;
    CNA_Handle title_update_registration = CNA_INVALID_HANDLE;
    CNA_Bool flag = CNA_TRUE;
    CNA_Bool screen_saver = CNA_TRUE;
    CNA_Bool trial = CNA_TRUE;
    CNA_NotificationPosition original_position = CNA_NOTIFICATION_POSITION_BOTTOM_CENTER;
    CNA_NotificationPosition changed_position = CNA_NOTIFICATION_POSITION_TOP_LEFT;
    uint64_t window_handle = UINT64_C(0);
    uint64_t freed = UINT64_MAX;
    Events events;

    cna_demo_init(&demo, "c_api.gamer.guide_dispatcher", argc, argv, 1);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.window_title = text_view(WindowTitle, sizeof(WindowTitle) - 1U);
    memset(&events, 0, sizeof(events));

    if (!cna_demo_check_result(
            &demo,
            cna_storage_set_app_name_ext(text_view(AppName, sizeof(AppName) - 1U)),
            "select an empty local-profile store for deterministic dispatcher startup") ||
        !cna_demo_check_result(
            &demo, cna_game_create(&info, &game),
            "create the Guide dispatcher game")) {
        goto cleanup;
    }

    cna_demo_check(
        &demo,
        cna_gamer_services_dispatcher_get_is_initialized(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_guide_get_is_visible(&flag) == CNA_RESULT_INVALID_STATE,
        "the fresh dispatcher is uninitialized and Guide visibility is unavailable");
    cna_demo_check(
        &demo,
        cna_gamer_services_dispatcher_update() == CNA_RESULT_SUCCESS &&
            cna_gamer_services_dispatcher_update_async(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_gamer_services_dispatcher_get_freed_gamer_count_ext(&freed) ==
                CNA_RESULT_SUCCESS &&
            freed == UINT64_C(0),
        "both pre-initialization pumps are harmless and retain no retired identities");
    cna_demo_check(
        &demo,
        cna_gamer_services_dispatcher_set_window_handle(
            UINT64_C(0x1234567890ABCDEF)) == CNA_RESULT_SUCCESS &&
            cna_gamer_services_dispatcher_get_window_handle(&window_handle) ==
                CNA_RESULT_SUCCESS &&
            window_handle == UINT64_C(0x1234567890ABCDEF),
        "the dispatcher preserves a full caller-supplied opaque window handle");

    cna_demo_check(
        &demo,
        cna_guide_get_notification_position(&original_position) == CNA_RESULT_SUCCESS &&
            original_position <= CNA_NOTIFICATION_POSITION_MAXIMUM,
        "the Guide starts with a canonical notification position");
    changed_position = original_position == CNA_NOTIFICATION_POSITION_TOP_LEFT
        ? CNA_NOTIFICATION_POSITION_BOTTOM_RIGHT
        : CNA_NOTIFICATION_POSITION_TOP_LEFT;
    cna_demo_check(
        &demo,
        cna_guide_set_notification_position(changed_position) == CNA_RESULT_SUCCESS &&
            cna_guide_get_notification_position(&original_position) == CNA_RESULT_SUCCESS &&
            original_position == changed_position &&
            cna_guide_set_notification_position(UINT32_C(99)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_guide_get_notification_position(&original_position) == CNA_RESULT_SUCCESS &&
            original_position == changed_position,
        "notification position round-trips while an undefined identity is atomic");
    cna_demo_check(
        &demo,
        cna_guide_get_simulate_trial_mode(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_guide_set_simulate_trial_mode(UINT8_C(9)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_guide_set_simulate_trial_mode(CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_guide_get_simulate_trial_mode(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "trial simulation validates canonical booleans and stores the requested state");
    cna_demo_check(
        &demo,
        cna_guide_get_is_trial_mode(&trial) == CNA_RESULT_SUCCESS && trial == CNA_TRUE,
        "before the first gamer-services update trial mode retains its documented initial value");

    cna_demo_check(
        &demo,
        cna_signed_in_gamer_subscribe_signed_in_ext(
            on_gamer_event, &events.signed_in, &signed_in_registration) ==
                CNA_RESULT_SUCCESS &&
            cna_signed_in_gamer_subscribe_signed_out_ext(
                on_gamer_event, &events.signed_out, &signed_out_registration) ==
                CNA_RESULT_SUCCESS &&
            cna_gamer_services_dispatcher_subscribe_installing_title_update_ext(
                on_title_update, &events.title_update, &title_update_registration) ==
                CNA_RESULT_SUCCESS,
        "three owned registrations attach to dispatcher identity and title-update events");
    if (!cna_demo_check_result(
            &demo, cna_gamer_services_component_create(game, &component),
            "create the canonical gamer-services component") ||
        !cna_demo_check_result(
            &demo, cna_game_components_add(game, component),
            "add the canonical component to the game") ||
        !cna_demo_check_result(
            &demo, cna_game_run_one_frame(game),
            "initialize and update the component in one game frame")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_gamer_services_dispatcher_get_is_initialized(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_guide_get_is_visible(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_gamer_services_dispatcher_get_window_handle(&window_handle) ==
                CNA_RESULT_SUCCESS &&
            window_handle != UINT64_C(0x1234567890ABCDEF),
        "component initialization installs the dispatcher and Guide and replaces the caller's window sentinel");
    cna_demo_check(
        &demo,
        cna_guide_get_is_trial_mode(&trial) == CNA_RESULT_SUCCESS && trial == CNA_TRUE &&
            cna_guide_set_simulate_trial_mode(CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_guide_get_is_trial_mode(&trial) == CNA_RESULT_SUCCESS && trial == CNA_TRUE &&
            cna_gamer_services_dispatcher_update() == CNA_RESULT_SUCCESS &&
            cna_guide_get_is_trial_mode(&trial) == CNA_RESULT_SUCCESS && trial == CNA_FALSE,
        "trial mode changes only when an initialized dispatcher update latches simulation state");
    cna_demo_check(
        &demo,
        cna_gamer_services_dispatcher_update_async(&flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_gamer_services_dispatcher_initialize(game) == CNA_RESULT_INVALID_STATE &&
            cna_gamer_services_dispatcher_get_freed_gamer_count_ext(&freed) ==
                CNA_RESULT_SUCCESS &&
            freed == UINT64_C(0),
        "the initialized async pump reports work, duplicate initialization is refused and identities remain retained");
    cna_demo_check(
        &demo,
        events.signed_in == 0 && events.signed_out == 0 && events.title_update == 0,
        "the empty local-profile run emits no synthetic identity or title-update events");

    if (cna_demo_check_result(
            &demo, cna_guide_get_is_screen_saver_enabled(&screen_saver),
            "read the platform screen-saver state")) {
        cna_demo_check(
            &demo,
            cna_guide_set_is_screen_saver_enabled(UINT8_C(9)) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_guide_set_is_screen_saver_enabled(
                    screen_saver == CNA_TRUE ? CNA_FALSE : CNA_TRUE) == CNA_RESULT_SUCCESS &&
                cna_guide_get_is_screen_saver_enabled(&flag) == CNA_RESULT_SUCCESS &&
                flag != screen_saver &&
                cna_guide_set_is_screen_saver_enabled(screen_saver) == CNA_RESULT_SUCCESS,
            "screen-saver mutation round-trips, restores and rejects a noncanonical boolean");
    }

    cna_demo_check(
        &demo,
        cna_gamer_unsubscribe_ext(signed_in_registration) == CNA_RESULT_SUCCESS &&
            cna_gamer_unsubscribe_ext(signed_out_registration) == CNA_RESULT_SUCCESS &&
            cna_gamer_unsubscribe_ext(title_update_registration) == CNA_RESULT_SUCCESS &&
            cna_gamer_unsubscribe_ext(signed_in_registration) == CNA_RESULT_INVALID_HANDLE,
        "all three registrations release exactly once through the shared unsubscribe route");
    signed_in_registration = CNA_INVALID_HANDLE;
    signed_out_registration = CNA_INVALID_HANDLE;
    title_update_registration = CNA_INVALID_HANDLE;

cleanup:
    (void)cna_guide_set_simulate_trial_mode(CNA_FALSE);
    (void)cna_guide_set_notification_position(CNA_NOTIFICATION_POSITION_BOTTOM_CENTER);
    if (title_update_registration != CNA_INVALID_HANDLE) {
        (void)cna_gamer_unsubscribe_ext(title_update_registration);
    }
    if (signed_out_registration != CNA_INVALID_HANDLE) {
        (void)cna_gamer_unsubscribe_ext(signed_out_registration);
    }
    if (signed_in_registration != CNA_INVALID_HANDLE) {
        (void)cna_gamer_unsubscribe_ext(signed_in_registration);
    }
    if (component != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_game_component_destroy(component),
            "remove and release the gamer-services component");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the dispatcher game");
    }
    return cna_demo_finish(&demo);
}
