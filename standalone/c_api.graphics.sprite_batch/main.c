/* SPDX-License-Identifier: MS-PL */
/*
 * Sprites from C: a Texture2D filled with RGBA pixels the program computes, and a SpriteBatch that
 * draws a ring of tinted, rotating copies of it. A whole frame of sprites is one array of
 * CNA_SpriteCommand handed to cna_sprite_batch_submit_many between begin and end. The final frame
 * also uses CNA_SpriteScaledCommand to place two selected texels by position, origin and
 * nonuniform scale, with exact pixel and texture-retention checks. Explicit state and transformed
 * default-effect Begin routes draw two more probes whose back-buffer pixels verify their behavior.
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

/* A queued C texture is retained until End or cancellation. The red corner command must
   never reach the back buffer: the existing corner pixel check below observes cancellation. */
static void check_cancelled_batch(SpriteState* state, CNA_Handle graphics_device)
{
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle batch = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo texture_info;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_SpriteCommand command;
    const CNA_Color red = {255, 0, 0, 255};
    CNA_Result result;
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 1;
    texture_info.height = 1;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    if (!cna_demo_check_result(state->demo, cna_texture2d_create(graphics_device, &texture_info, &texture),
                               "create the cancellation probe texture")) goto cleanup;
    if (!cna_demo_check_result(state->demo, cna_texture2d_set_data_rgba8(texture, &red, 1),
                               "upload the cancellation probe color")) goto cleanup;
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_create(graphics_device, &batch),
                               "create an independently owned cancellation batch")) goto cleanup;
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_begin(batch, &begin_info),
                               "begin a deferred cancellation interval")) goto cleanup;
    memset(&command, 0, sizeof(command));
    command.struct_size = (uint32_t)sizeof(command);
    command.struct_version = UINT32_C(1);
    command.texture = texture;
    command.destination = (CNA_Rectangle){4, 4, 20, 20};
    command.source = (CNA_Rectangle){0, 0, 1, 1};
    command.color = (CNA_Color){255, 255, 255, 255};
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_submit_many(batch, &command, 1),
                               "queue a red corner command without ending the interval")) goto cleanup;
    result = cna_texture2d_destroy(texture);
    cna_demo_check(state->demo, result == CNA_RESULT_INVALID_STATE,
                   "a texture retained by an active deferred batch cannot be destroyed");
    if (result == CNA_RESULT_SUCCESS) texture = CNA_INVALID_HANDLE;
    /* Deliberately no End: destroy cancels queued commands and releases their C references. */
    result = cna_sprite_batch_destroy(batch);
    if (cna_demo_check_result(state->demo, result,
                              "destroy the active batch without flushing its queued command"))
        batch = CNA_INVALID_HANDLE;
    if (texture != CNA_INVALID_HANDLE) {
        result = cna_texture2d_destroy(texture);
        if (cna_demo_check_result(state->demo, result,
                                  "cancelling the batch releases its retained texture for destruction"))
            texture = CNA_INVALID_HANDLE;
    }
cleanup:
    if (batch != CNA_INVALID_HANDLE) (void)cna_sprite_batch_destroy(batch);
    if (texture != CNA_INVALID_HANDLE) (void)cna_texture2d_destroy(texture);
}

/* Successful End is the other retention boundary: the gold command must be visible even
   after its texture is released. The last-frame readback checks the opposite corner. */
