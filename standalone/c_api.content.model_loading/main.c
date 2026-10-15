/* SPDX-License-Identifier: MS-PL */
#define _POSIX_C_SOURCE 200809L
/* Content-loaded Models, glTF material variants and both XNB processor-tag shapes in C. */
#include <CNA/C/cna.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
#include <unistd.h>
#endif

#include "CnaDemos/CApiDemo.h"

#define CUSTOM_TYPE_NAME "CNA.Demos.ModelTag"

typedef struct CustomTag {
    int32_t magic;
    float weight;
} CustomTag;

typedef struct Fixture {
    char root[256];
    char dictionary_path[320];
    char foreign_path[320];
    int ready;
} Fixture;

typedef struct State {
    CnaDemo* demo;
    CNA_Handle asset_manager;
    CNA_Handle fixture_manager;
    CNA_ModelHandle toy;
    CNA_Matrix view;
    CNA_Matrix projection;
    Fixture fixture;
    CustomTag custom;
    int custom_create_calls;
    int loads;
    int draws;
} State;

static const CNA_Color Background = {30U, 32U, 40U, 255U};

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static size_t push_seven_bit(uint8_t* data, size_t offset, uint32_t value)
{
    while (value >= UINT32_C(0x80)) {
        data[offset++] = (uint8_t)((value & UINT32_C(0x7f)) | UINT32_C(0x80));
        value >>= 7U;
    }
    data[offset++] = (uint8_t)value;
    return offset;
}

static size_t push_u32_le(uint8_t* data, size_t offset, uint32_t value)
{
    data[offset++] = (uint8_t)(value & UINT32_C(0xff));
    data[offset++] = (uint8_t)((value >> 8U) & UINT32_C(0xff));
    data[offset++] = (uint8_t)((value >> 16U) & UINT32_C(0xff));
    data[offset++] = (uint8_t)((value >> 24U) & UINT32_C(0xff));
    return offset;
}

static size_t push_f32_le(uint8_t* data, size_t offset, float value)
{
    uint32_t bits = 0U;
    memcpy(&bits, &value, sizeof(bits));
    return push_u32_le(data, offset, bits);
}

static size_t push_utf8(uint8_t* data, size_t offset, const char* text)
{
    const size_t length = strlen(text);
    offset = push_seven_bit(data, offset, (uint32_t)length);
    memcpy(data + offset, text, length);
    return offset + length;
}

static size_t push_reader_name(uint8_t* data, size_t offset, const char* name)
{
    const size_t length = strlen(name);
    offset = push_seven_bit(data, offset, (uint32_t)length);
    memcpy(data + offset, name, length);
    offset += length;
    return push_u32_le(data, offset, UINT32_C(0));
}

static size_t push_model_body(uint8_t* asset, size_t offset)
{
    int index;
    offset = push_u32_le(asset, offset, UINT32_C(1));
    offset = push_seven_bit(asset, offset, UINT32_C(2));
    offset = push_utf8(asset, offset, "Root");
    for (index = 0; index < 16; ++index) {
        offset = push_f32_le(asset, offset, (index % 5) == 0 ? 1.0F : 0.0F);
    }
    asset[offset++] = 0U;
    offset = push_u32_le(asset, offset, UINT32_C(0));
    offset = push_u32_le(asset, offset, UINT32_C(0));
    asset[offset++] = 1U;
    return offset;
}

static void finish_xnb(uint8_t* asset, size_t offset)
{
    asset[0] = (uint8_t)'X';
    asset[1] = (uint8_t)'N';
    asset[2] = (uint8_t)'B';
    asset[3] = (uint8_t)'w';
    asset[4] = 5U;
    asset[5] = 0U;
    (void)push_u32_le(asset, 6U, (uint32_t)offset);
}

static int write_bytes(const char* path, const uint8_t* data, size_t count)
{
    FILE* file = fopen(path, "wb");
    int ok;
    if (file == NULL) {
        return 0;
    }
    ok = fwrite(data, 1U, count, file) == count;
    return fclose(file) == 0 && ok;
}

