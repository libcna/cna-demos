/* SPDX-License-Identifier: MS-PL */
/* Discovered-session values, snapshots, equality and collection ownership through the C ABI. */
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

static CNA_AvailableNetworkSessionCreateInfo make_info(
    CNA_NetworkSessionPropertiesHandle properties,
    const char* address)
{
    static const char Host[] = "Synthetic Host";
    CNA_AvailableNetworkSessionCreateInfo result;
    memset(&result, 0, sizeof(result));
    result.struct_size = (uint32_t)sizeof(result);
    result.struct_version = UINT32_C(1);
    result.current_gamer_count = 3;
    result.open_private_gamer_slots = 1;
    result.open_public_gamer_slots = 4;
    result.session_type = CNA_NETWORK_SESSION_TYPE_SYSTEM_LINK;
    result.host_port = UINT16_C(34567);
    result.host_gamertag = (CNA_StringView){Host, (uint64_t)(sizeof(Host) - 1U)};
    result.host_address = (CNA_StringView){address, (uint64_t)strlen(address)};
    result.session_properties = properties;
    return result;
}

int main(int argc, char** argv)
{
    static const char AddressOne[] = "127.0.0.1";
    static const char AddressTwo[] = "127.0.0.2";
    static const char Host[] = "Synthetic Host";
    CnaDemo demo;
    CNA_NetworkSessionPropertiesHandle properties_a = CNA_INVALID_HANDLE;
    CNA_NetworkSessionPropertiesHandle properties_b = CNA_INVALID_HANDLE;
    CNA_NetworkSessionPropertiesHandle copied_properties = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionHandle session_a = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionHandle session_b = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionHandle session_c = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionHandle copied_session = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionHandle rejected_session = UINT64_C(99);
    CNA_AvailableNetworkSessionCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_AvailableNetworkSessionCreateInfo info;
    CNA_AvailableNetworkSessionCreateInfo invalid_info;
    CNA_QualityOfService quality_a;
    CNA_QualityOfService quality_b;
    CNA_QualityOfService observed_quality;
    CNA_OptionalInt32 property;
    CNA_AvailableNetworkSessionHandle inputs[2];
    char text[32];
    char before[32];
    uint64_t bytes = UINT64_C(0);
    int32_t integer = -1;
    uint16_t port = UINT16_C(0);
    CNA_NetworkSessionType type = UINT32_C(99);
    CNA_Bool flag = CNA_FALSE;

    cna_demo_init(&demo, "c_api.net.available_sessions", argc, argv, 1);
    if (!cna_demo_check_result(
            &demo, cna_network_session_properties_create(&properties_a),
            "create the first source property list") ||
        !cna_demo_check_result(
            &demo, cna_network_session_properties_create(&properties_b),
            "create the second source property list")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_set_item(properties_a, 0, present(11)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_session_properties_set_item(properties_b, 0, present(22)) ==
                CNA_RESULT_SUCCESS,
        "the source descriptions begin with distinct copied property values");
    memset(&quality_a, 0, sizeof(quality_a));
    quality_a.struct_size = (uint32_t)sizeof(quality_a);
    quality_a.struct_version = UINT32_C(1);
    memset(&quality_b, 0, sizeof(quality_b));
    quality_b.struct_size = (uint32_t)sizeof(quality_b);
    quality_b.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_quality_of_service_init_measured(INT64_C(123456), &quality_a) ==
                CNA_RESULT_SUCCESS &&
            cna_quality_of_service_init_measured(INT64_C(999999), &quality_b) ==
                CNA_RESULT_SUCCESS,
        "two exact measured quality samples are ready for independent descriptions");

    info = make_info(properties_a, AddressOne);
    invalid_info = info;
    invalid_info.reserved[2] = UINT8_C(1);
    cna_demo_check(
        &demo,
        cna_available_network_session_create_ext(
            &invalid_info, &quality_a, &rejected_session) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected_session == CNA_INVALID_HANDLE,
        "nonzero reserved creation bytes are refused without publishing a handle");
    if (!cna_demo_check_result(
            &demo,
            cna_available_network_session_create_ext(&info, &quality_a, &session_a),
            "create the first synthetic discovered-session description")) {
        goto cleanup;
    }
    info = make_info(properties_b, AddressOne);
    if (!cna_demo_check_result(
            &demo,
            cna_available_network_session_create_ext(&info, &quality_b, &session_b),
            "create an identity-equal description with different QoS and properties")) {
        goto cleanup;
    }
    info = make_info(properties_a, AddressTwo);
    if (!cna_demo_check_result(
            &demo,
            cna_available_network_session_create_ext(&info, NULL, &session_c),
            "create a description whose connect address differs")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_destroy(properties_a) == CNA_RESULT_SUCCESS &&
            cna_network_session_properties_destroy(properties_b) == CNA_RESULT_SUCCESS,
        "description creation copies both caller-owned property lists");
    properties_a = CNA_INVALID_HANDLE;
    properties_b = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_available_network_session_get_current_gamer_count(session_a, &integer) ==
                CNA_RESULT_SUCCESS &&
            integer == 3 &&
            cna_available_network_session_get_open_private_gamer_slots(session_a, &integer) ==
                CNA_RESULT_SUCCESS &&
            integer == 1 &&
            cna_available_network_session_get_open_public_gamer_slots(session_a, &integer) ==
                CNA_RESULT_SUCCESS &&
            integer == 4,
        "the discovered-session roster and open-slot counts retain exact values");
    cna_demo_check(
        &demo,
        cna_available_network_session_get_connect_port_ext(session_a, &port) ==
                CNA_RESULT_SUCCESS &&
            port == UINT16_C(34567) &&
            cna_available_network_session_get_session_type_ext(session_a, &type) ==
                CNA_RESULT_SUCCESS &&
            type == CNA_NETWORK_SESSION_TYPE_SYSTEM_LINK,
        "the advertised port and session type retain their exact identities");

    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(before));
    cna_demo_check(
        &demo,
        cna_available_network_session_get_host_gamertag_size(session_a, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == sizeof(Host) - 1U &&
            cna_available_network_session_copy_host_gamertag(
                session_a, text, bytes - 1U, &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == sizeof(Host) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_available_network_session_copy_host_gamertag(
                session_a, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(Host) - 1U && memcmp(text, Host, (size_t)bytes) == 0,
        "host gamertag reporting and copying are exact and atomic");
    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(before));
    cna_demo_check(
        &demo,
        cna_available_network_session_get_connect_address_size_ext(session_a, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == sizeof(AddressOne) - 1U &&
            cna_available_network_session_copy_connect_address_ext(
                session_a, text, bytes - 1U, &bytes) == CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == sizeof(AddressOne) - 1U && memcmp(text, before, sizeof(text)) == 0 &&
            cna_available_network_session_copy_connect_address_ext(
                session_a, text, sizeof(text), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(AddressOne) - 1U &&
            memcmp(text, AddressOne, (size_t)bytes) == 0,
        "connect-address reporting and copying are exact and atomic");

    memset(&observed_quality, 0xA5, sizeof(observed_quality));
    observed_quality.struct_size = (uint32_t)sizeof(observed_quality);
    observed_quality.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_available_network_session_get_quality_of_service(
            session_a, &observed_quality) == CNA_RESULT_SUCCESS &&
            observed_quality.is_available == CNA_TRUE &&
            observed_quality.average_roundtrip_ticks == INT64_C(123456) &&
            observed_quality.minimum_roundtrip_ticks == INT64_C(123456) &&
            observed_quality.bytes_per_second_downstream == 0 &&
            observed_quality.bytes_per_second_upstream == 0,
        "the copied quality sample retains its round trip without invented throughput");
    cna_demo_check(
        &demo,
        cna_available_network_session_copy_session_properties(
            session_a, &copied_properties) == CNA_RESULT_SUCCESS &&
            cna_network_session_properties_get_item(copied_properties, 0, &property) ==
                CNA_RESULT_SUCCESS &&
            property.has_value == CNA_TRUE && property.value == 11 &&
            cna_network_session_properties_destroy(copied_properties) == CNA_RESULT_SUCCESS,
        "copying session properties returns an independent list with the original value");
    copied_properties = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_available_network_session_equals(session_a, session_b, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_available_network_session_not_equals(session_a, session_c, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "value equality excludes QoS and properties but includes the connect address");

    inputs[0] = session_a;
    inputs[1] = session_c;
    if (!cna_demo_check_result(
            &demo,
            cna_available_network_session_collection_create_ext(inputs, 2U, &collection),
            "create a copied two-description discovery collection")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_available_network_session_destroy(session_a) == CNA_RESULT_SUCCESS &&
            cna_available_network_session_destroy(session_b) == CNA_RESULT_SUCCESS &&
            cna_available_network_session_destroy(session_c) == CNA_RESULT_SUCCESS,
        "the collection owns snapshots independent of all source handles");
    session_a = CNA_INVALID_HANDLE;
    session_b = CNA_INVALID_HANDLE;
    session_c = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_available_network_session_collection_get_count(collection, &integer) ==
                CNA_RESULT_SUCCESS &&
            integer == 2 &&
            cna_available_network_session_collection_get_is_disposed(collection, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the new discovery collection contains two live snapshots");
    rejected_session = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_available_network_session_collection_copy_session(
            collection, -1, &rejected_session) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected_session == CNA_INVALID_HANDLE &&
            cna_available_network_session_collection_copy_session(
                collection, 1, &copied_session) == CNA_RESULT_SUCCESS,
        "collection indexing refuses the lower bound atomically and copies a valid entry");
    cna_demo_check(
        &demo,
        cna_available_network_session_collection_dispose(collection) == CNA_RESULT_SUCCESS &&
            cna_available_network_session_collection_dispose(collection) == CNA_RESULT_SUCCESS &&
            cna_available_network_session_collection_get_is_disposed(collection, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_available_network_session_collection_get_count(collection, &integer) ==
                CNA_RESULT_SUCCESS &&
            integer == 2,
        "collection disposal is idempotent and preserves its readable snapshot values");
    cna_demo_check(
        &demo,
        cna_available_network_session_collection_destroy(collection) == CNA_RESULT_SUCCESS &&
            cna_available_network_session_collection_destroy(collection) ==
                CNA_RESULT_INVALID_HANDLE,
        "the disposed discovery collection releases exactly once");
    collection = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_available_network_session_get_connect_address_size_ext(
            copied_session, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == sizeof(AddressTwo) - 1U &&
            cna_available_network_session_destroy(copied_session) == CNA_RESULT_SUCCESS &&
            cna_available_network_session_destroy(copied_session) == CNA_RESULT_INVALID_HANDLE,
        "a copied entry survives its collection and releases exactly once");
    copied_session = CNA_INVALID_HANDLE;

cleanup:
    if (copied_session != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_destroy(copied_session);
    }
    if (collection != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_collection_destroy(collection);
    }
    if (session_c != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_destroy(session_c);
    }
    if (session_b != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_destroy(session_b);
    }
    if (session_a != CNA_INVALID_HANDLE) {
        (void)cna_available_network_session_destroy(session_a);
    }
    if (copied_properties != CNA_INVALID_HANDLE) {
        (void)cna_network_session_properties_destroy(copied_properties);
    }
    if (properties_b != CNA_INVALID_HANDLE) {
        (void)cna_network_session_properties_destroy(properties_b);
    }
    if (properties_a != CNA_INVALID_HANDLE) {
        (void)cna_network_session_properties_destroy(properties_a);
    }
    return cna_demo_finish(&demo);
}
