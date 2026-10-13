/* SPDX-License-Identifier: MS-PL */
/* Vertex declaration ownership and static/dynamic vertex-buffer transfers from the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct VertexBufferDemo {
    CnaDemo* demo;
    int loads;
    int content_lost_count;
} VertexBufferDemo;

static const CNA_VertexElement PositionColorElements[2] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION, 0U},
    {12, CNA_VERTEX_ELEMENT_FORMAT_COLOR, CNA_VERTEX_ELEMENT_USAGE_COLOR, 0U}
};

static CNA_VertexBufferTransfer transfer(
    CNA_SetDataOptions options, uint64_t start, uint64_t count)
{
    const CNA_VertexBufferTransfer value = {
        (uint32_t)sizeof(CNA_VertexBufferTransfer), UINT32_C(1),
        CNA_VERTEX_TYPE_POSITION_COLOR, options, start, count
    };
    return value;
}

static CNA_VertexBufferCreateInfo create_info(
    CNA_VertexDeclarationHandle declaration, CNA_Bool dynamic)
{
    CNA_VertexBufferCreateInfo value;
    memset(&value, 0, sizeof(value));
    value.struct_size = (uint32_t)sizeof(value);
    value.struct_version = UINT32_C(1);
    value.vertex_declaration = declaration;
    value.vertex_count = 3;
    value.buffer_usage = CNA_BUFFER_USAGE_NONE;
    value.dynamic = dynamic;
    return value;
}

static int element_equal(CNA_VertexElement left, CNA_VertexElement right)
{
    return left.offset == right.offset && left.format == right.format &&
        left.usage == right.usage && left.usage_index == right.usage_index;
}

static int elements_equal(
    const CNA_VertexElement* left, const CNA_VertexElement* right, size_t count)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (!element_equal(left[index], right[index])) {
            return 0;
        }
    }
    return 1;
}

static int bytes_are(const void* data, size_t count, uint8_t expected)
{
    const uint8_t* bytes = (const uint8_t*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static void on_content_lost(CNA_VertexBufferHandle vertex_buffer, void* context)
{
    VertexBufferDemo* state = (VertexBufferDemo*)context;
    if (vertex_buffer != CNA_INVALID_HANDLE) {
        ++state->content_lost_count;
    }
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    VertexBufferDemo* state = (VertexBufferDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_VertexDeclarationHandle empty = CNA_INVALID_HANDLE;
    CNA_VertexDeclarationHandle inferred = CNA_INVALID_HANDLE;
    CNA_VertexDeclarationHandle explicit_stride = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle static_buffer = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle dynamic_buffer = CNA_INVALID_HANDLE;
    CNA_VertexBufferEventRegistrationHandle registration = CNA_INVALID_HANDLE;
    CNA_VertexElement caller_elements[2];
    CNA_VertexElement copied_elements[2];
    CNA_VertexElement element_guard[2];
    CNA_VertexPositionColor source[4] = {
        {{1.0f, 2.0f, 3.0f}, {4U, 5U, 6U, 7U}},
        {{11.0f, 12.0f, 13.0f}, {14U, 15U, 16U, 17U}},
        {{21.0f, 22.0f, 23.0f}, {24U, 25U, 26U, 27U}},
        {{31.0f, 32.0f, 33.0f}, {34U, 35U, 36U, 37U}}
    };
    const CNA_VertexPositionColor replacement = {
        {41.0f, 42.0f, 43.0f}, {44U, 45U, 46U, 47U}
    };
    CNA_VertexPositionColor destination[4];
    CNA_VertexPositionColor raw[3];
    CNA_VertexBufferInfo info;
    CNA_VertexBufferTransfer window;
    CNA_VertexBufferBinding binding;
    uint64_t count = UINT64_MAX;
    int32_t stride = -1;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    cna_demo_check(
        state->demo,
        cna_game_get_graphics_device(game, &device) == CNA_RESULT_SUCCESS &&
            device != CNA_INVALID_HANDLE,
        "borrow the live graphics device for vertex-buffer allocation");
    if (device == CNA_INVALID_HANDLE) {
        return CNA_RESULT_SUCCESS;
    }

    cna_demo_check(
        state->demo,
        cna_vertex_declaration_create_empty(&empty) == CNA_RESULT_SUCCESS &&
            empty != CNA_INVALID_HANDLE &&
            cna_vertex_declaration_get_stride(empty, &stride) == CNA_RESULT_SUCCESS &&
            stride == 0 &&
            cna_vertex_declaration_copy_elements(empty, NULL, 0U, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 0U,
        "the empty declaration has zero stride and no elements");

    memcpy(caller_elements, PositionColorElements, sizeof(caller_elements));
    cna_demo_check(
        state->demo,
        cna_vertex_declaration_create(caller_elements, 2U, &inferred) ==
                CNA_RESULT_SUCCESS &&
            inferred != CNA_INVALID_HANDLE,
        "create a declaration whose stride is inferred from two caller elements");
    caller_elements[0].offset = 99;
    memset(copied_elements, 0x5A, sizeof(copied_elements));
    memcpy(element_guard, copied_elements, sizeof(element_guard));
    count = UINT64_MAX;
    stride = -1;
    cna_demo_check(
        state->demo,
        inferred != CNA_INVALID_HANDLE &&
            cna_vertex_declaration_get_stride(inferred, &stride) == CNA_RESULT_SUCCESS &&
            stride == (int32_t)sizeof(CNA_VertexPositionColor) &&
            cna_vertex_declaration_copy_elements(inferred, copied_elements, 1U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == 2U &&
            memcmp(copied_elements, element_guard, sizeof(copied_elements)) == 0,
        "the inferred declaration reports its packed stride and copies atomically");
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        inferred != CNA_INVALID_HANDLE &&
            cna_vertex_declaration_copy_elements(inferred, copied_elements, 2U, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 2U && elements_equal(copied_elements, PositionColorElements, 2U),
        "the declaration owns the original element values after caller storage changes");

    cna_demo_check(
        state->demo,
        cna_vertex_declaration_create_with_stride(
            32, PositionColorElements, 2U, &explicit_stride) == CNA_RESULT_SUCCESS &&
            cna_vertex_declaration_get_stride(explicit_stride, &stride) == CNA_RESULT_SUCCESS &&
            stride == 32,
        "an explicit declaration preserves a stride larger than its packed elements");

    if (inferred != CNA_INVALID_HANDLE) {
        CNA_VertexBufferCreateInfo static_info = create_info(inferred, CNA_FALSE);
        CNA_VertexBufferCreateInfo dynamic_info = create_info(inferred, CNA_TRUE);
        cna_demo_check(
            state->demo,
            cna_vertex_buffer_create(device, &static_info, &static_buffer) ==
                    CNA_RESULT_SUCCESS &&
                cna_vertex_buffer_create(device, &dynamic_info, &dynamic_buffer) ==
                    CNA_RESULT_SUCCESS &&
                static_buffer != CNA_INVALID_HANDLE && dynamic_buffer != CNA_INVALID_HANDLE,
            "create readable static and dynamic buffers from the same declaration");
    }
    if (inferred != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_declaration_destroy(inferred),
            "destroy the source declaration after both buffers copy it");
        inferred = CNA_INVALID_HANDLE;
    }

    memset(&info, 0xA5, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        static_buffer != CNA_INVALID_HANDLE &&
            cna_vertex_buffer_get_info(static_buffer, &info) == CNA_RESULT_SUCCESS &&
            info.vertex_count == 3 && info.buffer_usage == CNA_BUFFER_USAGE_NONE &&
            info.dynamic == CNA_FALSE && info.is_content_lost == CNA_FALSE &&
            info.has_renderer == CNA_TRUE &&
            info.vertex_stride == (int32_t)sizeof(CNA_VertexPositionColor) &&
            info.vertex_element_count == 2U && info.reserved0 == 0U &&
            cna_vertex_buffer_copy_declaration_elements(
                static_buffer, copied_elements, 2U, &count) == CNA_RESULT_SUCCESS &&
            count == 2U && elements_equal(copied_elements, PositionColorElements, 2U),
        "the static buffer reports canonical metadata and its retained declaration");

    memset(&binding, 0xA5, sizeof(binding));
    cna_demo_check(
        state->demo,
        static_buffer != CNA_INVALID_HANDLE &&
            cna_vertex_buffer_binding_init(static_buffer, 1, 2, &binding) ==
                CNA_RESULT_SUCCESS &&
            binding.vertex_buffer == static_buffer && binding.vertex_offset == 1 &&
            binding.instance_frequency == 2,
        "the binding initializer preserves the buffer, vertex offset and instance frequency");

    window = transfer(CNA_SET_DATA_NONE, 1U, 3U);
    cna_demo_check(
        state->demo,
        static_buffer != CNA_INVALID_HANDLE &&
            cna_vertex_buffer_set_data(static_buffer, &window, source, 4U) ==
                CNA_RESULT_SUCCESS,
        "a typed upload reads a three-element window from the caller array");
    memset(destination, 0x5A, sizeof(destination));
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_get_data(static_buffer, &window, destination, 3U, &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == 3U && bytes_are(destination, sizeof(destination), UINT8_C(0x5A)),
        "a short typed destination reports the vertex count without partial output");
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_get_data(static_buffer, &window, destination, 4U, &count) ==
                CNA_RESULT_SUCCESS &&
            count == 3U && bytes_are(&destination[0], sizeof(destination[0]), UINT8_C(0x5A)) &&
            memcmp(&destination[1], &source[1], 3U * sizeof(source[0])) == 0,
        "typed readback writes the requested caller window from native vertex zero");

    cna_demo_check(
        state->demo,
        cna_vertex_buffer_set_data_raw(
            static_buffer, &source[1], 3U * sizeof(source[0]), 3U,
            (uint32_t)sizeof(source[0])) == CNA_RESULT_SUCCESS,
        "a full raw upload accepts the declaration's exact byte stride");
    memset(raw, 0, sizeof(raw));
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_get_data_raw(
            static_buffer, sizeof(source[0]), raw, sizeof(raw), 1U,
            (uint32_t)sizeof(source[0])) == CNA_RESULT_SUCCESS &&
            memcmp(&raw[0], &source[2], sizeof(raw[0])) == 0,
        "raw readback offsets into the native buffer rather than the caller array");
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_set_data_raw_at(
            static_buffer, 2U * sizeof(source[0]), &replacement, sizeof(replacement), 1U,
            (uint32_t)sizeof(replacement)) == CNA_RESULT_SUCCESS,
        "a raw buffer-side upload replaces only the final vertex");
    window = transfer(CNA_SET_DATA_NONE, 0U, 3U);
    memset(destination, 0, sizeof(destination));
    count = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_get_data(static_buffer, &window, destination, 3U, &count) ==
                CNA_RESULT_SUCCESS &&
            memcmp(&destination[0], &source[1], sizeof(source[0])) == 0 &&
            memcmp(&destination[1], &source[2], sizeof(source[0])) == 0 &&
            memcmp(&destination[2], &replacement, sizeof(replacement)) == 0,
        "the raw window leaves both preceding vertices unchanged");

    cna_demo_check(
        state->demo,
        cna_vertex_buffer_set_data_raw_with_options(
            static_buffer, source, 3U * sizeof(source[0]), 3U,
            (uint32_t)sizeof(source[0]), CNA_SET_DATA_DISCARD) ==
                CNA_RESULT_NOT_SUPPORTED &&
            cna_vertex_buffer_set_data_raw_at_with_options(
                static_buffer, 0U, source, sizeof(source[0]), 1U,
                (uint32_t)sizeof(source[0]), CNA_SET_DATA_NO_OVERWRITE) ==
                CNA_RESULT_NOT_SUPPORTED,
        "streaming options are refused by the static-buffer contract");

    window = transfer(CNA_SET_DATA_DISCARD, 0U, 3U);
    cna_demo_check(
        state->demo,
        dynamic_buffer != CNA_INVALID_HANDLE &&
            cna_vertex_buffer_set_data(dynamic_buffer, &window, source, 3U) ==
                CNA_RESULT_SUCCESS,
        "the typed route carries a discard hint to a dynamic buffer");
    cna_demo_check(
        state->demo,
        dynamic_buffer != CNA_INVALID_HANDLE &&
            cna_vertex_buffer_set_data_raw_with_options(
                dynamic_buffer, source, 3U * sizeof(source[0]), 3U,
                (uint32_t)sizeof(source[0]), CNA_SET_DATA_DISCARD) == CNA_RESULT_SUCCESS &&
            cna_vertex_buffer_set_data_raw_at_with_options(
                dynamic_buffer, sizeof(source[0]), &replacement, sizeof(replacement), 1U,
                (uint32_t)sizeof(replacement), CNA_SET_DATA_NO_OVERWRITE) ==
                CNA_RESULT_SUCCESS,
        "the dynamic buffer accepts full discard and windowed no-overwrite uploads");
    memset(raw, 0x5A, sizeof(raw));
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_get_data_raw(
            dynamic_buffer, 0U, raw, sizeof(raw) - 1U, 3U,
            (uint32_t)sizeof(raw[0])) == CNA_RESULT_INVALID_ARGUMENT &&
            bytes_are(raw, sizeof(raw), UINT8_C(0x5A)),
        "a short raw destination is refused without partial output");
    memset(raw, 0, sizeof(raw));
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_get_data_raw(
            dynamic_buffer, 0U, raw, sizeof(raw), 3U,
            (uint32_t)sizeof(raw[0])) == CNA_RESULT_SUCCESS &&
            memcmp(&raw[0], &source[0], sizeof(raw[0])) == 0 &&
            memcmp(&raw[1], &replacement, sizeof(raw[1])) == 0 &&
            memcmp(&raw[2], &source[2], sizeof(raw[2])) == 0,
        "dynamic raw readback confirms the optioned upload window and retained neighbors");

    registration = UINT64_MAX;
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_subscribe_content_lost(
            static_buffer, on_content_lost, state, &registration) ==
                CNA_RESULT_NOT_SUPPORTED &&
            registration == CNA_INVALID_HANDLE,
        "ContentLost subscriptions are restricted to dynamic vertex buffers");
    cna_demo_check(
        state->demo,
        cna_vertex_buffer_subscribe_content_lost(
            dynamic_buffer, on_content_lost, state, &registration) == CNA_RESULT_SUCCESS &&
            registration != CNA_INVALID_HANDLE && state->content_lost_count == 0,
        "own a dynamic ContentLost subscription without fabricating a renderer event");
    if (registration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_buffer_unsubscribe_content_lost(registration),
            "unsubscribe and destroy the dynamic ContentLost registration");
        registration = CNA_INVALID_HANDLE;
    }

    if (dynamic_buffer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_buffer_destroy(dynamic_buffer),
            "destroy the dynamic vertex buffer");
        dynamic_buffer = CNA_INVALID_HANDLE;
    }
    if (static_buffer != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_buffer_destroy(static_buffer),
            "destroy the static vertex buffer");
        static_buffer = CNA_INVALID_HANDLE;
    }
    if (explicit_stride != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_declaration_destroy(explicit_stride),
            "destroy the explicit-stride declaration");
        explicit_stride = CNA_INVALID_HANDLE;
    }
    if (empty != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            state->demo, cna_vertex_declaration_destroy(empty),
            "destroy the empty declaration");
        empty = CNA_INVALID_HANDLE;
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    (void)game_time;
    (void)context;
    (void)out_error;
    return cna_game_request_exit(game);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    VertexBufferDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int manager_ready;

    cna_demo_init(&demo, "c_api.graphics.vertex_buffer_data", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.context = &state;
    memset(&create, 0, sizeof(create));
    create.struct_size = (uint32_t)sizeof(create);
    create.struct_version = UINT32_C(1);
    create.is_fixed_time_step = CNA_FALSE;
    create.target_elapsed_time_ticks = INT64_C(166667);
    create.callbacks = &callbacks;

    if (!cna_demo_check_result(
            &demo, cna_game_create(&create, &game), "create the owner game")) {
        return cna_demo_finish(&demo);
    }
    manager_ready = cna_demo_check_result(
        &demo, cna_demo_use_hidef(game, &manager),
        "create the HiDef device used by both vertex buffers");
    if (manager_ready) {
        cna_demo_check_result(
            &demo, cna_game_run(game), "run one frame of vertex-buffer validation");
        cna_demo_check(
            &demo, state.loads == 1,
            "the vertex-buffer audit ran in exactly one load callback");
        cna_demo_check(
            &demo, state.content_lost_count == 0,
            "the selected renderer raised no unsupported ContentLost event");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
