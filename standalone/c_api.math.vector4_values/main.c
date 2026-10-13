/* SPDX-License-Identifier: MS-PL */
/* Complete Vector4 value and homogeneous transform behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
}

static int vector_is(CNA_Vector4 value, float x, float y, float z, float w)
{
    return near(value.x, x) && near(value.y, y) && near(value.z, z) && near(value.w, w);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{X:1 Y:2 Z:2 W:4}";
    const CNA_Vector4 a = {2.0f, 4.0f, 6.0f, 8.0f};
    const CNA_Vector4 b = {1.0f, -2.0f, 3.0f, 4.0f};
    const CNA_Vector4 zero_value = {0.0f, 0.0f, 0.0f, 0.0f};
    const CNA_Matrix translation = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        10.0f, 20.0f, 30.0f, 1.0f
    };
    const float half_sqrt = 0.7071067811865475f;
    const CNA_Quaternion quarter_turn = {0.0f, 0.0f, half_sqrt, half_sqrt};
    const CNA_Vector4 source[4] = {
        {-9.0f, -8.0f, -7.0f, 1.0f}, {1.0f, 2.0f, 3.0f, 1.0f},
        {3.0f, 4.0f, 5.0f, 1.0f}, {5.0f, 6.0f, 7.0f, 1.0f}
    };
    CnaDemo demo;
    CNA_Vector4 zero = {9.0f, 8.0f, 7.0f, 6.0f};
    CNA_Vector4 one = zero;
    CNA_Vector4 unit = zero;
    CNA_Vector4 value = zero;
    CNA_Vector4 result = zero;
    CNA_Vector4 destination[4];
    CNA_Vector4 before_destination[4];
    CNA_Bool predicate = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float scalar = 0.0f;
    uint64_t text_bytes = 0U;
    char text[32];
    char before_text[32];

    cna_demo_init(&demo, "c_api.math.vector4_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_vector4_init(&zero) == CNA_RESULT_SUCCESS &&
            vector_is(zero, 0.0f, 0.0f, 0.0f, 0.0f) &&
            cna_vector4_init_xyzw(1.0f, 2.0f, 2.0f, 4.0f, &value) ==
                CNA_RESULT_SUCCESS &&
            vector_is(value, 1.0f, 2.0f, 2.0f, 4.0f) &&
            cna_vector4_init_scalar(2.0f, &one) == CNA_RESULT_SUCCESS &&
            vector_is(one, 2.0f, 2.0f, 2.0f, 2.0f),
        "default, component and replicated-scalar constructors produce exact values");
    cna_demo_check(
        &demo,
        cna_vector4_init_vector2_zw((CNA_Vector2){1.0f, 2.0f}, 3.0f, 4.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f, 4.0f) &&
            cna_vector4_init_vector3_w((CNA_Vector3){1.0f, 2.0f, 3.0f}, 4.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f, 4.0f),
        "Vector2-plus-ZW and Vector3-plus-W constructors agree on the same value");
    cna_demo_check(
        &demo,
        cna_vector4_get_zero(&zero) == CNA_RESULT_SUCCESS &&
            vector_is(zero, 0.0f, 0.0f, 0.0f, 0.0f) &&
            cna_vector4_get_one(&one) == CNA_RESULT_SUCCESS &&
            vector_is(one, 1.0f, 1.0f, 1.0f, 1.0f) &&
            cna_vector4_get_unit_x(&unit) == CNA_RESULT_SUCCESS &&
            vector_is(unit, 1.0f, 0.0f, 0.0f, 0.0f) &&
            cna_vector4_get_unit_y(&unit) == CNA_RESULT_SUCCESS &&
            vector_is(unit, 0.0f, 1.0f, 0.0f, 0.0f) &&
            cna_vector4_get_unit_z(&unit) == CNA_RESULT_SUCCESS &&
            vector_is(unit, 0.0f, 0.0f, 1.0f, 0.0f) &&
            cna_vector4_get_unit_w(&unit) == CNA_RESULT_SUCCESS &&
            vector_is(unit, 0.0f, 0.0f, 0.0f, 1.0f),
        "Zero, One and four unit vectors form the canonical Vector4 basis");
    cna_demo_check(
        &demo,
        cna_vector4_equals(value, (CNA_Vector4){1.0f, 2.0f, 2.0f, 4.0f}, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_vector4_not_equals(value, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_vector4_get_hash_code(value, &hash) == CNA_RESULT_SUCCESS &&
            cna_vector4_get_hash_code((CNA_Vector4){1.0f, 2.0f, 2.0f, 4.0f}, &equal_hash) ==
                CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "component equality, inequality and hashing agree for equal vectors");
    cna_demo_check(
        &demo,
        cna_vector4_length(value, &scalar) == CNA_RESULT_SUCCESS && near(scalar, 5.0f) &&
            cna_vector4_length_squared(value, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 25.0f) &&
            cna_vector4_normalize(value, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.2f, 0.4f, 0.4f, 0.8f) &&
            cna_vector4_normalize_in_place(&value) == CNA_RESULT_SUCCESS &&
            vector_is(value, 0.2f, 0.4f, 0.4f, 0.8f),
        "length and both normalization forms agree for the authored five-unit vector");
    cna_demo_check(
        &demo,
        cna_vector4_normalize(zero_value, &result) == CNA_RESULT_SUCCESS &&
            isnan(result.x) && isnan(result.y) && isnan(result.z) && isnan(result.w),
        "normalizing the zero Vector4 preserves canonical IEEE undefined components");

    value = (CNA_Vector4){1.0f, 2.0f, 2.0f, 4.0f};
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_vector4_get_string_size(value, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_vector4_copy_string(value, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical Vector4 text is copied exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_vector4_copy_string(value, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short Vector4 text destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_vector4_add(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 3.0f, 2.0f, 9.0f, 12.0f) &&
            cna_vector4_subtract(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 6.0f, 3.0f, 4.0f) &&
            cna_vector4_multiply(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, -8.0f, 18.0f, 32.0f) &&
            cna_vector4_divide(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, -2.0f, 2.0f, 2.0f) &&
            cna_vector4_negate(a, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, -2.0f, -4.0f, -6.0f, -8.0f),
        "component add, subtract, multiply, divide and negate retain all four axes");
    cna_demo_check(
        &demo,
        cna_vector4_multiply_scalar(a, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f, 4.0f) &&
            cna_vector4_divide_scalar(a, 2.0f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f, 4.0f) &&
            cna_vector4_divide_scalar(a, 0.0f, &result) == CNA_RESULT_SUCCESS &&
            isinf(result.x) && isinf(result.y) && isinf(result.z) && isinf(result.w),
        "scalar multiply/divide are reciprocal and zero division exposes IEEE infinities");
    cna_demo_check(
        &demo,
        cna_vector4_dot(a, b, &scalar) == CNA_RESULT_SUCCESS && scalar == 44.0f &&
            cna_vector4_distance(a, b, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, sqrtf(62.0f)) &&
            cna_vector4_distance_squared(a, b, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 62.0f,
        "dot product and distance metrics observe all four component differences");
    cna_demo_check(
        &demo,
        cna_vector4_barycentric(
            zero_value, (CNA_Vector4){2.0f, 4.0f, 6.0f, 8.0f},
            (CNA_Vector4){4.0f, 8.0f, 12.0f, 16.0f}, 0.25f, 0.5f, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 2.5f, 5.0f, 7.5f, 10.0f) &&
            cna_vector4_catmull_rom(
                zero_value, (CNA_Vector4){1.0f, 2.0f, 3.0f, 4.0f},
                (CNA_Vector4){2.0f, 4.0f, 6.0f, 8.0f},
                (CNA_Vector4){3.0f, 6.0f, 9.0f, 12.0f}, 0.5f, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 3.0f, 4.5f, 6.0f) &&
            cna_vector4_hermite(
                zero_value, zero_value, (CNA_Vector4){10.0f, 20.0f, 30.0f, 40.0f},
                zero_value, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 5.0f, 10.0f, 15.0f, 20.0f),
        "barycentric, Catmull-Rom and flat-tangent Hermite interpolate every component");
    cna_demo_check(
        &demo,
        cna_vector4_lerp(a, b, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 1.0f, 4.5f, 6.0f) &&
            cna_vector4_smooth_step(a, b, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 1.0f, 4.5f, 6.0f) &&
            cna_vector4_clamp(
                (CNA_Vector4){-2.0f, 9.0f, 4.0f, 8.0f},
                (CNA_Vector4){0.0f, 1.0f, 2.0f, 3.0f},
                (CNA_Vector4){3.0f, 5.0f, 3.0f, 6.0f}, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 5.0f, 3.0f, 6.0f) &&
            cna_vector4_max(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, 4.0f, 6.0f, 8.0f) &&
            cna_vector4_min(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, -2.0f, 3.0f, 4.0f),
        "linear/eased midpoint and component-wise bound selection are independent operations");

    cna_demo_check(
        &demo,
        cna_vector4_transform_vector2_matrix(
            (CNA_Vector2){1.0f, 2.0f}, translation, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 11.0f, 22.0f, 30.0f, 1.0f) &&
            cna_vector4_transform_vector3_matrix(
                (CNA_Vector3){1.0f, 2.0f, 3.0f}, translation, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 11.0f, 22.0f, 33.0f, 1.0f) &&
            cna_vector4_transform_matrix(
                (CNA_Vector4){1.0f, 2.0f, 3.0f, 2.0f}, translation, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 21.0f, 42.0f, 63.0f, 2.0f),
        "matrix transforms embed Vector2/3 with W one while Vector4 scales translation by W");
    cna_demo_check(
        &demo,
        cna_vector4_transform_vector2_quaternion(
            (CNA_Vector2){1.0f, 0.0f}, quarter_turn, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 1.0f, 0.0f, 1.0f) &&
            cna_vector4_transform_vector3_quaternion(
                (CNA_Vector3){1.0f, 0.0f, 3.0f}, quarter_turn, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 1.0f, 3.0f, 1.0f) &&
            cna_vector4_transform_quaternion(
                (CNA_Vector4){1.0f, 0.0f, 3.0f, 4.0f}, quarter_turn, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 1.0f, 3.0f, 4.0f),
        "quaternion transforms embed Vector2/3 with W one and preserve an authored Vector4 W");

    destination[0] = (CNA_Vector4){-1.0f, -1.0f, -1.0f, -1.0f};
    destination[1] = destination[0];
    destination[2] = destination[0];
    destination[3] = (CNA_Vector4){-2.0f, -2.0f, -2.0f, -2.0f};
    cna_demo_check(
        &demo,
        cna_vector4_transform_matrix_array(
            source, 3U, 1U, translation, destination, 3U, 0U, 2U) == CNA_RESULT_SUCCESS &&
            vector_is(destination[0], 11.0f, 22.0f, 33.0f, 1.0f) &&
            vector_is(destination[1], 13.0f, 24.0f, 35.0f, 1.0f) &&
            vector_is(destination[2], -1.0f, -1.0f, -1.0f, -1.0f) &&
            vector_is(destination[3], -2.0f, -2.0f, -2.0f, -2.0f) &&
            cna_vector4_transform_quaternion_array(
                source, 4U, 1U, (CNA_Quaternion){0.0f, 0.0f, 0.0f, 1.0f},
                destination, 3U, 0U, 3U) == CNA_RESULT_SUCCESS &&
            vector_is(destination[2], 5.0f, 6.0f, 7.0f, 1.0f),
        "bounded matrix and quaternion arrays transform only their selected windows");
    memcpy(before_destination, destination, sizeof(destination));
    cna_demo_check(
        &demo,
        cna_vector4_transform_matrix_array(
            source, 3U, 2U, translation, destination, 3U, 0U, 2U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(destination, before_destination, sizeof(destination)) == 0 &&
            cna_vector4_transform_matrix_array(
                NULL, 0U, 0U, translation, NULL, 0U, 0U, 0U) == CNA_RESULT_SUCCESS,
        "an invalid array window is atomic while the canonical empty null transform succeeds");

    return cna_demo_finish(&demo);
}