static void check_completed_batch(SpriteState* state, CNA_Handle graphics_device)
{
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle batch = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo texture_info;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_SpriteCommand command;
    const CNA_Color gold = {255, 215, 0, 255};
    CNA_Result result;
    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 1;
    texture_info.height = 1;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    if (!cna_demo_check_result(state->demo, cna_texture2d_create(graphics_device, &texture_info, &texture),
                               "create the completion probe texture")) goto cleanup;
    if (!cna_demo_check_result(state->demo, cna_texture2d_set_data_rgba8(texture, &gold, 1),
                               "upload the completion probe color")) goto cleanup;
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_create(graphics_device, &batch),
                               "create an independently owned completion batch")) goto cleanup;
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_begin(batch, &begin_info),
                               "begin a deferred completion interval")) goto cleanup;
    memset(&command, 0, sizeof(command));
    command.struct_size = (uint32_t)sizeof(command);
    command.struct_version = UINT32_C(1);
    command.texture = texture;
    command.destination = (CNA_Rectangle){776, 456, 20, 20};
    command.source = (CNA_Rectangle){0, 0, 1, 1};
    command.color = (CNA_Color){255, 255, 255, 255};
    /* The first command would visibly change the old clear-corner control. An invalid second
       texture must refuse the whole array before that valid first command enters the batch. */
    {
        CNA_SpriteCommand refused[2] = {command, command};
        refused[0].destination = (CNA_Rectangle){4, 4, 20, 20};
        refused[1].texture = CNA_INVALID_HANDLE;
        result = cna_sprite_batch_submit_many(batch, refused, 2);
        cna_demo_check(state->demo, result == CNA_RESULT_INVALID_HANDLE,
                       "an invalid second texture refuses the entire sprite command array");
    }
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_submit_many(batch, &command, 1),
                               "queue a gold corner command for a successful flush")) goto cleanup;
    result = cna_texture2d_destroy(texture);
    cna_demo_check(state->demo, result == CNA_RESULT_INVALID_STATE,
                   "the completion batch retains its texture before End");
    if (result == CNA_RESULT_SUCCESS) texture = CNA_INVALID_HANDLE;
    /* End emits the gold command and must release the texture before the batch is destroyed. */
    if (!cna_demo_check_result(state->demo, cna_sprite_batch_end(batch),
                               "End successfully flushes the gold corner command")) goto cleanup;
    if (texture != CNA_INVALID_HANDLE) {
        result = cna_texture2d_destroy(texture);
        if (cna_demo_check_result(state->demo, result,
                                  "successful End releases the texture while its batch is still alive"))
            texture = CNA_INVALID_HANDLE;
    }
    result = cna_sprite_batch_destroy(batch);
    if (cna_demo_check_result(state->demo, result, "destroy the completed batch after its texture"))
        batch = CNA_INVALID_HANDLE;
cleanup:
    if (batch != CNA_INVALID_HANDLE) (void)cna_sprite_batch_destroy(batch);
    if (texture != CNA_INVALID_HANDLE) (void)cna_texture2d_destroy(texture);
}

/* The position-and-scale overload family has different geometry from destination rectangles:
   origin is measured in source texels and is multiplied by scale before offsetting position.
   A four-texel strip keeps each one-texel source sample surrounded by the same colour, so the
   stable interior pixels below are exact even with SpriteBatch's default linear sampling. */
