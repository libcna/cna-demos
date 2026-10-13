/* SPDX-License-Identifier: MS-PL */
/* Matrix storage, properties and operations through the C ABI. */
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

static int quaternion_is(CNA_Quaternion value, float x, float y, float z, float w)
{
    return near(value.x, x) && near(value.y, y) && near(value.z, z) && near(value.w, w);
}

static CNA_Matrix filled(float value)
{
    CNA_Matrix result;
    result.m11 = result.m12 = result.m13 = result.m14 = value;
    result.m21 = result.m22 = result.m23 = result.m24 = value;
    result.m31 = result.m32 = result.m33 = result.m34 = value;
    result.m41 = result.m42 = result.m43 = result.m44 = value;
    return result;
}

static int matrix_is(CNA_Matrix left, CNA_Matrix right)
{
    return near(left.m11, right.m11) && near(left.m12, right.m12) &&
           near(left.m13, right.m13) && near(left.m14, right.m14) &&
           near(left.m21, right.m21) && near(left.m22, right.m22) &&
           near(left.m23, right.m23) && near(left.m24, right.m24) &&
           near(left.m31, right.m31) && near(left.m32, right.m32) &&
           near(left.m33, right.m33) && near(left.m34, right.m34) &&
           near(left.m41, right.m41) && near(left.m42, right.m42) &&
           near(left.m43, right.m43) && near(left.m44, right.m44);
}

