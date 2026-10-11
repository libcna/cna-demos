/* SPDX-License-Identifier: MS-PL */
/*
 * A CNA game written in C: the C ABI's version check, a game created with a table of lifecycle
 * callbacks (XNA's LoadContent/Update/Draw/UnloadContent/Exiting), CNA's own loop (cna_game_run)
 * and cna_game_request_exit -- the C counterpart of subclassing Game.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH]. Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

/* 60 updates per second, in TimeSpan ticks (100 ns): XNA's default TargetElapsedTime. */
#define TARGET_TICKS INT64_C(166667)

/* Everything the callbacks share, passed to CNA as the callback table's context pointer. */
typedef struct GameState {
    CnaDemo* demo;
    int loads;
    int updates;
    int draws;
    int unloads;
    int exits;
    int updates_before_load;
    int zero_steps;    /* updates with no elapsed time */
    int wrong_steps;   /* later updates whose game time did not advance by exactly the target */
    int64_t previous_total_ticks;
    int64_t last_total_ticks;
} GameState;

static CNA_Result load_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                               CNA_CallbackError* out_error)
{
    GameState* state = (GameState*)context;
    (void)game;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    printf("load_content\n");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    GameState* state = (GameState*)context;
    (void)out_error;
    if (state->loads == 0) {
        ++state->updates_before_load;
    }
    ++state->updates;
    /* XNA's Run calls Update once with zero elapsed time before its loop starts; after that, a
       fixed step advances game time by exactly the target each update. */
    if (game_time->elapsed_game_time_ticks == 0) {
        ++state->zero_steps;
    } else if (game_time->elapsed_game_time_ticks != TARGET_TICKS ||
               (state->updates > 2 && game_time->total_game_time_ticks - state->previous_total_ticks != TARGET_TICKS)) {
        ++state->wrong_steps;
    }
    state->previous_total_ticks = game_time->total_game_time_ticks;
    state->last_total_ticks = game_time->total_game_time_ticks;
    /* Returning anything but success would stop the loop; asking to exit ends it after this frame.
       (A fixed step runs extra updates without a draw when the machine falls behind, so the frame
       limit counts draws.) */
    if (state->demo->frames > 0 && state->draws >= state->demo->frames) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    GameState* state = (GameState*)context;
    const double seconds = (double)game_time->total_game_time_ticks / 1e7;
    const int phase = (int)(seconds * 120.0) % 256;
    CNA_Color color;
    CNA_Result result;
    (void)out_error;
    ++state->draws;
    /* The clear colour drifts with game time, so the frames visibly advance. */
    color.r = (uint8_t)(40 + phase / 4);
    color.g = (uint8_t)60;
    color.b = (uint8_t)(140 - phase / 4);
    color.a = (uint8_t)255;
    result = cna_game_clear(game, color);
    if (result == CNA_RESULT_SUCCESS && state->draws == state->demo->frames) {
        /* The graphics device is borrowed for the callback only: never store the handle. */
        CNA_Handle graphics_device = CNA_INVALID_HANDLE;
        if (cna_game_get_graphics_device(game, &graphics_device) == CNA_RESULT_SUCCESS) {
            cna_demo_capture(state->demo, graphics_device);
        }
    }
    return result;
}

static CNA_Result unload_content(CNA_Handle game, const CNA_GameTime* game_time, void* context,
                                 CNA_CallbackError* out_error)
{
    GameState* state = (GameState*)context;
    (void)game;
    (void)game_time;
    (void)out_error;
    ++state->unloads;
    printf("unload_content\n");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result exiting(CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    GameState* state = (GameState*)context;
    (void)game;
    (void)game_time;
    (void)out_error;
    ++state->exits;
    printf("exiting\n");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const char title[] = "CNA from C";
    CnaDemo demo;
    GameState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    uint32_t abi;

    cna_demo_init(&demo, "c_api.game.lifecycle_callbacks", argc, argv, 30);

    /* First, the ABI: the same major, and at least the minor these headers describe. */
    abi = cna_get_abi_version();
    printf("C ABI %u.%u.%u (headers %u.%u.%u)\n", (unsigned)(abi >> 16), (unsigned)((abi >> 8) & 0xFFU),
           (unsigned)(abi & 0xFFU), (unsigned)CNA_ABI_VERSION_MAJOR, (unsigned)CNA_ABI_VERSION_MINOR,
           (unsigned)CNA_ABI_VERSION_PATCH);
    if (!cna_demo_check(&demo, (abi >> 16) == CNA_ABI_VERSION_MAJOR && abi >= CNA_ABI_VERSION,
                        "the library's C ABI is compatible with the headers")) {
        return cna_demo_finish(&demo);
    }

    memset(&state, 0, sizeof(state));
    state.demo = &demo;

    /* Every versioned struct: zero it, then set its size and version. */
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.unload_content = unload_content;
    callbacks.exiting = exiting;
    callbacks.context = &state;

    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_TRUE;
    create_info.target_elapsed_time_ticks = TARGET_TICKS;
    create_info.window_title.data = title;
    create_info.window_title.byte_length = sizeof(title) - 1U;
    create_info.callbacks = &callbacks; /* copied: the table need not outlive this call */

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game), "cna_game_create")) {
        return cna_demo_finish(&demo);
    }

    /* HiDef, so that --screenshot can read the back buffer (see CnaDemos/CApiDemo.h). */
    cna_demo_check_result(&demo, cna_demo_use_hidef(game, &manager), "HiDef profile through a graphics device manager");

    /* CNA's loop: returns once the game has exited (cna_game_request_exit or the window closed). */
    cna_demo_check_result(&demo, cna_game_run(game), "cna_game_run returns after the exit request");
    printf("%d updates, %d draws, game time %.3f s\n", state.updates, state.draws,
           (double)state.last_total_ticks / 1e7);

    cna_demo_check(&demo, state.loads == 1 && state.updates_before_load == 0,
                   "load_content runs once, before the first update");
    if (demo.frames > 0) {
        cna_demo_check(&demo, state.draws == demo.frames && state.updates > state.draws,
                       "updates and draws run until the game asks to exit");
        cna_demo_check(&demo, state.zero_steps == 1, "the run's first update has zero elapsed time");
        cna_demo_check(&demo, state.wrong_steps == 0, "a fixed step: every later update advances game time by the target");
    }
    cna_demo_check(&demo, state.exits == 1 && state.unloads == 0, "exiting has run when cna_game_run returns");

    /* Teardown: owned children first (here the graphics device manager), then the game. Destroying
       the game is what disposes it and so raises unload_content -- XNA's UnloadContent comes with
       Dispose after Run, too. */
    cna_demo_check_result(&demo, cna_graphics_device_manager_destroy(manager), "cna_graphics_device_manager_destroy");
    cna_demo_check_result(&demo, cna_game_destroy(game), "cna_game_destroy");
    cna_demo_check(&demo, state.unloads == 1 && state.exits == 1, "unload_content ran once, in cna_game_destroy");
    return cna_demo_finish(&demo);
}