static int write_dictionary_model(const char* path)
{
    uint8_t asset[768];
    size_t offset = 10U;
    offset = push_seven_bit(asset, offset, UINT32_C(4));
    offset = push_reader_name(asset, offset, "Microsoft.Xna.Framework.Content.ModelReader");
    offset = push_reader_name(asset, offset, "Microsoft.Xna.Framework.Content.StringReader");
    offset = push_reader_name(
        asset, offset,
        "Microsoft.Xna.Framework.Content.DictionaryReader`2[[System.String],[System.Object]]");
    offset = push_reader_name(
        asset, offset, "Microsoft.Xna.Framework.Content.BoundingSphereReader");
    offset = push_seven_bit(asset, offset, UINT32_C(0));
    offset = push_seven_bit(asset, offset, UINT32_C(1));
    offset = push_model_body(asset, offset);
    offset = push_seven_bit(asset, offset, UINT32_C(3));
    offset = push_u32_le(asset, offset, UINT32_C(1));
    offset = push_seven_bit(asset, offset, UINT32_C(2));
    offset = push_utf8(asset, offset, "BoundingSphere");
    offset = push_seven_bit(asset, offset, UINT32_C(4));
    offset = push_f32_le(asset, offset, 1.0F);
    offset = push_f32_le(asset, offset, 2.0F);
    offset = push_f32_le(asset, offset, 3.0F);
    offset = push_f32_le(asset, offset, 4.0F);
    finish_xnb(asset, offset);
    return write_bytes(path, asset, offset);
}

static int write_foreign_model(const char* path)
{
    uint8_t asset[768];
    char canonical[256];
    uint64_t bytes = 0U;
    size_t offset = 10U;
    if (cna_reflective_type_reader_copy_canonical_name(
            text_view(CUSTOM_TYPE_NAME), canonical, sizeof(canonical) - 1U, &bytes) !=
            CNA_RESULT_SUCCESS ||
        bytes >= sizeof(canonical)) {
        return 0;
    }
    canonical[bytes] = '\0';
    offset = push_seven_bit(asset, offset, UINT32_C(3));
    offset = push_reader_name(asset, offset, "Microsoft.Xna.Framework.Content.ModelReader");
    offset = push_reader_name(asset, offset, "Microsoft.Xna.Framework.Content.StringReader");
    offset = push_reader_name(asset, offset, canonical);
    offset = push_seven_bit(asset, offset, UINT32_C(0));
    offset = push_seven_bit(asset, offset, UINT32_C(1));
    offset = push_model_body(asset, offset);
    offset = push_seven_bit(asset, offset, UINT32_C(3));
    offset = push_u32_le(asset, offset, UINT32_C(7));
    offset = push_f32_le(asset, offset, 2.5F);
    finish_xnb(asset, offset);
    return write_bytes(path, asset, offset);
}

static CNA_Result create_custom_tag(void* context, void** out_object)
{
    State* state = (State*)context;
    ++state->custom_create_calls;
    memset(&state->custom, 0, sizeof(state->custom));
    *out_object = &state->custom;
    return CNA_RESULT_SUCCESS;
}

static int register_custom_tag_reader(State* state)
{
    CNA_ReflectiveTypeReaderBuilderHandle builder = CNA_INVALID_HANDLE;
    int ok;
    if (cna_reflective_type_reader_builder_create(
            text_view(CUSTOM_TYPE_NAME), create_custom_tag, state, &builder) !=
        CNA_RESULT_SUCCESS) {
        return 0;
    }
    ok = cna_reflective_type_reader_builder_add_field(
             builder, CNA_CONTENT_FIELD_INT32, offsetof(CustomTag, magic)) ==
             CNA_RESULT_SUCCESS &&
        cna_reflective_type_reader_builder_add_field(
            builder, CNA_CONTENT_FIELD_SINGLE, offsetof(CustomTag, weight)) ==
            CNA_RESULT_SUCCESS &&
        cna_reflective_type_reader_builder_register_shared(builder) == CNA_RESULT_SUCCESS;
    return cna_reflective_type_reader_builder_destroy(builder) == CNA_RESULT_SUCCESS && ok;
}

static int prepare_fixture(State* state)
{
#if defined(__linux__)
    char template_path[] = "/tmp/cna-demos-model-loading-XXXXXX";
    char* root = mkdtemp(template_path);
    if (root == NULL || strlen(root) >= sizeof(state->fixture.root)) {
        return 0;
    }
    memcpy(state->fixture.root, root, strlen(root) + 1U);
    state->fixture.ready = 1;
    if (snprintf(
            state->fixture.dictionary_path, sizeof(state->fixture.dictionary_path),
            "%s/tagged_dictionary.xnb", root) < 0 ||
        snprintf(
            state->fixture.foreign_path, sizeof(state->fixture.foreign_path),
            "%s/tagged_foreign.xnb", root) < 0) {
        return 0;
    }
    return register_custom_tag_reader(state) &&
        write_dictionary_model(state->fixture.dictionary_path) &&
        write_foreign_model(state->fixture.foreign_path);
#else
    (void)state;
    return 0;
#endif
}