int main(int argc, char** argv)
{
    static const char ExpectedText[] =
        "{ {M11:1 M12:0 M13:0 M14:0} {M21:0 M22:1 M23:0 M24:0} "
        "{M31:0 M32:0 M33:1 M34:0} {M41:0 M42:0 M43:0 M44:1} }";
    const CNA_Matrix identity_expected = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    const CNA_Vector3 authored = {2.0f, 3.0f, 4.0f};
    CnaDemo demo;
    CNA_Matrix zero = identity_expected;
    CNA_Matrix identity = zero;
    CNA_Matrix value = zero;
    CNA_Matrix result = zero;
    CNA_Matrix scale = zero;
    CNA_Matrix translation_matrix = zero;
    CNA_Vector3 direction = {9.0f, 8.0f, 7.0f};
    CNA_Vector3 decomposed_scale = direction;
    CNA_Vector3 decomposed_translation = direction;
    CNA_Quaternion rotation = {9.0f, 8.0f, 7.0f, 6.0f};
    CNA_Bool predicate = UINT8_C(77);
    CNA_Bool decomposed = UINT8_C(77);
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float determinant = 0.0f;
    uint64_t text_bytes = 0U;
    char text[160];
    char before_text[160];

    cna_demo_init(&demo, "c_api.math.matrix_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_matrix_init(&zero) == CNA_RESULT_SUCCESS && matrix_is(zero, filled(0.0f)) &&
            cna_matrix_get_identity(&identity) == CNA_RESULT_SUCCESS &&
            matrix_is(identity, identity_expected),
        "default initialization is all-zero while Identity has a unit diagonal");
    cna_demo_check(
        &demo,
        cna_matrix_init_values(
            1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
            &value) == CNA_RESULT_SUCCESS &&
            value.m11 == 1.0f && value.m14 == 4.0f && value.m21 == 5.0f &&
            value.m24 == 8.0f && value.m31 == 9.0f && value.m34 == 12.0f &&
            value.m41 == 13.0f && value.m44 == 16.0f,
        "the 16-value constructor maps row-major arguments to every public field");

    value = identity;
    cna_demo_check(
        &demo,
        cna_matrix_set_right(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m11 == 2.0f && value.m12 == 3.0f && value.m13 == 4.0f &&
            cna_matrix_get_right(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f) &&
            cna_matrix_set_left(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m11 == -2.0f && value.m12 == -3.0f && value.m13 == -4.0f &&
            cna_matrix_get_left(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f),
        "Right stores row one directly while Left stores and retrieves its negation");
    value = identity;
    cna_demo_check(
        &demo,
        cna_matrix_set_up(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m21 == 2.0f && value.m22 == 3.0f && value.m23 == 4.0f &&
            cna_matrix_get_up(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f) &&
            cna_matrix_set_down(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m21 == -2.0f && value.m22 == -3.0f && value.m23 == -4.0f &&
            cna_matrix_get_down(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f),
        "Up stores row two directly while Down stores and retrieves its negation");
    value = identity;
    cna_demo_check(
        &demo,
        cna_matrix_set_backward(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m31 == 2.0f && value.m32 == 3.0f && value.m33 == 4.0f &&
            cna_matrix_get_backward(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f) &&
            cna_matrix_set_forward(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m31 == -2.0f && value.m32 == -3.0f && value.m33 == -4.0f &&
            cna_matrix_get_forward(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f),
        "Backward stores row three directly while Forward stores and retrieves its negation");
    value = identity;
    cna_demo_check(
        &demo,
        cna_matrix_set_translation(&value, authored) == CNA_RESULT_SUCCESS &&
            value.m41 == 2.0f && value.m42 == 3.0f && value.m43 == 4.0f && value.m44 == 1.0f &&
            cna_matrix_get_translation(value, &direction) == CNA_RESULT_SUCCESS &&
            vector_is(direction, 2.0f, 3.0f, 4.0f),
        "Translation uses row four without changing the homogeneous diagonal field");

    cna_demo_check(
        &demo,
        cna_matrix_determinant(identity, &determinant) == CNA_RESULT_SUCCESS &&
            determinant == 1.0f &&
            cna_matrix_equals(identity, identity_expected, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_matrix_not_equals(identity, zero, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_matrix_get_hash_code(identity, &hash) == CNA_RESULT_SUCCESS &&
            cna_matrix_get_hash_code(identity_expected, &equal_hash) == CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "determinant, equality, inequality and hashing observe the complete matrix");

    scale = identity;
    scale.m11 = 2.0f;
    scale.m22 = 3.0f;
    scale.m33 = 4.0f;
    scale.m41 = 5.0f;
    scale.m42 = 6.0f;
    scale.m43 = 7.0f;
    cna_demo_check(
        &demo,
        cna_matrix_decompose(
            scale, &decomposed_scale, &rotation, &decomposed_translation, &decomposed) ==
                CNA_RESULT_SUCCESS &&
            decomposed == CNA_TRUE && vector_is(decomposed_scale, 2.0f, 3.0f, 4.0f) &&
            quaternion_is(rotation, 0.0f, 0.0f, 0.0f, 1.0f) &&
            vector_is(decomposed_translation, 5.0f, 6.0f, 7.0f),
        "decomposition separates nonuniform scale, identity rotation and translation");
    cna_demo_check(
        &demo,
        cna_matrix_decompose(
            zero, &decomposed_scale, &rotation, &decomposed_translation, &decomposed) ==
                CNA_RESULT_SUCCESS &&
            decomposed == CNA_FALSE && quaternion_is(rotation, 0.0f, 0.0f, 0.0f, 1.0f) &&
            vector_is(decomposed_translation, 0.0f, 0.0f, 0.0f),
        "singular decomposition reports false while still publishing canonical outputs");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_matrix_get_string_size(identity, &text_bytes) == CNA_RESULT_SUCCESS &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            cna_matrix_copy_string(identity, text, text_bytes, &text_bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(text, ExpectedText, sizeof(ExpectedText) - 1U) == 0 &&
            (unsigned char)text[sizeof(ExpectedText) - 1U] == 0xA5U,
        "canonical Matrix text contains four rows and no terminator");
    memset(text, 0xA5, sizeof(text));
    memcpy(before_text, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_matrix_copy_string(identity, text, sizeof(ExpectedText) - 2U, &text_bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            text_bytes == sizeof(ExpectedText) - 1U &&
            memcmp(text, before_text, sizeof(text)) == 0,
        "a short Matrix text destination is refused without a partial write");

    cna_demo_check(
        &demo,
        cna_matrix_add(identity, identity, &result) == CNA_RESULT_SUCCESS &&
            result.m11 == 2.0f && result.m22 == 2.0f && result.m12 == 0.0f &&
            cna_matrix_subtract(identity, identity, &result) == CNA_RESULT_SUCCESS &&
            matrix_is(result, zero) &&
            cna_matrix_negate(identity, &result) == CNA_RESULT_SUCCESS &&
            result.m11 == -1.0f && result.m22 == -1.0f && result.m12 == 0.0f,
        "component add, subtract and negate preserve diagonal and off-diagonal values");
    cna_demo_check(
        &demo,
        cna_matrix_multiply_scalar(identity, 2.0f, &result) == CNA_RESULT_SUCCESS &&
            result.m11 == 2.0f && result.m12 == 0.0f &&
            cna_matrix_divide_scalar(identity, 2.0f, &result) == CNA_RESULT_SUCCESS &&
            result.m11 == 0.5f && result.m12 == 0.0f &&
            cna_matrix_divide(filled(1.0f), filled(2.0f), &result) == CNA_RESULT_SUCCESS &&
            matrix_is(result, filled(0.5f)),
        "scalar and component division remain distinct from matrix composition");
    cna_demo_check(
        &demo,
        cna_matrix_lerp(zero, identity, 0.5f, &result) == CNA_RESULT_SUCCESS &&
            result.m11 == 0.5f && result.m22 == 0.5f && result.m44 == 0.5f &&
            result.m12 == 0.0f,
        "matrix lerp interpolates every field independently");

    scale = identity;
    scale.m11 = 2.0f;
    scale.m22 = 3.0f;
    scale.m33 = 4.0f;
    translation_matrix = identity;
    translation_matrix.m41 = 1.0f;
    translation_matrix.m42 = 2.0f;
    translation_matrix.m43 = 3.0f;
    cna_demo_check(
        &demo,
        cna_matrix_multiply(scale, translation_matrix, &result) == CNA_RESULT_SUCCESS &&
            result.m11 == 2.0f && result.m22 == 3.0f && result.m33 == 4.0f &&
            result.m41 == 1.0f && result.m42 == 2.0f && result.m43 == 3.0f &&
            cna_matrix_multiply(translation_matrix, scale, &result) == CNA_RESULT_SUCCESS &&
            result.m41 == 2.0f && result.m42 == 6.0f && result.m43 == 12.0f,
        "row-vector composition order determines whether scale changes the translation row");

    cna_matrix_init_values(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, &value);
    cna_demo_check(
        &demo,
        cna_matrix_transpose(value, &result) == CNA_RESULT_SUCCESS &&
            result.m12 == 5.0f && result.m21 == 2.0f &&
            result.m34 == 15.0f && result.m43 == 12.0f,
        "transpose exchanges rows and columns across the complete matrix");
    cna_demo_check(
        &demo,
        cna_matrix_invert(translation_matrix, &result) == CNA_RESULT_SUCCESS &&
            result.m41 == -1.0f && result.m42 == -2.0f && result.m43 == -3.0f &&
            cna_matrix_invert(zero, &result) == CNA_RESULT_SUCCESS &&
            (!isfinite(result.m11)),
        "inverse negates translation while a singular zero matrix yields IEEE nonfinite fields");
    cna_demo_check(
        &demo,
        cna_matrix_transform(
            identity, (CNA_Quaternion){0.0f, 0.0f, 0.70710678f, 0.70710678f},
            &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 0.0f) && near(result.m12, 1.0f) &&
            near(result.m21, -1.0f) && near(result.m22, 0.0f),
        "quaternion transform applies a quarter-turn rotation to the matrix");

    return cna_demo_finish(&demo);
}
