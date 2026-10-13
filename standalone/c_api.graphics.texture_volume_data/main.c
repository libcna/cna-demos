/* SPDX-License-Identifier: MS-PL */
/* Texture3D boxes and TextureCube faces from C, including a minimal in-memory DDS cube. */
#include <CNA/C/cna.h>

#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct VolumeState {
    CnaDemo* demo;
    int loads;
} VolumeState;

static CNA_Texture3DTransfer volume_transfer(uint64_t start)
{
    CNA_Texture3DTransfer result;
    memset(&result, 0, sizeof(result));
    result.struct_size = (uint32_t)sizeof(result);
    result.struct_version = UINT32_C(1);
    result.right = 2;
    result.bottom = 2;
    result.back = 2;
    result.start_index = start;
    result.element_count = 8U;
    return result;
}

static CNA_TextureCubeTransfer cube_transfer(CNA_CubeMapFace face, uint64_t start)
{
    CNA_TextureCubeTransfer result;
    memset(&result, 0, sizeof(result));
    result.struct_size = (uint32_t)sizeof(result);
    result.struct_version = UINT32_C(1);
    result.face = face;
    result.start_index = start;
    result.element_count = 4U;
    return result;
}

static void push_u32(uint8_t* data, size_t* offset, uint32_t value)
{
    data[(*offset)++] = (uint8_t)(value & UINT32_C(0xff));
    data[(*offset)++] = (uint8_t)((value >> 8U) & UINT32_C(0xff));
    data[(*offset)++] = (uint8_t)((value >> 16U) & UINT32_C(0xff));
    data[(*offset)++] = (uint8_t)((value >> 24U) & UINT32_C(0xff));
}

static void push_tag(uint8_t* data, size_t* offset, const char tag[4])
{
    size_t index;
    for (index = 0U; index < 4U; ++index) {
        data[(*offset)++] = (uint8_t)tag[index];
    }
}

static size_t build_minimal_cube_dds(uint8_t data[176])
{
    size_t offset = 0U;
    int index;
    push_tag(data, &offset, "DDS ");
    push_u32(data, &offset, 124U);
    push_u32(data, &offset, UINT32_C(0x6));
    push_u32(data, &offset, 4U);
    push_u32(data, &offset, 4U);
    push_u32(data, &offset, 0U);
    push_u32(data, &offset, 0U);
    push_u32(data, &offset, 1U);
    for (index = 0; index < 11; ++index) {
        push_u32(data, &offset, 0U);
    }
    push_u32(data, &offset, 32U);
    push_u32(data, &offset, UINT32_C(0x4));
    push_tag(data, &offset, "DXT1");
    for (index = 0; index < 5; ++index) {
        push_u32(data, &offset, 0U);
    }
    push_u32(data, &offset, UINT32_C(0x1000));
    push_u32(data, &offset, UINT32_C(0xfe00));
    push_u32(data, &offset, 0U);
    push_u32(data, &offset, 0U);
    push_u32(data, &offset, 0U);
    memset(data + offset, 0, 48U); /* six 4x4 DXT1 faces */
    return offset + 48U;
}

