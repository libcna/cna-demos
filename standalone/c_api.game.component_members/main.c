/* SPDX-License-Identifier: MS-PL */
/* Public component properties, comparison, explicit lifecycle and event-registration ownership. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct Behavior {
    int initialized;
    int updated;
    int drawn;
    int loaded;
    int unloaded;
    int disposed;
    int64_t elapsed_ticks;
} Behavior;

typedef struct Events {
    int enabled;
    int update_order;
    int draw_order;
    int visible;
    int disposed;
} Events;

static void initialize_component(void* context)
{
    ++((Behavior*)context)->initialized;
}

static void update_component(const CNA_GameTime* game_time, void* context)
{
    Behavior* behavior = (Behavior*)context;
    ++behavior->updated;
    behavior->elapsed_ticks = game_time->elapsed_game_time_ticks;
}

static void draw_component(const CNA_GameTime* game_time, void* context)
{
    (void)game_time;
    ++((Behavior*)context)->drawn;
}

static void load_component(void* context)
{
    ++((Behavior*)context)->loaded;
}

static void unload_component(void* context)
{
    ++((Behavior*)context)->unloaded;
}

static void dispose_component(void* context)
{
    ++((Behavior*)context)->disposed;
}

static void count_event(void* context)
{
    ++*(int*)context;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GameCreateInfo info;
    CNA_GameComponentCallbacks callbacks;
    CNA_GameComponentHandle plain = CNA_INVALID_HANDLE;
    CNA_GameComponentHandle drawable = CNA_INVALID_HANDLE;
    CNA_GameComponentEventRegistrationHandle registrations[5];
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle owner = CNA_INVALID_HANDLE;
    CNA_GameTime time;
    Behavior plain_behavior;
    Behavior draw_behavior;
    Events events;
    CNA_Bool flag = UINT8_C(99);
    int32_t order = INT32_MIN;
    int i;

    cna_demo_init(&demo, "c_api.game.component_members", argc, argv, 1);
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    memset(&plain_behavior, 0, sizeof(plain_behavior));
    memset(&draw_behavior, 0, sizeof(draw_behavior));
    memset(&events, 0, sizeof(events));
    memset(&time, 0, sizeof(time));
    time.elapsed_game_time_ticks = INT64_C(123456);
    time.total_game_time_ticks = INT64_C(654321);
    for (i = 0; i < 5; ++i) {
        registrations[i] = CNA_INVALID_HANDLE;
    }

    if (!cna_demo_check_result(&demo, cna_game_create(&info, &game), "create the component game") ||
        !cna_demo_check_result(&demo, cna_game_run_one_frame(game), "initialize its graphics device")) {
        goto cleanup;
    }

    cna_demo_check_result(&demo, cna_game_component_callbacks_init(&callbacks),
                          "initialize the caller-owned callback table");
    callbacks.initialize = initialize_component;
    callbacks.update = update_component;
    callbacks.dispose = dispose_component;
    callbacks.context = &plain_behavior;
    cna_demo_check_result(&demo, cna_game_component_create(game, &callbacks, &plain),
                          "create an updateable component");

    callbacks.draw = draw_component;
    callbacks.load_content = load_component;
    callbacks.unload_content = unload_component;
    callbacks.context = &draw_behavior;
    cna_demo_check_result(&demo, cna_drawable_game_component_create(game, &callbacks, &drawable),
                          "create a drawable component");

    cna_demo_check(
        &demo,
        cna_game_component_get_game(plain, &owner) == CNA_RESULT_SUCCESS && owner == game &&
            cna_game_component_get_game(drawable, &owner) == CNA_RESULT_SUCCESS && owner == game,
        "both components retain the game they were created from");
    cna_demo_check(
        &demo,
        cna_game_component_get_enabled(plain, &flag) == CNA_RESULT_SUCCESS && flag == CNA_TRUE &&
            cna_game_component_get_update_order(plain, &order) == CNA_RESULT_SUCCESS && order == 0 &&
            cna_drawable_game_component_get_visible(drawable, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_drawable_game_component_get_draw_order(drawable, &order) == CNA_RESULT_SUCCESS &&
            order == 0,
        "enabled, visible and both ordering properties expose their canonical defaults");
    cna_demo_check(
        &demo,
        cna_drawable_game_component_get_draw_order(plain, &order) == CNA_RESULT_INVALID_STATE &&
            cna_drawable_game_component_get_visible(plain, &flag) == CNA_RESULT_INVALID_STATE &&
            cna_drawable_game_component_draw(plain, &time) == CNA_RESULT_INVALID_STATE,
        "an update-only component refuses drawable-only members");

    cna_demo_check(
        &demo,
        cna_game_component_subscribe(plain, CNA_GAME_COMPONENT_EVENT_ENABLED_CHANGED, count_event,
                                     &events.enabled, &registrations[0]) == CNA_RESULT_SUCCESS &&
            cna_game_component_subscribe(plain, CNA_GAME_COMPONENT_EVENT_UPDATE_ORDER_CHANGED,
                                         count_event, &events.update_order,
                                         &registrations[1]) == CNA_RESULT_SUCCESS &&
            cna_game_component_subscribe(drawable, CNA_GAME_COMPONENT_EVENT_DRAW_ORDER_CHANGED,
                                         count_event, &events.draw_order,
                                         &registrations[2]) == CNA_RESULT_SUCCESS &&
            cna_game_component_subscribe(drawable, CNA_GAME_COMPONENT_EVENT_VISIBLE_CHANGED,
                                         count_event, &events.visible,
                                         &registrations[3]) == CNA_RESULT_SUCCESS &&
            cna_game_component_subscribe(plain, CNA_GAME_COMPONENT_EVENT_DISPOSED, count_event,
                                         &events.disposed, &registrations[4]) == CNA_RESULT_SUCCESS,
        "five owned registrations observe independent component events");

    cna_demo_check(
        &demo,
        cna_game_component_set_enabled(plain, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_game_component_set_enabled(plain, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_game_component_set_update_order(plain, -4) == CNA_RESULT_SUCCESS &&
            cna_game_component_set_update_order(plain, -4) == CNA_RESULT_SUCCESS &&
            cna_drawable_game_component_set_draw_order(drawable, 6) == CNA_RESULT_SUCCESS &&
            cna_drawable_game_component_set_draw_order(drawable, 6) == CNA_RESULT_SUCCESS &&
            cna_drawable_game_component_set_visible(drawable, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            cna_drawable_game_component_set_visible(drawable, CNA_FALSE) == CNA_RESULT_SUCCESS &&
            events.enabled == 1 && events.update_order == 1 && events.draw_order == 1 &&
            events.visible == 1,
        "property events fire once for a real change and stay silent for the same value");
    cna_demo_check(
        &demo,
        cna_game_component_get_enabled(plain, &flag) == CNA_RESULT_SUCCESS && flag == CNA_FALSE &&
            cna_game_component_get_update_order(plain, &order) == CNA_RESULT_SUCCESS && order == -4 &&
            cna_drawable_game_component_get_visible(drawable, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_drawable_game_component_get_draw_order(drawable, &order) == CNA_RESULT_SUCCESS &&
            order == 6,
        "property getters read back the changed component state");
    cna_demo_check(
        &demo,
        cna_game_component_compare_to(plain, drawable, &order) == CNA_RESULT_SUCCESS && order == 4 &&
            cna_game_component_compare_to(drawable, plain, &order) == CNA_RESULT_SUCCESS && order == -4 &&
            cna_game_component_compare_to(plain, plain, &order) == CNA_RESULT_SUCCESS && order == 0,
        "comparison preserves the canonical other-order-minus-this-order direction");

    cna_demo_check(
        &demo,
        cna_game_component_initialize(plain) == CNA_RESULT_SUCCESS && plain_behavior.initialized == 1 &&
            cna_game_component_initialize(drawable) == CNA_RESULT_SUCCESS &&
            draw_behavior.initialized == 1 && draw_behavior.loaded == 1,
        "explicit initialization reaches both C handlers and loads drawable content");
    cna_demo_check(
        &demo,
        cna_game_component_update(plain, &time) == CNA_RESULT_SUCCESS &&
            plain_behavior.updated == 1 && plain_behavior.elapsed_ticks == INT64_C(123456) &&
            cna_drawable_game_component_draw(drawable, &time) == CNA_RESULT_SUCCESS &&
            draw_behavior.drawn == 1,
        "explicit update and draw run despite the disabled and invisible properties");

    cna_demo_check(
        &demo,
        cna_game_component_unsubscribe(registrations[0]) == CNA_RESULT_SUCCESS &&
            cna_game_component_unsubscribe(registrations[0]) == CNA_RESULT_INVALID_HANDLE &&
            cna_game_component_set_enabled(plain, CNA_TRUE) == CNA_RESULT_SUCCESS && events.enabled == 1,
        "unsubscribing is one-shot and stops later event delivery");
    registrations[0] = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_game_component_dispose(plain) == CNA_RESULT_SUCCESS &&
            cna_game_component_dispose(plain) == CNA_RESULT_SUCCESS && plain_behavior.disposed == 1 &&
            events.disposed == 1 && cna_game_component_dispose(drawable) == CNA_RESULT_SUCCESS &&
            draw_behavior.unloaded == 1 && draw_behavior.disposed == 1,
        "disposal is idempotent, raises one event and unloads drawable content once");

cleanup:
    for (i = 0; i < 5; ++i) {
        if (registrations[i] != CNA_INVALID_HANDLE) {
            cna_demo_check_result(&demo, cna_game_component_unsubscribe(registrations[i]),
                                  "release a component event registration");
        }
    }
    if (plain != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_component_destroy(plain), "destroy the updateable component");
    }
    if (drawable != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_component_destroy(drawable), "destroy the drawable component");
    }
    if (game != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the component game");
    }
    return cna_demo_finish(&demo);
}
