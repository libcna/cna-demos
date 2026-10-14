/* SPDX-License-Identifier: MS-PL */
/* Neutral SpriteFont glyph data, atlas ownership and complete CNB codec through C. */
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

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t expected)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static CNA_SpriteFontGlyph make_glyph(CNA_Char16 character, int32_t seed)
{
    CNA_SpriteFontGlyph glyph;
    memset(&glyph, 0, sizeof(glyph));
    glyph.struct_size = (uint32_t)sizeof(glyph);
    glyph.struct_version = UINT32_C(1);
    glyph.glyph_bounds.x = seed;
    glyph.glyph_bounds.y = seed + INT32_C(1);
    glyph.glyph_bounds.width = INT32_C(5);
    glyph.glyph_bounds.height = INT32_C(7);
    glyph.cropping.x = seed + INT32_C(2);
    glyph.cropping.y = seed + INT32_C(3);
    glyph.cropping.width = INT32_C(4);
    glyph.cropping.height = INT32_C(6);
    glyph.character = character;
    glyph.kerning.x = (float)seed + 0.25F;
    glyph.kerning.y = 5.0F;
    glyph.kerning.z = (float)seed + 0.75F;
    return glyph;
}

static int glyph_is(
    CNA_CnbSpriteFontDataHandle font, uint64_t index, CNA_Char16 character, int32_t seed)
{
    CNA_SpriteFontGlyph glyph;
    memset(&glyph, 0, sizeof(glyph));
    glyph.struct_size = (uint32_t)sizeof(glyph);
    glyph.struct_version = UINT32_C(1);
    return cna_cnb_sprite_font_data_get_glyph(font, index, &glyph) == CNA_RESULT_SUCCESS &&
           glyph.glyph_bounds.x == seed && glyph.glyph_bounds.y == seed + INT32_C(1) &&
           glyph.glyph_bounds.width == INT32_C(5) &&
           glyph.glyph_bounds.height == INT32_C(7) &&
           glyph.cropping.x == seed + INT32_C(2) &&
           glyph.cropping.y == seed + INT32_C(3) &&
           glyph.cropping.width == INT32_C(4) && glyph.cropping.height == INT32_C(6) &&
           glyph.character == character && glyph.reserved == UINT16_C(0) &&
           glyph.kerning.x == (float)seed + 0.25F && glyph.kerning.y == 5.0F &&
           glyph.kerning.z == (float)seed + 0.75F;
}

static int font_info_is(CNA_CnbSpriteFontDataHandle font, uint64_t glyph_count)
{
    CNA_CnbSpriteFontInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_SPRITE_FONT_INFO_STRUCT_VERSION;
    return cna_cnb_sprite_font_data_get_info(font, &info) == CNA_RESULT_SUCCESS &&
           info.glyph_count == glyph_count && info.line_spacing == INT32_C(18) &&
           info.spacing == 1.25F && info.default_character == (CNA_Char16)'?' &&
           info.has_default_character == CNA_TRUE && bytes_are(info.reserved, 5U, 0U);
}

static int atlas_is(
    CNA_CnbSpriteFontDataHandle font, const uint8_t* expected, size_t expected_size)
{
    CNA_CnbTextureDataHandle atlas = CNA_INVALID_HANDLE;
    CNA_CnbTextureInfo info;
    uint8_t pixels[128];
    uint64_t copied = UINT64_C(0);
    int ok;

    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_TEXTURE_INFO_STRUCT_VERSION;
    ok = cna_cnb_sprite_font_data_copy_atlas(font, &atlas) == CNA_RESULT_SUCCESS &&
         cna_cnb_texture_data_get_info(atlas, &info) == CNA_RESULT_SUCCESS &&
         info.width == UINT32_C(8) && info.height == UINT32_C(4) &&
         info.depth == UINT32_C(1) && info.face_count == UINT32_C(1) &&
         info.mip_count == UINT32_C(1) && info.representation_count == UINT32_C(1) &&
         cna_cnb_texture_data_copy_level(
             atlas, UINT64_C(0), UINT64_C(0), pixels, sizeof(pixels), &copied) ==
             CNA_RESULT_SUCCESS &&
         copied == (uint64_t)expected_size && memcmp(pixels, expected, expected_size) == 0;
    if (atlas != CNA_INVALID_HANDLE && cna_cnb_texture_data_destroy(atlas) != CNA_RESULT_SUCCESS) {
        ok = 0;
    }
    return ok;
}

