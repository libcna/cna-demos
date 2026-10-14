/* SPDX-License-Identifier: MS-PL */
/* Standalone and embedded CNB texture-schema round trips through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

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

static int release_document(CNA_CnbDocumentHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_document_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_writer(CNA_CnbWriterHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_writer_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int fill_rgba8(
    CNA_CnbTextureDataHandle texture,
    uint64_t representation,
    uint32_t face_count,
    uint32_t mip_count)
{
    uint32_t face;
    for (face = UINT32_C(0); face < face_count; ++face) {
        uint32_t mip;
        for (mip = UINT32_C(0); mip < mip_count; ++mip) {
            uint32_t width = UINT32_C(0);
            uint32_t height = UINT32_C(0);
            uint32_t depth = UINT32_C(0);
            uint64_t needed = UINT64_C(0);
            uint8_t payload[4096];
            uint64_t byte_index;
            if (cna_cnb_texture_data_get_level_dimensions(
                    texture, mip, &width, &height, &depth) != CNA_RESULT_SUCCESS ||
                cna_cnb_get_texture_level_byte_size(
                    CNA_CNB_TEXTURE_FORMAT_RGBA8, width, height, depth, &needed) !=
                    CNA_RESULT_SUCCESS ||
                needed > sizeof(payload)) {
                return 0;
            }
            for (byte_index = UINT64_C(0); byte_index < needed; ++byte_index) {
                payload[byte_index] =
                    (uint8_t)(face * UINT32_C(31) + mip * UINT32_C(7) + (uint32_t)byte_index);
            }
            if (cna_cnb_texture_data_set_level(
                    texture, representation, (uint64_t)face * mip_count + mip,
                    payload, needed) != CNA_RESULT_SUCCESS) {
                return 0;
            }
        }
    }
    return 1;
}

static int textures_are_equal(
    CNA_CnbTextureDataHandle left,
    CNA_CnbTextureDataHandle right)
{
    CNA_CnbTextureInfo a;
    CNA_CnbTextureInfo b;
    uint64_t representation_count = UINT64_C(0);
    uint64_t representation;
    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));
    a.struct_size = (uint32_t)sizeof(a);
    a.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    b.struct_size = (uint32_t)sizeof(b);
    b.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    if (cna_cnb_texture_data_get_info(left, &a) != CNA_RESULT_SUCCESS ||
        cna_cnb_texture_data_get_info(right, &b) != CNA_RESULT_SUCCESS ||
        a.width != b.width || a.height != b.height || a.depth != b.depth ||
        a.face_count != b.face_count || a.mip_count != b.mip_count ||
        a.representation_count != b.representation_count ||
        cna_cnb_texture_data_get_representation_count(left, &representation_count) !=
            CNA_RESULT_SUCCESS ||
        representation_count != a.representation_count) {
        return 0;
    }
    for (representation = UINT64_C(0); representation < representation_count;
         ++representation) {
        CNA_CnbTextureFormat left_format = CNA_CNB_TEXTURE_FORMAT_UNKNOWN;
        CNA_CnbTextureFormat right_format = CNA_CNB_TEXTURE_FORMAT_UNKNOWN;
        uint64_t level_count = UINT64_C(0);
        uint64_t level;
        if (cna_cnb_texture_data_get_representation_format(
                left, representation, &left_format) != CNA_RESULT_SUCCESS ||
            cna_cnb_texture_data_get_representation_format(
                right, representation, &right_format) != CNA_RESULT_SUCCESS ||
            left_format != right_format ||
            cna_cnb_texture_data_get_level_count(left, representation, &level_count) !=
                CNA_RESULT_SUCCESS) {
            return 0;
        }
        for (level = UINT64_C(0); level < level_count; ++level) {
            uint8_t left_bytes[4096];
            uint8_t right_bytes[4096];
            uint64_t left_size = UINT64_C(0);
            uint64_t right_size = UINT64_C(0);
            if (cna_cnb_texture_data_copy_level(
                    left, representation, level, left_bytes, sizeof(left_bytes), &left_size) !=
                    CNA_RESULT_SUCCESS ||
                cna_cnb_texture_data_copy_level(
                    right, representation, level, right_bytes, sizeof(right_bytes), &right_size) !=
                    CNA_RESULT_SUCCESS ||
                left_size != right_size ||
                memcmp(left_bytes, right_bytes, (size_t)left_size) != 0) {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    static const uint8_t AtlasPixels[16] = {
        1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U};
    static const uint8_t FontBody[4] = {0x53U, 0x46U, 0x4EU, 0x54U};
    CnaDemo demo;
    CNA_CnbTextureDataHandle texture = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle decoded = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle atlas = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle embedded = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle wrong = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_CnbChunkId font_chunk = UINT32_C(0);
    uint8_t image[65536];
    uint8_t short_image[65536];
    uint8_t block[16];
    uint64_t image_size = UINT64_C(0);
    uint64_t required = UINT64_C(0);
    uint64_t representation = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    uint32_t asset_type = UINT32_C(0);
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_texture_codecs", argc, argv, 1);

    if (!cna_demo_check(
            &demo,
            cna_cnb_texture_data_create(
                UINT32_C(4), UINT32_C(4), UINT32_C(1), UINT32_C(1), UINT32_C(3), &texture) ==
                CNA_RESULT_SUCCESS &&
                cna_cnb_texture_data_add_representation(
                    texture, CNA_CNB_TEXTURE_FORMAT_RGBA8, &representation) ==
                    CNA_RESULT_SUCCESS &&
                representation == UINT64_C(0) &&
                fill_rgba8(texture, representation, UINT32_C(1), UINT32_C(3)),
            "author a 4x4 two-dimensional texture with three complete Rgba8 mips")) {
        goto cleanup;
    }
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_texture2d(
                texture, text_view("textures/hero"), NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > CNA_CNB_FORMAT_HEADER_SIZE && image_size < sizeof(image),
            "the 2D encoder reports its exact nonzero document size")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    cna_demo_check(
        &demo,
        cna_cnb_encode_texture2d(
            texture, text_view("textures/hero"), short_image, image_size - UINT64_C(1),
            &required) == CNA_RESULT_BUFFER_TOO_SMALL &&
            required == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short 2D destination is reported without a partial document");
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_texture2d(
                texture, text_view("textures/hero"), image, sizeof(image), &required) ==
                    CNA_RESULT_SUCCESS &&
                required == image_size &&
                cna_cnb_document_parse(
                    image, image_size, text_view("hero.cnb"), NULL, &document) ==
                    CNA_RESULT_SUCCESS,
            "encode and parse the complete two-dimensional texture document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_document_get_asset_type_id(document, &asset_type) == CNA_RESULT_SUCCESS &&
            asset_type == CNA_CNB_ASSET_TYPE_TEXTURE2D &&
            cna_cnb_document_find_all(
                document, CNA_CNB_TEXTURE_CHUNK_PAYLOAD, NULL, UINT64_C(0), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(3),
        "the 2D document declares its asset identity and one payload per mip");
    cna_demo_check(
        &demo,
        cna_cnb_decode_texture2d(document, &decoded) == CNA_RESULT_SUCCESS &&
            textures_are_equal(texture, decoded),
        "the 2D decoder restores every shape, format and mip byte");
    wrong = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_decode_texture_cube(document, &wrong) == CNA_RESULT_IO &&
            wrong == CNA_INVALID_HANDLE &&
            cna_cnb_decode_texture3d(document, &wrong) == CNA_RESULT_IO &&
            wrong == CNA_INVALID_HANDLE,
        "cube and volume decoders refuse a 2D document without partial handles");
    cna_demo_check_result(&demo, cna_cnb_texture_data_destroy(decoded), "release the decoded 2D value");
    decoded = CNA_INVALID_HANDLE;
    cna_demo_check_result(&demo, cna_cnb_document_destroy(document), "release the parsed 2D document");
    document = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_cnb_encode_texture_cube(
            texture, text_view("wrong-shape"), image, sizeof(image), &required) == CNA_RESULT_IO,
        "the cube encoder refuses a one-faced texture description");
    cna_demo_check_result(&demo, cna_cnb_texture_data_destroy(texture), "release the authored 2D value");
    texture = CNA_INVALID_HANDLE;

    if (!cna_demo_check(
            &demo,
            cna_cnb_texture_data_create(
                UINT32_C(2), UINT32_C(2), UINT32_C(1), CNA_CNB_TEXTURE_CUBE_FACE_COUNT,
                UINT32_C(2), &texture) == CNA_RESULT_SUCCESS &&
                cna_cnb_texture_data_add_representation(
                    texture, CNA_CNB_TEXTURE_FORMAT_RGBA8, &representation) ==
                    CNA_RESULT_SUCCESS &&
                fill_rgba8(
                    texture, representation, CNA_CNB_TEXTURE_CUBE_FACE_COUNT, UINT32_C(2)),
            "author all six faces and both mips of a square cube texture")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_encode_texture_cube(
            texture, text_view("textures/sky"), image, sizeof(image), &image_size) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_document_parse(
                image, image_size, text_view("sky.cnb"), NULL, &document) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_decode_texture_cube(document, &decoded) == CNA_RESULT_SUCCESS &&
            textures_are_equal(texture, decoded),
        "the cube codec preserves face-major level order and every payload byte");
    releases_ok &= release_texture(&decoded);
    releases_ok &= release_document(&document);
    releases_ok &= release_texture(&texture);

    if (!cna_demo_check(
            &demo,
            cna_cnb_texture_data_create(
                UINT32_C(4), UINT32_C(2), UINT32_C(2), UINT32_C(1), UINT32_C(2), &texture) ==
                CNA_RESULT_SUCCESS &&
                cna_cnb_texture_data_add_representation(
                    texture, CNA_CNB_TEXTURE_FORMAT_RGBA8, &representation) ==
                    CNA_RESULT_SUCCESS &&
                fill_rgba8(texture, representation, UINT32_C(1), UINT32_C(2)),
            "author a two-level volume whose depth halves with its other dimensions")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_encode_texture3d(
            texture, text_view("textures/fog"), image, sizeof(image), &image_size) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_document_parse(
                image, image_size, text_view("fog.cnb"), NULL, &document) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_decode_texture3d(document, &decoded) == CNA_RESULT_SUCCESS &&
            textures_are_equal(texture, decoded),
        "the volume codec preserves both depth-aware mip payloads exactly");
    releases_ok &= release_texture(&decoded);
    releases_ok &= release_document(&document);
    releases_ok &= release_texture(&texture);

    memset(block, 0x11, sizeof(block));
    cna_demo_check(
        &demo,
        cna_cnb_texture_data_create(
            UINT32_C(4), UINT32_C(4), UINT32_C(1), UINT32_C(1), UINT32_C(1), &texture) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_texture_data_add_representation(
                texture, CNA_CNB_TEXTURE_FORMAT_BC7, &representation) == CNA_RESULT_SUCCESS &&
            cna_cnb_texture_data_set_level(
                texture, representation, UINT64_C(0), block, sizeof(block)) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_encode_texture2d(
                texture, text_view(""), image, sizeof(image), &required) == CNA_RESULT_IO,
        "schema version one refuses to encode a non-Rgba8 representation");
    releases_ok &= release_texture(&texture);

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_texture_data_create_rgba8(
                UINT32_C(2), UINT32_C(2), AtlasPixels, sizeof(AtlasPixels), &atlas),
            "create the sprite font's copied two-dimensional atlas")) {
        goto cleanup;
    }
    if (!cna_demo_check(
            &demo,
            cna_cnb_make_chunk_id('S', 'F', 'N', 'T', &font_chunk) == CNA_RESULT_SUCCESS &&
                cna_cnb_writer_create(
                    CNA_CNB_ASSET_TYPE_SPRITE_FONT, UINT32_C(1), &writer) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_writer_add_chunk(
                    writer, font_chunk, FontBody, sizeof(FontBody), CNA_CNB_CHUNK_FLAG_NONE,
                    UINT32_C(4)) == CNA_RESULT_SUCCESS,
            "create a non-texture SpriteFont document with its own schema chunk")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_writer_append_embedded_texture2d(
            writer, atlas, text_view("SpriteFont")) == CNA_RESULT_SUCCESS &&
            cna_cnb_writer_build(writer, image, sizeof(image), &image_size) == CNA_RESULT_SUCCESS &&
            cna_cnb_document_parse(
                image, image_size, text_view("font.cnb"), NULL, &document) == CNA_RESULT_SUCCESS,
        "append the standard texture chunks inside a complete SpriteFont document");
    cna_demo_check(
        &demo,
        cna_cnb_document_read_embedded_texture2d(
            document, text_view("SpriteFont"), &embedded) == CNA_RESULT_SUCCESS &&
            textures_are_equal(atlas, embedded),
        "the embedded reader restores the atlas without treating the document as a texture asset");
    wrong = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_decode_texture2d(document, &wrong) == CNA_RESULT_IO &&
            wrong == CNA_INVALID_HANDLE,
        "the standalone 2D decoder still rejects the SpriteFont asset identity");

cleanup:
    releases_ok &= release_texture(&wrong);
    releases_ok &= release_texture(&embedded);
    releases_ok &= release_texture(&atlas);
    releases_ok &= release_texture(&decoded);
    releases_ok &= release_texture(&texture);
    releases_ok &= release_document(&document);
    releases_ok &= release_writer(&writer);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining texture, document and writer handle exactly once");
    return cna_demo_finish(&demo);
}
