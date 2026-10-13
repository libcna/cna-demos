/* SPDX-License-Identifier: MS-PL */
/* Sorted CurveKeyCollection mutation, copying, cloning and ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_CurveKey key(float position, float value)
{
    CNA_CurveKey result = {0.0f, 0.0f, 0.0f, 0.0f, CNA_CURVE_CONTINUITY_SMOOTH};
    (void)cna_curve_key_init_position_value(position, value, &result);
    return result;
}

static int key_is(CNA_CurveKey value, float position, float authored_value)
{
    return value.position == position && value.value == authored_value &&
           value.tangent_in == 0.0f && value.tangent_out == 0.0f &&
           value.continuity == CNA_CURVE_CONTINUITY_SMOOTH;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CurveKeyCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_CurveKeyCollectionHandle clone = CNA_INVALID_HANDLE;
    const CNA_CurveKey key0 = key(0.0f, 10.0f);
    const CNA_CurveKey key1 = key(1.0f, 11.0f);
    const CNA_CurveKey key2 = key(2.0f, 12.0f);
    const CNA_CurveKey key3 = key(3.0f, 13.0f);
    CNA_CurveKey result = key(-1.0f, -1.0f);
    CNA_CurveKey copied[4];
    CNA_CurveKey before[4];
    CNA_CurveKey invalid = key2;
    uint64_t count = UINT64_MAX;
    CNA_Bool predicate = UINT8_C(77);
    int32_t index = -2;

    cna_demo_init(&demo, "c_api.math.curve_key_collection", argc, argv, 1);

    cna_demo_check(
        &demo,
        cna_curve_key_collection_create(&collection) == CNA_RESULT_SUCCESS &&
            collection != CNA_INVALID_HANDLE &&
            cna_curve_key_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 0U &&
            cna_curve_key_collection_get_is_read_only(collection, &predicate) ==
                CNA_RESULT_SUCCESS && predicate == CNA_FALSE,
        "a new owned collection is empty and mutable");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_add(collection, key2) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_add(collection, key0) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_add(collection, key1) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 3U &&
            cna_curve_key_collection_get(collection, 0, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 0.0f, 10.0f) &&
            cna_curve_key_collection_get(collection, 1, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 1.0f, 11.0f) &&
            cna_curve_key_collection_get(collection, 2, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 2.0f, 12.0f),
        "adding out of order stores keys in ascending position order");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_contains(collection, key1, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_curve_key_collection_index_of(collection, key1, &index) == CNA_RESULT_SUCCESS &&
            index == 1 &&
            cna_curve_key_collection_contains(
                collection, key(1.0f, 99.0f), &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE &&
            cna_curve_key_collection_index_of(
                collection, key(1.0f, 99.0f), &index) == CNA_RESULT_SUCCESS && index == -1,
        "membership and lookup use full key equality rather than position alone");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_set(collection, 0, key3) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get(collection, 0, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 1.0f, 11.0f) &&
            cna_curve_key_collection_get(collection, 1, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 2.0f, 12.0f) &&
            cna_curve_key_collection_get(collection, 2, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 3.0f, 13.0f),
        "replacing an entry restores ascending position order");

    copied[0] = key(-10.0f, -10.0f);
    copied[1] = key(-11.0f, -11.0f);
    copied[2] = key(-12.0f, -12.0f);
    copied[3] = key(-13.0f, -13.0f);
    memcpy(before, copied, sizeof(copied));
    cna_demo_check(
        &demo,
        cna_curve_key_collection_copy_to(collection, copied, 3U, 1, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL && count == 3U &&
            memcmp(copied, before, sizeof(copied)) == 0,
        "a short destination reports the required count without a partial write");
    cna_demo_check(
        &demo,
        cna_curve_key_collection_copy_to(collection, copied, 4U, 1, &count) ==
                CNA_RESULT_SUCCESS && count == 3U &&
            key_is(copied[0], -10.0f, -10.0f) && key_is(copied[1], 1.0f, 11.0f) &&
            key_is(copied[2], 2.0f, 12.0f) && key_is(copied[3], 3.0f, 13.0f),
        "an indexed copy preserves its prefix and writes the sorted collection exactly");

    invalid.continuity = UINT32_MAX;
    memcpy(before, copied, sizeof(copied));
    cna_demo_check(
        &demo,
        cna_curve_key_collection_set(collection, 1, invalid) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_curve_key_collection_copy_to(collection, copied, 4U, 1, &count) ==
                CNA_RESULT_SUCCESS && memcmp(copied, before, sizeof(copied)) == 0,
        "an invalid replacement is refused without changing collection contents");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_clone(collection, &clone) == CNA_RESULT_SUCCESS &&
            clone != CNA_INVALID_HANDLE && clone != collection &&
            cna_curve_key_collection_clear(collection) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == 0U &&
            cna_curve_key_collection_get_count(clone, &count) == CNA_RESULT_SUCCESS && count == 3U,
        "a cloned collection owns an independent copy of every key");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_remove(clone, key0, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_FALSE &&
            cna_curve_key_collection_remove(clone, key1, &predicate) == CNA_RESULT_SUCCESS &&
            predicate == CNA_TRUE &&
            cna_curve_key_collection_get_count(clone, &count) == CNA_RESULT_SUCCESS && count == 2U &&
            cna_curve_key_collection_remove_at(clone, 1) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get_count(clone, &count) == CNA_RESULT_SUCCESS && count == 1U &&
            cna_curve_key_collection_get(clone, 0, &result) == CNA_RESULT_SUCCESS &&
            key_is(result, 2.0f, 12.0f),
        "value and indexed removal report their effects and preserve the remaining key");

    result = key(-9.0f, -9.0f);
    cna_demo_check(
        &demo,
        cna_curve_key_collection_get(clone, 1, &result) == CNA_RESULT_INVALID_ARGUMENT &&
            key_is(result, -9.0f, -9.0f) &&
            cna_curve_key_collection_remove_at(clone, -1) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_curve_key_collection_get_count(clone, &count) == CNA_RESULT_SUCCESS && count == 1U,
        "invalid indexes are refused without changing outputs or collection size");

    cna_demo_check(
        &demo,
        cna_curve_key_collection_destroy(collection) == CNA_RESULT_SUCCESS &&
            cna_curve_key_collection_get_count(collection, &count) == CNA_RESULT_INVALID_HANDLE &&
            cna_curve_key_collection_destroy(collection) == CNA_RESULT_INVALID_HANDLE &&
            cna_curve_key_collection_destroy(clone) == CNA_RESULT_SUCCESS,
        "destroy invalidates an owned handle while the independent clone remains releasable");

    return cna_demo_finish(&demo);
}
