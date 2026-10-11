/* SPDX-License-Identifier: MS-PL */
/*
 * Game components from C. C cannot implement IUpdateable/IDrawable, so a component is a callback
 * table plus a context, and CNA supplies the object that implements the interfaces. Here an
 * updateable "clock" counts updates and two drawable panels overlap: the one with the higher
 * DrawOrder is on top. The game then lowers the orange panel's draw order, hides the blue panel,
 * disables the clock and removes it from the collection -- each change raising its event. A
 * fourth drawable with the highest draw order sees every finished frame and checks it.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

/* The two panels; they overlap in the square (350, 190) - (450, 340). */
typedef struct Panel {
    struct ComponentsState* owner;
    int32_t x;
    int32_t y;
    CNA_Color color;
} Panel;

typedef struct ComponentsState {
    CnaDemo* demo;
    CNA_Handle white;        /* owned 1x1 texture */
    CNA_Handle sprite_batch; /* owned */
    CNA_GameComponentHandle clock;
    CNA_GameComponentHandle blue;
    CNA_GameComponentHandle orange;
    CNA_GameComponentHandle inspector;
    Panel blue_panel;
    Panel orange_panel;
    int clock_initialized;
    int updates;
    int updates_when_disabled;
    int frames;
    int step;
    int added;
    int removed;
    int draw_order_changed;
    int visible_changed;
    int enabled_changed;
} ComponentsState;

static const CNA_Color background = {24, 26, 36, 255};

/* Frames per step: one change a second for a person watching, one a frame unattended. */
static int pace(const ComponentsState* state)
{
    return state->demo->automated ? 1 : 60;
}

/* --- the clock: an updateable component ------------------------------------------------------- */

static void clock_initialize(void* context)
{
    ((ComponentsState*)context)->clock_initialized = 1;
}

static void clock_update(const CNA_GameTime* game_time, void* context)
{
    (void)game_time;
    ++((ComponentsState*)context)->updates;
}

/* --- a panel: a drawable component ------------------------------------------------------------ */

static void panel_draw(const CNA_GameTime* game_time, void* context)
{
    const Panel* panel = (const Panel*)context;
    CNA_SpriteBatchBeginInfo begin_info;
    CNA_SpriteCommand c;
    (void)game_time;
    memset(&begin_info, 0, sizeof(begin_info));
    begin_info.struct_size = (uint32_t)sizeof(begin_info);
    begin_info.struct_version = UINT32_C(1);
    begin_info.sort_mode = CNA_SPRITE_SORT_MODE_DEFERRED;
    memset(&c, 0, sizeof(c));
    c.struct_size = (uint32_t)sizeof(c);
    c.struct_version = UINT32_C(1);
    c.texture = panel->owner->white;
    c.destination.x = panel->x;
    c.destination.y = panel->y;
    c.destination.width = 200;
    c.destination.height = 200;
    c.source.width = 1;
    c.source.height = 1;
    c.color = panel->color;
    c.effects = CNA_SPRITE_EFFECT_NONE;
    /* A draw handler has nowhere to report a failure; the inspector's pixel checks will. */
    if (cna_sprite_batch_begin(panel->owner->sprite_batch, &begin_info) == CNA_RESULT_SUCCESS) {
        (void)cna_sprite_batch_submit_many(panel->owner->sprite_batch, &c, 1U);
        (void)cna_sprite_batch_end(panel->owner->sprite_batch);
    }
}

/* --- the inspector: drawn last, it checks the finished frame ---------------------------------- */

static CNA_Color pixel(const CNA_Color* screen, int x, int y)
{
    return screen[y * 800 + x];
}