static int cleanup_fixture(Fixture* fixture)
{
#if defined(__linux__)
    int ok = 1;
    if (!fixture->ready) {
        return 1;
    }
    if (unlink(fixture->dictionary_path) != 0) {
        ok = 0;
    }
    if (unlink(fixture->foreign_path) != 0) {
        ok = 0;
    }
    if (rmdir(fixture->root) != 0) {
        ok = 0;
    }
    fixture->ready = 0;
    return ok;
#else
    (void)fixture;
    return 1;
#endif
}

static int create_manager(CNA_Handle device, const char* root, CNA_Handle* out_manager)
{
    CNA_ContentManagerCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.root_directory = text_view(root);
    return cna_content_manager_create(device, &info, out_manager) == CNA_RESULT_SUCCESS &&
        cna_content_manager_register_builtin_loaders(*out_manager) == CNA_RESULT_SUCCESS;
}

static void audit_variant_metadata(State* state)
{
    CNA_ModelHandle rejected = UINT64_C(99);
    CNA_ObjectDictionaryHandle dictionary = UINT64_C(99);
    CNA_Bool present = CNA_TRUE;
    void* object = &state->custom;
    uint64_t count = UINT64_MAX;
    uint64_t bytes = UINT64_MAX;
    int32_t selected = INT32_C(99);
    char name[16];
    char before[16];

    cna_demo_check(
        state->demo,
        cna_content_manager_load_model(
            state->asset_manager, text_view("models/absent"), &rejected) == CNA_RESULT_IO &&
            rejected == CNA_INVALID_HANDLE &&
        cna_content_manager_load_model(
            state->asset_manager, text_view(""), &rejected) == CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "Model loading rejects missing and empty asset names without publishing a handle");
    if (!cna_demo_check_result(
            state->demo,
            cna_content_manager_load_model(
                state->asset_manager, text_view("models/loose_paint_toy"), &state->toy),
            "load the material-variant glTF as a Model")) {
        return;
    }
    cna_demo_check(
        state->demo,
        cna_model_get_material_variant_count_ext(state->toy, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
        cna_model_get_material_variant_ext(state->toy, &selected) == CNA_RESULT_SUCCESS &&
            selected == INT32_C(-1),
        "the loaded glTF exposes two variants and starts on its default materials");

    memset(name, '#', sizeof(name));
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        state->demo,
        cna_model_get_material_variant_name_byte_count_ext(
            state->toy, UINT64_C(0), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(5) &&
        cna_model_copy_material_variant_name_ext(
            state->toy, UINT64_C(0), name, UINT64_C(4), &bytes) ==
            CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == UINT64_C(5) && memcmp(name, before, sizeof(name)) == 0 &&
        cna_model_copy_material_variant_name_ext(
            state->toy, UINT64_C(0), name, sizeof(name), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(5) && memcmp(name, "Ocean", 5U) == 0 && name[5] == '#',
        "Ocean reports exact UTF-8 bytes and a short destination remains untouched");
    memset(name, '#', sizeof(name));
    cna_demo_check(
        state->demo,
        cna_model_get_material_variant_name_byte_count_ext(
            state->toy, UINT64_C(1), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(4) &&
        cna_model_copy_material_variant_name_ext(
            state->toy, UINT64_C(1), name, sizeof(name), &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(4) && memcmp(name, "Gold", 4U) == 0 && name[4] == '#' &&
        cna_model_get_material_variant_name_byte_count_ext(
            state->toy, UINT64_C(2), &bytes) == CNA_RESULT_INVALID_ARGUMENT,
        "Gold follows Ocean in source order and the end index is refused");
    cna_demo_check(
        state->demo,
        cna_model_set_material_variant_ext(state->toy, INT32_C(0)) == CNA_RESULT_SUCCESS &&
        cna_model_get_material_variant_ext(state->toy, &selected) == CNA_RESULT_SUCCESS &&
            selected == INT32_C(0) &&
        cna_model_set_material_variant_ext(state->toy, INT32_C(1)) == CNA_RESULT_SUCCESS &&
        cna_model_get_material_variant_ext(state->toy, &selected) == CNA_RESULT_SUCCESS &&
            selected == INT32_C(1),
        "both imported material variants are selectable and observable");
    cna_demo_check(
        state->demo,
        cna_model_set_material_variant_ext(state->toy, INT32_C(2)) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        cna_model_set_material_variant_ext(state->toy, INT32_C(-2)) ==
            CNA_RESULT_INVALID_ARGUMENT &&
        cna_model_get_material_variant_ext(state->toy, &selected) == CNA_RESULT_SUCCESS &&
            selected == INT32_C(1) &&
        cna_model_set_material_variant_ext(state->toy, INT32_C(-1)) == CNA_RESULT_SUCCESS,
        "out-of-range selections are atomic and -1 restores default materials");

    cna_demo_check(
        state->demo,
        cna_model_get_content_tag_dictionary_ext(state->toy, &present, &dictionary) ==
            CNA_RESULT_SUCCESS &&
            present == CNA_FALSE && dictionary == CNA_INVALID_HANDLE &&
        cna_model_get_content_tag_foreign_object_ext(state->toy, &present, &object) ==
            CNA_RESULT_SUCCESS &&
            present == CNA_FALSE && object == NULL,
        "the untagged glTF distinguishes an absent processor tag from a load failure");
}

static void audit_dictionary_tag(State* state)
{
    CNA_ModelHandle model = CNA_INVALID_HANDLE;
    CNA_ObjectDictionaryHandle dictionary = CNA_INVALID_HANDLE;
    CNA_BoundingSphere sphere;
    CNA_Bool present = CNA_FALSE;
    void* object = &state->custom;
    uint64_t count = 0U;

    if (!cna_demo_check_result(
            state->demo,
            cna_content_manager_load_model(
                state->fixture_manager, text_view("tagged_dictionary"), &model),
            "load the dictionary-tagged XNB Model")) {
        return;
    }
    cna_demo_check(
        state->demo,
        cna_model_get_content_tag_dictionary_ext(model, &present, &dictionary) ==
            CNA_RESULT_SUCCESS &&
            present == CNA_TRUE && dictionary != CNA_INVALID_HANDLE &&
        cna_model_get_content_tag_foreign_object_ext(model, &present, &object) ==
            CNA_RESULT_SUCCESS &&
            present == CNA_FALSE && object == NULL,
        "a Dictionary<string, object> Model tag selects only its matching public route");
    cna_demo_check_result(
        state->demo, cna_model_destroy(model),
        "release the tagged Model before inspecting its independently owned dictionary");
    model = CNA_INVALID_HANDLE;
    cna_demo_check(
        state->demo,
        cna_object_dictionary_ext_get_count(dictionary, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
        cna_object_dictionary_ext_copy_value(
            dictionary, text_view("BoundingSphere"),
            CNA_OBJECT_DICTIONARY_VALUE_BOUNDING_SPHERE, &sphere, sizeof(sphere)) ==
            CNA_RESULT_SUCCESS &&
            sphere.center.x == 1.0F && sphere.center.y == 2.0F &&
            sphere.center.z == 3.0F && sphere.radius == 4.0F,
        "the dictionary tag outlives its Model and preserves the processor's exact sphere");
    cna_demo_check_result(
        state->demo, cna_object_dictionary_ext_destroy(dictionary),
        "destroy the independently owned Model tag dictionary");
}

static void audit_foreign_tag(State* state)
{
    CNA_ModelHandle model = CNA_INVALID_HANDLE;
    CNA_ObjectDictionaryHandle dictionary = UINT64_C(99);
    CNA_Bool present = CNA_FALSE;
    void* object = NULL;

    if (!cna_demo_check_result(
            state->demo,
            cna_content_manager_load_model(
                state->fixture_manager, text_view("tagged_foreign"), &model),
            "load the caller-reader-tagged XNB Model")) {
        return;
    }
    cna_demo_check(
        state->demo,
        cna_model_get_content_tag_foreign_object_ext(model, &present, &object) ==
            CNA_RESULT_SUCCESS &&
            present == CNA_TRUE && object == &state->custom &&
            state->custom_create_calls == 1 && state->custom.magic == INT32_C(7) &&
            state->custom.weight == 2.5F &&
        cna_model_get_content_tag_dictionary_ext(model, &present, &dictionary) ==
            CNA_RESULT_SUCCESS &&
            present == CNA_FALSE && dictionary == CNA_INVALID_HANDLE,
        "a shared reflective Model tag returns the caller's exact object only through its route");
    cna_demo_check_result(
        state->demo, cna_model_destroy(model),
        "release the foreign-tagged Model while the caller-owned tag remains valid");
    cna_demo_check(
        state->demo,
        state->custom.magic == INT32_C(7) && state->custom.weight == 2.5F,
        "destroying the Model neither frees nor mutates its caller-owned foreign tag");
}

static CNA_Result read_pixel(CNA_Handle device, int32_t x, int32_t y, CNA_Color* out_color)
{
    CNA_BackBufferReadback readback;
    memset(&readback, 0, sizeof(readback));
    readback.struct_size = (uint32_t)sizeof(readback);
    readback.struct_version = UINT32_C(1);
    readback.has_source_rectangle = CNA_TRUE;
    readback.source_rectangle = (CNA_Rectangle){x, y, 1, 1};
    readback.element_count = UINT64_C(1);
    return cna_graphics_device_get_backbuffer_data_window(
        device, &readback, out_color, UINT64_C(1));
}

static int render_variant(State* state, CNA_Handle device, int32_t variant, CNA_Color* body)
{
    CNA_Matrix world;
    return cna_matrix_get_identity(&world) == CNA_RESULT_SUCCESS &&
        cna_model_set_material_variant_ext(state->toy, variant) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_clear_color_depth(device, Background, 1.0F) == CNA_RESULT_SUCCESS &&
        cna_model_draw(state->toy, world, state->view, state->projection) == CNA_RESULT_SUCCESS &&
        read_pixel(device, INT32_C(400), INT32_C(200), body) == CNA_RESULT_SUCCESS;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_DepthStencilState depth;
    CNA_RasterizerState rasterizer;
    CNA_BlendState blend;
    CNA_SamplerState sampler;
    (void)game_time;
    (void)out_error;
    ++state->loads;
    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the graphics device for content-loaded Models")) {
        return CNA_RESULT_SUCCESS;
    }
    if (!cna_demo_check(
            state->demo,
            create_manager(device, "assets", &state->asset_manager) &&
                prepare_fixture(state) &&
                create_manager(device, state->fixture.root, &state->fixture_manager),
            "create asset and private-fixture ContentManagers with their built-in readers")) {
        return CNA_RESULT_SUCCESS;
    }
    audit_variant_metadata(state);
    audit_dictionary_tag(state);
    audit_foreign_tag(state);
    cna_demo_check(
        state->demo,
        cna_matrix_create_look_at(
            (CNA_Vector3){0.0F, 2.0F, 4.5F}, (CNA_Vector3){0.0F, 0.4F, 0.0F},
            (CNA_Vector3){0.0F, 1.0F, 0.0F}, &state->view) == CNA_RESULT_SUCCESS &&
        cna_matrix_create_perspective_field_of_view(
            0.7853981634F, 800.0F / 480.0F, 0.1F, 100.0F,
            &state->projection) == CNA_RESULT_SUCCESS &&
        cna_depth_stencil_state_init(
            CNA_DEPTH_STENCIL_STATE_PRESET_DEFAULT, &depth) == CNA_RESULT_SUCCESS &&
        cna_rasterizer_state_init(
            CNA_RASTERIZER_STATE_PRESET_CULL_COUNTER_CLOCKWISE, &rasterizer) ==
            CNA_RESULT_SUCCESS &&
        cna_blend_state_init(CNA_BLEND_STATE_PRESET_OPAQUE, &blend) == CNA_RESULT_SUCCESS &&
        cna_sampler_state_init(CNA_SAMPLER_STATE_PRESET_POINT_CLAMP, &sampler) ==
            CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_depth_stencil_state(device, &depth) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_rasterizer_state(device, &rasterizer) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_blend_state(device, &blend) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_set_sampler_state(
            device, CNA_SHADER_STAGE_PIXEL, UINT32_C(0), &sampler) == CNA_RESULT_SUCCESS,
        "prepare the authored camera and deterministic PointClamp Model drawing state");
    return CNA_RESULT_SUCCESS;
}

static CNA_Result update(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    (void)game_time;
    (void)out_error;
    if (state->draws != 0) {
        return cna_game_request_exit(game);
    }
    return CNA_RESULT_SUCCESS;
}

static CNA_Result draw(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    State* state = (State*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Color red = {0U, 0U, 0U, 0U};
    CNA_Color blue = {0U, 0U, 0U, 0U};
    CNA_Color gold = {0U, 0U, 0U, 0U};
    CNA_Color wheel = {255U, 255U, 255U, 255U};
    CNA_Matrix world;
    int rendered;
    (void)game_time;
    (void)out_error;
    ++state->draws;
    if (cna_game_get_graphics_device(game, &device) != CNA_RESULT_SUCCESS ||
        state->toy == CNA_INVALID_HANDLE) {
        return CNA_RESULT_INVALID_STATE;
    }
    rendered = render_variant(state, device, INT32_C(-1), &red) &&
        render_variant(state, device, INT32_C(0), &blue) &&
        render_variant(state, device, INT32_C(1), &gold) &&
        cna_matrix_get_identity(&world) == CNA_RESULT_SUCCESS &&
        read_pixel(device, INT32_C(295), INT32_C(300), &wheel) == CNA_RESULT_SUCCESS;
    cna_demo_check(
        state->demo,
        rendered && red.r > 180U && red.b < 60U &&
            blue.b > blue.r + 40U && blue.b > blue.g + 40U &&
            gold.r > 200U && gold.g > 150U && gold.b < 80U,
        "default, Ocean and Gold draw red, blue-dominant PBR and clamped gold body pixels");
    cna_demo_check(
        state->demo,
        rendered && wheel.r < 40U && wheel.g < 40U && wheel.b < 40U,
        "the wheels without a variant mapping retain their black material");
    cna_demo_check_result(
        state->demo, cna_model_set_material_variant_ext(state->toy, INT32_C(-1)),
        "restore the default material after the pixel audit");
    return CNA_RESULT_SUCCESS;
}

static int cleanup_state(State* state)
{
    int ok = 1;
    if (state->toy != CNA_INVALID_HANDLE) {
        ok &= cna_model_destroy(state->toy) == CNA_RESULT_SUCCESS;
        state->toy = CNA_INVALID_HANDLE;
    }
    if (state->fixture_manager != CNA_INVALID_HANDLE) {
        ok &= cna_content_manager_destroy(state->fixture_manager) == CNA_RESULT_SUCCESS;
        state->fixture_manager = CNA_INVALID_HANDLE;
    }
    if (state->asset_manager != CNA_INVALID_HANDLE) {
        ok &= cna_content_manager_destroy(state->asset_manager) == CNA_RESULT_SUCCESS;
        state->asset_manager = CNA_INVALID_HANDLE;
    }
    ok &= cleanup_fixture(&state->fixture);
    return ok;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    State state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int device_ready;

    cna_demo_init(&demo, "c_api.content.model_loading", argc, argv, 1);
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.asset_manager = CNA_INVALID_HANDLE;
    state.fixture_manager = CNA_INVALID_HANDLE;
    state.toy = CNA_INVALID_HANDLE;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.update = update;
    callbacks.draw = draw;
    callbacks.context = &state;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.is_fixed_time_step = CNA_TRUE;
    info.target_elapsed_time_ticks = INT64_C(166667);
    info.callbacks = &callbacks;

    if (!cna_demo_check_result(
            &demo, cna_game_create(&info, &game),
            "create the content-loaded Model owner game")) {
        return cna_demo_finish(&demo);
    }
    device_ready = cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_graphics_profile(
            manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_preferred_back_buffer_width(
            manager, INT32_C(800)) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_set_preferred_back_buffer_height(
            manager, INT32_C(480)) == CNA_RESULT_SUCCESS &&
        cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    if (cna_demo_check(
            &demo, device_ready,
            "create an explicit HiDef 800x480 graphics device")) {
        cna_demo_check_result(
            &demo, cna_game_run(game),
            "run the complete content-loaded Model audit");
        cna_demo_check(
            &demo, state.loads == 1 && state.draws == 1,
            "the audit used exactly one load and one draw callback");
    }
    cna_demo_check(
        &demo, cleanup_state(&state),
        "release the loaded Model, both ContentManagers and private fixtures");
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
