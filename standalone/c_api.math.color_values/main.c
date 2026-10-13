/* SPDX-License-Identifier: MS-PL */
/* CNA_Color construction, packing, formatting and arithmetic through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int color_is(CNA_Color value, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return value.r == r && value.g == g && value.b == b && value.a == a;
}

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.00001f;
}

int main(int argc, char** argv)
{
    static const char DebugText[] = "1 2 3 4";
    static const char CanonicalText[] = "{R:1 G:2 B:3 A:4}";
    CnaDemo demo;
    CNA_Color value = {9U, 8U, 7U, 6U};
    CNA_Color result = value;
    CNA_Vector3 vector3 = {9.0f, 8.0f, 7.0f};
    CNA_Vector4 vector4 = {9.0f, 8.0f, 7.0f, 6.0f};
    CNA_Bool predicate = UINT8_C(77);
    uint32_t packed = 0U;
    int32_t hash = 0;
    uint64_t byte_count = 0U;
    char text[32];
    char before[32];

    cna_demo_init(&demo, "c_api.math.color_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        sizeof(CNA_Color) == 4U &&
            cna_color_init_vector4((CNA_Vector4){0.0f, 0.5f, 1.0f, 2.0f}, &value) ==
                CNA_RESULT_SUCCESS &&
            color_is(value, 0U, 128U, 255U, 255U) &&
            cna_color_init_vector3((CNA_Vector3){1.0f, -1.0f, 0.25f}, &value) ==
                CNA_RESULT_SUCCESS &&
            color_is(value, 255U, 0U, 64U, 255U),
        "vector constructors clamp unit channels and use XNA nearest-byte rounding");
    cna_demo_check(
        &demo,
        cna_color_init_float_rgb(0.1f, 0.2f, 0.3f, &value) == CNA_RESULT_SUCCESS &&
            color_is(value, 26U, 51U, 76U, 255U) &&
            cna_color_init_float_rgba(0.1f, 0.2f, 0.3f, 0.4f, &value) ==
                CNA_RESULT_SUCCESS &&
            color_is(value, 26U, 51U, 76U, 102U),
        "scalar floating-point constructors distinguish implicit and explicit alpha");
    cna_demo_check(
        &demo,
        cna_color_init_int_rgb(-1, 128, 300, &value) == CNA_RESULT_SUCCESS &&
            color_is(value, 0U, 128U, 255U, 255U) &&
            cna_color_init_int_rgba(-1, 128, 300, 64, &value) == CNA_RESULT_SUCCESS &&
            color_is(value, 0U, 128U, 255U, 64U),
        "integer constructors clamp all channels and supply opaque RGB alpha");
    cna_demo_check(
        &demo,
        cna_color_init_bytes_rgb(1U, 2U, 3U, &value) == CNA_RESULT_SUCCESS &&
            color_is(value, 1U, 2U, 3U, 255U) &&
            cna_color_init_bytes_rgba(1U, 2U, 3U, 4U, &value) == CNA_RESULT_SUCCESS &&
            color_is(value, 1U, 2U, 3U, 4U),
        "byte constructors preserve exact channels and the RGB form supplies opaque alpha");

    cna_demo_check(
        &demo,
        cna_color_get_packed_value(value, &packed) == CNA_RESULT_SUCCESS &&
            packed == UINT32_C(0x04030201) &&
            cna_color_set_packed_value(&value, UINT32_C(0xa1b2c3d4)) == CNA_RESULT_SUCCESS &&
            color_is(value, 0xd4U, 0xc3U, 0xb2U, 0xa1U),
        "packed values use the documented AABBGGRR bit order in both directions");
    cna_demo_check(
        &demo,
        cna_color_get_packed_value(CNA_COLOR_CORNFLOWER_BLUE, &packed) == CNA_RESULT_SUCCESS &&
            packed == UINT32_C(0xffed9564) &&
            cna_color_equals(CNA_COLOR_AQUA, CNA_COLOR_CYAN, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE,
        "selected named constants expose canonical packed bytes and color aliases");

    value = (CNA_Color){64U, 128U, 255U, 32U};
    predicate = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_color_equals(value, (CNA_Color){64U, 128U, 255U, 32U}, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_color_not_equals(value, (CNA_Color){64U, 128U, 254U, 32U}, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_color_get_hash_code(value, &hash) == CNA_RESULT_SUCCESS &&
            hash == (int32_t)UINT32_C(0x20ff8040),
        "equality, inequality and hashing observe the same four packed channels");
    cna_demo_check(
        &demo,
        cna_color_to_vector3(value, &vector3) == CNA_RESULT_SUCCESS &&
            near(vector3.x, 64.0f / 255.0f) && near(vector3.y, 128.0f / 255.0f) &&
            near(vector3.z, 1.0f) &&
            cna_color_to_vector4(value, &vector4) == CNA_RESULT_SUCCESS &&
            near(vector4.x, vector3.x) && near(vector4.y, vector3.y) &&
            near(vector4.z, vector3.z) && near(vector4.w, 32.0f / 255.0f),
        "vector conversion normalizes RGB and only the Vector4 form exposes alpha");

    value = (CNA_Color){1U, 2U, 3U, 4U};
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_color_get_debug_string_byte_count(value, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == sizeof(DebugText) - 1U &&
            cna_color_copy_debug_string(value, text, byte_count, &byte_count) ==
                CNA_RESULT_SUCCESS &&
            byte_count == sizeof(DebugText) - 1U &&
            memcmp(text, DebugText, sizeof(DebugText) - 1U) == 0 &&
            (unsigned char)text[sizeof(DebugText) - 1U] == 0xA5U,
        "debug text uses exact unterminated channel bytes");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_color_get_string_byte_count(value, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == sizeof(CanonicalText) - 1U &&
            cna_color_copy_string(value, text, byte_count, &byte_count) == CNA_RESULT_SUCCESS &&
            byte_count == sizeof(CanonicalText) - 1U &&
            memcmp(text, CanonicalText, sizeof(CanonicalText) - 1U) == 0 &&
            (unsigned char)text[sizeof(CanonicalText) - 1U] == 0xA5U,
        "canonical text uses exact unterminated labeled-channel bytes");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    byte_count = 0U;
    cna_demo_check(
        &demo,
        cna_color_copy_string(value, text, sizeof(CanonicalText) - 2U, &byte_count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            byte_count == sizeof(CanonicalText) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_color_copy_debug_string(value, NULL, 0U, &byte_count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            byte_count == sizeof(DebugText) - 1U,
        "short and zero-capacity text requests report required sizes without partial writes");

    cna_demo_check(
        &demo,
        cna_color_lerp(CNA_COLOR_BLACK, CNA_COLOR_WHITE, 0.5f, &result) ==
                CNA_RESULT_SUCCESS &&
            color_is(result, 127U, 127U, 127U, 255U) &&
            cna_color_lerp(CNA_COLOR_BLACK, CNA_COLOR_WHITE, 2.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            color_is(result, 255U, 255U, 255U, 255U) &&
            cna_color_lerp(CNA_COLOR_BLACK, CNA_COLOR_WHITE, -1.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            color_is(result, 0U, 0U, 0U, 255U),
        "lerp truncates the midpoint and clamps amounts outside zero to one");
    value = (CNA_Color){64U, 128U, 255U, 32U};
    cna_demo_check(
        &demo,
        cna_color_multiply(value, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            color_is(result, 32U, 64U, 127U, 16U) &&
            cna_color_multiply(value, -1.0f, &result) == CNA_RESULT_SUCCESS &&
            color_is(result, 0U, 0U, 0U, 0U),
        "scalar multiplication truncates fractions and clamps negative channels to zero");
    cna_demo_check(
        &demo,
        cna_color_from_non_premultiplied_vector4(
            (CNA_Vector4){0.8f, 0.4f, 0.2f, 0.5f}, &result) == CNA_RESULT_SUCCESS &&
            color_is(result, 102U, 51U, 26U, 128U) &&
            cna_color_from_non_premultiplied_int(255, 128, 64, 128, &result) ==
                CNA_RESULT_SUCCESS &&
            color_is(result, 128U, 64U, 32U, 128U),
        "both premultiplication families scale RGB by alpha with their documented rounding");
    cna_demo_check(
        &demo,
        cna_color_pack_from_vector4(
            &result, (CNA_Vector4){2.0f, -1.0f, NAN, INFINITY}) == CNA_RESULT_SUCCESS &&
            color_is(result, 255U, 0U, 0U, 255U),
        "native vector packing saturates infinities and treats NaN as zero");

    return cna_demo_finish(&demo);
}
