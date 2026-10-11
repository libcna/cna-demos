/* SPDX-License-Identifier: MS-PL */
/*
 * Render to texture from C: a 128x128 render target receives four tinted squares, is read back
 * as RGBA8 pixels, and is then drawn as an ordinary sprite texture -- three times larger -- on
 * the back buffer. A render-target handle is a Texture2D handle too.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define TARGET 128

typedef struct TargetState {
    CnaDemo* demo;
    CNA_Handle white;         /* owned 1x1 texture */
    CNA_Handle render_target; /* owned */
    CNA_Handle sprite_batch;  /* owned */
    int frames;
} TargetState;

static const CNA_Color background = {24, 26, 36, 255};
static const CNA_Color target_clear = {40, 110, 120, 255};

static CNA_SpriteCommand sprite(CNA_Handle texture, int32_t x, int32_t y, int32_t size, int32_t source_size,
                                CNA_Color color)
{
    CNA_SpriteCommand c;
    memset(&c, 0, sizeof(c));
    c.struct_size = (uint32_t)sizeof(c);
    c.struct_version = UINT32_C(1);
    c.texture = texture;
    c.destination.x = x;
    c.destination.y = y;
    c.destination.width = size;
    c.destination.height = size;
    c.source.width = source_size;
    c.source.height = source_size;
    c.color = color;
    c.effects = CNA_SPRITE_EFFECT_NONE;
    return c;
}

static CNA_Result draw_sprites(TargetState* state, const CNA_SpriteCommand* commands, uint32_t count)
{
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_Result result;
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_sprite_batch_submit_many(state->sprite_batch, commands, count);
    if (result == CNA_RESULT_SUCCESS) {
        return cna_sprite_batch_end(state->sprite_batch);
    }
    (void)cna_sprite_batch_end(state->sprite_batch); /* end the batch even after a failure */
    return result;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    TargetState* state = (TargetState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo texture_info;
    CNA_RenderTarget2DCreateInfo target_info;
    const CNA_Color white = {255, 255, 255, 255};
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 1;
    texture_info.height = 1;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(graphics_device, &texture_info, &state->white);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_texture2d_set_data_rgba8(state->white, &white, 1);
    }
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    /* The off-screen target: Color format, no depth buffer, contents preserved between uses. */
    memset(&target_info, 0, sizeof(target_info));
    target_info.struct_size = (uint32_t)sizeof(target_info);
    target_info.struct_version = UINT32_C(1);
    target_info.width = TARGET;
    target_info.height = TARGET;
    target_info.mip_map = CNA_FALSE;
    target_info.format = CNA_SURFACE_FORMAT_COLOR;
    target_info.depth_format = CNA_DEPTH_FORMAT_NONE;
    target_info.multi_sample_count = 0;
    target_info.usage = CNA_RENDER_TARGET_USAGE_PRESERVE_CONTENTS;
    result = cna_render_target2d_create(graphics_device, &target_info, &state->render_target);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    return cna_sprite_batch_create(graphics_device, &state->sprite_batch);
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    TargetState* state = (TargetState*)context;
    (void)game_time;
    (void)out_error;
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    TargetState* state = (TargetState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_SpriteCommand squares[4];
    CNA_SpriteCommand shown;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->frames;
    result = cna_game_get_graphics_device(game, &graphics_device);

    /* 1. Into the render target: its own clear colour and four squares. */
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_set_render_target2d(graphics_device, state->render_target);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_game_clear(game, target_clear);
    }
    if (result == CNA_RESULT_SUCCESS) {
        squares[0] = sprite(state->white, 16, 16, 40, 1, (CNA_Color){230, 70, 60, 255});
        squares[1] = sprite(state->white, 72, 16, 40, 1, (CNA_Color){240, 200, 60, 255});
        squares[2] = sprite(state->white, 16, 72, 40, 1, (CNA_Color){90, 210, 100, 255});
        squares[3] = sprite(state->white, 72, 72, 40, 1, (CNA_Color){240, 240, 240, 255});
        result = draw_sprites(state, squares, 4U);
    }

    /* 2. Back to the back buffer (an invalid handle), where the target is a texture. */
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_graphics_device_set_render_target2d(graphics_device, CNA_INVALID_HANDLE);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_game_clear(game, background);
    }
    if (result == CNA_RESULT_SUCCESS) {
        shown = sprite(state->render_target, 400 - 3 * TARGET / 2, 240 - 3 * TARGET / 2, 3 * TARGET, TARGET,
                       (CNA_Color){255, 255, 255, 255});
        result = draw_sprites(state, &shown, 1U);
    }

    if (result == CNA_RESULT_SUCCESS && state->demo->frames > 0 && state->frames == state->demo->frames) {
        static CNA_Color target_pixels[TARGET * TARGET];
        static CNA_Color screen[800 * 480];
        uint64_t count = 0U;
        /* The target's own pixels, read like any Color texture's. */
        if (cna_demo_check_result(state->demo,
                                  cna_texture2d_get_data_rgba8(state->render_target, target_pixels,
                                                               TARGET * TARGET, &count),
                                  "cna_texture2d_get_data_rgba8 on the render target")) {
            const CNA_Color red = target_pixels[30 * TARGET + 30];
            const CNA_Color gap = target_pixels[64 * TARGET + 64];
            printf("  target: square %u,%u,%u  between the squares %u,%u,%u\n", red.r, red.g, red.b, gap.r, gap.g,
                   gap.b);
            cna_demo_check(state->demo, red.r > 200 && red.g < 100, "the target holds the red square");
            cna_demo_check(state->demo, gap.r == target_clear.r && gap.g == target_clear.g,
                           "and its own clear colour between the squares");
        }
        if (cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, screen, 800U * 480U, &count) ==
            CNA_RESULT_SUCCESS) {
            /* The yellow square, three times larger: target (90, 30) lands at (208 + 270, 48 + 90). */
            const CNA_Color yellow = screen[(48 + 90) * 800 + (208 + 270)];
            printf("  back buffer: scaled yellow square %u,%u,%u\n", yellow.r, yellow.g, yellow.b);
            cna_demo_check(state->demo, yellow.r > 200 && yellow.g > 160 && yellow.b < 100,
                           "the back buffer shows the target three times larger");
        }
        cna_demo_capture(state->demo, graphics_device);
    }
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    TargetState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.render_target", argc, argv, 5);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.white = CNA_INVALID_HANDLE;
    state.render_target = CNA_INVALID_HANDLE;
    state.sprite_batch = CNA_INVALID_HANDLE;
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
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile (back-buffer read-back)");
    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");

    /* Owned resources go before the game (which refuses to be destroyed while they live). */
    if (state.sprite_batch != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sprite_batch_destroy(state.sprite_batch), "cna_sprite_batch_destroy");
    }
    if (state.render_target != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_render_target_destroy(state.render_target), "cna_render_target_destroy");
    }
    if (state.white != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_texture2d_destroy(state.white), "cna_texture2d_destroy");
    }
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