static void check_volume(VolumeState* state, CNA_Handle device)
{
    const CNA_Texture3DCreateInfo create_info = {
        sizeof(CNA_Texture3DCreateInfo), UINT32_C(1), 2U, 2U, 2U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR, 0U
    };
    CNA_Handle texture = CNA_INVALID_HANDLE;
    CNA_Texture3DInfo info;
    CNA_TextureInfo common;
    CNA_Texture3DTransfer window = volume_transfer(1U);
    CNA_Texture3DTransfer raw_window = volume_transfer(0U);
    CNA_Color authored[10];
    CNA_Color output[10];
    CNA_Color before[10];
    uint8_t raw[32];
    uint64_t required = 0U;
    uint32_t index;
    CNA_Result result = cna_texture3d_create(device, &create_info, &texture);
    if (result == CNA_RESULT_NOT_SUPPORTED) {
        cna_demo_check(state->demo, texture == CNA_INVALID_HANDLE,
                       "the selected renderer explicitly refuses volume storage");
        return;
    }
    if (!cna_demo_check_result(state->demo, result, "create a 2x2x2 Color Texture3D")) {
        return;
    }

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    memset(&common, 0, sizeof(common));
    common.struct_size = (uint32_t)sizeof(common);
    common.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_texture3d_get_info(texture, &info) == CNA_RESULT_SUCCESS &&
            info.width == 2U && info.height == 2U && info.depth == 2U && info.level_count == 1U &&
            info.format == CNA_SURFACE_FORMAT_COLOR && info.reserved == 0U &&
            cna_texture_get_info(texture, &common) == CNA_RESULT_SUCCESS &&
            common.level_count == 1U && common.format == CNA_SURFACE_FORMAT_COLOR,
        "Texture3D-specific and common information agree on the authored volume");

    for (index = 0U; index < 10U; ++index) {
        authored[index] = (CNA_Color){(uint8_t)(10U + index), (uint8_t)(30U + index),
                                      (uint8_t)(50U + index), 255U};
    }
    memset(output, 0x5A, sizeof(output));
    memcpy(before, output, sizeof(before));
    cna_demo_check(
        state->demo,
        cna_texture3d_set_data(texture, &window, authored, 10U) == CNA_RESULT_SUCCESS &&
            cna_texture3d_get_data(texture, &window, output, 8U, &required) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            required == 8U && memcmp(output, before, sizeof(output)) == 0,
        "a volume upload uses its source window and short readback is atomic");
    cna_demo_check(
        state->demo,
        cna_texture3d_get_data(texture, &window, output, 10U, &required) == CNA_RESULT_SUCCESS &&
            required == 8U && memcmp(&output[1], &authored[1], 8U * sizeof(CNA_Color)) == 0 &&
            memcmp(&output[0], &before[0], sizeof(CNA_Color)) == 0 &&
            memcmp(&output[9], &before[9], sizeof(CNA_Color)) == 0,
        "volume readback writes exactly the eight selected caller elements");

    for (index = 0U; index < sizeof(raw); index += 4U) {
        raw[index] = (uint8_t)(100U + index);
        raw[index + 1U] = (uint8_t)(101U + index);
        raw[index + 2U] = (uint8_t)(102U + index);
        raw[index + 3U] = 255U;
    }
    memset(output, 0, sizeof(output));
    cna_demo_check(
        state->demo,
        cna_texture3d_set_data_bytes(texture, &raw_window, raw, sizeof(raw) - 1U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_texture3d_set_data_bytes(texture, &raw_window, raw, sizeof(raw)) == CNA_RESULT_SUCCESS &&
            cna_texture3d_get_data(texture, &raw_window, output, 10U, &required) ==
                CNA_RESULT_SUCCESS &&
            required == 8U && memcmp(output, raw, sizeof(raw)) == 0,
        "raw volume bytes require the exact box size and replace all eight voxels");

    cna_demo_check_result(state->demo, cna_texture3d_destroy(texture),
                          "destroy the transferred Texture3D");
}

