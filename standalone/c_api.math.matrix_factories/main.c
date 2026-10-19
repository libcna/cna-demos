/* SPDX-License-Identifier: MS-PL */
/* Complete Matrix factory behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
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

static CNA_Matrix identity_value(void)
{
    const CNA_Matrix result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return result;
}

int main(int argc, char** argv)
{
    const float quarter_turn = 1.5707963267948966f;
    const float half_sqrt = 0.7071067811865475f;
    const CNA_Vector3 up = {0.0f, 1.0f, 0.0f};
    const CNA_Vector3 forward = {0.0f, 0.0f, -1.0f};
    const CNA_Vector3 position = {1.0f, 2.0f, 3.0f};
    const CNA_Quaternion z_rotation = {0.0f, 0.0f, half_sqrt, half_sqrt};
    const CNA_Plane ground = {{0.0f, 1.0f, 0.0f}, -2.0f};
    const CNA_Matrix identity = identity_value();
    CnaDemo demo;
    CNA_Matrix result = identity;
    CNA_Matrix expected = identity;
    CNA_Matrix axis_rotation = identity;
    CNA_Matrix quaternion_rotation = identity;
    CNA_Matrix ypr_rotation = identity;
    CNA_Matrix direct_rotation = identity;
    CNA_Matrix before = identity;

    cna_demo_init(&demo, "c_api.math.matrix_factories", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_matrix_create_billboard(
            position, (CNA_Vector3){1.0f, 2.0f, 13.0f}, up, NULL, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, -1.0f) && near(result.m22, 1.0f) &&
            near(result.m33, -1.0f) && near(result.m41, 1.0f) &&
            near(result.m42, 2.0f) && near(result.m43, 3.0f) && near(result.m44, 1.0f),
        "a spherical billboard faces its camera and retains the object position");
    cna_demo_check(
        &demo,
        cna_matrix_create_billboard(position, position, up, &forward, &result) ==
                CNA_RESULT_SUCCESS &&
            matrix_is(
                result,
                (CNA_Matrix){
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    1.0f, 2.0f, 3.0f, 1.0f}),
        "a coincident billboard uses the optional camera-forward fallback");
    cna_demo_check(
        &demo,
        cna_matrix_create_constrained_billboard(
            position, (CNA_Vector3){1.0f, 2.0f, 13.0f}, up, NULL, NULL, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, -1.0f) && near(result.m22, 1.0f) &&
            near(result.m33, -1.0f) && near(result.m41, 1.0f) &&
            near(result.m42, 2.0f) && near(result.m43, 3.0f) && near(result.m44, 1.0f),
        "a constrained billboard keeps its authored rotation axis as row two");

    cna_demo_check(
        &demo,
        cna_matrix_create_from_axis_angle(
            (CNA_Vector3){0.0f, 0.0f, 1.0f}, quarter_turn, &axis_rotation) ==
                CNA_RESULT_SUCCESS &&
            cna_matrix_create_from_quaternion(z_rotation, &quaternion_rotation) ==
                CNA_RESULT_SUCCESS &&
            cna_matrix_create_from_yaw_pitch_roll(
                0.0f, 0.0f, quarter_turn, &ypr_rotation) == CNA_RESULT_SUCCESS &&
            cna_matrix_create_rotation_z(quarter_turn, &direct_rotation) == CNA_RESULT_SUCCESS &&
            matrix_is(axis_rotation, quaternion_rotation) &&
            matrix_is(axis_rotation, ypr_rotation) && matrix_is(axis_rotation, direct_rotation) &&
            near(axis_rotation.m11, 0.0f) && near(axis_rotation.m12, 1.0f) &&
            near(axis_rotation.m21, -1.0f) && near(axis_rotation.m22, 0.0f),
        "axis, quaternion, yaw-pitch-roll and direct Z factories agree on a quarter turn");
    cna_demo_check(
        &demo,
        cna_matrix_create_rotation_x(quarter_turn, &result) == CNA_RESULT_SUCCESS &&
            near(result.m22, 0.0f) && near(result.m23, 1.0f) &&
            near(result.m32, -1.0f) &&
            cna_matrix_create_rotation_y(quarter_turn, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 0.0f) && near(result.m13, -1.0f) &&
            near(result.m31, 1.0f),
        "direct X and Y factories place sine terms for row-vector rotation");

    cna_demo_check(
        &demo,
        cna_matrix_create_world(position, forward, up, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 1.0f) && near(result.m22, 1.0f) && near(result.m33, 1.0f) &&
            near(result.m41, 1.0f) && near(result.m42, 2.0f) && near(result.m43, 3.0f) &&
            cna_matrix_create_look_at(
                position, (CNA_Vector3){1.0f, 2.0f, 2.0f}, up, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, 1.0f) && near(result.m22, 1.0f) && near(result.m33, 1.0f) &&
            near(result.m41, -1.0f) && near(result.m42, -2.0f) && near(result.m43, -3.0f),
        "world and look-at factories encode opposite translations for aligned frames");

    cna_demo_check(
        &demo,
        cna_matrix_create_orthographic(4.0f, 2.0f, 1.0f, 11.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, 0.5f) && near(result.m22, 1.0f) &&
            near(result.m33, -0.1f) && near(result.m43, -0.1f) && near(result.m44, 1.0f) &&
            cna_matrix_create_orthographic_off_center(
                -2.0f, 6.0f, -1.0f, 3.0f, 1.0f, 11.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, 0.25f) && near(result.m22, 0.5f) &&
            near(result.m41, -0.5f) && near(result.m42, -0.5f) &&
            near(result.m33, -0.1f) && near(result.m43, -0.1f),
        "centered and off-center orthographic factories encode scale and view offset");

    cna_demo_check(
        &demo,
        cna_matrix_create_perspective(4.0f, 2.0f, 1.0f, 11.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, 0.5f) && near(result.m22, 1.0f) &&
            near(result.m33, -1.1f) && near(result.m34, -1.0f) &&
            near(result.m43, -1.1f) && near(result.m44, 0.0f) &&
            cna_matrix_create_perspective_field_of_view(
                quarter_turn, 2.0f, 1.0f, 11.0f, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 0.5f) && near(result.m22, 1.0f) && near(result.m34, -1.0f),
        "dimension and field-of-view perspective factories agree at a ninety-degree field");
    cna_demo_check(
        &demo,
        cna_matrix_create_perspective_off_center(
            -1.0f, 3.0f, -1.0f, 1.0f, 1.0f, 11.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            near(result.m11, 0.5f) && near(result.m22, 1.0f) &&
            near(result.m31, 0.5f) && near(result.m32, 0.0f) &&
            near(result.m34, -1.0f) && near(result.m43, -1.1f),
        "an off-center perspective stores its horizontal shift in row three");
    result = identity;
    before = result;
    cna_demo_check(
        &demo,
        cna_matrix_create_perspective(4.0f, 2.0f, 0.0f, 11.0f, &result) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            matrix_is(result, before) &&
            cna_matrix_create_perspective_field_of_view(
                0.0f, 2.0f, 1.0f, 11.0f, &result) == CNA_RESULT_INVALID_ARGUMENT &&
            matrix_is(result, before) &&
            cna_matrix_create_perspective_off_center(
                -1.0f, 3.0f, -1.0f, 1.0f, 12.0f, 11.0f, &result) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            matrix_is(result, before),
        "all perspective families reject invalid depth or field ranges atomically");

    cna_demo_check(
        &demo,
        cna_matrix_create_perspective_field_of_view(
            CNA_MATH_PI, 2.0f, 1.0f, 11.0f, &result) == CNA_RESULT_INVALID_ARGUMENT &&
            matrix_is(result, before) &&
            cna_matrix_create_perspective_field_of_view(
                nextafterf(CNA_MATH_PI, 0.0f), 2.0f, 1.0f, 11.0f, &result) ==
                CNA_RESULT_SUCCESS && result.m22 > 0.0f,
        "the public Pi boundary is refused atomically while its preceding float is accepted");

    cna_demo_check(
        &demo,
        cna_matrix_create_scale_scalar(2.0f, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 2.0f) && near(result.m22, 2.0f) && near(result.m33, 2.0f) &&
            cna_matrix_create_scale_xyz(2.0f, 3.0f, 4.0f, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 2.0f) && near(result.m22, 3.0f) && near(result.m33, 4.0f) &&
            cna_matrix_create_scale_vector3(
                (CNA_Vector3){5.0f, 6.0f, 7.0f}, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 5.0f) && near(result.m22, 6.0f) && near(result.m33, 7.0f) &&
            near(result.m44, 1.0f),
        "scalar, component and vector scale factories populate the diagonal");

    CNA_Vector3 scale = {0}, translation = {0};
    CNA_Quaternion rotation = {0};
    CNA_Bool decomposed = CNA_FALSE;
    CNA_Matrix reflected = identity, rebuilt = identity;
    cna_demo_check(
        &demo,
        cna_matrix_create_scale_xyz(-2, 3, 4, &reflected) == CNA_RESULT_SUCCESS &&
            cna_matrix_decompose(reflected, &scale, &rotation, &translation, &decomposed) ==
                CNA_RESULT_SUCCESS && decomposed == CNA_TRUE &&
            scale.x == 2 && scale.y == 3 && scale.z == -4 &&
            cna_matrix_create_scale_vector3(scale, &rebuilt) == CNA_RESULT_SUCCESS &&
            cna_matrix_create_from_quaternion(rotation, &result) == CNA_RESULT_SUCCESS &&
            cna_matrix_multiply(rebuilt, result, &rebuilt) == CNA_RESULT_SUCCESS &&
            matrix_is(rebuilt, reflected),
        "decomposing a reflected scale retains its sign and recomposes through the C ABI");
    cna_demo_check(
        &demo,
        cna_matrix_create_scale_xyz(1, 0, 1, &result) == CNA_RESULT_SUCCESS &&
            cna_matrix_decompose(result, &scale, &rotation, &translation, &decomposed) ==
                CNA_RESULT_SUCCESS && decomposed == CNA_TRUE &&
            scale.x == 1 && scale.y == 0 && scale.z == 1 &&
            isnan(rotation.x) && isnan(rotation.y) && isnan(rotation.z) && isnan(rotation.w),
        "a zero scale axis succeeds with XNA's undefined rotation");
    cna_demo_check(
        &demo,
        cna_matrix_create_scale_xyz(1e-5f, 1, 1, &result) == CNA_RESULT_SUCCESS &&
            cna_matrix_decompose(result, &scale, &rotation, &translation, &decomposed) ==
                CNA_RESULT_SUCCESS && decomposed == CNA_FALSE &&
            scale.x == 1e-5f && scale.y == 1 && scale.z == 1 &&
            rotation.x == 0 && rotation.y == 0 && rotation.z == 0 && rotation.w == 1,
        "tiny nonzero scale refusal is a false decomposition result, not a C ABI error");

    cna_demo_check(
        &demo,
        cna_matrix_create_shadow(
            (CNA_Vector3){0.0f, -1.0f, 0.0f}, ground, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, -1.0f) && near(result.m22, 0.0f) &&
            near(result.m33, -1.0f) && near(result.m42, -2.0f) && near(result.m44, -1.0f) &&
            cna_matrix_create_reflection(ground, &result) == CNA_RESULT_SUCCESS &&
            near(result.m11, 1.0f) && near(result.m22, -1.0f) &&
            near(result.m33, 1.0f) && near(result.m42, 4.0f) && near(result.m44, 1.0f),
        "shadow and reflection factories project onto and mirror across an offset plane");

    expected = identity;
    expected.m41 = 1.0f;
    expected.m42 = 2.0f;
    expected.m43 = 3.0f;
    cna_demo_check(
        &demo,
        cna_matrix_create_translation_xyz(1.0f, 2.0f, 3.0f, &result) ==
                CNA_RESULT_SUCCESS &&
            matrix_is(result, expected) &&
            cna_matrix_create_translation_vector3(position, &result) == CNA_RESULT_SUCCESS &&
            matrix_is(result, expected),
        "component and vector translation factories agree on the homogeneous row");

    return cna_demo_finish(&demo);
}
