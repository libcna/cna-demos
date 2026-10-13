/* SPDX-License-Identifier: MS-PL */
/* Visible Guide keyboard and message-box rendering through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct GuideRenderState {
    CnaDemo* demo;
    CNA_Handle content;
    CNA_Handle font;
    CNA_Handle font_atlas;
    CNA_Handle white_pixel;
    CNA_Handle sprite_batch;
    int draws;
    int dialog;
    int keyboard_completions;
} GuideRenderState;

static const CNA_Color Background = {18U, 35U, 54U, 255U};

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static void on_completed(void* context)
{
    ++*(int*)context;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    GuideRenderState* state = (GuideRenderState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_ContentManagerCreateInfo content_info;
    CNA_Texture2DCreateInfo texture_info;
    const CNA_Color white = {255U, 255U, 255U, 255U};
    CNA_Result result;
    (void)game_time;
    (void)out_error;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&content_info, 0, sizeof(content_info));
    content_info.struct_size = (uint32_t)sizeof(content_info);
    content_info.struct_version = UINT32_C(1);
    content_info.root_directory = text_view("assets", sizeof("assets") - 1U);
    result = cna_content_manager_create(device, &content_info, &state->content);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_content_manager_load_sprite_font(
        state->content, text_view("fonts/pixel_font", sizeof("fonts/pixel_font") - 1U),
        &state->font, &state->font_atlas);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 1U;
    texture_info.height = 1U;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(device, &texture_info, &state->white_pixel);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_texture2d_set_data_rgba8(state->white_pixel, &white, UINT64_C(1));
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    return cna_sprite_batch_create(device, &state->sprite_batch);
}

static CNA_Result begin_keyboard(GuideRenderState* state)
{
    static const char Title[] = "Account password";
    static const char Description[] = "Enter the local secret";
    static const char Password[] = "secret";
    CNA_Result result = cna_guide_begin_show_keyboard_input(
        CNA_PLAYER_INDEX_TWO, text_view(Title, sizeof(Title) - 1U),
        text_view(Description, sizeof(Description) - 1U),
        text_view(Password, sizeof(Password) - 1U), CNA_TRUE, on_completed,
        &state->keyboard_completions);
    if (cna_demo_check_result(
            state->demo, result, "open a pending password keyboard for visible rendering")) {
        state->dialog = 1;
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result begin_message_box(GuideRenderState* state)
{
    static const char Title[] = "Choose a save slot";
    static const char Body[] = "Which local slot should receive the progress?";
    static const char First[] = "One";
    static const char Second[] = "Two";
    static const char Third[] = "Three";
    CNA_StringView buttons[3];
    CNA_Result result;

    result = cna_guide_simulate_keyboard_input_cancel_ext();
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS && state->keyboard_completions == 1,
        "canceling the rendered keyboard completes its request exactly once");
    result = cna_guide_reset_pending_keyboard_input_ext();
    if (!cna_demo_check_result(
            state->demo, result, "release the rendered keyboard request before the next overlay")) {
        return CNA_RESULT_SUCCESS;
    }
    buttons[0] = text_view(First, sizeof(First) - 1U);
    buttons[1] = text_view(Second, sizeof(Second) - 1U);
    buttons[2] = text_view(Third, sizeof(Third) - 1U);
    result = cna_guide_begin_show_message_box(
        CNA_PLAYER_INDEX_THREE, text_view(Title, sizeof(Title) - 1U),
        text_view(Body, sizeof(Body) - 1U), buttons, UINT64_C(3), 1,
        CNA_MESSAGE_BOX_ICON_ALERT, NULL, NULL);
    if (cna_demo_check_result(
            state->demo, result, "open a pending three-button message box for visible rendering")) {
        state->dialog = 2;
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    GuideRenderState* state = (GuideRenderState*)context;
    (void)game_time;
    (void)out_error;
    if (state->demo->frames > 0 && state->draws >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    if (state->dialog == 0) {
        return begin_keyboard(state);
    }
    if (state->dialog == 1 && state->draws == 1) {
        return begin_message_box(state);
    }
    return CNA_RESULT_SUCCESS;
}

static void check_changed_pixels(
    GuideRenderState* state, CNA_Handle device, const char* overlay_name)
{
    CNA_BackBufferInfo info;
    CNA_Color* pixels = NULL;
    uint64_t pixel_count;
    uint64_t copied = 0U;
    uint64_t changed = 0U;
    uint32_t min_x = UINT32_MAX;
    uint32_t min_y = UINT32_MAX;
    uint32_t max_x = 0U;
    uint32_t max_y = 0U;
    uint64_t index;
    CNA_Result result;

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    result = cna_graphics_device_get_backbuffer_info(device, &info);
    if (!cna_demo_check_result(state->demo, result, "query the dialog back-buffer dimensions")) {
        return;
    }
    pixel_count = (uint64_t)info.width * (uint64_t)info.height;
    pixels = (CNA_Color*)malloc((size_t)pixel_count * sizeof(CNA_Color));
    if (pixels == NULL) {
        cna_demo_check(state->demo, 0, "allocate the dialog back-buffer readback");
        return;
    }
    result = cna_graphics_device_get_backbuffer_data_rgba8(
        device, pixels, pixel_count, &copied);
    if (!cna_demo_check_result(state->demo, result, "read the rendered Guide overlay")) {
        free(pixels);
        return;
    }
    for (index = 0U; index < copied; ++index) {
        const CNA_Color pixel = pixels[index];
        if (pixel.r != Background.r || pixel.g != Background.g ||
            pixel.b != Background.b || pixel.a != Background.a) {
            const uint32_t x = (uint32_t)(index % info.width);
            const uint32_t y = (uint32_t)(index / info.width);
            ++changed;
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }
    printf(
        "  %s changed %llu pixels in [%u,%u]-[%u,%u]\n", overlay_name,
        (unsigned long long)changed, (unsigned)min_x, (unsigned)min_y,
        (unsigned)max_x, (unsigned)max_y);
    cna_demo_check(
        state->demo,
        copied == pixel_count && changed > UINT64_C(1000) && min_x < max_x &&
            min_y < max_y && max_x - min_x > 200U && max_y - min_y > 100U,
        state->dialog == 1
            ? "the password keyboard draws a substantial two-dimensional overlay"
            : "the message box draws a substantial two-dimensional overlay");
    free(pixels);
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    GuideRenderState* state = (GuideRenderState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_Result result;
    const char* overlay_name = state->dialog == 1 ? "password keyboard" : "message box";
    (void)game_time;
    (void)out_error;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_game_clear(game, Background);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    if (state->dialog == 1) {
        result = cna_guide_render_pending_keyboard_input_ext(
            device, state->sprite_batch, state->font, state->white_pixel);
    } else {
        result = cna_guide_render_pending_message_box_ext(
            device, state->sprite_batch, state->font, state->white_pixel);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_end(state->sprite_batch);
    } else {
        (void)cna_sprite_batch_end(state->sprite_batch);
    }
    if (!cna_demo_check_result(state->demo, result, overlay_name)) {
        return CNA_RESULT_SUCCESS;
    }
    ++state->draws;
    check_changed_pixels(state, device, overlay_name);
    if (state->draws == state->demo->frames) {
        cna_demo_capture(state->demo, device);
    }
    return CNA_RESULT_SUCCESS;
}

static void release_handle(
    GuideRenderState* state, CNA_Handle* handle,
    CNA_Result (*destroy)(CNA_Handle), const char* label)
{
    if (*handle != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, destroy(*handle), label);
        *handle = CNA_INVALID_HANDLE;
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    GuideRenderState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.gamer.guide_rendering", argc, argv, 2);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.content = CNA_INVALID_HANDLE;
    state.font = CNA_INVALID_HANDLE;
    state.font_atlas = CNA_INVALID_HANDLE;
    state.white_pixel = CNA_INVALID_HANDLE;
    state.sprite_batch = CNA_INVALID_HANDLE;
    (void)cna_guide_reset_pending_keyboard_input_ext();
    (void)cna_guide_reset_pending_message_box_ext();

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.window_title = text_view(
        "C Guide dialog rendering", sizeof("C Guide dialog rendering") - 1U);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the Guide rendering game")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager), "select HiDef for overlay pixel readback");
    cna_demo_check_result(&demo, cna_game_run(game), "run both visible Guide dialog frames");

    (void)cna_guide_reset_pending_keyboard_input_ext();
    (void)cna_guide_reset_pending_message_box_ext();
    release_handle(&state, &state.sprite_batch, cna_sprite_batch_destroy, "destroy the Guide SpriteBatch");
    release_handle(&state, &state.font, cna_sprite_font_destroy, "destroy the Guide validation font");
    release_handle(&state, &state.font_atlas, cna_texture2d_destroy, "destroy the validation font atlas");
    release_handle(&state, &state.white_pixel, cna_texture2d_destroy, "destroy the white panel pixel");
    release_handle(&state, &state.content, cna_content_manager_destroy, "destroy the Guide content manager");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the Guide graphics-device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the Guide rendering game");
    return cna_demo_finish(&demo);
}
