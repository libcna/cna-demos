/* SPDX-License-Identifier: MS-PL */
/* Local NetworkSession constructor overloads and local-gamer admission through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* data, uint64_t byte_length)
{
    CNA_StringView value;
    value.data = data;
    value.byte_length = byte_length;
    return value;
}

static CNA_OptionalInt32 present(int32_t value)
{
    CNA_OptionalInt32 result = {CNA_TRUE, {0U, 0U, 0U}, 0};
    result.value = value;
    return result;
}

static void on_completed(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    static const char FirstName[] = "Constructor One";
    static const char SecondName[] = "Constructor Two";
    CnaDemo demo;
    CNA_SignedInGamerHandle first = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle second = CNA_INVALID_HANDLE;
    CNA_SignedInGamerHandle published[1];
    CNA_Handle explicit_gamers[2];
    CNA_NetworkSessionPropertiesHandle properties = CNA_INVALID_HANDLE;
    CNA_NetworkSessionPropertiesHandle copied = CNA_INVALID_HANDLE;
    CNA_NetworkSessionHandle session = CNA_INVALID_HANDLE;
    CNA_OptionalInt32 property = {CNA_FALSE, {0U, 0U, 0U}, 0};
    int completions = 0;
    int32_t count = -1;
    int32_t maximum = -1;
    int32_t private_slots = -1;

    cna_demo_init(&demo, "c_api.net.session_construction", argc, argv, 1);
    cna_demo_check(
        &demo,
        cna_network_session_get_instance_count_ext(&count) == CNA_RESULT_SUCCESS &&
            count == 0 &&
            cna_network_session_get_active_action_count_ext(&count) == CNA_RESULT_SUCCESS &&
            count == 0,
        "the isolated process starts without sessions or pending constructor actions");

    if (!cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(FirstName, sizeof(FirstName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_ONE, &first),
            "create the first signed-in gamer") ||
        !cna_demo_check_result(
            &demo,
            cna_signed_in_gamer_create_ext(
                text_view(SecondName, sizeof(SecondName) - 1U), CNA_FALSE, CNA_FALSE,
                CNA_PLAYER_INDEX_TWO, &second),
            "create the second signed-in gamer")) {
        goto cleanup;
    }
    published[0] = first;
    cna_demo_check(
        &demo,
        cna_gamer_set_signed_in_gamers_ext(published, UINT64_C(1)) == CNA_RESULT_SUCCESS &&
            cna_gamer_get_signed_in_gamer_count(&count) == CNA_RESULT_SUCCESS && count == 1,
        "the process-wide signed-in collection publishes exactly the first gamer");

    if (cna_demo_check_result(
            &demo,
            cna_network_session_create(CNA_NETWORK_SESSION_TYPE_LOCAL, 1, 6, &session),
            "create a Local session from the published signed-in collection")) {
        cna_demo_check(
            &demo,
            cna_network_session_get_max_gamers(session, &maximum) == CNA_RESULT_SUCCESS &&
                maximum == 6 &&
                cna_network_session_get_gamer_count(
                    session, CNA_NETWORK_SESSION_ROSTER_LOCAL, &count) == CNA_RESULT_SUCCESS &&
                count == 1,
            "the basic overload preserves its limit and admits the published gamer");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the session made by the basic overload");
        session = CNA_INVALID_HANDLE;
    }

    if (!cna_demo_check_result(
            &demo, cna_network_session_properties_create(&properties),
            "create constructor properties")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_session_properties_set_item(properties, 0, present(17)) ==
            CNA_RESULT_SUCCESS,
        "author a distinctive constructor property");
    if (cna_demo_check_result(
            &demo,
            cna_network_session_create_with_properties(
                CNA_NETWORK_SESSION_TYPE_LOCAL, 2, 8, 2, properties, &session),
            "create a Local session with private slots and copied properties")) {
        cna_demo_check(
            &demo,
            cna_network_session_get_private_gamer_slots(session, &private_slots) ==
                    CNA_RESULT_SUCCESS &&
                private_slots == 2 &&
                cna_network_session_copy_session_properties(session, &copied) ==
                    CNA_RESULT_SUCCESS &&
                cna_network_session_properties_get_item(copied, 0, &property) ==
                    CNA_RESULT_SUCCESS &&
                property.has_value == CNA_TRUE && property.value == 17,
            "the properties overload preserves private slots and an independent property copy");
        if (copied != CNA_INVALID_HANDLE) {
            cna_demo_check_result(
                &demo, cna_network_session_properties_destroy(copied),
                "release the copied constructor properties");
            copied = CNA_INVALID_HANDLE;
        }
        cna_demo_check(
            &demo,
            cna_network_session_add_local_gamer(session, second) == CNA_RESULT_SUCCESS &&
                cna_network_session_get_gamer_count(
                    session, CNA_NETWORK_SESSION_ROSTER_LOCAL, &count) == CNA_RESULT_SUCCESS &&
                count == 2 &&
                cna_network_session_add_local_gamer(session, second) ==
                    CNA_RESULT_INVALID_ARGUMENT,
            "a second signed-in gamer joins once and duplicate admission is refused");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the properties-overload session");
        session = CNA_INVALID_HANDLE;
    }

    if (cna_demo_check_result(
            &demo,
            cna_network_session_create_async(
                CNA_NETWORK_SESSION_TYPE_LOCAL, 1, 7, on_completed, &completions, &session),
            "create a Local session through the basic asynchronous pair")) {
        cna_demo_check(
            &demo,
            completions == 1 &&
                cna_network_session_get_max_gamers(session, &maximum) == CNA_RESULT_SUCCESS &&
                maximum == 7,
            "the basic asynchronous pair completes once and publishes its configured session");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the basic asynchronous session");
        session = CNA_INVALID_HANDLE;
    }

    if (cna_demo_check_result(
            &demo,
            cna_network_session_create_with_properties_async(
                CNA_NETWORK_SESSION_TYPE_LOCAL, 1, 9, 3, properties, on_completed,
                &completions, &session),
            "create a Local session through the properties asynchronous pair")) {
        cna_demo_check(
            &demo,
            completions == 2 &&
                cna_network_session_get_private_gamer_slots(session, &private_slots) ==
                    CNA_RESULT_SUCCESS &&
                private_slots == 3,
            "the properties asynchronous pair completes once and preserves private slots");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the properties asynchronous session");
        session = CNA_INVALID_HANDLE;
    }

    explicit_gamers[0] = first;
    explicit_gamers[1] = second;
    if (cna_demo_check_result(
            &demo,
            cna_network_session_create_with_local_gamers_async(
                CNA_NETWORK_SESSION_TYPE_LOCAL, explicit_gamers, UINT64_C(2), 10, 1,
                properties, on_completed, &completions, &session),
            "create a Local session through the explicit-gamer asynchronous pair")) {
        cna_demo_check(
            &demo,
            completions == 3 &&
                cna_network_session_get_gamer_count(
                    session, CNA_NETWORK_SESSION_ROSTER_LOCAL, &count) == CNA_RESULT_SUCCESS &&
                count == 2 &&
                cna_network_session_get_max_gamers(session, &maximum) == CNA_RESULT_SUCCESS &&
                maximum == 10,
            "the explicit asynchronous pair completes once with both requested local gamers");
        cna_demo_check_result(
            &demo, cna_network_session_destroy(session),
            "destroy the explicit-gamer asynchronous session");
        session = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo,
        cna_network_session_get_instance_count_ext(&count) == CNA_RESULT_SUCCESS &&
            count == 0 &&
            cna_network_session_get_active_action_count_ext(&count) == CNA_RESULT_SUCCESS &&
            count == 0,
        "all constructor products and pending actions are gone");

cleanup:
    if (copied != CNA_INVALID_HANDLE) {
        (void)cna_network_session_properties_destroy(copied);
    }
    if (session != CNA_INVALID_HANDLE) {
        (void)cna_network_session_destroy(session);
    }
    if (properties != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_network_session_properties_destroy(properties),
            "release the caller-owned source properties");
    }
    cna_demo_check_result(
        &demo, cna_gamer_set_signed_in_gamers_ext(NULL, UINT64_C(0)),
        "clear the process-wide signed-in collection");
    if (second != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(second),
            "release the second signed-in gamer");
    }
    if (first != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_signed_in_gamer_destroy(first),
            "release the first signed-in gamer");
    }
    return cna_demo_finish(&demo);
}