static void check_scaled_batch(SpriteState* state, CNA_Handle graphics_device)
{
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle batch = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo texture_info;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_SpriteScaledCommand commands[2];
    CNA_SpriteScaledCommand refused[2];
    const CNA_Color cyan = {0, 255, 255, 255};
    const CNA_Color magenta = {255, 0, 255, 255};
    const CNA_Color pixels[4] = {cyan, cyan, magenta, magenta};
    CNA_Result result;

    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 4;
    texture_info.height = 1;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    if (!cna_demo_check_result(
            state->demo, cna_texture2d_create(graphics_device, &texture_info, &texture),
            "create the scaled-command source strip"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_texture2d_set_data_rgba8(texture, pixels, 4),
            "upload paired cyan and magenta source texels"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_create(graphics_device, &batch),
            "create an independently owned scaled-command batch"))
        goto cleanup;

    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_begin(batch, &begin_info),
            "begin the deferred scaled-command interval"))
        goto cleanup;

    memset(commands, 0, sizeof(commands));
    commands[0].struct_size = (uint32_t)sizeof(commands[0]);
    commands[0].struct_version = UINT32_C(1);
    commands[0].texture = texture;
    commands[0].position = (CNA_Vector2){64.0f, 64.0f};
    commands[0].source = (CNA_Rectangle){0, 0, 1, 1};
    commands[0].color = (CNA_Color){255, 255, 255, 255};
    commands[0].origin = (CNA_Vector2){1.0f, 1.0f};
    commands[0].scale = (CNA_Vector2){12.0f, 8.0f};
    commands[0].effects = CNA_SPRITE_EFFECT_NONE;
    commands[1] = commands[0];
    commands[1].position = (CNA_Vector2){80.0f, 56.0f};
    commands[1].source = (CNA_Rectangle){3, 0, 1, 1};
    commands[1].origin = (CNA_Vector2){0.0f, 0.0f};
    commands[1].scale = (CNA_Vector2){6.0f, 14.0f};

    refused[0] = commands[0];
    refused[0].position = (CNA_Vector2){32.0f, 40.0f};
    refused[0].origin = (CNA_Vector2){0.0f, 0.0f};
    refused[0].scale = (CNA_Vector2){8.0f, 6.0f};
    refused[1] = commands[1];
    refused[1].texture = CNA_INVALID_HANDLE;
    result = cna_sprite_batch_submit_scaled_many(batch, refused, UINT64_C(2));
    cna_demo_check(
        state->demo, result == CNA_RESULT_INVALID_HANDLE,
        "an invalid second texture refuses the entire position-and-scale command array");

    if (!cna_demo_check_result(
            state->demo,
            cna_sprite_batch_submit_scaled_many(batch, commands, UINT64_C(2)),
            "queue two selected texels with distinct positions, origins and nonuniform scales"))
        goto cleanup;
    result = cna_texture2d_destroy(texture);
    cna_demo_check(
        state->demo, result == CNA_RESULT_INVALID_STATE,
        "the scaled-command batch retains its source texture before End");
    if (result == CNA_RESULT_SUCCESS) {
        texture = CNA_INVALID_HANDLE;
    }
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_end(batch),
            "End flushes both accepted position-and-scale commands"))
        goto cleanup;
    if (texture != CNA_INVALID_HANDLE) {
        result = cna_texture2d_destroy(texture);
        if (cna_demo_check_result(
                state->demo, result,
                "successful End releases the scaled-command texture"))
            texture = CNA_INVALID_HANDLE;
    }
    result = cna_sprite_batch_destroy(batch);
    if (cna_demo_check_result(
            state->demo, result, "destroy the completed scaled-command batch"))
        batch = CNA_INVALID_HANDLE;

cleanup:
    if (batch != CNA_INVALID_HANDLE) (void)cna_sprite_batch_destroy(batch);
    if (texture != CNA_INVALID_HANDLE) (void)cna_texture2d_destroy(texture);
}

/* These two Begin routes need behavioral evidence beyond accepting their arguments. Opaque makes
   the half-alpha red texel replace the clear colour without blending, while the default sprite
   effect applies a nonidentity transform to move the green command sixty pixels to the right. */
