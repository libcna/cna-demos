/* SPDX-License-Identifier: MS-PL */
/*
 * Pixel-stage texture-slot binding from C. One owned Texture2D is bound, read back, cleared from
 * one slot or every matching slot, and finally destroyed while bound to prove automatic cleanup.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct TextureSlotDemo {
    CnaDemo* demo;
    int loads;
} TextureSlotDemo;

static int all_zero(const uint8_t* bytes, size_t count)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != 0U) {
            return 0;
        }
    }
    return 1;
}

static int read_pixel_slot(CNA_Handle device, uint32_t slot, CNA_TextureSlotInfo* out_info)
{
    memset(out_info, 0xA5, sizeof(*out_info));
    out_info->struct_size = (uint32_t)sizeof(*out_info);
    out_info->struct_version = UINT32_C(1);
    return cna_graphics_device_get_texture(
               device, CNA_SHADER_STAGE_PIXEL, slot, out_info) == CNA_RESULT_SUCCESS &&
        out_info->struct_size == (uint32_t)sizeof(*out_info) &&
        out_info->struct_version == UINT32_C(1) &&
        all_zero(out_info->reserved, sizeof(out_info->reserved));
}

static int slot_is_empty(CNA_Handle device, uint32_t slot)
{
    CNA_TextureSlotInfo info;
    return read_pixel_slot(device, slot, &info) && info.bound == CNA_FALSE &&
        info.texture == CNA_INVALID_HANDLE;
}

static int slot_is_texture(CNA_Handle device, uint32_t slot, CNA_Handle texture)
{
    CNA_TextureSlotInfo info;
    return read_pixel_slot(device, slot, &info) && info.bound == CNA_TRUE &&
        info.texture == texture;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    const uint32_t first_slot = CNA_TEXTURE_COLLECTION_MAX_TEXTURES - UINT32_C(2);
    const uint32_t second_slot = CNA_TEXTURE_COLLECTION_MAX_TEXTURES - UINT32_C(1);
    TextureSlotDemo* state = (TextureSlotDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Handle stale_texture = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo create_info;
    CNA_Result destroy_result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    cna_demo_check(
        state->demo,
        cna_game_get_graphics_device(game, &device) == CNA_RESULT_SUCCESS &&
            device != CNA_INVALID_HANDLE,
        "borrow the live graphics device for this callback");
    cna_demo_check(
        state->demo,
        slot_is_empty(device, first_slot) && slot_is_empty(device, second_slot),
        "the fresh device starts with both selected pixel texture slots empty");

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.width = 2U;
    create_info.height = 2U;
    create_info.format = CNA_SURFACE_FORMAT_COLOR;
    cna_demo_check(
        state->demo,
        cna_texture2d_create(device, &create_info, &texture) == CNA_RESULT_SUCCESS &&
            texture != CNA_INVALID_HANDLE,
        "create one owned Color Texture2D for pixel-stage binding");
    if (texture == CNA_INVALID_HANDLE) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check(
        state->demo,
        cna_graphics_device_set_texture(
            device, CNA_SHADER_STAGE_PIXEL, first_slot, texture) == CNA_RESULT_SUCCESS &&
            slot_is_texture(device, first_slot, texture) && slot_is_empty(device, second_slot),
        "binding one slot reports both native occupancy and the exact owning C handle");
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_texture(
            device, CNA_SHADER_STAGE_PIXEL, first_slot, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            slot_is_empty(device, first_slot) && slot_is_empty(device, second_slot),
        "the invalid handle clears exactly the selected slot");
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_texture(
            device, CNA_SHADER_STAGE_PIXEL, first_slot, texture) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_set_texture(
                device, CNA_SHADER_STAGE_PIXEL, second_slot, texture) == CNA_RESULT_SUCCESS &&
            slot_is_texture(device, first_slot, texture) &&
            slot_is_texture(device, second_slot, texture),
        "the same owned texture can occupy two pixel-stage slots at once");
    cna_demo_check(
        state->demo,
        cna_graphics_device_unbind_texture(device, texture) == CNA_RESULT_SUCCESS &&
            slot_is_empty(device, first_slot) && slot_is_empty(device, second_slot),
        "whole-device unbinding clears every pixel slot holding the texture");
    cna_demo_check(
        state->demo,
        cna_graphics_device_unbind_texture(device, texture) == CNA_RESULT_SUCCESS &&
            slot_is_empty(device, first_slot) && slot_is_empty(device, second_slot),
        "unbinding an already absent live texture is an idempotent no-op");
    cna_demo_check(
        state->demo,
        cna_graphics_device_set_texture(
            device, CNA_SHADER_STAGE_PIXEL, first_slot, texture) == CNA_RESULT_SUCCESS &&
            slot_is_texture(device, first_slot, texture),
        "rebind the texture before releasing its owning handle");

    stale_texture = texture;
    destroy_result = cna_texture2d_destroy(texture);
    if (destroy_result == CNA_RESULT_SUCCESS) {
        texture = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        state->demo,
        destroy_result == CNA_RESULT_SUCCESS && slot_is_empty(device, first_slot) &&
            slot_is_empty(device, second_slot) &&
            cna_graphics_device_set_texture(
                device, CNA_SHADER_STAGE_PIXEL, first_slot, stale_texture) ==
                CNA_RESULT_INVALID_HANDLE,
        "typed destruction automatically unbinds the texture and invalidates its handle");

    if (texture != CNA_INVALID_HANDLE) {
        (void)cna_texture2d_destroy(texture);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                         CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    TextureSlotDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.texture_slots", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_FALSE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(
            &demo, cna_game_create(&create_info, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }

    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device whose pixel texture slots are audited");
    if (manager_ready) {
        cna_demo_check_result(&demo, cna_game_run(game),
                              "run through load and the immediate exit request");
        cna_demo_check(&demo, state.loads == 1,
                       "the texture-slot audit ran in exactly one load callback");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
