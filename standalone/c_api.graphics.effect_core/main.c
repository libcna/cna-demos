/* SPDX-License-Identifier: MS-PL */
/*
 * Core Effect C API: an empty Effect exposes its owned reflection graph, current-technique rules,
 * clone/disposal behavior and retained child views. A real XNA Effect Framework binary exercises
 * format validation and the selected renderer's explicit compiled-effect capability; a capable
 * build additionally checks its reflected graph, pass application and clone.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct EffectCoreState {
    CnaDemo* demo;
    int loads;
    CNA_EffectHandle empty;
    CNA_EffectHandle compiled;
    CNA_EffectHandle clone;
    CNA_EffectParameterCollectionHandle empty_parameters;
    CNA_EffectTechniqueCollectionHandle empty_techniques;
    CNA_EffectTechniqueHandle empty_current;
    CNA_EffectTechniqueHandle empty_indexed;
    CNA_EffectTechniqueHandle foreign;
    CNA_EffectPassCollectionHandle empty_passes;
    CNA_EffectPassHandle empty_pass;
    CNA_EffectParameterCollectionHandle compiled_parameters;
    CNA_EffectTechniqueCollectionHandle compiled_techniques;
    CNA_EffectTechniqueHandle compiled_current;
    CNA_EffectTechniqueHandle compiled_first;
    CNA_EffectTechniqueHandle compiled_second;
    CNA_EffectPassCollectionHandle compiled_first_passes;
    CNA_EffectPassCollectionHandle compiled_second_passes;
    CNA_EffectPassHandle compiled_first_pass;
    CNA_EffectPassHandle compiled_second_pass;
} EffectCoreState;

/* Read the refusal before another effect call replaces the thread-local diagnostic. */
static int non_current_pass_is_refused(CNA_EffectPassHandle pass)
{
    static const char expected[] = "System.InvalidOperationException";
    CNA_ErrorInfo info = {0};
    char exception_type[sizeof(expected)] = {0};
    uint64_t needed = 0, copied = 0;
    CNA_Result result = cna_effect_pass_apply(pass);
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    if (result != CNA_RESULT_INVALID_STATE ||
        cna_error_get_last_info(&info) != CNA_RESULT_SUCCESS ||
        info.result != CNA_RESULT_INVALID_STATE || info.category != CNA_ERROR_CATEGORY_STATE ||
        cna_error_get_last_exception_type_size_ext(&needed) != CNA_RESULT_SUCCESS ||
        needed != sizeof(expected) - 1U ||
        cna_error_copy_last_exception_type_ext(exception_type, needed, &copied) !=
            CNA_RESULT_SUCCESS ||
        copied != needed || memcmp(exception_type, expected, sizeof(expected)) != 0) {
        return 0;
    }
    printf("  non-current pass diagnostic: %s (INVALID_STATE / STATE)\n", exception_type);
    return 1;
}

static CNA_StringView text_view(const char* text)
{
    CNA_StringView view;
    view.data = text;
    view.byte_length = (uint64_t)strlen(text);
    return view;
}

