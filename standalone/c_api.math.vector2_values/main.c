/* SPDX-License-Identifier: MS-PL */
/* Complete Vector2 value and transform behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
}

static int vector_is(CNA_Vector2 value, float x, float y)
{
    return near(value.x, x) && near(value.y, y);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{X:3 Y:4}";
    const CNA_Vector2 a = {2.0f, 4.0f};
    const CNA_Vector2 b = {1.0f, -2.0f};
    const CNA_Matrix translation = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        10.0f, 20.0f, 0.0f, 1.0f
    };
    const float half_sqrt = 0.7071067811865475f;
    const CNA_Quaternion quarter_turn = {0.0f, 0.0f, half_sqrt, half_sqrt};
    const CNA_Vector2 source[4] = {
        {-9.0f, -8.0f}, {1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}
    };
    CnaDemo demo;
    CNA_Vector2 zero = {9.0f, 8.0f};
    CNA_Vector2 one = zero;
    CNA_Vector2 unit_x = zero;
    CNA_Vector2 unit_y = zero;
    CNA_Vector2 value = zero;
    CNA_Vector2 result = zero;
    CNA_Vector2 destination[4];
    CNA_Vector2 before_destination[4];
    CNA_Bool predicate = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float scalar = 0.0f;
    uint64_t text_bytes = 0U;
    char text[32];
    char before_text[32];

    cna_demo_init(&demo, "c_api.math.vector2_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_vector2_init(&zero) == CNA_RESULT_SUCCESS && vector_is(zero, 0.0f, 0.0f) &&
            cna_vector2_init_xy(3.0f, 4.0f, &value) == CNA_RESULT_SUCCESS &&
            vector_is(value, 3.0f, 4.0f) &&
            cna_vector2_init_scalar(2.0f, &one) == CNA_RESULT_SUCCESS &&
            vector_is(one, 2.0f, 2.0f),
        "default, component and replicated-scalar constructors produce exact values");
    cna_demo_check(
        &demo,
        cna_vector2_get_zero(&zero) == CNA_RESULT_SUCCESS && vector_is(zero, 0.0f, 0.0f) &&
            cna_vector2_get_one(&one) == CNA_RESULT_SUCCESS && vector_is(one, 1.0f, 1.0f) &&
            cna_vector2_get_unit_x(&unit_x) == CNA_RESULT_SUCCESS &&
            vector_is(unit_x, 1.0f, 0.0f) &&
            cna_vector2_get_unit_y(&unit_y) == CNA_RESULT_SUCCESS &&
            vector_is(unit_y, 0.0f, 1.0f),
        "Zero, One, UnitX and UnitY form the canonical Vector2 basis values");
    cna_demo_check(
        &demo,
        cna_vector2_equals(value, (CNA_Vector2){3.0f, 4.0f}, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_vector2_not_equals(value, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_vector2_get_hash_code(value, &hash) == CNA_RESULT_SUCCESS &&
            cna_vector2_get_hash_code((CNA_Vector2){3.0f, 4.0f}, &equal_hash) ==
                CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "component equality, inequality and hashing agree for equal vectors");
    cna_demo_check(
        &demo,
        cna_vector2_length(value, &scalar) == CNA_RESULT_SUCCESS && near(scalar, 5.0f) &&
            cna_vector2_length_squared(value, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 25.0f) &&
            cna_vector2_normalize(value, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.6f, 0.8f) &&
            cna_vector2_normalize_in_place(&value) == CNA_RESULT_SUCCESS &&
            vector_is(value, 0.6f, 0.8f),
        "length and both normalization forms agree for the 3-4-5 vector");

    value = (CNA_Vector2){3.0f, 4.0f};
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_vector2_get_string_size(value, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_vector2_copy_string(value, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical Vector2 text is copied exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_vector2_copy_string(value, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short Vector2 text destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_vector2_add(a, b, &result) == CNA_RESULT_SUCCESS && vector_is(result, 3.0f, 2.0f) &&
            cna_vector2_subtract(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 6.0f) &&
            cna_vector2_multiply(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, -8.0f) &&
            cna_vector2_divide(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, -2.0f) &&
            cna_vector2_negate(a, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, -2.0f, -4.0f),
        "component add, subtract, multiply, divide and negate retain both axes");
    cna_demo_check(
        &demo,
        cna_vector2_multiply_scalar(a, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f) &&
            cna_vector2_divide_scalar(a, 2.0f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f) &&
            cna_vector2_divide_scalar(a, 0.0f, &result) == CNA_RESULT_SUCCESS &&
            isinf(result.x) && isinf(result.y),
        "scalar multiply/divide are reciprocal and zero division exposes IEEE infinities");
    cna_demo_check(
        &demo,
        cna_vector2_dot(a, b, &scalar) == CNA_RESULT_SUCCESS && scalar == -6.0f &&
            cna_vector2_distance(a, b, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, sqrtf(37.0f)) &&
            cna_vector2_distance_squared(a, b, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 37.0f,
        "dot product and distance metrics observe both component differences");
    cna_demo_check(
        &demo,
        cna_vector2_barycentric(
            (CNA_Vector2){0.0f, 0.0f}, (CNA_Vector2){2.0f, 4.0f},
            (CNA_Vector2){4.0f, 8.0f}, 0.25f, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.5f, 5.0f) &&
            cna_vector2_catmull_rom(
                (CNA_Vector2){0.0f, 0.0f}, (CNA_Vector2){1.0f, 2.0f},
                (CNA_Vector2){2.0f, 4.0f}, (CNA_Vector2){3.0f, 6.0f}, 0.5f, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 3.0f) &&
            cna_vector2_hermite(
                (CNA_Vector2){0.0f, 0.0f}, zero, (CNA_Vector2){10.0f, 20.0f}, zero,
                0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 5.0f, 10.0f),
        "barycentric, Catmull-Rom and flat-tangent Hermite interpolate each component");
    cna_demo_check(
        &demo,
        cna_vector2_lerp(a, b, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 1.0f) &&
            cna_vector2_smooth_step(a, b, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 1.0f) &&
            cna_vector2_clamp(
                (CNA_Vector2){-2.0f, 9.0f}, (CNA_Vector2){0.0f, 1.0f},
                (CNA_Vector2){3.0f, 5.0f}, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 5.0f) &&
            cna_vector2_max(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, 4.0f) &&
            cna_vector2_min(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, -2.0f),
        "linear/eased midpoint and component-wise bound selection are independent operations");
    cna_demo_check(
        &demo,
        cna_vector2_reflect(
            (CNA_Vector2){1.0f, -1.0f}, (CNA_Vector2){0.0f, 1.0f}, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 1.0f),
        "reflection across an upward normal reverses only the incident Y component");

    cna_demo_check(
        &demo,
        cna_vector2_transform_matrix((CNA_Vector2){1.0f, 2.0f}, translation, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 11.0f, 22.0f) &&
            cna_vector2_transform_normal((CNA_Vector2){1.0f, 2.0f}, translation, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f) &&
            cna_vector2_transform_quaternion(
                (CNA_Vector2){1.0f, 0.0f}, quarter_turn, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 1.0f),
        "matrix points include translation, normals omit it and a quaternion rotates in XY");

    destination[0] = (CNA_Vector2){-1.0f, -1.0f};
    destination[1] = destination[0];
    destination[2] = destination[0];
    destination[3] = (CNA_Vector2){-2.0f, -2.0f};
    cna_demo_check(
        &demo,
        cna_vector2_transform_matrix_array(
            source, 3U, 1U, translation, destination, 3U, 0U, 2U) == CNA_RESULT_SUCCESS &&
            vector_is(destination[0], 11.0f, 22.0f) &&
            vector_is(destination[1], 13.0f, 24.0f) &&
            vector_is(destination[2], -1.0f, -1.0f) &&
            vector_is(destination[3], -2.0f, -2.0f) &&
            cna_vector2_transform_quaternion_array(
                source, 4U, 1U, (CNA_Quaternion){0.0f, 0.0f, 0.0f, 1.0f},
                destination, 3U, 0U, 3U) == CNA_RESULT_SUCCESS &&
            vector_is(destination[2], 5.0f, 6.0f) &&
            cna_vector2_transform_normal_array(
                source, 4U, 1U, translation, destination, 3U, 0U, 3U) ==
                CNA_RESULT_SUCCESS &&
            vector_is(destination[0], 1.0f, 2.0f) &&
            vector_is(destination[2], 5.0f, 6.0f),
        "bounded matrix, quaternion and normal arrays transform only selected windows");
    memcpy(before_destination, destination, sizeof(destination));
    cna_demo_check(
        &demo,
        cna_vector2_transform_matrix_array(
            source, 3U, 2U, translation, destination, 3U, 0U, 2U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(destination, before_destination, sizeof(destination)) == 0 &&
            cna_vector2_transform_matrix_array(
                NULL, 0U, 0U, translation, NULL, 0U, 0U, 0U) == CNA_RESULT_SUCCESS,
        "an invalid array window is atomic while the canonical empty null transform succeeds");

    return cna_demo_finish(&demo);
}
