/* SPDX-License-Identifier: MS-PL */
/* Curve key-view ownership, tangent computation and evaluation through the C ABI. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>

#include "CnaDemos/CApiDemo.h"

static int near(float left, float right)
{
    return fabsf(left - right) <= 0.0001f;
}

static CNA_CurveKey key(float position, float value, float tangent_in, float tangent_out)
{
    CNA_CurveKey result = {0.0f, 0.0f, 0.0f, 0.0f, CNA_CURVE_CONTINUITY_SMOOTH};
    (void)cna_curve_key_init_tangents(
        position, value, tangent_in, tangent_out, &result);
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CurveHandle curve = CNA_INVALID_HANDLE;
    CNA_CurveHandle clone = CNA_INVALID_HANDLE;
    CNA_CurveKeyCollectionHandle keys = CNA_INVALID_HANDLE;
    CNA_CurveKeyCollectionHandle clone_keys = CNA_INVALID_HANDLE;
    CNA_CurveKey key0 = key(0.0f, 0.0f, 2.0f, 2.0f);
    CNA_CurveKey key1 = key(1.0f, 2.0f, 2.0f, 2.0f);
    CNA_CurveKey key2 = key(2.0f, 4.0f, 2.0f, 2.0f);
    CNA_CurveKey computed = key0;
    CNA_CurveLoopType loop = UINT32_MAX;
    CNA_Bool is_constant = CNA_FALSE;
    uint64_t count = UINT64_MAX;
    float value = -1.0f;

    cna_demo_init(&demo, "c_api.math.curve_evaluation", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_curve_create(&curve) == CNA_RESULT_SUCCESS && curve != CNA_INVALID_HANDLE &&
            cna_curve_get_is_constant(curve, &is_constant) == CNA_RESULT_SUCCESS &&
            is_constant == CNA_TRUE &&
            cna_curve_get_pre_loop(curve, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_CONSTANT &&
            cna_curve_get_post_loop(curve, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_CONSTANT &&
            cna_curve_evaluate(curve, 10.0f, &value) == CNA_RESULT_SUCCESS && value == 0.0f,
        "an empty curve is constant, evaluates to zero and defaults to constant loops");

    cna_demo_check(
        &demo,
        cna_curve_get_keys(curve, &keys) == CNA_RESULT_SUCCESS &&
            keys != CNA_INVALID_HANDLE &&
            cna_curve_key_collection_add(keys, key2) == CNA_RESULT_SUCCESS &&
            cna_curve_get_is_constant(curve, &is_constant) == CNA_RESULT_SUCCESS &&
            is_constant == CNA_TRUE &&
            cna_curve_evaluate(curve, -5.0f, &value) == CNA_RESULT_SUCCESS && value == 4.0f,
        "the mutable key view updates its curve and one key remains constant everywhere");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_add(keys, key0) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_add(keys, key1) == CNA_RESULT_SUCCESS &&
            cna_curve_get_is_constant(curve, &is_constant) == CNA_RESULT_SUCCESS &&
            is_constant == CNA_FALSE &&
            cna_curve_evaluate(curve, 0.5f, &value) == CNA_RESULT_SUCCESS && near(value, 1.0f),
        "three authored linear keys interpolate their midpoint");

    cna_demo_check(
        &demo,
        cna_curve_compute_tangents(curve, CNA_CURVE_TANGENT_LINEAR) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get(keys, 1, &computed) == CNA_RESULT_SUCCESS &&
            near(computed.tangent_in, 2.0f) && near(computed.tangent_out, 2.0f),
        "whole-curve matching Linear tangents derive the middle slope");
    cna_demo_check(
        &demo,
        cna_curve_compute_tangents_in_out(
            curve, CNA_CURVE_TANGENT_FLAT, CNA_CURVE_TANGENT_SMOOTH) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get(keys, 1, &computed) == CNA_RESULT_SUCCESS &&
            near(computed.tangent_in, 0.0f) && near(computed.tangent_out, 2.0f),
        "whole-curve incoming and outgoing tangent modes can differ");
    cna_demo_check(
        &demo,
        cna_curve_compute_tangent(curve, 1, CNA_CURVE_TANGENT_LINEAR) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get(keys, 1, &computed) == CNA_RESULT_SUCCESS &&
            near(computed.tangent_in, 2.0f) && near(computed.tangent_out, 2.0f),
        "one key can recompute matching tangents without rebuilding the curve");
    cna_demo_check(
        &demo,
        cna_curve_compute_tangent_in_out(
            curve, 1, CNA_CURVE_TANGENT_FLAT, CNA_CURVE_TANGENT_LINEAR) ==
                CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get(keys, 1, &computed) == CNA_RESULT_SUCCESS &&
            near(computed.tangent_in, 0.0f) && near(computed.tangent_out, 2.0f) &&
            cna_curve_key_collection_get(keys, 0, &computed) == CNA_RESULT_SUCCESS &&
            near(computed.tangent_in, 0.0f) && near(computed.tangent_out, 2.0f),
        "one key can use separate tangent modes while its neighbour remains unchanged");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_set(keys, 0, key0) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_set(keys, 1, key1) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_set(keys, 2, key2) == CNA_RESULT_SUCCESS &&
            cna_curve_set_pre_loop(curve, CNA_CURVE_LOOP_CONSTANT) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_CONSTANT) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, -1.0f, &value) == CNA_RESULT_SUCCESS && near(value, 0.0f) &&
            cna_curve_evaluate(curve, 3.0f, &value) == CNA_RESULT_SUCCESS && near(value, 4.0f),
        "Constant loops hold the first and last values outside the key range");
    cna_demo_check(
        &demo,
        cna_curve_set_pre_loop(curve, CNA_CURVE_LOOP_LINEAR) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_LINEAR) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, -1.0f, &value) == CNA_RESULT_SUCCESS && near(value, -2.0f) &&
            cna_curve_evaluate(curve, 3.0f, &value) == CNA_RESULT_SUCCESS && near(value, 6.0f),
        "Linear loops extend the equal authored endpoint slopes");
    cna_demo_check(
        &demo,
        cna_curve_set_pre_loop(curve, CNA_CURVE_LOOP_CYCLE) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_CYCLE) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, -0.5f, &value) == CNA_RESULT_SUCCESS && near(value, 3.0f) &&
            cna_curve_evaluate(curve, 2.5f, &value) == CNA_RESULT_SUCCESS && near(value, 1.0f),
        "Cycle wraps positions on both sides without changing the value range");
    cna_demo_check(
        &demo,
        cna_curve_set_pre_loop(curve, CNA_CURVE_LOOP_CYCLE_OFFSET) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_CYCLE_OFFSET) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, -0.5f, &value) == CNA_RESULT_SUCCESS && near(value, -1.0f) &&
            cna_curve_evaluate(curve, 2.5f, &value) == CNA_RESULT_SUCCESS && near(value, 5.0f),
        "CycleOffset wraps positions and shifts each repeated value range");
    cna_demo_check(
        &demo,
        cna_curve_set_pre_loop(curve, CNA_CURVE_LOOP_OSCILLATE) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_OSCILLATE) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, -0.5f, &value) == CNA_RESULT_SUCCESS && near(value, 1.0f) &&
            cna_curve_evaluate(curve, 2.5f, &value) == CNA_RESULT_SUCCESS && near(value, 3.0f) &&
            cna_curve_get_pre_loop(curve, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_OSCILLATE &&
            cna_curve_get_post_loop(curve, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_OSCILLATE,
        "Oscillate mirrors alternate cycles and both loop properties read back");

    cna_demo_check(
        &demo,
        cna_curve_clone(curve, &clone) == CNA_RESULT_SUCCESS && clone != CNA_INVALID_HANDLE &&
            clone != curve && cna_curve_get_keys(clone, &clone_keys) == CNA_RESULT_SUCCESS &&
            cna_curve_key_set_value(&key1, 20.0f) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_set(keys, 1, key1) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, 1.0f, &value) == CNA_RESULT_SUCCESS && value == 20.0f &&
            cna_curve_evaluate(clone, 1.0f, &value) == CNA_RESULT_SUCCESS && value == 2.0f &&
            cna_curve_get_pre_loop(clone, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_OSCILLATE,
        "a curve clone independently owns keys and loop state");

    cna_demo_check(
        &demo,
        cna_curve_set_pre_loop(curve, UINT32_MAX) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_curve_get_pre_loop(curve, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_OSCILLATE &&
            cna_curve_set_post_loop(curve, UINT32_MAX) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_curve_get_post_loop(curve, &loop) == CNA_RESULT_SUCCESS &&
            loop == CNA_CURVE_LOOP_OSCILLATE &&
            cna_curve_compute_tangent(curve, -1, CNA_CURVE_TANGENT_LINEAR) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_curve_compute_tangents(curve, UINT32_MAX) == CNA_RESULT_INVALID_ARGUMENT,
        "invalid loop and tangent requests preserve the existing curve");

    cna_demo_check(
        &demo,
        cna_curve_key_set_continuity(&key0, CNA_CURVE_CONTINUITY_STEP) == CNA_RESULT_SUCCESS &&
            cna_curve_key_set_continuity(&key1, CNA_CURVE_CONTINUITY_STEP) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_set(keys, 0, key0) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_set(keys, 1, key1) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, 0.5f, &value) == CNA_RESULT_SUCCESS && value == 0.0f &&
            cna_curve_evaluate(curve, 1.5f, &value) == CNA_RESULT_SUCCESS && value == 20.0f &&
            cna_curve_evaluate(curve, 2.0f, &value) == CNA_RESULT_SUCCESS && value == 4.0f,
        "Step holds the preceding value in both segments and switches at the next key");

    cna_demo_check(
        &demo,
        cna_curve_key_set_tangent_out(&key2, 5.0f) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_set(keys, 2, key2) == CNA_RESULT_SUCCESS &&
            cna_curve_set_post_loop(curve, CNA_CURVE_LOOP_LINEAR) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, 3.0f, &value) == CNA_RESULT_SUCCESS && value == 9.0f,
        "Linear post-loop uses the distinct last outgoing tangent through the C API");

    cna_demo_check(
        &demo,
        cna_curve_destroy(curve) == CNA_RESULT_SUCCESS &&
            cna_curve_evaluate(curve, 0.0f, &value) == CNA_RESULT_INVALID_HANDLE &&
            cna_curve_key_collection_get_count(keys, &count) == CNA_RESULT_SUCCESS && count == 3U &&
            cna_curve_key_collection_destroy(keys) == CNA_RESULT_SUCCESS &&
            cna_curve_destroy(clone) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get_count(clone_keys, &count) == CNA_RESULT_SUCCESS &&
            count == 3U && cna_curve_key_collection_destroy(clone_keys) == CNA_RESULT_SUCCESS &&
            cna_curve_destroy(clone) == CNA_RESULT_INVALID_HANDLE,
        "key views retain their curves after the public curve handles are destroyed");

    return cna_demo_finish(&demo);
}