static int bytes_are(const void* data, size_t count, unsigned char value)
{
    const unsigned char* bytes = (const unsigned char*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static int expect_effect_text(
    CNA_EffectHandle effect,
    const char* expected,
    CNA_Result (*get_count)(CNA_EffectHandle, uint64_t*),
    CNA_Result (*copy)(CNA_EffectHandle, char*, uint64_t, uint64_t*))
{
    char destination[64];
    const uint64_t expected_count = (uint64_t)strlen(expected);
    uint64_t count = UINT64_MAX;
    memset(destination, 0x5A, sizeof(destination));
    if (get_count(effect, &count) != CNA_RESULT_SUCCESS || count != expected_count) {
        return 0;
    }
    if (expected_count > UINT64_C(0) &&
        (copy(effect, destination, expected_count - UINT64_C(1), &count) !=
             CNA_RESULT_BUFFER_TOO_SMALL ||
         count != expected_count || !bytes_are(destination, sizeof(destination), 0x5A))) {
        return 0;
    }
    if (copy(effect, destination, expected_count, &count) != CNA_RESULT_SUCCESS ||
        count != expected_count || memcmp(destination, expected, (size_t)expected_count) != 0) {
        return 0;
    }
    return expected_count == sizeof(destination) ||
        (unsigned char)destination[expected_count] == 0x5A;
}

static CNA_Result load_compiled_fixture(
    CNA_Handle game, uint8_t* bytes, size_t capacity, uint64_t* out_count)
{
    char path[1024];
    uint64_t path_count = UINT64_C(0);
    FILE* input;
    size_t count;
    int written;
    CNA_Result result = cna_title_location_copy_path(
        game, path, (uint64_t)sizeof(path) - UINT64_C(64), &path_count);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    written = snprintf(
        path + path_count, sizeof(path) - (size_t)path_count,
        "/assets/effects/CnaConformanceEffect.fxb");
    if (written < 0 || (size_t)written >= sizeof(path) - (size_t)path_count) {
        return CNA_RESULT_OVERFLOW;
    }
    input = fopen(path, "rb");
    if (input == NULL) {
        return CNA_RESULT_IO;
    }
    count = fread(bytes, 1U, capacity, input);
    if (ferror(input) != 0 || count == capacity) {
        (void)fclose(input);
        return CNA_RESULT_IO;
    }
    if (fclose(input) != 0) {
        return CNA_RESULT_IO;
    }
    *out_count = (uint64_t)count;
    return count > 0U ? CNA_RESULT_SUCCESS : CNA_RESULT_IO;
}

static CNA_Result inspect_empty_effect(
    EffectCoreState* state, CNA_Handle device)
{
    CnaDemo* demo = state->demo;
    CNA_EffectHandle empty_clone = CNA_INVALID_HANDLE;
    CNA_EffectParameterCollectionHandle clone_parameters = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueCollectionHandle clone_techniques = CNA_INVALID_HANDLE;
    CNA_Handle owner = CNA_INVALID_HANDLE;
    CNA_Bool is_compiled = CNA_TRUE;
    CNA_Bool has_renderer = CNA_TRUE;
    CNA_Bool is_sprite = CNA_TRUE;
    CNA_Bool disposed = CNA_FALSE;
    CNA_Bool found = CNA_FALSE;
    CNA_EffectTechniqueHandle retained = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle alternate = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle alternate_current = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle alternate_found = CNA_INVALID_HANDLE;
    uint64_t count = UINT64_MAX;
    uint64_t current_identity = UINT64_C(0);
    uint64_t indexed_identity = UINT64_C(0);
    CNA_Result result;

    if (!cna_demo_check_result(
            demo, cna_effect_create_empty(device, &state->empty),
            "create the minimal concrete Effect on the callback's graphics device")) {
        return CNA_RESULT_INVALID_STATE;
    }
    cna_demo_check(
        demo,
        cna_effect_get_graphics_device(state->empty, &owner) == CNA_RESULT_SUCCESS &&
            owner == device &&
            expect_effect_text(
                state->empty, "Microsoft.Xna.Framework.Graphics.Effect",
                cna_effect_get_type_name_byte_count, cna_effect_copy_type_name),
        "the empty Effect reports its borrowed owner and exact runtime type-name bytes");
    cna_demo_check(
        demo,
        expect_effect_text(
            state->empty, "", cna_effect_get_vertex_source_byte_count,
            cna_effect_copy_vertex_source) &&
            expect_effect_text(
                state->empty, "", cna_effect_get_fragment_source_byte_count,
                cna_effect_copy_fragment_source) &&
            cna_effect_get_is_compiled_ext(state->empty, &is_compiled) == CNA_RESULT_SUCCESS &&
            is_compiled == CNA_FALSE &&
            cna_effect_has_renderer(state->empty, &has_renderer) == CNA_RESULT_SUCCESS &&
            has_renderer == CNA_FALSE &&
            cna_effect_is_exact_stock_sprite_effect(state->empty, &is_sprite) ==
                CNA_RESULT_SUCCESS &&
            is_sprite == CNA_FALSE,
        "a hand-built base Effect has empty source, no compiled runtime, no renderer and no stock identity");

    result = cna_effect_get_parameters(state->empty, &state->empty_parameters);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_techniques(state->empty, &state->empty_techniques);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_current_technique(state->empty, &state->empty_current);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_collection_get_at(
            state->empty_techniques, UINT64_C(0), &state->empty_indexed);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_collection_get_count(state->empty_parameters, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_technique_collection_get_count(state->empty_techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_effect_technique_get_identity(state->empty_current, &current_identity) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(state->empty_indexed, &indexed_identity) ==
                CNA_RESULT_SUCCESS &&
            current_identity == indexed_identity,
        "the empty Effect exposes one current default technique and an empty parameter collection");

    result = cna_effect_technique_get_passes(state->empty_current, &state->empty_passes);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_pass_collection_get_at(
            state->empty_passes, UINT64_C(0), &state->empty_pass);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && cna_effect_apply(state->empty) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(state->empty_pass) == CNA_RESULT_SUCCESS,
        "base Effect and its effect-owned current P0 pass apply through the canonical dispatch");

    result = cna_effect_technique_create_named(text_view("Foreign"), &state->foreign);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_set_current_technique(state->empty, CNA_INVALID_HANDLE) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_effect_set_current_technique(state->empty, state->foreign) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_effect_get_current_technique(state->empty, &retained) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(retained, &current_identity) ==
                CNA_RESULT_SUCCESS &&
            current_identity == indexed_identity,
        "current technique refuses null and foreign values without changing the selection");
    if (retained != CNA_INVALID_HANDLE) {
        result = cna_effect_technique_destroy(retained);
        cna_demo_check(demo, result == CNA_RESULT_SUCCESS,
                       "the owned current-technique verification view releases cleanly");
    }

    found = CNA_FALSE;
    result = cna_effect_technique_collection_add_named(
        state->empty_techniques, text_view("Alternate"), &alternate);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_set_current_technique(state->empty, alternate);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_current_technique(state->empty, &alternate_current);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_technique_collection_find(
                state->empty_techniques, text_view("Alternate"), &found,
                &alternate_found) == CNA_RESULT_SUCCESS &&
            found == CNA_TRUE &&
            cna_effect_technique_get_identity(alternate, &current_identity) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(alternate_current, &indexed_identity) ==
                CNA_RESULT_SUCCESS &&
            current_identity == indexed_identity &&
            cna_effect_set_current_technique(state->empty, state->empty_indexed) ==
                CNA_RESULT_SUCCESS,
        "a technique added through the Effect's own collection is selectable and reads back by identity");
    if (alternate_found != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            demo, cna_effect_technique_destroy(alternate_found),
            "release the Alternate lookup view before compiled-effect inspection");
    }
    if (alternate_current != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            demo, cna_effect_technique_destroy(alternate_current),
            "release the Alternate current-technique view before compiled-effect inspection");
    }
    if (alternate != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            demo, cna_effect_technique_destroy(alternate),
            "release the Alternate insertion view before compiled-effect inspection");
    }

    result = cna_effect_clone(state->empty, &empty_clone);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_parameters(empty_clone, &clone_parameters);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_techniques(empty_clone, &clone_techniques);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_get_is_compiled_ext(empty_clone, &is_compiled) == CNA_RESULT_SUCCESS &&
            is_compiled == CNA_FALSE &&
            cna_effect_parameter_collection_get_count(clone_parameters, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_technique_collection_get_count(clone_techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) && cna_effect_apply(empty_clone) == CNA_RESULT_SUCCESS,
        "an empty Effect clone owns a fresh canonical graph and preserves the uncompiled state");
    if (empty_clone != CNA_INVALID_HANDLE) {
        result = cna_effect_dispose(empty_clone);
        cna_demo_check(
            demo,
            result == CNA_RESULT_SUCCESS &&
                cna_graphics_resource_get_is_disposed(empty_clone, &disposed) ==
                    CNA_RESULT_SUCCESS &&
                disposed == CNA_TRUE && cna_effect_apply(empty_clone) == CNA_RESULT_INVALID_STATE &&
                cna_effect_apply(state->empty) == CNA_RESULT_SUCCESS,
            "disposing the clone invalidates only its application while the source stays usable");
    }
    if (clone_techniques != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            demo, cna_effect_technique_collection_destroy(clone_techniques),
            "release the cloned technique collection");
    }
    if (clone_parameters != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            demo, cna_effect_parameter_collection_destroy(clone_parameters),
            "release the cloned parameter collection");
    }
    if (empty_clone != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            demo, cna_effect_destroy(empty_clone), "destroy the disposed empty Effect clone");
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result inspect_compiled_effect(
    EffectCoreState* state, CNA_Handle game, CNA_Handle device)
{
    static uint8_t code[8192];
    static const uint8_t mgfx[4] = {'M', 'G', 'F', 'X'};
    static const uint8_t malformed[4] = {1U, 2U, 3U, 4U};
    CnaDemo* demo = state->demo;
    CNA_EffectHandle refused = UINT64_MAX;
    CNA_EffectParameterCollectionHandle clone_parameters = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueCollectionHandle clone_techniques = CNA_INVALID_HANDLE;
    CNA_Handle owner = CNA_INVALID_HANDLE;
    CNA_Bool supported = CNA_FALSE;
    CNA_Bool is_compiled = CNA_FALSE;
    CNA_Bool has_renderer = CNA_FALSE;
    CNA_Bool is_sprite = CNA_TRUE;
    CNA_Bool disposed = CNA_FALSE;
    CNA_Bool found = CNA_FALSE;
    uint64_t code_count = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    uint64_t first_identity = UINT64_C(0);
    uint64_t current_identity = UINT64_C(0);
    CNA_Result result;

    result = load_compiled_fixture(game, code, sizeof(code), &code_count);
    cna_demo_check(
        demo, result == CNA_RESULT_SUCCESS && code_count == UINT64_C(5000),
        "load the committed 5000-byte XNA Effect Framework conformance fixture");
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    cna_demo_check(
        demo,
        cna_effect_create_compiled(device, NULL, UINT64_C(0), &refused) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            refused == CNA_INVALID_HANDLE &&
            cna_effect_create_compiled(device, malformed, sizeof(malformed), &refused) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            refused == CNA_INVALID_HANDLE,
        "the compiled constructor rejects empty and malformed bytecode without publishing a handle");
    cna_demo_check(
        demo,
        cna_effect_create_compiled(device, mgfx, sizeof(mgfx), &refused) ==
                CNA_RESULT_NOT_SUPPORTED &&
            refused == CNA_INVALID_HANDLE,
        "the compiled constructor identifies and refuses the distinct MGFX container atomically");

    result = cna_graphics_device_supports_capability(
        device, CNA_GRAPHICS_CAPABILITY_COMPILED_EFFECTS, &supported);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_create_compiled(device, code, code_count, &state->compiled);
    }
    if (supported == CNA_FALSE) {
        cna_demo_check(
            demo, result == CNA_RESULT_NOT_SUPPORTED && state->compiled == CNA_INVALID_HANDLE,
            "a renderer without compiled-effect capability refuses the valid fixture explicitly");
        return CNA_RESULT_SUCCESS;
    }
    if (!cna_demo_check_result(
            demo, result,
            "create a real compiled Effect on a renderer that advertises the capability")) {
        return result;
    }

    cna_demo_check(
        demo,
        cna_effect_get_graphics_device(state->compiled, &owner) == CNA_RESULT_SUCCESS &&
            owner == device &&
            cna_effect_get_is_compiled_ext(state->compiled, &is_compiled) == CNA_RESULT_SUCCESS &&
            is_compiled == CNA_TRUE &&
            cna_effect_has_renderer(state->compiled, &has_renderer) == CNA_RESULT_SUCCESS &&
            has_renderer == CNA_FALSE &&
            cna_effect_is_exact_stock_sprite_effect(state->compiled, &is_sprite) ==
                CNA_RESULT_SUCCESS &&
            is_sprite == CNA_FALSE &&
            expect_effect_text(
                state->compiled, "Microsoft.Xna.Framework.Graphics.Effect",
                cna_effect_get_type_name_byte_count, cna_effect_copy_type_name),
        "the compiled Effect reports its device, bytecode runtime, no source program and non-stock concrete identity");

    result = cna_effect_get_parameters(state->compiled, &state->compiled_parameters);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_techniques(state->compiled, &state->compiled_techniques);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_current_technique(state->compiled, &state->compiled_current);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_collection_get_at(
            state->compiled_techniques, UINT64_C(0), &state->compiled_first);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_collection_get_count(state->compiled_parameters, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(6) &&
            cna_effect_technique_collection_get_count(state->compiled_techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_effect_technique_get_identity(state->compiled_current, &current_identity) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(state->compiled_first, &first_identity) ==
                CNA_RESULT_SUCCESS &&
            current_identity == first_identity,
        "compiled reflection exposes six parameters and two techniques with the first selected");

    result = cna_effect_technique_get_passes(
        state->compiled_first, &state->compiled_first_passes);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_pass_collection_get_at(
            state->compiled_first_passes, UINT64_C(0), &state->compiled_first_pass);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(state->compiled_first_passes, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && cna_effect_apply(state->compiled) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(state->compiled_first_pass) == CNA_RESULT_SUCCESS,
        "the selected compiled technique owns two passes and its first pass applies successfully");

    found = CNA_FALSE;
    result = cna_effect_technique_collection_find(
        state->compiled_techniques, text_view("SecondTechnique"), &found,
        &state->compiled_second);
    if (result == CNA_RESULT_SUCCESS && found == CNA_TRUE) {
        result = cna_effect_set_current_technique(state->compiled, state->compiled_second);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_get_passes(
            state->compiled_second, &state->compiled_second_passes);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_pass_collection_get_at(
            state->compiled_second_passes, UINT64_C(0), &state->compiled_second_pass);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(state->compiled_second_passes, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            non_current_pass_is_refused(state->compiled_first_pass) &&
            cna_effect_apply(state->compiled) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(state->compiled_second_pass) == CNA_RESULT_SUCCESS,
        "switching current technique rejects the old owned pass and enables the new P1 pass");
    cna_demo_check(
        demo,
        cna_effect_set_current_technique(state->compiled, state->compiled_first) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(state->compiled_first_pass) == CNA_RESULT_SUCCESS,
        "restoring the first compiled technique makes its pass applicable again");

    result = cna_effect_clone(state->compiled, &state->clone);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_parameters(state->clone, &clone_parameters);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_get_techniques(state->clone, &clone_techniques);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_get_is_compiled_ext(state->clone, &is_compiled) == CNA_RESULT_SUCCESS &&
            is_compiled == CNA_TRUE &&
            cna_effect_parameter_collection_get_count(clone_parameters, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(6) &&
            cna_effect_technique_collection_get_count(clone_techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && cna_effect_apply(state->clone) == CNA_RESULT_SUCCESS,
        "a compiled clone owns an independent complete reflection graph and renderer runtime");
    if (clone_parameters != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_collection_destroy(clone_parameters);
    }
    if (clone_techniques != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_collection_destroy(clone_techniques);
    }

    result = cna_effect_dispose(state->clone);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_graphics_resource_get_is_disposed(state->clone, &disposed) == CNA_RESULT_SUCCESS &&
            disposed == CNA_TRUE && cna_effect_apply(state->clone) == CNA_RESULT_INVALID_STATE &&
            cna_effect_apply(state->compiled) == CNA_RESULT_SUCCESS,
        "disposing the clone invalidates only its application while the source remains operational");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    EffectCoreState* state = (EffectCoreState*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle destroyed_empty;
    uint64_t count = UINT64_C(0);
    CNA_Result result;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    result = cna_game_get_graphics_device(game, &device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = inspect_empty_effect(state, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }
    result = inspect_compiled_effect(state, game, device);
    if (result != CNA_RESULT_SUCCESS) {
        return result;
    }

    destroyed_empty = state->empty;
    result = cna_effect_destroy(state->empty);
    if (result == CNA_RESULT_SUCCESS) {
        state->empty = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        state->demo,
        result == CNA_RESULT_SUCCESS && cna_effect_apply(destroyed_empty) == CNA_RESULT_INVALID_HANDLE &&
            cna_effect_parameter_collection_get_count(state->empty_parameters, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_technique_collection_get_count(state->empty_techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && cna_effect_pass_apply(state->empty_pass) == CNA_RESULT_SUCCESS,
        "owned reflection views retain the native graph after the root Effect handle is destroyed");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context, CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

static void cleanup_effect_state(EffectCoreState* state)
{
    if (state->compiled_second_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(state->compiled_second_pass);
    }
    if (state->compiled_first_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(state->compiled_first_pass);
    }
    if (state->empty_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(state->empty_pass);
    }
    if (state->compiled_second_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(state->compiled_second_passes);
    }
    if (state->compiled_first_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(state->compiled_first_passes);
    }
    if (state->empty_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(state->empty_passes);
    }
    if (state->compiled_second != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(state->compiled_second);
    }
    if (state->compiled_first != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(state->compiled_first);
    }
    if (state->compiled_current != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(state->compiled_current);
    }
    if (state->empty_indexed != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(state->empty_indexed);
    }
    if (state->empty_current != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(state->empty_current);
    }
    if (state->foreign != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(state->foreign);
    }
    if (state->compiled_techniques != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_collection_destroy(state->compiled_techniques);
    }
    if (state->compiled_parameters != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_collection_destroy(state->compiled_parameters);
    }
    if (state->empty_techniques != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_collection_destroy(state->empty_techniques);
    }
    if (state->empty_parameters != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_collection_destroy(state->empty_parameters);
    }
    if (state->clone != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->clone);
    }
    if (state->compiled != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->compiled);
    }
    if (state->empty != CNA_INVALID_HANDLE) {
        (void)cna_effect_destroy(state->empty);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    EffectCoreState state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.effect_core", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    memset(&callbacks, 0, sizeof(callbacks));
    memset(&create_info, 0, sizeof(create_info));
    state.demo = &demo;
    state.empty = CNA_INVALID_HANDLE;
    state.compiled = CNA_INVALID_HANDLE;
    state.clone = CNA_INVALID_HANDLE;
    state.empty_parameters = CNA_INVALID_HANDLE;
    state.empty_techniques = CNA_INVALID_HANDLE;
    state.empty_current = CNA_INVALID_HANDLE;
    state.empty_indexed = CNA_INVALID_HANDLE;
    state.foreign = CNA_INVALID_HANDLE;
    state.empty_passes = CNA_INVALID_HANDLE;
    state.empty_pass = CNA_INVALID_HANDLE;
    state.compiled_parameters = CNA_INVALID_HANDLE;
    state.compiled_techniques = CNA_INVALID_HANDLE;
    state.compiled_current = CNA_INVALID_HANDLE;
    state.compiled_first = CNA_INVALID_HANDLE;
    state.compiled_second = CNA_INVALID_HANDLE;
    state.compiled_first_passes = CNA_INVALID_HANDLE;
    state.compiled_second_passes = CNA_INVALID_HANDLE;
    state.compiled_first_pass = CNA_INVALID_HANDLE;
    state.compiled_second_pass = CNA_INVALID_HANDLE;
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.is_fixed_time_step = CNA_FALSE;
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;

    if (!cna_demo_check_result(&demo, cna_game_create(&create_info, &game),
                               "create the owner game")) {
        return cna_demo_finish(&demo);
    }
    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the graphics device whose Effect graphs are audited");
    if (manager_ready) {
        cna_demo_check_result(
            &demo, cna_game_run(game), "run the one-callback Effect audit");
        cna_demo_check(
            &demo, state.loads == 1,
            "the core Effect audit ran in exactly one load callback");
    }
    cleanup_effect_state(&state);
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager after every Effect view is released");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
