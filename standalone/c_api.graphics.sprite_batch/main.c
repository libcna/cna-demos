/* SPDX-License-Identifier: MS-PL */
/*
 * Sprites from C: a Texture2D filled with RGBA pixels the program computes, and a SpriteBatch that
 * draws a ring of tinted, rotating copies of it. A whole frame of sprites is one array of
 * CNA_SpriteCommand handed to cna_sprite_batch_submit_many between begin and end.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define TEXTURE_SIZE 16
#define RING 12

typedef struct SpriteState {
    CnaDemo* demo;
    CNA_Handle texture;      /* owned */
    CNA_Handle sprite_batch; /* owned */
    int frames;
} SpriteState;

static const CNA_Color background = {30, 34, 48, 255};

/* A white disc with a soft edge, premultiplied alpha (XNA's convention for SpriteBatch). */
static void make_disc(CNA_Color* pixels)
{
    int x;
    int y;
    for (y = 0; y < TEXTURE_SIZE; ++y) {
        for (x = 0; x < TEXTURE_SIZE; ++x) {
            const double dx = x + 0.5 - TEXTURE_SIZE / 2.0;
            const double dy = y + 0.5 - TEXTURE_SIZE / 2.0;
            const double d = sqrt(dx * dx + dy * dy);
            const double alpha = d < 6.0 ? 1.0 : (d < 8.0 ? (8.0 - d) / 2.0 : 0.0);
            const uint8_t v = (uint8_t)(alpha * 255.0 + 0.5);
            CNA_Color* p = &pixels[y * TEXTURE_SIZE + x];
            p->r = v;
            p->g = v;
            p->b = v;
            p->a = v;
        }
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    SpriteState* state = (SpriteState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo texture_info;
    CNA_Color pixels[TEXTURE_SIZE * TEXTURE_SIZE];
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    /* Borrowed for this callback; the texture and batch created from it are owned and outlive it. */
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = TEXTURE_SIZE;
    texture_info.height = TEXTURE_SIZE;
    texture_info.mip_map = CNA_FALSE;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(graphics_device, &texture_info, &state->texture);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    make_disc(pixels);
    result = cna_texture2d_set_data_rgba8(state->texture, pixels, TEXTURE_SIZE * TEXTURE_SIZE);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    return cna_sprite_batch_create(graphics_device, &state->sprite_batch);
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    SpriteState* state = (SpriteState*)context;
    (void)game_time;
    (void)out_error;
    if (state->demo->frames > 0 && state->frames >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Color hue(int index)
{
    static const CNA_Color colors[6] = {
        {255, 80, 80, 255}, {255, 200, 60, 255}, {120, 230, 90, 255},
        {70, 210, 230, 255}, {90, 120, 255, 255}, {220, 100, 240, 255},
    };
    return colors[index % 6];
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    const double pi = 3.14159265358979323846;
    SpriteState* state = (SpriteState*)context;
    CNA_SpriteCommand commands[RING + 1];
    CNA_SpriteBatchBeginInfo begin_info;
    const double seconds = (double)game_time->total_game_time_ticks / 1e7;
    CNA_Result result;
    int i;
    (void)out_error;
    ++state->frames;
    result = cna_game_clear(game, background);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    /* One command per sprite: destination rectangle, source rectangle, tint, rotation, origin. */
    memset(commands, 0, sizeof(commands));
    for (i = 0; i <= RING; ++i) {
        CNA_SpriteCommand* c = &commands[i];
        const double angle = 2.0 * pi * i / RING + seconds * 0.5;
        const int size = i < RING ? 48 : 96;
        c->struct_size = (uint32_t)sizeof(*c);
        c->struct_version = UINT32_C(1);
        c->texture = state->texture;
        c->destination.x = i < RING ? (int32_t)(400.0 + 160.0 * cos(angle)) : 400;
        c->destination.y = i < RING ? (int32_t)(240.0 + 160.0 * sin(angle)) : 240;
        c->destination.width = size;
        c->destination.height = size;
        c->source.width = TEXTURE_SIZE;
        c->source.height = TEXTURE_SIZE;
        c->color = i < RING ? hue(i) : (CNA_Color){255, 255, 255, 255};
        c->rotation = (float)(seconds * 2.0);
        c->origin.x = TEXTURE_SIZE / 2.0f; /* rotate about the sprite's centre */
        c->origin.y = TEXTURE_SIZE / 2.0f;
        c->effects = CNA_SPRITE_EFFECT_NONE;
    }

    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    result = cna_sprite_batch_begin(state->sprite_batch, &begin_info);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = cna_sprite_batch_submit_many(state->sprite_batch, commands, RING + 1);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_end(state->sprite_batch);
    } else {
        (void)cna_sprite_batch_end(state->sprite_batch); /* end the batch even after a failure */
    }

    if (result == CNA_RESULT_SUCCESS && state->demo->frames > 0 && state->frames == state->demo->frames) {
        /* The last frame: read the back buffer back and look at the centre and a corner. */
        CNA_Handle graphics_device = CNA_INVALID_HANDLE;
        static CNA_Color pixels[800 * 480];
        uint64_t count = 0U;
        CNA_Result readback;
        result = cna_game_get_graphics_device(game, &graphics_device);
        if (result != CNA_RESULT_SUCCESS) {
            return result;
        }
        readback = cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, pixels, 800U * 480U, &count);
        if (readback == CNA_RESULT_NOT_SUPPORTED) {
            printf("  this renderer cannot read its back buffer back; pixel checks skipped\n");
        } else if (cna_demo_check_result(state->demo, readback, "cna_graphics_device_get_backbuffer_data_rgba8") &&
                   cna_demo_check(state->demo, count == 800U * 480U, "an 800x480 back buffer")) {
            const CNA_Color centre = pixels[240 * 800 + 400];
            const CNA_Color corner = pixels[10 * 800 + 10];
            printf("  centre %u,%u,%u  corner %u,%u,%u\n", centre.r, centre.g, centre.b, corner.r, corner.g,
                   corner.b);
            cna_demo_check(state->demo, centre.r > 240 && centre.g > 240 && centre.b > 240,
                           "the large untinted disc is white at the centre");
            cna_demo_check(state->demo, corner.r == background.r && corner.g == background.g &&
                                            corner.b == background.b,
                           "away from the sprites the clear colour remains");
        }
        cna_demo_capture(state->demo, graphics_device);
    }
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    SpriteState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.sprite_batch", argc, argv, 20);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.texture = CNA_INVALID_HANDLE;
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

    /* The texture and the batch are owned by this program: released after the run, before the
       game (which refuses to be destroyed while they live). */
    if (state.sprite_batch != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sprite_batch_destroy(state.sprite_batch), "cna_sprite_batch_destroy");
    }
    if (state.texture != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_texture2d_destroy(state.texture), "cna_texture2d_destroy");
    }
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
