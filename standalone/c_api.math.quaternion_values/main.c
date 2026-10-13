/* SPDX-License-Identifier: MS-PL */
/* Quaternion construction, composition and interpolation through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.00002f;
}

static int quaternion_is(CNA_Quaternion value, float x, float y, float z, float w)
{
    return near(value.x, x) && near(value.y, y) && near(value.z, z) && near(value.w, w);
}

static CNA_Matrix quarter_turn_z_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m12 = 1.0f;
    value.m21 = -1.0f;
    value.m33 = 1.0f;
    value.m44 = 1.0f;
    return value;
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] = "{X:1 Y:2 Z:3 W:4}";
    const float half_pi = 1.5707963267948966f;
    const float half_sqrt = 0.7071067811865475f;
    const CNA_Quaternion x_quarter = {half_sqrt, 0.0f, 0.0f, half_sqrt};
    const CNA_Quaternion y_quarter = {0.0f, half_sqrt, 0.0f, half_sqrt};
    CnaDemo demo;
    CNA_Quaternion value = {9.0f, 8.0f, 7.0f, 6.0f};
    CNA_Quaternion other = value;
    CNA_Quaternion result = value;
    CNA_Quaternion identity = value;
    CNA_Quaternion axis_rotation = value;
    CNA_Bool predicate = UINT8_C(77);
    float scalar = 0.0f;
    int32_t hash = 0;
    int32_t equal_hash = 1;
    uint64_t text_bytes = 0U;
    char text[32];
    char before[32];

    cna_demo_init(&demo, "c_api.math.quaternion_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_quaternion_init_xyzw(1.0f, 2.0f, 3.0f, 4.0f, &value) ==
                CNA_RESULT_SUCCESS &&
            quaternion_is(value, 1.0f, 2.0f, 3.0f, 4.0f) &&
            cna_quaternion_init_vector3_w(
                (CNA_Vector3){1.0f, 2.0f, 3.0f}, 4.0f, &other) == CNA_RESULT_SUCCESS &&
            quaternion_is(other, 1.0f, 2.0f, 3.0f, 4.0f),
        "component and vector-scalar constructors produce the same authored value");
    memset(&identity, 0, sizeof(identity));
    cna_demo_check(
        &demo,
        cna_quaternion_get_identity(&identity) == CNA_RESULT_SUCCESS &&
            quaternion_is(identity, 0.0f, 0.0f, 0.0f, 1.0f) &&
            memcmp(&(CNA_Quaternion){0}, &identity, sizeof(identity)) != 0,
        "the rotation identity is distinct from C's zero-initialized quaternion value");
    predicate = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_quaternion_equals(value, other, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_quaternion_not_equals(value, identity, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_quaternion_get_hash_code(value, &hash) == CNA_RESULT_SUCCESS &&
            cna_quaternion_get_hash_code(other, &equal_hash) == CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "component equality, inequality and hashing agree for equal quaternion values");
    cna_demo_check(
        &demo,
        cna_quaternion_length((CNA_Quaternion){1.0f, 2.0f, 2.0f, 4.0f}, &scalar) ==
                CNA_RESULT_SUCCESS &&
            near(scalar, 5.0f) &&
            cna_quaternion_length_squared(
                (CNA_Quaternion){1.0f, 2.0f, 2.0f, 4.0f}, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, 25.0f),
        "length and squared length observe all four components");

    value = (CNA_Quaternion){1.0f, 2.0f, 3.0f, 4.0f};
    cna_demo_check(
        &demo,
        cna_quaternion_conjugate(value, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, -1.0f, -2.0f, -3.0f, 4.0f) &&
            cna_quaternion_conjugate_in_place(&value) == CNA_RESULT_SUCCESS &&
            quaternion_is(value, -1.0f, -2.0f, -3.0f, 4.0f),
        "returned and in-place conjugation negate only the vector part");
    value = (CNA_Quaternion){1.0f, 2.0f, 2.0f, 4.0f};
    cna_demo_check(
        &demo,
        cna_quaternion_normalize(value, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.2f, 0.4f, 0.4f, 0.8f) &&
            cna_quaternion_normalize_in_place(&value) == CNA_RESULT_SUCCESS &&
            quaternion_is(value, 0.2f, 0.4f, 0.4f, 0.8f),
        "returned and in-place normalization produce the same unit quaternion");
    cna_demo_check(
        &demo,
        cna_quaternion_normalize((CNA_Quaternion){0.0f, 0.0f, 0.0f, 0.0f}, &result) ==
                CNA_RESULT_SUCCESS &&
            isnan(result.x) && isnan(result.y) && isnan(result.z) && isnan(result.w),
        "normalizing the zero quaternion preserves canonical IEEE undefined components");

    value = (CNA_Quaternion){1.0f, 2.0f, 3.0f, 4.0f};
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_quaternion_get_string_size(value, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_quaternion_copy_string(value, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical text is copied exactly without a terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    text_bytes = 0U;
    cna_demo_check(
        &demo,
        cna_quaternion_copy_string(
            value, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_quaternion_copy_string(value, NULL, 0U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U,
        "short and zero-capacity text requests report size without partial writes");

    value = (CNA_Quaternion){1.0f, 2.0f, 3.0f, 4.0f};
    cna_demo_check(
        &demo,
        cna_quaternion_add(value, identity, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 1.0f, 2.0f, 3.0f, 5.0f) &&
            cna_quaternion_subtract(value, identity, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 1.0f, 2.0f, 3.0f, 3.0f) &&
            cna_quaternion_multiply_scalar(value, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.5f, 1.0f, 1.5f, 2.0f) &&
            cna_quaternion_negate(value, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, -1.0f, -2.0f, -3.0f, -4.0f),
        "component arithmetic remains distinct from rotational composition");

    cna_demo_check(
        &demo,
        cna_quaternion_create_from_axis_angle(
            (CNA_Vector3){0.0f, 0.0f, 1.0f}, half_pi, &axis_rotation) ==
                CNA_RESULT_SUCCESS &&
            quaternion_is(axis_rotation, 0.0f, 0.0f, half_sqrt, half_sqrt) &&
            cna_quaternion_create_from_yaw_pitch_roll(0.0f, 0.0f, half_pi, &result) ==
                CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.0f, 0.0f, half_sqrt, half_sqrt) &&
            cna_quaternion_create_from_rotation_matrix(quarter_turn_z_matrix(), &result) ==
                CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.0f, 0.0f, half_sqrt, half_sqrt),
        "axis-angle, roll and rotation-matrix forms agree on a quarter turn around Z");
    cna_demo_check(
        &demo,
        cna_quaternion_multiply(x_quarter, y_quarter, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.5f, 0.5f, 0.5f, 0.5f) &&
            cna_quaternion_concatenate(x_quarter, y_quarter, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.5f, 0.5f, -0.5f, 0.5f),
        "concatenating X then Y reverses the multiplication operands as XNA specifies");
    cna_demo_check(
        &demo,
        cna_quaternion_inverse(axis_rotation, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.0f, 0.0f, -half_sqrt, half_sqrt) &&
            cna_quaternion_divide(axis_rotation, axis_rotation, &result) == CNA_RESULT_SUCCESS &&
            quaternion_is(result, 0.0f, 0.0f, 0.0f, 1.0f),
        "inverse reverses the rotation and dividing it by itself yields identity");
    cna_demo_check(
        &demo,
        cna_quaternion_dot(identity, axis_rotation, &scalar) == CNA_RESULT_SUCCESS &&
            near(scalar, half_sqrt),
        "the identity-to-quarter-turn dot product is the half-angle cosine");
    cna_demo_check(
        &demo,
        cna_quaternion_lerp(identity, axis_rotation, 0.25f, &value) == CNA_RESULT_SUCCESS &&
            quaternion_is(value, 0.0f, 0.0f, 0.18736555f, 0.98229027f) &&
            cna_quaternion_slerp(identity, axis_rotation, 0.25f, &other) ==
                CNA_RESULT_SUCCESS &&
            quaternion_is(other, 0.0f, 0.0f, 0.19509032f, 0.98078528f) &&
            !near(value.z, other.z),
        "normalized linear and spherical interpolation take distinct quarter-step paths");

    return cna_demo_finish(&demo);
}