static int same(CNA_Color a, CNA_Color b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

static void inspector_draw(const CNA_GameTime* game_time, void* context)
{
    static CNA_Color screen[800 * 480];
    ComponentsState* state = (ComponentsState*)context;
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    uint64_t count = 0U;
    CNA_Color overlap;
    CNA_Color blue_only;
    (void)game_time;
    ++state->frames;
    if ((state->frames % pace(state) != 0 || state->frames > 3 * pace(state)) &&
        state->frames != state->demo->frames) {
        return;
    }
    if (cna_drawable_game_component_get_graphics_device(state->inspector, &graphics_device) != CNA_RESULT_SUCCESS ||
        cna_graphics_device_get_backbuffer_data_rgba8(graphics_device, screen, 800U * 480U, &count) !=
            CNA_RESULT_SUCCESS) {
        cna_demo_check(state->demo, 0, "the inspector reads the finished frame");
        return;
    }
    overlap = pixel(screen, 400, 265);
    blue_only = pixel(screen, 270, 160);
    printf("  frame %d: overlap %u,%u,%u  blue-only %u,%u,%u\n", state->frames, overlap.r, overlap.g, overlap.b,
           blue_only.r, blue_only.g, blue_only.b);
    if (state->frames == pace(state)) {
        cna_demo_check(state->demo, same(overlap, state->orange_panel.color),
                       "draw orders 1 and 2: the orange panel is drawn last, on top");
    } else if (state->frames == 2 * pace(state)) {
        cna_demo_check(state->demo, same(overlap, state->blue_panel.color),
                       "orange set to draw order 0: the blue panel is on top now");
    } else if (state->frames == 3 * pace(state)) {
        cna_demo_check(state->demo, same(overlap, state->orange_panel.color) && same(blue_only, background),
                       "blue made invisible: it is not drawn at all");
    }
    if (state->frames == state->demo->frames) {
        cna_demo_capture(state->demo, graphics_device);
    }
}

/* --- events --------------------------------------------------------------------------------- */

static void on_added(CNA_GameComponentHandle component, void* context)
{
    (void)component;
    ++((ComponentsState*)context)->added;
}

static void on_removed(CNA_GameComponentHandle component, void* context)
{
    ComponentsState* state = (ComponentsState*)context;
    if (component == state->clock) {
        ++state->removed;
    }
}

static void on_draw_order_changed(void* context)
{
    ++((ComponentsState*)context)->draw_order_changed;
}

static void on_visible_changed(void* context)
{
    ++((ComponentsState*)context)->visible_changed;
}

static void on_enabled_changed(void* context)
{
    ++((ComponentsState*)context)->enabled_changed;
}

/* --- the game ------------------------------------------------------------------------------- */

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    ComponentsState* state = (ComponentsState*)context;
    static const CNA_Color white = {255, 255, 255, 255};
    CNA_Handle graphics_device = CNA_INVALID_HANDLE;
    CNA_Texture2DCreateInfo info;
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    result = cna_game_get_graphics_device(game, &graphics_device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.width = 1U;
    info.height = 1U;
    info.format = CNA_SURFACE_FORMAT_COLOR;
    result = cna_texture2d_create(graphics_device, &info, &state->white);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_texture2d_set_data_rgba8(state->white, &white, 1U);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_sprite_batch_create(graphics_device, &state->sprite_batch);
    }
    return result;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    ComponentsState* state = (ComponentsState*)context;
    CNA_Result result = CNA_RESULT_SUCCESS;
    (void)game_time;
    (void)out_error;
    /* One change per step; the components see it in this frame's update and draw passes. */
    if (state->frames == pace(state) && state->step == 0) {
        state->step = 1;
        result = cna_drawable_game_component_set_draw_order(state->orange, 0);
    } else if (state->frames == 2 * pace(state) && state->step == 1) {
        state->step = 2;
        result = cna_drawable_game_component_set_visible(state->blue, CNA_FALSE);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_game_component_set_enabled(state->clock, CNA_FALSE);
            state->updates_when_disabled = state->updates;
        }
    } else if (state->frames == 3 * pace(state) && state->step == 2) {
        CNA_Bool removed = CNA_FALSE;
        uint64_t count = 0U;
        state->step = 3;
        result = cna_game_components_remove(game, state->clock, &removed);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_game_components_get_count(game, &count);
        }
        cna_demo_check(state->demo, removed == CNA_TRUE && count == 3U,
                       "cna_game_components_remove takes the clock out: three components left");
    }
    if (result == CNA_RESULT_SUCCESS && state->demo->frames > 0 && state->frames >= state->demo->frames) {
        result = cna_game_request_exit(game);
    }
    return result;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    /* The game's own draw runs first; the drawable components draw after it, by DrawOrder. */
    return cna_game_clear(game, background);
}