static void check_stateful_batch(SpriteState* state, CNA_Handle graphics_device)
{
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle batch = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo texture_info;
    CNA_BlendState blend;
    CNA_SamplerState sampler;
    CNA_DepthStencilState depth;
    CNA_RasterizerState rasterizer;
    CNA_SpriteCommand command;
    CNA_Matrix translation;
    const CNA_Color pixels[2] = {{128, 0, 0, 128}, {0, 255, 0, 255}};
    CNA_Result result;

    memset(&texture_info, 0, sizeof(texture_info));
    texture_info.struct_size = (uint32_t)sizeof(texture_info);
    texture_info.struct_version = UINT32_C(1);
    texture_info.width = 2;
    texture_info.height = 1;
    texture_info.format = CNA_SURFACE_FORMAT_COLOR;
    if (!cna_demo_check_result(
            state->demo, cna_texture2d_create(graphics_device, &texture_info, &texture),
            "create the stateful-Begin source strip"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_texture2d_set_data_rgba8(texture, pixels, UINT64_C(2)),
            "upload half-alpha red and opaque green stateful-Begin texels"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_create(graphics_device, &batch),
            "create an independently owned stateful-Begin batch"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_blend_state_init(CNA_BLEND_STATE_PRESET_OPAQUE, &blend),
            "initialize the explicit Opaque blend state"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sampler_state_init(CNA_SAMPLER_STATE_PRESET_POINT_CLAMP, &sampler),
            "initialize the explicit PointClamp sampler state"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_depth_stencil_state_init(CNA_DEPTH_STENCIL_STATE_PRESET_NONE, &depth),
            "initialize the explicit depth/stencil None state"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_rasterizer_state_init(CNA_RASTERIZER_STATE_PRESET_CULL_NONE, &rasterizer),
            "initialize the explicit CullNone rasterizer state"))
        goto cleanup;

    memset(&command, 0, sizeof(command));
    command.struct_size = (uint32_t)sizeof(command);
    command.struct_version = UINT32_C(1);
    command.texture = texture;
    command.destination = (CNA_Rectangle){100, 90, 12, 12};
    command.source = (CNA_Rectangle){0, 0, 1, 1};
    command.color = (CNA_Color){255, 255, 255, 255};
    if (!cna_demo_check_result(
            state->demo,
            cna_sprite_batch_begin_with_states(
                batch, CNA_SPRITE_SORT_MODE_DEFERRED, &blend, &sampler, &depth, &rasterizer),
            "begin with explicit Opaque, PointClamp, None and CullNone states"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_submit_many(batch, &command, UINT64_C(1)),
            "queue the half-alpha red explicit-state command"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_end(batch),
            "End flushes the explicit-state command"))
        goto cleanup;

    if (!cna_demo_check_result(
            state->demo, cna_matrix_create_translation_xyz(60.0f, 0.0f, 0.0f, &translation),
            "create the nonidentity SpriteBatch translation"))
        goto cleanup;
    command.destination = (CNA_Rectangle){100, 120, 12, 12};
    command.source = (CNA_Rectangle){1, 0, 1, 1};
    if (!cna_demo_check_result(
            state->demo,
            cna_sprite_batch_begin_with_effect(
                batch, CNA_SPRITE_SORT_MODE_DEFERRED, &blend, &sampler, &depth, &rasterizer,
                CNA_INVALID_HANDLE, &translation),
            "begin with the default effect and a sixty-pixel translation"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_submit_many(batch, &command, UINT64_C(1)),
            "queue the green transformed-effect command"))
        goto cleanup;
    if (!cna_demo_check_result(
            state->demo, cna_sprite_batch_end(batch),
            "End flushes the transformed default-effect command"))
        goto cleanup;

    result = cna_texture2d_destroy(texture);
    if (cna_demo_check_result(
            state->demo, result,
            "both stateful End calls release the shared probe texture"))
        texture = CNA_INVALID_HANDLE;
    result = cna_sprite_batch_destroy(batch);
    if (cna_demo_check_result(
            state->demo, result, "destroy the completed stateful-Begin batch"))
        batch = CNA_INVALID_HANDLE;

cleanup:
    if (batch != CNA_INVALID_HANDLE) (void)cna_sprite_batch_destroy(batch);
    if (texture != CNA_INVALID_HANDLE) (void)cna_texture2d_destroy(texture);
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
        check_cancelled_batch(state, graphics_device);
        check_completed_batch(state, graphics_device);
        check_scaled_batch(state, graphics_device);
        check_stateful_batch(state, graphics_device);
        readback = cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, pixels, 800U * 480U, &count);
        if (readback == CNA_RESULT_NOT_SUPPORTED) {
            printf("  this renderer cannot read its back buffer back; pixel checks skipped\n");
        } else if (cna_demo_check_result(state->demo, readback, "cna_graphics_device_get_backbuffer_data_rgba8") &&
                   cna_demo_check(state->demo, count == 800U * 480U, "an 800x480 back buffer")) {
            const CNA_Color centre = pixels[240 * 800 + 400];
            const CNA_Color corner = pixels[10 * 800 + 10];
            const CNA_Color completed = pixels[470 * 800 + 790];
            const CNA_Color refused_scaled = pixels[43 * 800 + 35];
            const CNA_Color cyan_first = pixels[57 * 800 + 53];
            const CNA_Color cyan_last = pixels[62 * 800 + 62];
            const CNA_Color cyan_left = pixels[60 * 800 + 51];
            const CNA_Color cyan_below = pixels[64 * 800 + 58];
            const CNA_Color magenta_first = pixels[57 * 800 + 81];
            const CNA_Color magenta_last = pixels[68 * 800 + 84];
            const CNA_Color magenta_left = pixels[62 * 800 + 79];
            const CNA_Color magenta_below = pixels[70 * 800 + 82];
            const CNA_Color opaque_red = pixels[95 * 800 + 105];
            const CNA_Color untranslated_green = pixels[125 * 800 + 105];
            const CNA_Color translated_green = pixels[125 * 800 + 165];
            printf("  centre %u,%u,%u  corner %u,%u,%u\n", centre.r, centre.g, centre.b, corner.r, corner.g,
                   corner.b);
            cna_demo_check(state->demo, centre.r > 240 && centre.g > 240 && centre.b > 240,
                           "the large untinted disc is white at the centre");
            cna_demo_check(state->demo, corner.r == background.r && corner.g == background.g &&
                                            corner.b == background.b,
                           "away from the sprites the clear colour remains");
            cna_demo_check(state->demo, completed.r == 255 && completed.g == 215 && completed.b == 0,
                           "the successfully flushed gold command survives texture and batch destruction");
            cna_demo_check(
                state->demo,
                refused_scaled.r == background.r && refused_scaled.g == background.g &&
                    refused_scaled.b == background.b,
                "the valid first scaled command from the refused array never reached the frame");
            cna_demo_check(
                state->demo,
                cyan_first.r == 0 && cyan_first.g == 255 && cyan_first.b == 255 &&
                    cyan_last.r == 0 && cyan_last.g == 255 && cyan_last.b == 255 &&
                    cyan_left.r == background.r && cyan_left.g == background.g &&
                    cyan_left.b == background.b && cyan_below.r == background.r &&
                    cyan_below.g == background.g && cyan_below.b == background.b,
                "the cyan source texel spans 12x8 pixels before its origin position");
            cna_demo_check(
                state->demo,
                magenta_first.r == 255 && magenta_first.g == 0 && magenta_first.b == 255 &&
                    magenta_last.r == 255 && magenta_last.g == 0 && magenta_last.b == 255 &&
                    magenta_left.r == background.r && magenta_left.g == background.g &&
                    magenta_left.b == background.b && magenta_below.r == background.r &&
                    magenta_below.g == background.g && magenta_below.b == background.b,
                "the selected magenta texel starts at its position and spans 6x14 pixels");
            cna_demo_check(
                state->demo,
                opaque_red.r == 128 && opaque_red.g == 0 && opaque_red.b == 0,
                "the explicit Opaque state writes the half-alpha source without blending");
            cna_demo_check(
                state->demo,
                untranslated_green.r == background.r && untranslated_green.g == background.g &&
                    untranslated_green.b == background.b && translated_green.r == 0 &&
                    translated_green.g == 255 && translated_green.b == 0,
                "the transformed default effect moves the green command sixty pixels right");
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
