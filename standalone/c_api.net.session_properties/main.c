/* SPDX-License-Identifier: MS-PL */
/* Network QoS and fixed session-property collection behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_OptionalInt32 present(int32_t value)
{
    CNA_OptionalInt32 result = {CNA_TRUE, {0U, 0U, 0U}, 0};
    result.value = value;
    return result;
}

static CNA_OptionalInt32 absent(void)
{
    CNA_OptionalInt32 result = {CNA_FALSE, {0U, 0U, 0U}, 0};
    return result;
}

static int same_optional(CNA_OptionalInt32 left, CNA_OptionalInt32 right)
{
    return left.has_value == right.has_value && left.value == right.value &&
        left.reserved[0] == 0U && left.reserved[1] == 0U && left.reserved[2] == 0U;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_QualityOfService quality;
    CNA_QualityOfService malformed;
    CNA_NetworkSessionPropertiesHandle properties = CNA_INVALID_HANDLE;
    CNA_NetworkSessionPropertyEnumeratorHandle enumerator = CNA_INVALID_HANDLE;
    CNA_OptionalInt32 item = absent();
    CNA_OptionalInt32 destination[10];
    CNA_OptionalInt32 before[10];
    CNA_NetworkSessionJoinError join_error = CNA_NETWORK_SESSION_JOIN_ERROR_SESSION_FULL;
    CNA_Bool flag = CNA_TRUE;
    CNA_Bool removed = UINT8_C(77);
    uint64_t copied = UINT64_C(0);
    int32_t count = -1;
    int32_t index = -2;
    int slot;

    cna_demo_init(&demo, "c_api.net.session_properties", argc, argv, 1);
    memset(&quality, 0xA5, sizeof(quality));
    quality.struct_size = (uint32_t)sizeof(quality);
    quality.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_quality_of_service_init(&quality) == CNA_RESULT_SUCCESS &&
            quality.struct_size == sizeof(quality) && quality.struct_version == UINT32_C(1) &&
            quality.is_available == CNA_FALSE && quality.reserved[0] == 0U &&
            quality.reserved[1] == 0U && quality.reserved[2] == 0U &&
            quality.reserved[3] == 0U && quality.reserved[4] == 0U &&
            quality.reserved[5] == 0U && quality.reserved[6] == 0U &&
            quality.average_roundtrip_ticks == INT64_C(0) &&
            quality.minimum_roundtrip_ticks == INT64_C(0) &&
            quality.bytes_per_second_downstream == 0 &&
            quality.bytes_per_second_upstream == 0,
        "the default QualityOfService is a canonical unavailable zero measurement");
    cna_demo_check(
        &demo,
        cna_quality_of_service_init_measured(INT64_C(123456), &quality) ==
                CNA_RESULT_SUCCESS &&
            quality.is_available == CNA_TRUE &&
            quality.average_roundtrip_ticks == INT64_C(123456) &&
            quality.minimum_roundtrip_ticks == INT64_C(123456) &&
            quality.bytes_per_second_downstream == 0 &&
            quality.bytes_per_second_upstream == 0,
        "one measured round trip becomes both average and minimum without invented throughput");
    malformed = quality;
    malformed.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_quality_of_service_init(NULL) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_quality_of_service_init(&malformed) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_quality_of_service_init_measured(INT64_C(1), &malformed) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "QualityOfService initialization refuses null and unknown structure versions");

    cna_demo_check(
        &demo,
        cna_network_session_properties_destroy(CNA_INVALID_HANDLE) ==
                CNA_RESULT_INVALID_HANDLE &&
            cna_net_get_last_join_error(&join_error, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            join_error == CNA_NETWORK_SESSION_JOIN_ERROR_SESSION_NOT_FOUND,
        "an ordinary handle failure leaves no join error and publishes the canonical default value");

    if (!cna_demo_check_result(
            &demo, cna_network_session_properties_create(&properties),
            "create the fixed eight-slot session-property list")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_get_count(properties, &count) == CNA_RESULT_SUCCESS &&
            count == 8 &&
            cna_network_session_properties_get_is_read_only(properties, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the canonical list reports eight slots and its interface read-only flag is false");
    flag = CNA_TRUE;
    for (slot = 0; slot < 8; ++slot) {
        if (cna_network_session_properties_get_item(properties, slot, &item) !=
                CNA_RESULT_SUCCESS ||
            !same_optional(item, absent())) {
            flag = CNA_FALSE;
        }
    }
    cna_demo_check(&demo, flag == CNA_TRUE,
                   "all eight newly created property slots are canonical absent values");
    cna_demo_check(
        &demo,
        cna_network_session_properties_set_item(properties, 0, present(7)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_properties_set_item(properties, 2, present(9)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_properties_set_item(properties, 7, present(INT32_MIN)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_properties_get_item(properties, 7, &item) ==
                CNA_RESULT_SUCCESS &&
            same_optional(item, present(INT32_MIN)),
        "indexed replacement preserves present values including INT32_MIN");
    cna_demo_check(
        &demo,
        cna_network_session_properties_index_of(properties, present(9), &index) ==
                CNA_RESULT_SUCCESS &&
            index == 2 &&
            cna_network_session_properties_index_of(properties, present(42), &index) ==
                CNA_RESULT_SUCCESS &&
            index == -1 &&
            cna_network_session_properties_contains(properties, absent(), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_session_properties_contains(properties, present(42), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "IndexOf and Contains distinguish present, absent and missing values");
    removed = UINT8_C(77);
    cna_demo_check(
        &demo,
        cna_network_session_properties_add(properties, present(1)) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_network_session_properties_insert(properties, 0, present(1)) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_network_session_properties_remove_at(properties, 0) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_network_session_properties_remove(properties, present(7), &removed) ==
                CNA_RESULT_NOT_SUPPORTED &&
            removed == UINT8_C(77) &&
            cna_network_session_properties_clear(properties) == CNA_RESULT_NOT_SUPPORTED &&
            cna_network_session_properties_get_count(properties, &count) == CNA_RESULT_SUCCESS &&
            count == 8,
        "all structural mutations are refused without changing the fixed list or remove output");
    cna_demo_check(
        &demo,
        cna_network_session_properties_get_item(properties, -1, &item) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_properties_get_item(properties, 8, &item) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_properties_set_item(properties, -1, present(1)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_session_properties_set_item(properties, 8, present(1)) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "indexed reads and replacements reject both bounds outside zero through seven");

    memset(destination, 0xA5, sizeof(destination));
    memcpy(before, destination, sizeof(before));
    cna_demo_check(
        &demo,
        cna_network_session_properties_copy_to(
            properties, destination, UINT64_C(9), 2, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == UINT64_C(8) && memcmp(destination, before, sizeof(before)) == 0,
        "a short shifted destination reports all eight elements without a partial copy");
    cna_demo_check(
        &demo,
        cna_network_session_properties_copy_to(
            properties, destination, UINT64_C(10), 2, &copied) == CNA_RESULT_SUCCESS &&
            copied == UINT64_C(8) && same_optional(destination[2], present(7)) &&
            same_optional(destination[3], absent()) &&
            same_optional(destination[4], present(9)) &&
            same_optional(destination[9], present(INT32_MIN)),
        "CopyTo places the complete ordered list at the caller's selected array index");

    if (!cna_demo_check_result(
            &demo,
            cna_network_session_properties_create_enumerator(properties, &enumerator),
            "create a live forward enumerator over the property list")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_property_enumerator_get_current(enumerator, &item) ==
                CNA_RESULT_INVALID_STATE,
        "an enumerator has no Current value before its first MoveNext");
    flag = CNA_TRUE;
    for (slot = 0; slot < 8; ++slot) {
        const CNA_OptionalInt32 expected = slot == 0 ? present(7) :
            (slot == 2 ? present(9) : (slot == 7 ? present(INT32_MIN) : absent()));
        CNA_Bool has_current = CNA_FALSE;
        if (cna_network_session_property_enumerator_move_next(enumerator, &has_current) !=
                CNA_RESULT_SUCCESS ||
            has_current != CNA_TRUE ||
            cna_network_session_property_enumerator_get_current(enumerator, &item) !=
                CNA_RESULT_SUCCESS ||
            !same_optional(item, expected)) {
            flag = CNA_FALSE;
        }
    }
    cna_demo_check(&demo, flag == CNA_TRUE,
                   "the enumerator visits all eight values in index order");
    cna_demo_check(
        &demo,
        cna_network_session_property_enumerator_move_next(enumerator, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_network_session_property_enumerator_get_current(enumerator, &item) ==
                CNA_RESULT_INVALID_STATE &&
            cna_network_session_property_enumerator_reset(enumerator) == CNA_RESULT_SUCCESS &&
            cna_network_session_property_enumerator_get_current(enumerator, &item) ==
                CNA_RESULT_INVALID_STATE &&
            cna_network_session_property_enumerator_move_next(enumerator, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_network_session_property_enumerator_get_current(enumerator, &item) ==
                CNA_RESULT_SUCCESS &&
            same_optional(item, present(7)),
        "enumeration ends without Current and Reset returns it to before the first item");
    cna_demo_check(
        &demo,
        cna_network_session_properties_destroy(properties) == CNA_RESULT_INVALID_STATE,
        "the property-list parent refuses destruction while its enumerator is live");
    cna_demo_check(
        &demo,
        cna_network_session_property_enumerator_destroy(enumerator) == CNA_RESULT_SUCCESS &&
            cna_network_session_property_enumerator_destroy(enumerator) ==
                CNA_RESULT_INVALID_HANDLE,
        "the enumerator releases exactly once");
    enumerator = CNA_INVALID_HANDLE;

cleanup:
    if (enumerator != CNA_INVALID_HANDLE) {
        (void)cna_network_session_property_enumerator_destroy(enumerator);
    }
    if (properties != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_network_session_properties_destroy(properties) == CNA_RESULT_SUCCESS &&
                cna_network_session_properties_destroy(properties) == CNA_RESULT_INVALID_HANDLE,
            "the property list releases once after its child is gone");
    }
    return cna_demo_finish(&demo);
}
