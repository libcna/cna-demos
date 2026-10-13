/* SPDX-License-Identifier: MS-PL */
/* Detached NetworkGamer and NetworkMachine values, validation and ownership through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>

#include "CnaDemos/CApiDemo.h"

int main(int argc, char** argv)
{
    static const char Name[] = "Remote Value";
    static const char InvalidUtf8[] = {(char)0x80};
    CnaDemo demo;
    CNA_NetworkGamerHandle gamer = CNA_INVALID_HANDLE;
    CNA_NetworkGamerHandle rejected_gamer = UINT64_C(99);
    CNA_NetworkGamerHandle borrowed_gamer = UINT64_C(99);
    CNA_NetworkMachineHandle machine = CNA_INVALID_HANDLE;
    CNA_NetworkMachineHandle copied_machine = CNA_INVALID_HANDLE;
    CNA_Handle session = UINT64_C(99);
    CNA_Bool value = CNA_TRUE;
    uint8_t id = UINT8_C(0);
    int32_t count = -1;
    int64_t ticks = INT64_C(0);

    cna_demo_init(&demo, "c_api.net.gamer_machine_values", argc, argv, 1);
    cna_demo_check(
        &demo,
        cna_network_gamer_create(
            CNA_INVALID_HANDLE,
            (CNA_StringView){InvalidUtf8, UINT64_C(1)},
            &rejected_gamer) == CNA_RESULT_ENCODING &&
            rejected_gamer == CNA_INVALID_HANDLE,
        "invalid UTF-8 is refused without publishing a gamer handle");
    if (!cna_demo_check_result(
            &demo,
            cna_network_gamer_create(
                CNA_INVALID_HANDLE,
                (CNA_StringView){Name, (uint64_t)(sizeof(Name) - 1U)},
                &gamer),
            "create a detached named NetworkGamer")) {
        return cna_demo_finish(&demo);
    }

    cna_demo_check(
        &demo,
        cna_network_gamer_get_has_left_session(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_has_voice(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_is_guest(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE,
        "a detached gamer starts present, without voice and without guest state");
    cna_demo_check(
        &demo,
        cna_network_gamer_get_is_host(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_is_local(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_is_muted_by_local_user(gamer, &value) ==
                CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_is_private_slot(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE,
        "a detached NetworkGamer has no host, local, mute or private-slot role");
    cna_demo_check(
        &demo,
        cna_network_gamer_get_is_ready(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_is_talking(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_get_id(gamer, &id) == CNA_RESULT_SUCCESS && id == UINT8_C(0) &&
            cna_network_gamer_get_roundtrip_ticks(gamer, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(0),
        "ready, talking, identifier and round-trip values begin at their neutral defaults");
    cna_demo_check(
        &demo,
        cna_network_gamer_get_session(gamer, &session) == CNA_RESULT_SUCCESS &&
            session == CNA_INVALID_HANDLE,
        "the detached gamer reports no owning session");

    cna_demo_check(
        &demo,
        cna_network_gamer_set_id_ext(gamer, UINT8_C(255)) == CNA_RESULT_SUCCESS &&
            cna_network_gamer_get_id(gamer, &id) == CNA_RESULT_SUCCESS && id == UINT8_C(255) &&
            cna_network_gamer_set_roundtrip_ticks_ext(gamer, INT64_C(1234567)) ==
                CNA_RESULT_SUCCESS &&
            cna_network_gamer_get_roundtrip_ticks(gamer, &ticks) == CNA_RESULT_SUCCESS &&
            ticks == INT64_C(1234567),
        "transport-owned identifier and measured round-trip state retain exact values");
    cna_demo_check(
        &demo,
        cna_network_gamer_set_is_host_ext(gamer, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_network_gamer_get_is_host(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_network_gamer_set_has_left_session_ext(gamer, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_network_gamer_get_has_left_session(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE,
        "transport-owned host and departure flags are observable through their public getters");
    cna_demo_check(
        &demo,
        cna_network_gamer_set_is_host_ext(gamer, (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_gamer_get_is_host(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE &&
            cna_network_gamer_set_has_left_session_ext(gamer, (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_network_gamer_get_has_left_session(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_TRUE,
        "noncanonical booleans are refused without changing transport-owned flags");
    cna_demo_check(
        &demo,
        cna_network_gamer_set_has_left_session_ext(gamer, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_network_gamer_set_is_ready(gamer, CNA_TRUE) == CNA_RESULT_INVALID_STATE &&
            cna_network_gamer_get_is_ready(gamer, &value) == CNA_RESULT_SUCCESS &&
            value == CNA_FALSE &&
            cna_network_gamer_set_is_ready(gamer, (CNA_Bool)2) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "ready publication rejects a remote detached gamer and invalid booleans atomically");

    if (!cna_demo_check_result(
            &demo, cna_network_machine_create(&machine),
            "create an independently owned empty NetworkMachine")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_network_machine_get_gamer_count(machine, &count) == CNA_RESULT_SUCCESS &&
            count == 0,
        "a standalone machine has no session-populated gamers");
    borrowed_gamer = UINT64_C(99);
    cna_demo_check(
        &demo,
        cna_network_machine_get_gamer(machine, 0, &borrowed_gamer) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            borrowed_gamer == CNA_INVALID_HANDLE &&
            cna_network_machine_remove_from_session(machine) == CNA_RESULT_INVALID_STATE,
        "an empty machine refuses indexed borrowing and live-session removal");
    cna_demo_check(
        &demo,
        cna_network_gamer_set_machine(gamer, machine) == CNA_RESULT_SUCCESS &&
            cna_network_machine_destroy(machine) == CNA_RESULT_SUCCESS,
        "setting a gamer's machine copies the value so the source handle can be released");
    machine = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_network_gamer_copy_machine(gamer, &copied_machine) == CNA_RESULT_SUCCESS &&
            copied_machine != CNA_INVALID_HANDLE &&
            cna_network_machine_get_gamer_count(copied_machine, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 0,
        "copying the gamer's machine returns a distinct owned handle with the same empty state");
    cna_demo_check(
        &demo,
        cna_network_machine_destroy(copied_machine) == CNA_RESULT_SUCCESS &&
            cna_network_machine_destroy(copied_machine) == CNA_RESULT_INVALID_HANDLE,
        "the copied NetworkMachine releases exactly once");
    copied_machine = CNA_INVALID_HANDLE;

cleanup:
    if (copied_machine != CNA_INVALID_HANDLE) {
        (void)cna_network_machine_destroy(copied_machine);
    }
    if (machine != CNA_INVALID_HANDLE) {
        (void)cna_network_machine_destroy(machine);
    }
    if (gamer != CNA_INVALID_HANDLE) {
        cna_demo_check(
            &demo,
            cna_network_gamer_destroy(gamer) == CNA_RESULT_SUCCESS &&
                cna_network_gamer_destroy(gamer) == CNA_RESULT_INVALID_HANDLE,
            "the NetworkGamer releases exactly once");
    }
    return cna_demo_finish(&demo);
}
