/* Genuine-backed ISSUE-141 contracts through the actual C ABI, without Game.Run. */
#include <CNA/C/cna.h>
#include <stdio.h>

typedef struct State {
    CNA_Handle game;
    int callbacks;
    int events;
    int membership_errors;
} State;

static int checks;
static int failures;

static void check(int ok, const char* name)
{
    ++checks;
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    }
}

static void disposing(void* context)
{
    ++((State*)context)->callbacks;
}

static void disposed(void* context)
{
    State* state = (State*)context;
    uint64_t count = UINT64_MAX;
    ++state->events;
    if (cna_game_components_get_count(state->game, &count) != CNA_RESULT_SUCCESS || count != 0) {
        ++state->membership_errors;
    }
}

int main(void)
{
    CNA_GameCallbacks game_callbacks = {sizeof(CNA_GameCallbacks), UINT32_C(1), 0, 0, 0, 0, 0, 0};
    CNA_GameCreateInfo info = {
        sizeof(CNA_GameCreateInfo), UINT32_C(1), CNA_TRUE, {0, 0, 0, 0, 0, 0, 0},
        INT64_C(166667), {"Component disposal", UINT64_C(18)}, &game_callbacks
    };
    State state = {CNA_INVALID_HANDLE, 0, 0, 0};
    CNA_GameComponentCallbacks callbacks;
    CNA_GameComponentHandle component = CNA_INVALID_HANDLE;
    CNA_GameComponentEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    if (cna_game_create(&info, &state.game) != CNA_RESULT_SUCCESS ||
        cna_game_component_callbacks_init(&callbacks) != CNA_RESULT_SUCCESS) {
        puts("SETUP FAILED");
        return 2;
    }
    callbacks.dispose = disposing;
    callbacks.context = &state;
    if (cna_game_component_create(state.game, &callbacks, &component) != CNA_RESULT_SUCCESS ||
        cna_game_components_add(state.game, component) != CNA_RESULT_SUCCESS ||
        cna_game_component_subscribe(component, CNA_GAME_COMPONENT_EVENT_DISPOSED,
                                     disposed, &state, &registration) != CNA_RESULT_SUCCESS) {
        puts("COMPONENT SETUP FAILED");
        return 2;
    }
    check(cna_game_component_dispose(component) == CNA_RESULT_SUCCESS, "first disposal succeeds");
    check(state.callbacks == 1 && state.events == 1, "first callback and event");
    check(state.membership_errors == 0, "removal precedes first disposed event");
    check(cna_game_component_dispose(component) == CNA_RESULT_SUCCESS, "second disposal succeeds");
    check(state.callbacks == 2, "second explicit disposal invokes override callback");
    check(state.events == 2, "second explicit disposal raises disposed event");
    check(state.membership_errors == 0, "all disposed events observe absent membership");
    check(cna_game_component_unsubscribe(registration) == CNA_RESULT_SUCCESS, "unsubscribe succeeds");
    const int callbacks_before_release = state.callbacks;
    const int events_before_release = state.events;
    check(cna_game_component_destroy(component) == CNA_RESULT_SUCCESS, "release succeeds");
    check(state.callbacks == callbacks_before_release && state.events == events_before_release,
          "destruction does not repeat managed callbacks");
    check(cna_game_destroy(state.game) == CNA_RESULT_SUCCESS, "game cleanup succeeds");
    printf("checks=%d failed=%d\n", checks, failures);
    return failures ? 1 : 0;
}
