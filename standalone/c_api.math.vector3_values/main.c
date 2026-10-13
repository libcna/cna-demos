/* SPDX-License-Identifier: MS-PL */
/* Complete Vector3 value and transform behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
}

static int vector_is(CNA_Vector3 value, float x, float y, float z)
{
    return near(value.x, x) && near(value.y, y) && near(value.z, z);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{X:2 Y:3 Z:6}";
    const CNA_Vector3 a = {2.0f, 4.0f, 6.0f};
    const CNA_Vector3 b = {1.0f, -2.0f, 3.0f};
    const CNA_Vector3 zero_value = {0.0f, 0.0f, 0.0f};
    const CNA_Matrix translation = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        10.0f, 20.0f, 30.0f, 1.0f
    };
    const float half_sqrt = 0.7071067811865475f;
    const CNA_Quaternion quarter_turn = {0.0f, 0.0f, half_sqrt, half_sqrt};
    const CNA_Vector3 source[4] = {
        {-9.0f, -8.0f, -7.0f}, {1.0f, 2.0f, 3.0f},
        {3.0f, 4.0f, 5.0f}, {5.0f, 6.0f, 7.0f}
    };
    CnaDemo demo;
    CNA_Vector3 zero = {9.0f, 8.0f, 7.0f};
    CNA_Vector3 one = zero;
    CNA_Vector3 direction = zero;
    CNA_Vector3 value = zero;
    CNA_Vector3 result = zero;
    CNA_Vector3 destination[4];
    CNA_Vector3 before_destination[4];
    CNA_Bool predicate = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float scalar = 0.0f;
    uint64_t text_bytes = 0U;
    char text[32];
    char before_text[32];

    cna_demo_init(&demo, "c_api.math.vector3_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_vector3_init(&zero) == CNA_RESULT_SUCCESS && vector_is(zero, 0.0f, 0.0f, 0.0f) &&
            cna_vector3_init_xyz(2.0f, 3.0f, 6.0f, &value) == CNA_RESULT_SUCCESS &&
            vector_is(value, 2.0f, 3.0f, 6.0f) &&
            cna_vector3_init_scalar(2.0f, &one) == CNA_RESULT_SUCCESS &&
            vector_is(one, 2.0f, 2.0f, 2.0f) &&
            cna_vector3_init_vector2_z((CNA_Vector2){4.0f, 5.0f}, 6.0f, &one) ==
                CNA_RESULT_SUCCESS &&
            vector_is(one, 4.0f, 5.0f, 6.0f),
        "default, component, scalar and Vector2-plus-Z constructors preserve their inputs");
    cna_demo_check(
        &demo,
        cna_vector3_get_zero(&zero) == CNA_RESULT_SUCCESS && vector_is(zero, 0.0f, 0.0f, 0.0f) &&
            cna_vector3_get_one(&one) == CNA_RESULT_SUCCESS && vector_is(one, 1.0f, 1.0f, 1.0f) &&
            cna_vector3_get_unit_x(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 1.0f, 0.0f, 0.0f) &&
            cna_vector3_get_unit_y(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 0.0f, 1.0f, 0.0f) &&
            cna_vector3_get_unit_z(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 0.0f, 0.0f, 1.0f),
        "Zero, One and the three unit vectors form the canonical Vector3 basis");
    cna_demo_check(
        &demo,
        cna_vector3_get_up(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 0.0f, 1.0f, 0.0f) &&
            cna_vector3_get_down(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 0.0f, -1.0f, 0.0f) &&
            cna_vector3_get_right(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 1.0f, 0.0f, 0.0f) &&
            cna_vector3_get_left(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, -1.0f, 0.0f, 0.0f) &&
            cna_vector3_get_forward(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 0.0f, 0.0f, -1.0f) &&
            cna_vector3_get_backward(&direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 0.0f, 0.0f, 1.0f),
        "named directions retain XNA's right-handed axes and negative-Z Forward");
    cna_demo_check(
        &demo,
        cna_vector3_equals(value, (CNA_Vector3){2.0f, 3.0f, 6.0f}, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_vector3_not_equals(value, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_vector3_get_hash_code(value, &hash) == CNA_RESULT_SUCCESS &&
            cna_vector3_get_hash_code((CNA_Vector3){2.0f, 3.0f, 6.0f}, &equal_hash) ==
                CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "component equality, inequality and hashing agree for equal vectors");
    cna_demo_check(
        &demo,
        cna_vector3_length(value, &scalar) == CNA_RESULT_SUCCESS && near(scalar, 7.0f) &&
            cna_vector3_length_squared(value, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 49.0f) &&
            cna_vector3_normalize(value, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f / 7.0f, 3.0f / 7.0f, 6.0f / 7.0f) &&
            cna_vector3_normalize_in_place(&value) == CNA_RESULT_SUCCESS &&
            vector_is(value, 2.0f / 7.0f, 3.0f / 7.0f, 6.0f / 7.0f),
        "length and both normalization forms agree for the authored seven-unit vector");
    cna_demo_check(
        &demo,
        cna_vector3_normalize(zero_value, &result) == CNA_RESULT_SUCCESS &&
            isnan(result.x) && isnan(result.y) && isnan(result.z),
        "normalizing the zero Vector3 preserves canonical IEEE undefined components");

    value = (CNA_Vector3){2.0f, 3.0f, 6.0f};
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_vector3_get_string_size(value, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_vector3_copy_string(value, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical Vector3 text is copied exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_vector3_copy_string(value, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short Vector3 text destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_vector3_add(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 3.0f, 2.0f, 9.0f) &&
            cna_vector3_subtract(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 6.0f, 3.0f) &&
            cna_vector3_multiply(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, -8.0f, 18.0f) &&
            cna_vector3_divide(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, -2.0f, 2.0f) &&
            cna_vector3_negate(a, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, -2.0f, -4.0f, -6.0f),
        "component add, subtract, multiply, divide and negate retain all three axes");
    cna_demo_check(
        &demo,
        cna_vector3_multiply_scalar(a, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f) &&
            cna_vector3_divide_scalar(a, 2.0f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f) &&
            cna_vector3_divide_scalar(a, 0.0f, &result) == CNA_RESULT_SUCCESS &&
            isinf(result.x) && isinf(result.y) && isinf(result.z),
        "scalar multiply/divide are reciprocal and zero division exposes IEEE infinities");
    cna_demo_check(
        &demo,
        cna_vector3_dot(a, b, &scalar) == CNA_RESULT_SUCCESS && scalar == 12.0f &&
            cna_vector3_cross(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 24.0f, 0.0f, -8.0f) &&
            cna_vector3_distance(a, b, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, sqrtf(46.0f)) &&
            cna_vector3_distance_squared(a, b, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == 46.0f,
        "dot, cross and distance metrics distinguish scalar and perpendicular results");
    cna_demo_check(
        &demo,
        cna_vector3_barycentric(
            zero_value, (CNA_Vector3){2.0f, 4.0f, 6.0f},
            (CNA_Vector3){4.0f, 8.0f, 12.0f}, 0.25f, 0.5f, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 2.5f, 5.0f, 7.5f) &&
            cna_vector3_catmull_rom(
                zero_value, (CNA_Vector3){1.0f, 2.0f, 3.0f},
                (CNA_Vector3){2.0f, 4.0f, 6.0f}, (CNA_Vector3){3.0f, 6.0f, 9.0f},
                0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 3.0f, 4.5f) &&
            cna_vector3_hermite(
                zero_value, zero_value, (CNA_Vector3){10.0f, 20.0f, 30.0f}, zero_value,
                0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 5.0f, 10.0f, 15.0f),
        "barycentric, Catmull-Rom and flat-tangent Hermite interpolate every component");
    cna_demo_check(
        &demo,
        cna_vector3_lerp(a, b, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 1.0f, 4.5f) &&
            cna_vector3_smooth_step(a, b, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.5f, 1.0f, 4.5f) &&
            cna_vector3_clamp(
                (CNA_Vector3){-2.0f, 9.0f, 4.0f}, (CNA_Vector3){0.0f, 1.0f, 2.0f},
                (CNA_Vector3){3.0f, 5.0f, 3.0f}, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 5.0f, 3.0f) &&
            cna_vector3_max(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 2.0f, 4.0f, 6.0f) &&
            cna_vector3_min(a, b, &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, -2.0f, 3.0f),
        "linear/eased midpoint and component-wise bound selection are independent operations");
    cna_demo_check(
        &demo,
        cna_vector3_reflect(
            (CNA_Vector3){1.0f, -1.0f, 2.0f}, (CNA_Vector3){0.0f, 1.0f, 0.0f},
            &result) == CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 1.0f, 2.0f),
        "reflection across an upward normal reverses only the incident Y component");

    cna_demo_check(
        &demo,
        cna_vector3_transform_matrix((CNA_Vector3){1.0f, 2.0f, 3.0f}, translation, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 11.0f, 22.0f, 33.0f) &&
            cna_vector3_transform_normal((CNA_Vector3){1.0f, 2.0f, 3.0f}, translation, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 1.0f, 2.0f, 3.0f) &&
            cna_vector3_transform_quaternion(
                (CNA_Vector3){1.0f, 0.0f, 3.0f}, quarter_turn, &result) ==
                CNA_RESULT_SUCCESS &&
            vector_is(result, 0.0f, 1.0f, 3.0f),
        "matrix points include translation, normals omit it and a quaternion rotates in XY");

    destination[0] = (CNA_Vector3){-1.0f, -1.0f, -1.0f};
    destination[1] = destination[0];
    destination[2] = destination[0];
    destination[3] = (CNA_Vector3){-2.0f, -2.0f, -2.0f};
    cna_demo_check(
        &demo,
        cna_vector3_transform_matrix_array(
            source, 3U, 1U, translation, destination, 3U, 0U, 2U) == CNA_RESULT_SUCCESS &&
            vector_is(destination[0], 11.0f, 22.0f, 33.0f) &&
            vector_is(destination[1], 13.0f, 24.0f, 35.0f) &&
            vector_is(destination[2], -1.0f, -1.0f, -1.0f) &&
            vector_is(destination[3], -2.0f, -2.0f, -2.0f) &&
            cna_vector3_transform_quaternion_array(
                source, 4U, 1U, (CNA_Quaternion){0.0f, 0.0f, 0.0f, 1.0f},
                destination, 3U, 0U, 3U) == CNA_RESULT_SUCCESS &&
            vector_is(destination[2], 5.0f, 6.0f, 7.0f) &&
            cna_vector3_transform_normal_array(
                source, 4U, 1U, translation, destination, 3U, 0U, 3U) ==
                CNA_RESULT_SUCCESS &&
            vector_is(destination[0], 1.0f, 2.0f, 3.0f) &&
            vector_is(destination[2], 5.0f, 6.0f, 7.0f),
        "bounded matrix, quaternion and normal arrays transform only selected windows");
    memcpy(before_destination, destination, sizeof(destination));
    cna_demo_check(
        &demo,
        cna_vector3_transform_matrix_array(
            source, 3U, 2U, translation, destination, 3U, 0U, 2U) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            memcmp(destination, before_destination, sizeof(destination)) == 0 &&
            cna_vector3_transform_matrix_array(
                NULL, 0U, 0U, translation, NULL, 0U, 0U, 0U) == CNA_RESULT_SUCCESS,
        "an invalid array window is atomic while the canonical empty null transform succeeds");

    return cna_demo_finish(&demo);
}
