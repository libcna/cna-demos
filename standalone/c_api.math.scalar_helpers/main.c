/* SPDX-License-Identifier: MS-PL */
/* Scalar MathHelper-compatible routes through the C ABI. */
#include <CNA/C/cna.h>

#include <limits.h>
#include <math.h>
#include <stdint.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.00001f;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    float value = -99.0f;
    int32_t integer = -99;
    CNA_Bool predicate = UINT8_C(77);

    cna_demo_init(&demo, "c_api.math.scalar_helpers", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_MATH_E == 2.71828175f && CNA_MATH_LOG10_E == 0.4342945f &&
            CNA_MATH_LOG2_E == 1.442695f && CNA_MATH_PI == 3.14159274f &&
            CNA_MATH_PI_OVER_2 == 1.57079637f && CNA_MATH_PI_OVER_4 == 0.7853982f &&
            CNA_MATH_TWO_PI == 6.28318548f &&
            CNA_MATH_MACHINE_EPSILON_FLOAT == 5.96046448E-8f,
        "the C constants expose CNA's exact single-precision MathHelper values");
    cna_demo_check(
        &demo,
        cna_math_barycentric(1.0f, 3.0f, 5.0f, 0.25f, 0.5f, &value) ==
                CNA_RESULT_SUCCESS &&
            near(value, 3.5f) &&
            cna_math_lerp(10.0f, 20.0f, 0.3f, &value) == CNA_RESULT_SUCCESS &&
            near(value, 13.0f),
        "barycentric weights and linear interpolation produce their affine combinations");
    cna_demo_check(
        &demo,
        cna_math_catmull_rom(0.0f, 10.0f, 20.0f, 30.0f, 0.5f, &value) ==
                CNA_RESULT_SUCCESS &&
            near(value, 15.0f) &&
            cna_math_hermite(0.0f, 0.0f, 10.0f, 0.0f, 0.5f, &value) ==
                CNA_RESULT_SUCCESS &&
            near(value, 5.0f) &&
            cna_math_smooth_step(0.0f, 10.0f, 0.2f, &value) == CNA_RESULT_SUCCESS &&
            near(value, 1.04f),
        "Catmull-Rom, flat-tangent Hermite and SmoothStep expose distinct scalar curves");
    cna_demo_check(
        &demo,
        cna_math_smooth_step(0.0f, 10.0f, -1.0f, &value) == CNA_RESULT_SUCCESS &&
            value == 0.0f &&
            cna_math_smooth_step(0.0f, 10.0f, 2.0f, &value) == CNA_RESULT_SUCCESS &&
            value == 10.0f,
        "SmoothStep clamps its amount before applying the easing polynomial");
    cna_demo_check(
        &demo,
        cna_math_clamp_float(3.0f, 0.0f, 2.0f, &value) == CNA_RESULT_SUCCESS &&
            value == 2.0f &&
            cna_math_clamp_int32(12, -3, 7, &integer) == CNA_RESULT_SUCCESS && integer == 7 &&
            cna_math_clamp_float(NAN, 0.0f, 1.0f, &value) == CNA_RESULT_SUCCESS &&
            isnan(value),
        "float and integer clamp select inclusive bounds while a NaN float remains NaN");
    cna_demo_check(
        &demo,
        cna_math_distance(-4.0f, 6.0f, &value) == CNA_RESULT_SUCCESS && value == 10.0f &&
            cna_math_min(3.0f, -2.0f, &value) == CNA_RESULT_SUCCESS && value == -2.0f &&
            cna_math_max(3.0f, -2.0f, &value) == CNA_RESULT_SUCCESS && value == 3.0f,
        "distance is absolute while minimum and maximum retain the selected operand");
    cna_demo_check(
        &demo,
        cna_math_to_degrees(CNA_MATH_PI_OVER_4, &value) == CNA_RESULT_SUCCESS &&
            near(value, 45.0f) &&
            cna_math_to_radians(350.0f, &value) == CNA_RESULT_SUCCESS &&
            cna_math_wrap_angle(value, &value) == CNA_RESULT_SUCCESS &&
            cna_math_to_degrees(value, &value) == CNA_RESULT_SUCCESS &&
            fabsf(value - -10.0f) <= 0.001f &&
            cna_math_wrap_angle(-CNA_MATH_PI, &value) == CNA_RESULT_SUCCESS &&
            near(value, CNA_MATH_PI),
        "degree conversion and WrapAngle select the canonical shorter signed turn");
    cna_demo_check(
        &demo,
        cna_math_wrap_angle(INFINITY, &value) == CNA_RESULT_SUCCESS && isnan(value),
        "wrapping an infinite angle returns the canonical undefined NaN result");
    cna_demo_check(
        &demo,
        cna_math_within_epsilon(
            0.0f, CNA_MATH_MACHINE_EPSILON_FLOAT * 0.5f, &predicate) ==
                CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_math_within_epsilon(
                0.0f, CNA_MATH_MACHINE_EPSILON_FLOAT, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE,
        "WithinEpsilon uses a strict boundary rather than including the exact epsilon");
    cna_demo_check(
        &demo,
        cna_math_closest_msaa_power(0, &integer) == CNA_RESULT_SUCCESS && integer == 0 &&
            cna_math_closest_msaa_power(1, &integer) == CNA_RESULT_SUCCESS && integer == 0 &&
            cna_math_closest_msaa_power(3, &integer) == CNA_RESULT_SUCCESS && integer == 2 &&
            cna_math_closest_msaa_power(8, &integer) == CNA_RESULT_SUCCESS && integer == 8 &&
            cna_math_closest_msaa_power(INT32_MAX, &integer) == CNA_RESULT_SUCCESS &&
            integer == INT32_C(1073741824),
        "the MSAA helper selects zero for 1x and the largest bounded power of two otherwise");
    integer = 77;
    cna_demo_check(
        &demo,
        cna_math_closest_msaa_power(-1, &integer) == CNA_RESULT_INVALID_ARGUMENT &&
            integer == 77,
        "a negative MSAA request is refused without changing the caller's output");

    return cna_demo_finish(&demo);
}
