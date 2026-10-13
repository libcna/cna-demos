/* SPDX-License-Identifier: MS-PL */
/* Caller-built SpriteFont tables, mutable layout state and retained atlas ownership from C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct SpriteFontState {
    CnaDemo* demo;
    int loads;
} SpriteFontState;

static int bytes_are(const void* data, size_t count, uint8_t expected)
{
    const uint8_t* bytes = (const uint8_t*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static int rectangle_equal(CNA_Rectangle left, CNA_Rectangle right)
{
    return left.x == right.x && left.y == right.y && left.width == right.width &&
           left.height == right.height;
}

static int glyph_equal(const CNA_SpriteFontGlyph* left, const CNA_SpriteFontGlyph* right)
{
    return left->struct_size == (uint32_t)sizeof(CNA_SpriteFontGlyph) &&
           left->struct_version == UINT32_C(1) && left->reserved == 0U &&
           rectangle_equal(left->glyph_bounds, right->glyph_bounds) &&
           rectangle_equal(left->cropping, right->cropping) &&
           left->character == right->character && left->kerning.x == right->kerning.x &&
           left->kerning.y == right->kerning.y && left->kerning.z == right->kerning.z;
}

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    SpriteFontState* state = (SpriteFontState*)context;
    const CNA_SpriteFontGlyph expected[3] = {
        {sizeof(CNA_SpriteFontGlyph), UINT32_C(1), {0, 0, 2, 6}, {1, -1, 3, 7},
         (CNA_Char16)'?', 0U, {-1.0f, 2.0f, -0.5f}},
        {sizeof(CNA_SpriteFontGlyph), UINT32_C(1), {2, 0, 3, 6}, {-2, 1, 4, 8},
         (CNA_Char16)'A', 0U, {1.0f, 3.0f, 2.0f}},
        {sizeof(CNA_SpriteFontGlyph), UINT32_C(1), {5, 0, 3, 6}, {0, 2, 3, 9},
         (CNA_Char16)'B', 0U, {0.5f, 4.0f, -1.0f}}
    };
    CNA_SpriteFontGlyph authored[3];
    CNA_SpriteFontGlyph copied[3];
    CNA_SpriteFontGlyph short_glyphs[3];
    CNA_Char16 characters[3] = {UINT16_C(0xEEEE), UINT16_C(0xEEEE), UINT16_C(0xEEEE)};
    const CNA_Char16 untouched_characters[3] = {
        UINT16_C(0xEEEE), UINT16_C(0xEEEE), UINT16_C(0xEEEE)
    };
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle atlas = CNA_INVALID_HANDLE;
    CNA_Handle font = CNA_INVALID_HANDLE;
    const CNA_Texture2DCreateInfo texture_info = {
        sizeof(CNA_Texture2DCreateInfo), UINT32_C(1), 8U, 8U,
        CNA_FALSE, {0U, 0U, 0U}, CNA_SURFACE_FORMAT_COLOR
    };
    CNA_SpriteFontCreateInfo create_info;
    CNA_SpriteFontInfo info;
    CNA_Vector2 measured = {0.0f, 0.0f};
    uint64_t count = 0U;
    CNA_Result result;
    (void)game_time;
    (void)out_error;

    ++state->loads;
    if (!cna_demo_check_result(state->demo, cna_game_get_graphics_device(game, &device),
                               "get the live graphics device for the font atlas") ||
        !cna_demo_check_result(state->demo, cna_texture2d_create(device, &texture_info, &atlas),
                               "create the 8x8 glyph atlas")) {
        return CNA_RESULT_SUCCESS;
    }

    memcpy(authored, expected, sizeof(authored));
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.texture = atlas;
    create_info.glyphs = authored;
    create_info.glyph_count = 3U;
    create_info.line_spacing = 12;
    create_info.spacing = 1.25f;
    create_info.default_character = (CNA_Char16)'?';
    create_info.has_default_character = CNA_TRUE;
    result = cna_sprite_font_create(&create_info, &font);
    cna_demo_check_result(state->demo, result, "create the SpriteFont from three caller glyphs");
    memset(authored, 0xA5, sizeof(authored));

    if (font != CNA_INVALID_HANDLE) {
        memset(&info, 0xA5, sizeof(info));
        info.struct_size = (uint32_t)sizeof(info);
        info.struct_version = UINT32_C(1);
        cna_demo_check(
            state->demo,
            cna_sprite_font_get_info(font, &info) == CNA_RESULT_SUCCESS &&
                info.struct_size == sizeof(CNA_SpriteFontInfo) &&
                info.struct_version == UINT32_C(1) && info.character_count == 3U &&
                info.line_spacing == 12 && info.spacing == 1.25f &&
                info.has_default_character == CNA_TRUE &&
                info.default_character == (CNA_Char16)'?' &&
                bytes_are(info.reserved, sizeof(info.reserved), 0U),
            "font information reports the authored count, layout, fallback and canonical padding");

        count = 0U;
        cna_demo_check(
            state->demo,
            cna_sprite_font_copy_characters(font, characters, 2U, &count) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                count == 3U && memcmp(characters, untouched_characters, sizeof(characters)) == 0,
            "a short character copy reports its size without partially writing the array");
        cna_demo_check(
            state->demo,
            cna_sprite_font_copy_characters(font, characters, 3U, &count) == CNA_RESULT_SUCCESS &&
                count == 3U && characters[0] == (CNA_Char16)'?' &&
                characters[1] == (CNA_Char16)'A' && characters[2] == (CNA_Char16)'B',
            "the character collection preserves the constructor order");

        memset(short_glyphs, 0x5A, sizeof(short_glyphs));
        memset(copied, 0, sizeof(copied));
        count = 0U;
        cna_demo_check(
            state->demo,
            cna_sprite_font_copy_glyphs(font, short_glyphs, 2U, &count) ==
                    CNA_RESULT_BUFFER_TOO_SMALL &&
                count == 3U && bytes_are(short_glyphs, sizeof(short_glyphs), UINT8_C(0x5A)),
            "a short glyph copy reports its size without partially writing the array");
        result = cna_sprite_font_copy_glyphs(font, copied, 3U, &count);
        cna_demo_check_result(state->demo, result, "copy the complete glyph table back from the font");
        cna_demo_check(
            state->demo,
            result == CNA_RESULT_SUCCESS && count == 3U && glyph_equal(&copied[0], &expected[0]) &&
                glyph_equal(&copied[1], &expected[1]) && glyph_equal(&copied[2], &expected[2]),
            "copied glyph bounds, cropping and kerning remain exact after caller storage changes");
        cna_demo_check(
            state->demo,
            copied[0].character == characters[0] && copied[1].character == characters[1] &&
                copied[2].character == characters[2],
            "glyph and character copies are parallel and use the same order");

        cna_demo_check(
            state->demo,
            cna_sprite_font_measure_utf8(font, (CNA_StringView){"AB\nZ", UINT64_C(4)},
                                         &measured) == CNA_RESULT_SUCCESS &&
                measured.x == 11.75f && measured.y == 24.0f,
            "initial measurement applies asymmetric bearings, spacing, line height and fallback");

        cna_demo_check(
            state->demo,
            cna_sprite_font_set_default_character(font, CNA_TRUE, (CNA_Char16)'A') ==
                    CNA_RESULT_SUCCESS &&
                cna_sprite_font_set_line_spacing(font, 17) == CNA_RESULT_SUCCESS &&
                cna_sprite_font_set_spacing(font, 2.5f) == CNA_RESULT_SUCCESS,
            "the public setters change fallback and both layout properties");
        cna_demo_check(
            state->demo,
            cna_sprite_font_set_default_character(font, CNA_TRUE, (CNA_Char16)'Z') ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_sprite_font_set_spacing(font, NAN) == CNA_RESULT_INVALID_ARGUMENT,
            "missing fallbacks and non-finite spacing are rejected");
        memset(&info, 0, sizeof(info));
        info.struct_size = (uint32_t)sizeof(info);
        info.struct_version = UINT32_C(1);
        cna_demo_check(
            state->demo,
            cna_sprite_font_get_info(font, &info) == CNA_RESULT_SUCCESS &&
                info.line_spacing == 17 && info.spacing == 2.5f &&
                info.has_default_character == CNA_TRUE &&
                info.default_character == (CNA_Char16)'A',
            "rejected setters preserve the last valid mutable state");
        cna_demo_check(
            state->demo,
            cna_sprite_font_measure_utf8(font, (CNA_StringView){"AB\nZ", UINT64_C(4)},
                                         &measured) == CNA_RESULT_SUCCESS &&
                measured.x == 13.0f && measured.y == 34.0f,
            "measurement observes the changed spacing, line height and fallback");

        cna_demo_check(
            state->demo,
            cna_sprite_font_set_default_character(font, CNA_FALSE, (CNA_Char16)'Z') ==
                    CNA_RESULT_SUCCESS,
            "clearing the fallback ignores the value argument");
        memset(&info, 0, sizeof(info));
        info.struct_size = (uint32_t)sizeof(info);
        info.struct_version = UINT32_C(1);
        cna_demo_check(
            state->demo,
            cna_sprite_font_get_info(font, &info) == CNA_RESULT_SUCCESS &&
                info.has_default_character == CNA_FALSE && info.default_character == 0U &&
                cna_sprite_font_measure_utf8(font, (CNA_StringView){"Z", UINT64_C(1)},
                                             &measured) == CNA_RESULT_INVALID_ARGUMENT,
            "cleared fallback is canonical and leaves a missing glyph unresolved");

        cna_demo_check(
            state->demo,
            cna_texture2d_destroy(atlas) == CNA_RESULT_INVALID_STATE,
            "the font retains its source atlas until the font is destroyed");
        cna_demo_check_result(state->demo, cna_sprite_font_destroy(font),
                              "destroy the caller-built SpriteFont");
        font = CNA_INVALID_HANDLE;
    }

    if (font != CNA_INVALID_HANDLE) {
        (void)cna_sprite_font_destroy(font);
    }
    if (atlas != CNA_INVALID_HANDLE) {
        cna_demo_check_result(state->demo, cna_texture2d_destroy(atlas),
                              "destroy the atlas after releasing the font");
    }
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    SpriteFontState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;

    cna_demo_init(&demo, "c_api.graphics.sprite_font_state", argc, argv, 1);
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
                              "run the SpriteFont state callback");
    }
    cna_demo_check(&demo, state.loads == 1, "the SpriteFont callback ran exactly once");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager),
                              "cna_graphics_device_manager_destroy");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