static CNA_Result create_component(CnaDemo* demo, CNA_Handle game, int drawable, CNA_GameComponentCallbacks* callbacks,
                                   CNA_GameComponentHandle* out_component, const char* what)
{
    CNA_Result result = drawable ? cna_drawable_game_component_create(game, callbacks, out_component)
                                 : cna_game_component_create(game, callbacks, out_component);
    cna_demo_check_result(demo, result, what);
    return result;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    ComponentsState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_GameComponentCallbacks component;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_GameComponentEventRegistrationHandle registrations[5];
    CNA_Bool drawable = CNA_FALSE;
    uint64_t count = 0U;
    int i;

    cna_demo_init(&demo, "c_api.game.components", argc, argv, 6);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.white = CNA_INVALID_HANDLE;
    state.sprite_batch = CNA_INVALID_HANDLE;
    state.clock = state.blue = state.orange = state.inspector = CNA_INVALID_HANDLE;
    state.blue_panel = (Panel){&state, 250, 140, {60, 110, 230, 255}};
    state.orange_panel = (Panel){&state, 350, 190, {240, 140, 40, 255}};
    for (i = 0; i < 5; ++i) {
        registrations[i] = CNA_INVALID_HANDLE;
    }
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile (back-buffer read-back)");
    cna_demo_check_result(&demo, cna_game_components_subscribe_added(game, on_added, &state, &registrations[0]),
                          "cna_game_components_subscribe_added");
    cna_demo_check_result(&demo, cna_game_components_subscribe_removed(game, on_removed, &state, &registrations[1]),
                          "cna_game_components_subscribe_removed");

    /* Each component is a callback table; a null handler is a lifecycle step it does not take. */
    (void)cna_game_component_callbacks_init(&component); /* cannot fail for a non-null output */
    component.initialize = clock_initialize;
    component.update = clock_update;
    component.context = &state;
    create_component(&demo, game, 0, &component, &state.clock, "cna_game_component_create (clock)");
    (void)cna_game_component_callbacks_init(&component);
    component.draw = panel_draw;
    component.context = &state.blue_panel;
    create_component(&demo, game, 1, &component, &state.blue, "cna_drawable_game_component_create (blue)");
    component.context = &state.orange_panel;
    create_component(&demo, game, 1, &component, &state.orange, "cna_drawable_game_component_create (orange)");
    (void)cna_game_component_callbacks_init(&component);
    component.draw = inspector_draw;
    component.context = &state;
    create_component(&demo, game, 1, &component, &state.inspector, "cna_drawable_game_component_create (inspector)");

    cna_demo_check(&demo,
                   cna_game_component_get_is_drawable(state.clock, &drawable) == CNA_RESULT_SUCCESS &&
                       drawable == CNA_FALSE,
                   "the clock is updateable only");
    cna_demo_check(&demo,
                   cna_drawable_game_component_set_draw_order(state.blue, 1) == CNA_RESULT_SUCCESS &&
                       cna_drawable_game_component_set_draw_order(state.orange, 2) == CNA_RESULT_SUCCESS &&
                       cna_drawable_game_component_set_draw_order(state.inspector, 100) == CNA_RESULT_SUCCESS,
                   "cna_drawable_game_component_set_draw_order: blue 1, orange 2, inspector 100");
    cna_demo_check_result(&demo,
                          cna_game_component_subscribe(state.orange, CNA_GAME_COMPONENT_EVENT_DRAW_ORDER_CHANGED,
                                                       on_draw_order_changed, &state, &registrations[2]),
                          "cna_game_component_subscribe(DRAW_ORDER_CHANGED)");
    cna_demo_check_result(&demo,
                          cna_game_component_subscribe(state.blue, CNA_GAME_COMPONENT_EVENT_VISIBLE_CHANGED,
                                                       on_visible_changed, &state, &registrations[3]),
                          "cna_game_component_subscribe(VISIBLE_CHANGED)");
    cna_demo_check_result(&demo,
                          cna_game_component_subscribe(state.clock, CNA_GAME_COMPONENT_EVENT_ENABLED_CHANGED,
                                                       on_enabled_changed, &state, &registrations[4]),
                          "cna_game_component_subscribe(ENABLED_CHANGED)");

    /* Created components exist on their own until the game's collection holds them. */
    cna_demo_check(&demo,
                   cna_game_components_add(game, state.clock) == CNA_RESULT_SUCCESS &&
                       cna_game_components_add(game, state.blue) == CNA_RESULT_SUCCESS &&
                       cna_game_components_add(game, state.orange) == CNA_RESULT_SUCCESS &&
                       cna_game_components_add(game, state.inspector) == CNA_RESULT_SUCCESS,
                   "cna_game_components_add (all four)");
    cna_demo_check(&demo, cna_game_components_get_count(game, &count) == CNA_RESULT_SUCCESS && count == 4U,
                   "the collection holds four components");
    cna_demo_check(&demo, state.added == 4, "the added event ran once per component");
    cna_demo_check(&demo, state.clock_initialized == 0, "no component is initialised before the game runs");

    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run");
    printf("  clock: %d updates, %d when it was disabled\n", state.updates, state.updates_when_disabled);
    cna_demo_check(&demo, state.clock_initialized == 1, "the game initialised the clock");
    cna_demo_check(&demo, state.updates_when_disabled > 0 && state.updates == state.updates_when_disabled,
                   "the clock was updated every frame while enabled, and not after");
    cna_demo_check(&demo, state.draw_order_changed == 1 && state.visible_changed == 1 && state.enabled_changed == 1,
                   "each change raised its event once");
    cna_demo_check(&demo, state.removed == 1, "removing the clock raised the removed event");

    /* Registrations keep their component alive, so they go first; then the components; then the
       resources and the game. */
    for (i = 0; i < 5; ++i) {
        if (registrations[i] != CNA_INVALID_HANDLE) {
            cna_demo_check_result(&demo, cna_game_component_unsubscribe(registrations[i]),
                                  "cna_game_component_unsubscribe");
        }
    }
    cna_demo_check_result(&demo, cna_game_component_destroy(state.clock), "cna_game_component_destroy (clock)");
    cna_demo_check_result(&demo, cna_game_component_destroy(state.blue), "cna_game_component_destroy (blue)");
    cna_demo_check_result(&demo, cna_game_component_destroy(state.orange), "cna_game_component_destroy (orange)");
    cna_demo_check_result(&demo, cna_game_component_destroy(state.inspector),
                          "cna_game_component_destroy (inspector)");
    if (state.sprite_batch != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_sprite_batch_destroy(state.sprite_batch), "cna_sprite_batch_destroy");
    }
    if (state.white != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_texture2d_destroy(state.white), "cna_texture2d_destroy");
    }
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    return cna_demo_finish(&demo);
}