static int release_font(CNA_CnbSpriteFontDataHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_sprite_font_data_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
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

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CnbSpriteFontDataHandle font = CNA_INVALID_HANDLE;
    CNA_CnbSpriteFontDataHandle decoded = CNA_INVALID_HANDLE;
    CNA_CnbSpriteFontDataHandle stale_font = CNA_INVALID_HANDLE;
    CNA_CnbTextureDataHandle atlas = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbSpriteFontInfo info;
    CNA_SpriteFontGlyph question;
    CNA_SpriteFontGlyph letter_a;
    CNA_SpriteFontGlyph invalid;
    CNA_SpriteFontGlyph untouched;
    uint8_t pixels[128];
    uint8_t image[65536];
    uint8_t second_image[65536];
    uint8_t short_image[65536];
    uint64_t index = UINT64_MAX;
    uint64_t image_size = UINT64_C(0);
    uint64_t second_size = UINT64_C(0);
    uint64_t glyph_count = UINT64_C(0);
    size_t byte_index;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_sprite_font_data", argc, argv, 1);
    cna_demo_check(
        &demo,
        CNA_CNB_SPRITE_FONT_SCHEMA_VERSION == UINT32_C(1) &&
            CNA_CNB_SPRITE_FONT_HEADER_STRIDE == UINT32_C(24) &&
            CNA_CNB_SPRITE_FONT_RECTANGLE_STRIDE == UINT32_C(16) &&
            CNA_CNB_SPRITE_FONT_KERNING_STRIDE == UINT32_C(12) &&
            CNA_CNB_SPRITE_FONT_CHARACTER_STRIDE == UINT32_C(4) &&
            CNA_CNB_MAX_SPRITE_FONT_GLYPHS == UINT32_C(65536),
        "the SpriteFont schema publishes its fixed version, strides and glyph ceiling");
    if (!cna_demo_check_result(
            &demo, cna_cnb_sprite_font_data_create(&font),
            "create an empty neutral SpriteFont description")) {
        goto cleanup;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_SPRITE_FONT_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_get_info(font, &info) == CNA_RESULT_SUCCESS &&
            info.glyph_count == UINT64_C(0) && info.line_spacing == INT32_C(0) &&
            info.spacing == 0.0F && info.default_character == (CNA_Char16)0 &&
            info.has_default_character == CNA_FALSE && bytes_are(info.reserved, 5U, 0U),
        "a new font reports canonical empty metrics and no default character");

    info.glyph_count = UINT64_C(99);
    info.line_spacing = INT32_C(18);
    info.spacing = 1.25F;
    info.default_character = (CNA_Char16)'?';
    info.has_default_character = CNA_TRUE;
    cna_demo_check_result(
        &demo, cna_cnb_sprite_font_data_set_info(font, &info),
        "set whole-font metrics while the derived glyph count is ignored");
    cna_demo_check(
        &demo, font_info_is(font, UINT64_C(0)),
        "whole-font metrics round-trip with a still-empty glyph table");
    info.has_default_character = (CNA_Bool)2U;
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_set_info(font, &info) == CNA_RESULT_INVALID_ARGUMENT &&
            font_info_is(font, UINT64_C(0)),
        "a noncanonical default-character flag is refused without changing metrics");
    info.has_default_character = CNA_TRUE;

    letter_a = make_glyph((CNA_Char16)'A', INT32_C(20));
    question = make_glyph((CNA_Char16)'?', INT32_C(10));
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_add_glyph(font, &letter_a, &index) == CNA_RESULT_SUCCESS &&
            index == UINT64_C(0) &&
            cna_cnb_sprite_font_data_add_glyph(font, &question, &index) == CNA_RESULT_SUCCESS &&
            index == UINT64_C(1) && font_info_is(font, UINT64_C(2)),
        "appending glyphs publishes sequential indices and a derived live count");
    invalid = make_glyph((CNA_Char16)'Z', INT32_C(30));
    invalid.reserved = UINT16_C(1);
    index = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_add_glyph(font, &invalid, &index) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            index == UINT64_MAX && font_info_is(font, UINT64_C(2)),
        "an invalid glyph rolls back every parallel array and preserves the output index");
    cna_demo_check(
        &demo,
        glyph_is(font, UINT64_C(0), (CNA_Char16)'A', INT32_C(20)) &&
            glyph_is(font, UINT64_C(1), (CNA_Char16)'?', INT32_C(10)),
        "glyph reads restore both complete authored rows before sorting");
    memset(&untouched, 0xA5, sizeof(untouched));
    untouched.struct_size = (uint32_t)sizeof(untouched);
    untouched.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_get_glyph(font, UINT64_C(2), &untouched) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            untouched.character == (CNA_Char16)UINT16_C(0xA5A5),
        "a glyph index at Count is refused without overwriting the caller row");

    for (byte_index = 0U; byte_index < sizeof(pixels); ++byte_index) {
        pixels[byte_index] = (uint8_t)(byte_index * 3U + 1U);
    }
    if (!cna_demo_check(
            &demo,
            cna_cnb_texture_data_create_rgba8(
                UINT32_C(8), UINT32_C(4), pixels, sizeof(pixels), &atlas) ==
                    CNA_RESULT_SUCCESS &&
                cna_cnb_sprite_font_data_set_atlas(font, atlas) == CNA_RESULT_SUCCESS &&
                cna_cnb_texture_data_destroy(atlas) == CNA_RESULT_SUCCESS,
            "copy an 8-by-4 RGBA atlas into the font and release the source")) {
        atlas = CNA_INVALID_HANDLE;
        goto cleanup;
    }
    atlas = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo, atlas_is(font, pixels, sizeof(pixels)) && atlas_is(font, pixels, sizeof(pixels)),
        "each atlas copy is independent and restores the exact owned level bytes");

    image_size = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_encode_sprite_font(
            font, text_view("Fonts/Hud"), NULL, UINT64_C(0), &image_size) == CNA_RESULT_IO &&
            image_size == UINT64_C(0),
        "encoding refuses the deliberately descending character map");
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_set_glyph(font, UINT64_C(0), &question) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_sprite_font_data_set_glyph(font, UINT64_C(1), &letter_a) ==
                CNA_RESULT_SUCCESS &&
            glyph_is(font, UINT64_C(0), (CNA_Char16)'?', INT32_C(10)) &&
            glyph_is(font, UINT64_C(1), (CNA_Char16)'A', INT32_C(20)),
        "replacement sorts the same glyph rows into strict character order");
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_set_glyph(font, UINT64_C(2), &letter_a) ==
            CNA_RESULT_INVALID_ARGUMENT,
        "glyph replacement rejects an index at Count");

    image_size = UINT64_C(0);
    if (!cna_demo_check(
            &demo,
            cna_cnb_encode_sprite_font(
                font, text_view("Fonts/Hud"), NULL, UINT64_C(0), &image_size) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                image_size > UINT64_C(0) && image_size <= sizeof(image),
            "size the complete SpriteFont document without a destination")) {
        goto cleanup;
    }
    memset(short_image, 0xA5, sizeof(short_image));
    second_size = UINT64_C(0);
    cna_demo_check(
        &demo,
        cna_cnb_encode_sprite_font(
            font, text_view("Fonts/Hud"), short_image, image_size - UINT64_C(1),
            &second_size) == CNA_RESULT_BUFFER_TOO_SMALL &&
            second_size == image_size && bytes_are(short_image, sizeof(short_image), 0xA5U),
        "a short document destination is sized without a partial write");
    cna_demo_check(
        &demo,
        cna_cnb_encode_sprite_font(
            font, text_view("Fonts/Hud"), image, sizeof(image), &second_size) ==
                CNA_RESULT_SUCCESS &&
            second_size == image_size &&
            cna_cnb_encode_sprite_font(
                font, text_view("Fonts/Hud"), second_image, sizeof(second_image),
                &glyph_count) == CNA_RESULT_SUCCESS &&
            glyph_count == image_size && memcmp(image, second_image, (size_t)image_size) == 0,
        "complete SpriteFont encoding is exact and repeatable");
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_document_parse(image, image_size, text_view("font-memory"), NULL, &document),
            "parse the complete SpriteFont document")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo, cna_cnb_decode_sprite_font(document, &decoded),
            "decode the neutral SpriteFont data from its document")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        font_info_is(decoded, UINT64_C(2)) &&
            glyph_is(decoded, UINT64_C(0), (CNA_Char16)'?', INT32_C(10)) &&
            glyph_is(decoded, UINT64_C(1), (CNA_Char16)'A', INT32_C(20)) &&
            atlas_is(decoded, pixels, sizeof(pixels)),
        "decoding restores metrics, ordered glyph rows and the embedded atlas");

    stale_font = font;
    cna_demo_check_result(
        &demo, cna_cnb_sprite_font_data_destroy(font), "release the original font description");
    font = CNA_INVALID_HANDLE;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = CNA_CNB_SPRITE_FONT_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_sprite_font_data_get_info(stale_font, &info) == CNA_RESULT_INVALID_HANDLE &&
            font_info_is(decoded, UINT64_C(2)),
        "the stale original is rejected while the decoded font remains independent");

cleanup:
    releases_ok &= release_texture(&atlas);
    releases_ok &= release_font(&decoded);
    releases_ok &= release_font(&font);
    releases_ok &= release_document(&document);
    cna_demo_check(&demo, releases_ok, "release every remaining font, texture and document handle");
    return cna_demo_finish(&demo);
}
