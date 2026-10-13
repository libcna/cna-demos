/* SPDX-License-Identifier: MS-PL */
/* Indexed component collection mutation and the named subset of the canonical service container. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct Changes {
    int added;
    int removed;
    CNA_GameComponentHandle last_added;
    CNA_GameComponentHandle last_removed;
} Changes;

static void on_added(CNA_GameComponentHandle component, void* context)
{
    Changes* changes = (Changes*)context;
    ++changes->added;
    changes->last_added = component;
}

static void on_removed(CNA_GameComponentHandle component, void* context)
{
    Changes* changes = (Changes*)context;
    ++changes->removed;
    changes->last_removed = component;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo info;
    CNA_GameComponentCallbacks callbacks;
    CNA_GameComponentHandle first = CNA_INVALID_HANDLE;
    CNA_GameComponentHandle second = CNA_INVALID_HANDLE;
    CNA_GameComponentHandle found = CNA_INVALID_HANDLE;
    CNA_GameComponentEventRegistrationHandle added = CNA_INVALID_HANDLE;
    CNA_GameComponentEventRegistrationHandle removed = CNA_INVALID_HANDLE;
    CNA_Handle game = CNA_INVALID_HANDLE;
    Changes changes;
    CNA_Bool flag = UINT8_C(99);
    CNA_Bool manager_present = CNA_FALSE;
    CNA_Bool device_present = CNA_FALSE;
    uint64_t count = UINT64_MAX;
    int32_t index = INT32_MIN;

    cna_demo_init(&demo, "c_api.game.component_collection", argc, argv, 1);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    memset(&changes, 0, sizeof(changes));

    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the collection game") ||
        !cna_demo_check_result(&demo, cna_game_run_one_frame(game), "initialize its runtime services")) {
        goto cleanup;
    }
    cna_demo_check_result(&demo, cna_game_component_callbacks_init(&callbacks),
                          "initialize an empty component callback table");
    cna_demo_check_result(&demo, cna_game_component_create(game, &callbacks, &first),
                          "create the first caller-owned component");
    cna_demo_check_result(&demo, cna_game_component_create(game, &callbacks, &second),
                          "create the second caller-owned component");
    cna_demo_check(
        &demo,
        cna_game_components_subscribe_added(game, on_added, &changes, &added) == CNA_RESULT_SUCCESS &&
            cna_game_components_subscribe_removed(game, on_removed, &changes, &removed) ==
                CNA_RESULT_SUCCESS,
        "owned registrations expose collection additions and removals");

    cna_demo_check(
        &demo,
        cna_game_components_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 0U &&
            cna_game_components_add(game, first) == CNA_RESULT_SUCCESS && changes.added == 1 &&
            changes.last_added == first && cna_game_components_contains(game, first, &flag) ==
                CNA_RESULT_SUCCESS && flag == CNA_TRUE &&
            cna_game_components_index_of(game, first, &index) == CNA_RESULT_SUCCESS && index == 0 &&
            cna_game_components_get_at(game, 0U, &found) == CNA_RESULT_SUCCESS && found == first,
        "add, membership, index and indexed lookup agree on the first component");
    cna_demo_check(
        &demo,
        cna_game_components_contains(game, second, &flag) == CNA_RESULT_SUCCESS && flag == CNA_FALSE &&
            cna_game_components_index_of(game, second, &index) == CNA_RESULT_SUCCESS && index == -1,
        "an absent component has false membership and the canonical minus-one index");
    cna_demo_check(
        &demo,
        cna_game_components_insert(game, 0U, second) == CNA_RESULT_SUCCESS && changes.added == 2 &&
            changes.last_added == second && cna_game_components_get_at(game, 0U, &found) ==
                CNA_RESULT_SUCCESS && found == second &&
            cna_game_components_index_of(game, first, &index) == CNA_RESULT_SUCCESS && index == 1,
        "insertion shifts the first component and reports the inserted handle");
    cna_demo_check(
        &demo,
        cna_game_components_insert(game, 3U, first) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_game_components_get_at(game, 2U, &found) == CNA_RESULT_INVALID_ARGUMENT &&
            found == CNA_INVALID_HANDLE &&
            cna_game_components_remove_at(game, 2U) == CNA_RESULT_INVALID_ARGUMENT,
        "indexed operations reject positions beyond the current collection");
    cna_demo_check(
        &demo,
        cna_game_components_remove_at(game, 0U) == CNA_RESULT_SUCCESS && changes.removed == 1 &&
            changes.last_removed == second && cna_game_components_contains(game, second, &flag) ==
                CNA_RESULT_SUCCESS && flag == CNA_FALSE &&
            cna_game_components_add(game, second) == CNA_RESULT_SUCCESS && changes.added == 3,
        "remove-at leaves the component owned and reusable");
    cna_demo_check(
        &demo,
        cna_game_components_clear(game) == CNA_RESULT_SUCCESS && changes.removed == 3 &&
            cna_game_components_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 0U &&
            cna_game_components_remove(game, first, &flag) == CNA_RESULT_SUCCESS && flag == CNA_FALSE,
        "clear raises one removal per member and removing an absent component is a no-op");

    cna_demo_check(
        &demo,
        cna_game_services_contains_ext(game, CNA_GAME_SERVICE_TYPE_GRAPHICS_DEVICE_MANAGER,
                                       &manager_present) == CNA_RESULT_SUCCESS &&
            cna_game_services_contains_ext(game, CNA_GAME_SERVICE_TYPE_GRAPHICS_DEVICE_SERVICE,
                                           &device_present) == CNA_RESULT_SUCCESS &&
            cna_game_services_contains_ext(game, CNA_GAME_SERVICE_TYPE_MAXIMUM + 1U, &flag) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "named service lookup reports both runtime service states and rejects unknown identities");
    cna_demo_check(
        &demo,
        cna_game_services_remove_ext(game, CNA_GAME_SERVICE_TYPE_GRAPHICS_DEVICE_MANAGER) ==
                CNA_RESULT_SUCCESS &&
            cna_game_services_remove_ext(game, CNA_GAME_SERVICE_TYPE_GRAPHICS_DEVICE_MANAGER) ==
                CNA_RESULT_SUCCESS &&
            cna_game_services_contains_ext(game, CNA_GAME_SERVICE_TYPE_GRAPHICS_DEVICE_MANAGER,
                                           &flag) == CNA_RESULT_SUCCESS && flag == CNA_FALSE,
        "service removal is idempotent and changes later lookup without invalidating the game");
    (void)manager_present;
    (void)device_present;

cleanup:
    if (added != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_component_unsubscribe(added),
                              "release the added-event registration");
    }
    if (removed != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_component_unsubscribe(removed),
                              "release the removed-event registration");
    }
    if (first != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_component_destroy(first), "destroy the first component");
    }
    if (second != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_component_destroy(second), "destroy the second component");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the collection game");
    }
    return cna_demo_finish(&demo);
}