static void check_cube(VolumeState* state, CNA_Handle device)
{
    const CNA_TextureCubeCreateInfo create_info = {
        sizeof(CNA_TextureCubeCreateInfo), UINT32_C(1), 2U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR, 0U
    };
    CNA_Handle cube = CNA_INVALID_HANDLE;
    CNA_Handle decoded = CNA_INVALID_HANDLE;
    CNA_TextureCubeInfo info;
    CNA_TextureInfo common;
    CNA_Color authored[6];
    CNA_Color output[6];
    CNA_Color before[6];
    uint64_t required = 0U;
    uint32_t face;
    uint8_t dds[176];
    size_t dds_size;
    CNA_Result result = cna_texturecube_create(device, &create_info, &cube);
    if (result == CNA_RESULT_NOT_SUPPORTED) {
        cna_demo_check(state->demo, cube == CNA_INVALID_HANDLE,
                       "the selected renderer explicitly refuses cube storage");
        return;
    }
    if (!cna_demo_check_result(state->demo, result, "create a 2x2 Color TextureCube")) {
        return;
    }

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    memset(&common, 0, sizeof(common));
    common.struct_size = (uint32_t)sizeof(common);
    common.struct_version = UINT32_C(1);
    cna_demo_check(
        state->demo,
        cna_texturecube_get_info(cube, &info) == CNA_RESULT_SUCCESS && info.size == 2U &&
            info.level_count == 1U && info.format == CNA_SURFACE_FORMAT_COLOR && info.reserved == 0U &&
            cna_texture_get_info(cube, &common) == CNA_RESULT_SUCCESS &&
            common.level_count == 1U && common.format == CNA_SURFACE_FORMAT_COLOR,
        "TextureCube-specific and common information agree on all six square faces");

    for (face = CNA_CUBE_MAP_FACE_POSITIVE_X; face <= CNA_CUBE_MAP_FACE_NEGATIVE_Z; ++face) {
        CNA_TextureCubeTransfer window = cube_transfer(face, 1U);
        uint32_t index;
        for (index = 0U; index < 6U; ++index) {
            authored[index] = (CNA_Color){(uint8_t)(face * 20U + index),
                                          (uint8_t)(80U + face),
                                          (uint8_t)(140U + index), 255U};
        }
        memset(output, 0xA5, sizeof(output));
        if (cna_texturecube_set_data(cube, &window, authored, 6U) != CNA_RESULT_SUCCESS ||
            cna_texturecube_get_data(cube, &window, output, 6U, &required) != CNA_RESULT_SUCCESS ||
            required != 4U || memcmp(&output[1], &authored[1], 4U * sizeof(CNA_Color)) != 0) {
            break;
        }
    }
    cna_demo_check(state->demo, face > CNA_CUBE_MAP_FACE_NEGATIVE_Z,
                   "all six cube faces retain independent offset-window Color payloads");

    {
        CNA_TextureCubeTransfer window = cube_transfer(CNA_CUBE_MAP_FACE_NEGATIVE_Z, 1U);
        memset(output, 0x5A, sizeof(output));
        memcpy(before, output, sizeof(before));
        cna_demo_check(
            state->demo,
            cna_texturecube_get_data(cube, &window, output, 4U, &required) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                required == 4U && memcmp(output, before, sizeof(output)) == 0,
            "a short cube-face read reports the region size without a partial write");
    }

    dds_size = build_minimal_cube_dds(dds);
    result = cna_texturecube_create_from_dds_memory(device, dds, (uint64_t)dds_size, &decoded);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    {
        const CNA_Result info_result = result == CNA_RESULT_SUCCESS
            ? cna_texturecube_get_info(decoded, &info) : CNA_RESULT_INVALID_STATE;
        const int valid_dds = dds_size == sizeof(dds) &&
            ((result == CNA_RESULT_SUCCESS && decoded != CNA_INVALID_HANDLE &&
              info_result == CNA_RESULT_SUCCESS && info.size == 4U && info.level_count == 1U &&
              info.format == CNA_SURFACE_FORMAT_COLOR) ||
             (result == CNA_RESULT_NOT_SUPPORTED && decoded == CNA_INVALID_HANDLE));
        cna_demo_check(
            state->demo, valid_dds,
            "an in-memory 4x4 DXT1 cube DDS decodes to Color or reports a storage refusal");
    }

    {
        const CNA_Result decoded_release = decoded != CNA_INVALID_HANDLE
            ? cna_texturecube_destroy(decoded) : CNA_RESULT_SUCCESS;
        const CNA_Result cube_release = cna_texturecube_destroy(cube);
        cna_demo_check(state->demo,
                       decoded_release == CNA_RESULT_SUCCESS && cube_release == CNA_RESULT_SUCCESS,
                       "destroy the decoded and transferred cube resources independently");
    }
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    VolumeState* state = (VolumeState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "get the HiDef device for volume textures")) {
        return CNA_RESULT_SUCCESS;
    }
    check_volume(state, device);
    check_cube(state, device);
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    VolumeState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.texture_volume_data", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
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
    if (cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager),
                              "create a HiDef graphics device")) {
        cna_demo_check_result(&demo, cna_game_run_one_frame(game),
                              "run the Texture3D and TextureCube transfer callback");
    }
    cna_demo_check(&demo, state.loads == 1, "the volume-texture callback ran exactly once");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "cna_graphics_device_manager_destroy");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
