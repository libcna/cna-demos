/* SPDX-License-Identifier: MS-PL */
/* Complete fixed-layout CurveKey value behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <stddef.h>
#include <stdint.h>

#include "CnaDemos/CApiDemo.h"

typedef struct CurveKeyAlignmentProbe {
    char prefix;
    CNA_CurveKey value;
} CurveKeyAlignmentProbe;

_Static_assert(sizeof(CNA_CurveKey) == 20U, "CNA_CurveKey size changed");
_Static_assert(offsetof(CurveKeyAlignmentProbe, value) == 4U, "CNA_CurveKey alignment changed");
_Static_assert(offsetof(CNA_CurveKey, position) == 0U, "position offset changed");
_Static_assert(offsetof(CNA_CurveKey, value) == 4U, "value offset changed");
_Static_assert(offsetof(CNA_CurveKey, tangent_in) == 8U, "tangent_in offset changed");
_Static_assert(offsetof(CNA_CurveKey, tangent_out) == 12U, "tangent_out offset changed");
_Static_assert(offsetof(CNA_CurveKey, continuity) == 16U, "continuity offset changed");

static int key_is(
    CNA_CurveKey key,
    float position,
    float value,
    float tangent_in,
    float tangent_out,
    CNA_CurveContinuity continuity)
{
    return key.position == position && key.value == value && key.tangent_in == tangent_in &&
           key.tangent_out == tangent_out && key.continuity == continuity;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CurveKey basic = {9.0f, 8.0f, 7.0f, 6.0f, UINT32_C(5)};
    CNA_CurveKey tangents = basic;
    CNA_CurveKey full = basic;
    CNA_CurveKey clone = basic;
    CNA_CurveKey before = basic;
    CNA_CurveContinuity continuity = UINT32_C(99);
    CNA_Bool predicate = UINT8_C(77);
    int32_t comparison = 99;
    int32_t hash = 0;
    int32_t equal_hash = 1;
    float scalar = -1.0f;

    cna_demo_init(&demo, "c_api.math.curve_key_values", argc, argv, 1);

    cna_demo_check(
        &demo,
        sizeof(CNA_CurveKey) == 20U && offsetof(CurveKeyAlignmentProbe, value) == 4U &&
            offsetof(CNA_CurveKey, position) == 0U &&
            offsetof(CNA_CurveKey, value) == 4U &&
            offsetof(CNA_CurveKey, tangent_in) == 8U &&
            offsetof(CNA_CurveKey, tangent_out) == 12U &&
            offsetof(CNA_CurveKey, continuity) == 16U,
        "CurveKey is a five-field fixed-layout C record");
    cna_demo_check(
        &demo,
        cna_curve_key_init_position_value(1.0f, 2.0f, &basic) == CNA_RESULT_SUCCESS &&
            key_is(basic, 1.0f, 2.0f, 0.0f, 0.0f, CNA_CURVE_CONTINUITY_SMOOTH),
        "the position-value constructor supplies zero tangents and Smooth continuity");
    cna_demo_check(
        &demo,
        cna_curve_key_init_tangents(3.0f, 4.0f, 5.0f, 6.0f, &tangents) ==
                CNA_RESULT_SUCCESS &&
            key_is(tangents, 3.0f, 4.0f, 5.0f, 6.0f, CNA_CURVE_CONTINUITY_SMOOTH) &&
            cna_curve_key_init_full(
                7.0f, 8.0f, 9.0f, 10.0f, CNA_CURVE_CONTINUITY_STEP, &full) ==
                CNA_RESULT_SUCCESS &&
            key_is(full, 7.0f, 8.0f, 9.0f, 10.0f, CNA_CURVE_CONTINUITY_STEP),
        "tangent and full constructors retain every authored field");

    cna_demo_check(
        &demo,
        cna_curve_key_get_continuity(full, &continuity) == CNA_RESULT_SUCCESS &&
            continuity == CNA_CURVE_CONTINUITY_STEP &&
            cna_curve_key_get_position(full, &scalar) == CNA_RESULT_SUCCESS && scalar == 7.0f &&
            cna_curve_key_get_value(full, &scalar) == CNA_RESULT_SUCCESS && scalar == 8.0f &&
            cna_curve_key_get_tangent_in(full, &scalar) == CNA_RESULT_SUCCESS && scalar == 9.0f &&
            cna_curve_key_get_tangent_out(full, &scalar) == CNA_RESULT_SUCCESS && scalar == 10.0f,
        "property getters read the same fixed-layout record fields");
    cna_demo_check(
        &demo,
        cna_curve_key_set_continuity(&full, CNA_CURVE_CONTINUITY_SMOOTH) ==
                CNA_RESULT_SUCCESS &&
            cna_curve_key_set_value(&full, 11.0f) == CNA_RESULT_SUCCESS &&
            cna_curve_key_set_tangent_in(&full, -2.0f) == CNA_RESULT_SUCCESS &&
            cna_curve_key_set_tangent_out(&full, 3.0f) == CNA_RESULT_SUCCESS &&
            key_is(full, 7.0f, 11.0f, -2.0f, 3.0f, CNA_CURVE_CONTINUITY_SMOOTH),
        "mutable continuity, value and tangent properties update in place");
    before = full;
    cna_demo_check(
        &demo,
        cna_curve_key_set_continuity(&full, UINT32_MAX) == CNA_RESULT_INVALID_ARGUMENT &&
            key_is(full, before.position, before.value, before.tangent_in, before.tangent_out,
                   before.continuity) &&
            cna_curve_key_init_full(
                0.0f, 0.0f, 0.0f, 0.0f, UINT32_MAX, &full) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            key_is(full, before.position, before.value, before.tangent_in, before.tangent_out,
                   before.continuity),
        "undefined continuity is refused without changing an existing key");

    cna_demo_check(
        &demo,
        cna_curve_key_clone(full, &clone) == CNA_RESULT_SUCCESS &&
            key_is(clone, 7.0f, 11.0f, -2.0f, 3.0f, CNA_CURVE_CONTINUITY_SMOOTH) &&
            cna_curve_key_set_value(&clone, 20.0f) == CNA_RESULT_SUCCESS &&
            full.value == 11.0f && clone.value == 20.0f,
        "cloning copies the value and later mutation remains independent");

    cna_demo_check(
        &demo,
        cna_curve_key_compare_to(basic, tangents, &comparison) == CNA_RESULT_SUCCESS &&
            comparison < 0 &&
            cna_curve_key_compare_to(tangents, basic, &comparison) == CNA_RESULT_SUCCESS &&
            comparison > 0 &&
            cna_curve_key_compare_to(
                basic,
                (CNA_CurveKey){1.0f, 999.0f, 8.0f, 7.0f, CNA_CURVE_CONTINUITY_STEP},
                &comparison) == CNA_RESULT_SUCCESS && comparison == 0,
        "ordering compares positions while ignoring all other fields");
    cna_demo_check(
        &demo,
        cna_curve_key_equals(full, before, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_curve_key_not_equals(full, clone, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_curve_key_get_hash_code(full, &hash) == CNA_RESULT_SUCCESS &&
            cna_curve_key_get_hash_code(before, &equal_hash) == CNA_RESULT_SUCCESS &&
            hash == equal_hash,
        "equality and hashing observe all five fields");

    cna_demo_check(
        &demo,
        cna_curve_key_get_hash_code(
            (CNA_CurveKey){-0.0f, 1, 2, 3, CNA_CURVE_CONTINUITY_SMOOTH}, &hash) == CNA_RESULT_SUCCESS &&
            (uint32_t)hash == UINT32_C(0xbfc00000) &&
            cna_curve_key_get_hash_code(
                (CNA_CurveKey){0, 1, 2, 3, CNA_CURVE_CONTINUITY_STEP}, &equal_hash) == CNA_RESULT_SUCCESS &&
            (uint32_t)equal_hash == UINT32_C(0xbfc00001),
        "signed zero is canonical and CurveKey uses XNA's wrapped field-hash sum");

    return cna_demo_finish(&demo);
}
