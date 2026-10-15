/* SPDX-License-Identifier: MS-PL */
/* Local Guide sign-in rendering and service-backed social refusals through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SocialState {
    CnaDemo* demo;
    CNA_SignedInGamerHandle target;
    int opened;
    int overlay_frames;
    int audited;
} SocialState;

static const CNA_Color Background = {20U, 38U, 58U, 255U};

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static int read_last_exception_type(char* type, size_t capacity)
{
    uint64_t bytes = UINT64_MAX;
    uint64_t copied = UINT64_MAX;
    if (cna_error_get_last_exception_type_size_ext(&bytes) != CNA_RESULT_SUCCESS ||
        bytes == 0U || bytes >= capacity ||
        cna_error_copy_last_exception_type_ext(type, bytes, &copied) != CNA_RESULT_SUCCESS ||
        copied != bytes) {
        return 0;
    }
    type[bytes] = '\0';
    return 1;
}

static void audit_offline_social_calls(SocialState* state)
{
    static const char Message[] = "A local message draft";
    static const char Session[] = "phone-session";
    static const char ServiceUnavailableType[] =
        "Microsoft.Xna.Framework.GamerServices.GamerServicesNotAvailableException";
    static const char PrivilegeType[] =
        "Microsoft.Xna.Framework.GamerServices.GamerPrivilegeException";
    static const char UnsupportedType[] = "System.NotSupportedException";
    CNA_GamerHandle recipients[1];
    CNA_ErrorInfo error_info;
    CNA_Result result;
    CNA_Bool visible = CNA_TRUE;
    char exception_type[96];
    recipients[0] = state->target;

    cna_demo_check(
        state->demo,
        cna_guide_show_compose_message(
            CNA_PLAYER_INDEX_ONE, text_view(Message, sizeof(Message) - 1U),
            recipients, UINT64_C(1)) == CNA_RESULT_NOT_SUPPORTED,
        "compose-message refuses a valid draft and recipient without an account service");
    cna_demo_check(
        state->demo,
        cna_guide_show_friend_request(CNA_PLAYER_INDEX_ONE, state->target) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_guide_show_gamer_card(CNA_PLAYER_INDEX_ONE, state->target) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_guide_show_player_review(CNA_PLAYER_INDEX_ONE, state->target) ==
                CNA_RESULT_NOT_SUPPORTED,
        "friend request, gamer card and player review accept the target but require service identity");
    result = cna_guide_show_friends(CNA_PLAYER_INDEX_ONE);
    memset(&error_info, 0, sizeof(error_info));
    error_info.struct_size = (uint32_t)sizeof(error_info);
    error_info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_NOT_SUPPORTED &&
            read_last_exception_type(exception_type, sizeof(exception_type)) &&
            cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
            error_info.result == CNA_RESULT_NOT_SUPPORTED &&
            error_info.category == CNA_ERROR_CATEGORY_NOT_SUPPORTED &&
            strcmp(exception_type, ServiceUnavailableType) == 0,
        "friends reports the exact unavailable-account-service exception to a binding");
    cna_demo_check(
        state->demo,
        cna_guide_show_messages(CNA_PLAYER_INDEX_ONE) == CNA_RESULT_NOT_SUPPORTED &&
            cna_guide_show_players(CNA_PLAYER_INDEX_ONE) == CNA_RESULT_NOT_SUPPORTED &&
            cna_guide_show_achievements_ext(CNA_PLAYER_INDEX_ONE) ==
                CNA_RESULT_NOT_SUPPORTED,
        "messages, recent players and achievements refuse without a service player");
    cna_demo_check(
        state->demo,
        cna_guide_show_party(CNA_PLAYER_INDEX_ONE) == CNA_RESULT_NOT_SUPPORTED &&
            cna_guide_show_party_sessions(CNA_PLAYER_INDEX_ONE) ==
                CNA_RESULT_NOT_SUPPORTED,
        "party and party-session screens refuse without a service player");
    cna_demo_check(
        state->demo,
        cna_guide_show_game_invite(
            CNA_PLAYER_INDEX_ONE, recipients, UINT64_C(1)) ==
                CNA_RESULT_NOT_SUPPORTED,
        "the player invitation overload reports its unavailable offline service");
    result = cna_guide_show_game_invite_for_session(
        text_view(Session, sizeof(Session) - 1U));
    memset(&error_info, 0, sizeof(error_info));
    error_info.struct_size = (uint32_t)sizeof(error_info);
    error_info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_NOT_SUPPORTED &&
            read_last_exception_type(exception_type, sizeof(exception_type)) &&
            cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
            error_info.result == CNA_RESULT_NOT_SUPPORTED &&
            error_info.category == CNA_ERROR_CATEGORY_NOT_SUPPORTED &&
            strcmp(exception_type, UnsupportedType) == 0,
        "the phone-only invitation overload reports the exact unsupported-platform exception");
    result = cna_guide_show_marketplace(CNA_PLAYER_INDEX_ONE);
    memset(&error_info, 0, sizeof(error_info));
    error_info.struct_size = (uint32_t)sizeof(error_info);
    error_info.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_INVALID_STATE &&
            read_last_exception_type(exception_type, sizeof(exception_type)) &&
            cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
            error_info.result == CNA_RESULT_INVALID_STATE &&
            error_info.category == CNA_ERROR_CATEGORY_STATE &&
            strcmp(exception_type, PrivilegeType) == 0,
        "marketplace reports the exact missing purchase-privilege exception to a binding");
    cna_demo_check(
        state->demo,
        cna_guide_get_is_visible(&visible) == CNA_RESULT_SUCCESS && visible == CNA_FALSE,
        "none of the refused service screens leaves a partial Guide overlay open");
    state->audited = 1;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    static const char AlreadyVisibleType[] =
        "Microsoft.Xna.Framework.GamerServices.GuideAlreadyVisibleException";
    SocialState* state = (SocialState*)context;
    CNA_ErrorInfo error_info;
    CNA_Bool visible = CNA_FALSE;
    CNA_Result result;
    char exception_type[96];
    (void)game_time;
    (void)out_error;

    if (!state->opened) {
        cna_demo_check(
            state->demo,
            cna_guide_show_sign_in(3, CNA_FALSE) == CNA_RESULT_INVALID_ARGUMENT &&
                cna_guide_show_sign_in(1, CNA_TRUE) == CNA_RESULT_NOT_SUPPORTED &&
                cna_guide_get_is_visible(&visible) == CNA_RESULT_SUCCESS &&
                visible == CNA_FALSE,
            "sign-in rejects an invalid pane count and unavailable online-only mode atomically");
        result = cna_guide_show_sign_in(1, CNA_FALSE);
        cna_demo_check(
            state->demo,
            result == CNA_RESULT_SUCCESS &&
                cna_guide_get_is_visible(&visible) == CNA_RESULT_SUCCESS &&
                visible == CNA_TRUE,
            "offline one-pane sign-in opens the local-profile Guide");
        state->opened = result == CNA_RESULT_SUCCESS;
        if (state->opened) {
            result = cna_guide_show_sign_in(1, CNA_FALSE);
            memset(&error_info, 0, sizeof(error_info));
            error_info.struct_size = (uint32_t)sizeof(error_info);
            error_info.struct_version = UINT32_C(1);
            cna_demo_check(
                state->demo,
                result == CNA_RESULT_INVALID_STATE &&
                    read_last_exception_type(exception_type, sizeof(exception_type)) &&
                    cna_error_get_last_info(&error_info) == CNA_RESULT_SUCCESS &&
                    error_info.result == CNA_RESULT_INVALID_STATE &&
                    error_info.category == CNA_ERROR_CATEGORY_STATE &&
                    strcmp(exception_type, AlreadyVisibleType) == 0 &&
                    cna_guide_get_is_visible(&visible) == CNA_RESULT_SUCCESS &&
                    visible == CNA_TRUE,
                "a second valid sign-in reports GuideAlreadyVisibleException and preserves the overlay");
        }
        return CNA_RESULT_SUCCESS;
    }
    if (state->overlay_frames >= state->demo->frames && !state->audited) {
        cna_demo_check(
            state->demo,
            cna_guide_simulate_keyboard_input_cancel_ext() == CNA_RESULT_SUCCESS &&
                cna_guide_get_is_visible(&visible) == CNA_RESULT_SUCCESS &&
                visible == CNA_FALSE,
            "the sign-in screen closes through its documented Escape simulation");
        audit_offline_social_calls(state);
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_clear(game, Background);
}

static CNA_Result end_draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    SocialState* state = (SocialState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_BackBufferInfo info;
    CNA_Color* pixels = NULL;
    uint64_t pixel_count;
    uint64_t copied = 0U;
    uint64_t changed = 0U;
    uint64_t accent_pixels = 0U;
    uint64_t panel_pixels = 0U;
    uint64_t index;
    CNA_Color corner = {0U, 0U, 0U, 0U};
    CNA_Result result;
    (void)game_time;
    (void)out_error;

    ++state->overlay_frames;
    if (state->overlay_frames < state->demo->frames) {
        return CNA_RESULT_SUCCESS;
    }
    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    result = cna_graphics_device_get_backbuffer_info(device, &info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    pixel_count = (uint64_t)info.width * (uint64_t)info.height;
    pixels = (CNA_Color*)malloc((size_t)pixel_count * sizeof(CNA_Color));
    if (pixels == NULL) {
        return CNA_RESULT_INTERNAL;
    }
    result = cna_graphics_device_get_backbuffer_data_rgba8(
        device, pixels, pixel_count, &copied);
    if (result == CNA_RESULT_SUCCESS) {
        for (index = 0U; index < copied; ++index) {
            const CNA_Color pixel = pixels[index];
            if (pixel.r != Background.r || pixel.g != Background.g ||
                pixel.b != Background.b || pixel.a != Background.a) {
                ++changed;
            }
            if (pixel.r > 240U && pixel.g > 100U && pixel.g < 160U &&
                pixel.b > 50U && pixel.b < 120U) {
                ++accent_pixels;
            }
            if (pixel.r == 21U && pixel.g == 27U && pixel.b == 38U) {
                ++panel_pixels;
            }
        }
        if (info.width == 800U && info.height == 480U && copied == pixel_count) {
            corner = pixels[10U * info.width + 10U];
        }
    }
    free(pixels);
    printf(
        "  local sign-in changed %llu of %llu pixels; accent %llu, panel %llu\n",
        (unsigned long long)changed, (unsigned long long)pixel_count,
        (unsigned long long)accent_pixels, (unsigned long long)panel_pixels);
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS && copied == pixel_count && changed == pixel_count &&
            info.width == 800U && info.height == 480U && corner.r < Background.r &&
            corner.g < Background.g && accent_pixels > UINT64_C(1000) &&
            panel_pixels > UINT64_C(10000),
        "the dimmed background, orange header and panel prove the local sign-in screen is visible");
    cna_demo_capture(state->demo, device);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const char AppName[] = "cna-demos-c-api-guide-social-offline";
    static const char TargetName[] = "Offline Target";
    CnaDemo demo;
    SocialState state;
    CNA_GameCallbacks callbacks;
    CNA_GameFrameHooks hooks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_GameComponentHandle component = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.gamer.guide_social_offline", argc, argv, 20);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.target = CNA_INVALID_HANDLE;
    cna_demo_check_result(
        &demo, cna_storage_set_app_name_ext(text_view(AppName, sizeof(AppName) - 1U)),
        "select an isolated local-profile store");
    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(TargetName, sizeof(TargetName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_FOUR, &state.target),
            "create a valid offline social-screen target")) {
        return cna_demo_finish(&demo);
    }

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.window_title = text_view(
        "C Guide offline social", sizeof("C Guide offline social") - 1U);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the offline Guide game") ||
        !cna_demo_check_result(
            &demo, cna_demo_use_hidef(game, &manager),
            "select HiDef for sign-in back-buffer readback") ||
        !cna_demo_check_result(
            &demo, cna_gamer_services_component_create(game, &component),
            "create the gamer-services component") ||
        !cna_demo_check_result(
            &demo, cna_game_components_add(game, component),
            "add the component that installs and draws the Guide")) {
        goto cleanup;
    }
    memset(&hooks, 0, sizeof(hooks));
    hooks.struct_size = (uint32_t)sizeof(hooks);
    hooks.struct_version = UINT32_C(1);
    hooks.end_draw = end_draw;
    hooks.context = &state;
    if (!cna_demo_check_result(
            &demo, cna_game_set_frame_hooks_ext(game, &hooks),
            "inspect the frame after the automatic Guide overlay draws")) {
        goto cleanup;
    }
    cna_demo_check_result(
        &demo, cna_game_run(game), "run the local sign-in and offline social audit");
    cna_demo_check(
        &demo, state.overlay_frames == demo.frames && state.audited == 1,
        "the rendered sign-in and all offline social checks completed before exit");

cleanup:
    (void)cna_guide_simulate_keyboard_input_cancel_ext();
    if (component != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_game_component_destroy(component),
            "remove and release the gamer-services component");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the offline Guide graphics-device manager");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the offline Guide game");
    }
    if (state.target != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(state.target),
            "release the offline social-screen target");
    }
    return cna_demo_finish(&demo);
}
