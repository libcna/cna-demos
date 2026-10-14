/* SPDX-License-Identifier: MS-PL */
/* Frozen CNB texture formats and neutral texture-data ownership through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SelectionContext {
    CNA_CnbTextureFormat accepted;
    uint32_t calls;
} SelectionContext;

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t value)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static CNA_Bool accept_one(CNA_CnbTextureFormat format, void* opaque)
{
    SelectionContext* context = (SelectionContext*)opaque;
    context->calls += UINT32_C(1);
    return format == context->accepted ? CNA_TRUE : CNA_FALSE;
}

static CNA_Bool accept_none(CNA_CnbTextureFormat format, void* opaque)
{
    SelectionContext* context = (SelectionContext*)opaque;
    (void)format;
    context->calls += UINT32_C(1);
    return CNA_FALSE;
}

static int release_texture(CNA_CnbTextureDataHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_texture_data_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const uint8_t Rgba[16] = {
        1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U};
    CnaDemo demo;
    CNA_CnbTextureDataHandle rgba_texture = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle texture = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle stale_texture = CNA_INVALID_HANDLE;
    CNA_CnbTextureInfo info;
    CNA_CnbTextureFormat format = CNA_CNB_TEXTURE_FORMAT_UNKNOWN;
    CNA_SurfaceFormat surface = CNA_SURFACE_FORMAT_COLOR;
    SelectionContext selection;
    uint8_t bytes[256];
    uint8_t short_bytes[256];
    char name[64];
    uint64_t size = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    uint64_t index = UINT64_C(0);
    uint32_t unit = UINT32_C(0);
    uint32_t width = UINT32_C(0);
    uint32_t height = UINT32_C(0);
    uint32_t depth = UINT32_C(0);
    uint32_t value;
    CNA_Bool flag = CNA_FALSE;
    int formats_ok = 1;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_texture_data", argc, argv, 1);

    cna_demo_check(
        &demo,
        sizeof(CNA_CnbTextureFormat) == sizeof(uint32_t) &&
            CNA_CNB_TEXTURE_FORMAT_UNKNOWN == UINT32_C(0) &&
            CNA_CNB_TEXTURE_FORMAT_RGBA8 == UINT32_C(1) &&
            CNA_CNB_TEXTURE_FORMAT_BC1 == UINT32_C(22) &&
            CNA_CNB_TEXTURE_FORMAT_BC7_SRGB == UINT32_C(27) &&
            CNA_CNB_TEXTURE_FORMAT_MAXIMUM == UINT32_C(27),
        "texture format identifiers retain their fixed-width wire values");
    cna_demo_check(
        &demo,
        cna_cnb_is_known_texture_format(CNA_CNB_TEXTURE_FORMAT_UNKNOWN, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_cnb_is_known_texture_format(CNA_CNB_TEXTURE_FORMAT_RGBA8, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_cnb_is_known_texture_format(CNA_CNB_TEXTURE_FORMAT_MAXIMUM, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_cnb_is_known_texture_format(CNA_CNB_TEXTURE_FORMAT_MAXIMUM + UINT32_C(1), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "known-format classification accepts exactly the published identity range");

    memset(name, 0xA5, sizeof(name));
    cna_demo_check(
        &demo,
        cna_cnb_get_texture_format_name_size(CNA_CNB_TEXTURE_FORMAT_RGBA8, &size) ==
                CNA_RESULT_SUCCESS &&
            size == UINT64_C(5) &&
            cna_cnb_copy_texture_format_name(
                CNA_CNB_TEXTURE_FORMAT_RGBA8, name, size - UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == size && bytes_are((const uint8_t*)name, sizeof(name), 0xA5U) &&
            cna_cnb_copy_texture_format_name(
                CNA_CNB_TEXTURE_FORMAT_RGBA8, name, sizeof(name), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(5) && memcmp(name, "Rgba8", 5U) == 0,
        "the canonical format name copies exactly and atomically");
    memset(name, 0, sizeof(name));
    cna_demo_check(
        &demo,
        cna_cnb_get_texture_format_name_size(UINT32_C(999), &size) == CNA_RESULT_SUCCESS &&
            size > UINT64_C(0) && size < sizeof(name) &&
            cna_cnb_copy_texture_format_name(
                UINT32_C(999), name, sizeof(name), &count) == CNA_RESULT_SUCCESS &&
            count == size,
        "an unknown wire value still has a stable diagnostic rendering");

    for (value = UINT32_C(1); value <= CNA_CNB_TEXTURE_FORMAT_MAXIMUM; ++value) {
        CNA_CnbTextureFormat round_trip = CNA_CNB_TEXTURE_FORMAT_UNKNOWN;
        surface = CNA_SURFACE_FORMAT_COLOR;
        if (cna_cnb_is_block_compressed_texture_format(value, &flag) != CNA_RESULT_SUCCESS ||
            (flag == CNA_TRUE) != (value >= CNA_CNB_TEXTURE_FORMAT_BC1) ||
            cna_cnb_get_texture_format_unit_bytes(value, &unit) != CNA_RESULT_SUCCESS ||
            unit == UINT32_C(0) ||
            cna_cnb_texture_format_to_surface_format(value, &surface) != CNA_RESULT_SUCCESS ||
            cna_cnb_texture_format_from_surface_format(surface, &round_trip) !=
                CNA_RESULT_SUCCESS ||
            round_trip != value) {
            formats_ok = 0;
        }
    }
    cna_demo_check(
        &demo, formats_ok,
        "all 27 formats agree on block classification, unit size and runtime round trip");
    cna_demo_check(
        &demo,
        cna_cnb_get_texture_format_unit_bytes(CNA_CNB_TEXTURE_FORMAT_UNKNOWN, &unit) ==
                CNA_RESULT_SUCCESS &&
            unit == UINT32_C(0) &&
            cna_cnb_texture_format_to_surface_format(
                CNA_CNB_TEXTURE_FORMAT_UNKNOWN, &surface) == CNA_RESULT_IO,
        "the unknown format has no storage unit or runtime surface mapping");
    cna_demo_check(
        &demo,
        cna_cnb_get_texture_level_byte_size(
            CNA_CNB_TEXTURE_FORMAT_RGBA8, UINT32_C(4), UINT32_C(4), UINT32_C(2), &size) ==
                CNA_RESULT_SUCCESS &&
            size == UINT64_C(128) &&
            cna_cnb_get_texture_level_byte_size(
                CNA_CNB_TEXTURE_FORMAT_BC7, UINT32_C(1), UINT32_C(1), UINT32_C(1), &size) ==
                CNA_RESULT_SUCCESS &&
            size == UINT64_C(16) &&
            cna_cnb_get_texture_level_byte_size(
                CNA_CNB_TEXTURE_FORMAT_BC1, UINT32_C(5), UINT32_C(4), UINT32_C(1), &size) ==
                CNA_RESULT_SUCCESS &&
            size == UINT64_C(16),
        "level sizing handles volume texels and rounds BC dimensions to full blocks");
    size = UINT64_C(0xA5A5A5A5A5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_get_texture_level_byte_size(
            CNA_CNB_TEXTURE_FORMAT_RGBA8, UINT32_C(0), UINT32_C(4), UINT32_C(1), &size) ==
                CNA_RESULT_IO &&
            cna_cnb_get_texture_level_byte_size(
                CNA_CNB_TEXTURE_FORMAT_UNKNOWN, UINT32_C(4), UINT32_C(4), UINT32_C(1), &size) ==
                CNA_RESULT_IO,
        "level sizing refuses zero dimensions and unknown formats");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_texture_data_create_rgba8(
                UINT32_C(2), UINT32_C(2), Rgba, sizeof(Rgba), &rgba_texture),
            "create a copied single-level Rgba8 texture description")) {
        goto cleanup;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_info(rgba_texture, &info) == CNA_RESULT_SUCCESS &&
            info.width == UINT32_C(2) && info.height == UINT32_C(2) &&
            info.depth == UINT32_C(1) && info.face_count == UINT32_C(1) &&
            info.mip_count == UINT32_C(1) && info.representation_count == UINT32_C(1),
        "the convenience texture publishes its complete versioned shape");
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_representation_count(rgba_texture, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_cnb_texture_data_get_representation_format(
                rgba_texture, UINT64_C(0), &format) == CNA_RESULT_SUCCESS &&
            format == CNA_CNB_TEXTURE_FORMAT_RGBA8 &&
            cna_cnb_texture_data_get_level_count(rgba_texture, UINT64_C(0), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "the convenience texture owns one Rgba8 representation and one level");
    memset(short_bytes, 0xA5, sizeof(short_bytes));
    memset(bytes, 0, sizeof(bytes));
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_copy_level(
            rgba_texture, UINT64_C(0), UINT64_C(0), short_bytes, UINT64_C(15), &size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            size == sizeof(Rgba) && bytes_are(short_bytes, sizeof(short_bytes), 0xA5U) &&
            cna_cnb_texture_data_copy_level(
                rgba_texture, UINT64_C(0), UINT64_C(0), bytes, sizeof(bytes), &size) ==
                CNA_RESULT_SUCCESS &&
            size == sizeof(Rgba) && memcmp(bytes, Rgba, sizeof(Rgba)) == 0,
        "level copying returns the exact owned pixels and preserves a short destination");
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_level_dimensions(
            rgba_texture, UINT32_C(0), &width, &height, &depth) == CNA_RESULT_SUCCESS &&
            width == UINT32_C(2) && height == UINT32_C(2) && depth == UINT32_C(1) &&
            cna_cnb_texture_data_get_level_dimensions(
                rgba_texture, UINT32_C(9), &width, &height, &depth) == CNA_RESULT_SUCCESS &&
            width == UINT32_C(1) && height == UINT32_C(1) && depth == UINT32_C(1),
        "mip dimensions halve independently and clamp every axis at one");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_texture_data_add_representation(
                rgba_texture, CNA_CNB_TEXTURE_FORMAT_BC7, &index),
            "append a block-compressed representation after Rgba8")) {
        goto cleanup;
    }
    memset(bytes, 0x3C, 16U);
    cna_demo_check(
        &demo,
        index == UINT64_C(1) &&
            cna_cnb_texture_data_set_level(
                rgba_texture, index, UINT64_C(0), bytes, UINT64_C(16)) == CNA_RESULT_SUCCESS &&
            cna_cnb_texture_data_get_representation_count(rgba_texture, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_cnb_texture_data_get_representation_format(rgba_texture, index, &format) ==
                CNA_RESULT_SUCCESS &&
            format == CNA_CNB_TEXTURE_FORMAT_BC7,
        "the appended representation keeps its index, format and copied level bytes");
    selection.accepted = CNA_CNB_TEXTURE_FORMAT_BC7;
    selection.calls = UINT32_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_select_representation(
            rgba_texture, accept_one, &selection, &flag, &index) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && index == UINT64_C(1) && selection.calls == UINT32_C(2),
        "selection visits representations in preference order and stops at the first match");
    selection.calls = UINT32_C(0);
    index = UINT64_C(777);
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_select_representation(
            rgba_texture, accept_none, &selection, &flag, &index) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && index == UINT64_C(777) && selection.calls == UINT32_C(2),
        "unsupported representations are an ordinary absence with an untouched index");
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_representation_format(
            rgba_texture, UINT64_C(9), &format) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_texture_data_get_level_count(rgba_texture, UINT64_C(9), &count) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_texture_data_copy_level(
                rgba_texture, UINT64_C(0), UINT64_C(9), bytes, sizeof(bytes), &size) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "caller-supplied representation and level bounds are argument errors");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_texture_data_create(
                UINT32_C(8), UINT32_C(4), UINT32_C(2), UINT32_C(1), UINT32_C(3), &texture),
            "create an empty three-level volume description")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_add_representation(
            texture, CNA_CNB_TEXTURE_FORMAT_RGBA8, &index) == CNA_RESULT_SUCCESS &&
            index == UINT64_C(0) &&
            cna_cnb_texture_data_get_level_count(texture, index, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(3),
        "a general texture representation allocates one slot per declared mip");
    memset(bytes, 0x11, sizeof(bytes));
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_set_level(
            texture, UINT64_C(0), UINT64_C(0), bytes, UINT64_C(256)) == CNA_RESULT_SUCCESS &&
            cna_cnb_texture_data_set_level(
                texture, UINT64_C(0), UINT64_C(1), bytes, UINT64_C(32)) == CNA_RESULT_SUCCESS &&
            cna_cnb_texture_data_set_level(
                texture, UINT64_C(0), UINT64_C(2), bytes, UINT64_C(8)) == CNA_RESULT_SUCCESS,
        "the volume owns exact payloads for its 8x4x2, 4x2x1 and 2x1x1 levels");
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_info(texture, &info) == CNA_RESULT_SUCCESS &&
            info.width == UINT32_C(8) && info.height == UINT32_C(4) &&
            info.depth == UINT32_C(2) && info.face_count == UINT32_C(1) &&
            info.mip_count == UINT32_C(3) && info.representation_count == UINT32_C(1) &&
            cna_cnb_texture_data_get_level_dimensions(
                texture, UINT32_C(2), &width, &height, &depth) == CNA_RESULT_SUCCESS &&
            width == UINT32_C(2) && height == UINT32_C(1) && depth == UINT32_C(1),
        "the general description reports its full shape and deepest mip dimensions");
    stale_texture = texture;
    cna_demo_check_result(
        &demo, cna_cnb_texture_data_destroy(texture), "release the general texture description");
    texture = CNA_INVALID_HANDLE;
    count = UINT64_C(0xA5A5A5A5A5A5A5A5);
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_get_representation_count(stale_texture, &count) ==
            CNA_RESULT_INVALID_HANDLE,
        "a released texture description rejects stale access");
    texture = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_create_rgba8(
            UINT32_C(2), UINT32_C(2), Rgba, UINT64_C(15), &texture) == CNA_RESULT_IO &&
            texture == CNA_INVALID_HANDLE &&
            cna_cnb_texture_data_create(
                UINT32_C(2), UINT32_C(2), UINT32_C(1), UINT32_C(0), UINT32_C(1), &texture) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            texture == CNA_INVALID_HANDLE,
        "invalid pixel length and zero faces publish no texture handle");

cleanup:
    releases_ok &= release_texture(&texture);
    releases_ok &= release_texture(&rgba_texture);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining texture-data handle exactly once");
    return cna_demo_finish(&demo);
}
