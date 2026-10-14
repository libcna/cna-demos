/* SPDX-License-Identifier: MS-PL */
/* Morph-target ownership, tracks, CPU blending and ModelMeshPart upload through C. */
#include <CNA/C/cna.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct MorphDemo {
    CnaDemo* demo;
    CNA_MorphTargetDataEXTHandle data;
    int loads;
} MorphDemo;

static int nearly_equal(float left, float right)
{
    return fabsf(left - right) < 0.00001F;
}

static int audit_data(CnaDemo* demo, CNA_MorphTargetDataEXTHandle data)
{
    static const char TypeName[] =
        "Microsoft.Xna.Framework.Graphics.MorphTargetDataEXT";
    const float changed_weights[2] = {0.25F, 0.75F};
    const float blend_weights[2] = {1.0F, 1.0F};
    const float replacement_weights[2] = {0.5F, 0.25F};
    const CNA_MorphWeightKeyframeEXTDescriptor replacement_key = {
        2.0, replacement_weights, UINT64_C(2), NULL, UINT64_C(0), NULL, UINT64_C(0)};
    CNA_MorphWeightTrackEXTDescriptor replacement_track = {
        &replacement_key, UINT64_C(1), CNA_TRUE, CNA_FALSE};
    const CNA_Vector3 tangent[1] = {{0.5F, 0.0F, -0.5F}};
    const uint32_t triangle[3] = {0U, 1U, 2U};
    char type_name[sizeof(TypeName) - 1U];
    uint8_t base[32];
    uint8_t blended[32];
    uint8_t short_bytes[4] = {0xA5U, 0xA5U, 0xA5U, 0xA5U};
    float base_values[8];
    CNA_Vector3 vector[1];
    float weights[2] = {-1.0F, -1.0F};
    float key_weights[2] = {-1.0F, -1.0F};
    float in_tangents[2] = {-1.0F, -1.0F};
    float out_tangents[2] = {-1.0F, -1.0F};
    float position[3];
    float normal[3];
    uint32_t indices[3] = {9U, 9U, 9U};
    CNA_Bool step = CNA_TRUE;
    CNA_Bool cubic = CNA_FALSE;
    CNA_Bool recompute = CNA_TRUE;
    uint64_t count = UINT64_MAX;
    uint64_t count1 = UINT64_MAX;
    uint64_t count2 = UINT64_MAX;
    double seconds = -1.0;
    int32_t stride = INT32_C(0);

    cna_demo_check(
        demo,
        cna_morph_target_data_ext_get_type_name_byte_count(data, &count) ==
                CNA_RESULT_SUCCESS &&
            count == sizeof(TypeName) - 1U &&
            cna_morph_target_data_ext_copy_type_name(
                data, type_name, sizeof(type_name), &count) == CNA_RESULT_SUCCESS &&
            memcmp(type_name, TypeName, sizeof(type_name)) == 0 &&
            cna_morph_target_data_ext_get_stride(data, &stride) == CNA_RESULT_SUCCESS &&
            stride == INT32_C(32),
        "the owned object exposes its exact native identity and 32-byte vertex stride");
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_get_base_vertex_byte_count(data, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(32) &&
            cna_morph_target_data_ext_copy_base_vertex_bytes(
                data, short_bytes, sizeof(short_bytes), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            short_bytes[0] == 0xA5U && short_bytes[3] == 0xA5U &&
            cna_morph_target_data_ext_copy_base_vertex_bytes(
                data, base, sizeof(base), &count) == CNA_RESULT_SUCCESS &&
            (memcpy(base_values, base, sizeof(base_values)), base_values[5] == 1.0F),
        "base-pose bytes are deeply owned and short copies are atomic");
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_get_target_count(data, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_morph_target_data_ext_copy_position_deltas(
                data, UINT64_C(0), vector, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            vector[0].z == 1.0F &&
            cna_morph_target_data_ext_copy_normal_deltas(
                data, UINT64_C(0), NULL, UINT64_C(0), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_morph_target_data_ext_copy_position_deltas(
                data, UINT64_C(1), vector, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            vector[0].x == 2.0F &&
            cna_morph_target_data_ext_copy_normal_deltas(
                data, UINT64_C(1), vector, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            vector[0].x == 1.0F,
        "both position streams and the optional normal stream retain exact target order");
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_copy_weights(
            data, weights, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            weights[0] == 0.0F && weights[1] == 0.0F &&
            cna_morph_target_data_ext_set_weights(
                data, changed_weights, UINT64_C(1)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_morph_target_data_ext_set_weights(
                data, changed_weights, UINT64_C(2)) == CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_copy_weights(
                data, weights, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            weights[0] == 0.25F && weights[1] == 0.75F,
        "current weights replace atomically and must match the target count");
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_get_weight_track_info(
            data, &count, &step, &cubic) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && step == CNA_FALSE && cubic == CNA_TRUE &&
            cna_morph_target_data_ext_copy_weight_keyframe(
                data, UINT64_C(1), &seconds, key_weights, UINT64_C(2), &count,
                in_tangents, UINT64_C(2), &count1, out_tangents, UINT64_C(2), &count2) ==
                CNA_RESULT_SUCCESS &&
            seconds == 1.0 && count == UINT64_C(2) && count1 == UINT64_C(2) &&
            count2 == UINT64_C(2) && key_weights[0] == 1.0F &&
            in_tangents[0] == 0.0F && out_tangents[1] == 0.0F,
        "the copied cubic track preserves flags, time, weights and both tangent arrays");
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_set_weight_track(data, &replacement_track) ==
                CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_get_weight_track_info(
                data, &count, &step, &cubic) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) && step == CNA_TRUE && cubic == CNA_FALSE &&
            cna_morph_target_data_ext_copy_weight_keyframe(
                data, UINT64_C(0), &seconds, key_weights, UINT64_C(2), &count,
                NULL, UINT64_C(0), &count1, NULL, UINT64_C(0), &count2) ==
                CNA_RESULT_SUCCESS &&
            seconds == 2.0 && key_weights[0] == 0.5F && key_weights[1] == 0.25F &&
            count1 == UINT64_C(0) && count2 == UINT64_C(0),
        "weight-track replacement deeply copies a step key without optional tangents");
    replacement_track.step_interpolation = UINT8_C(2);
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_set_weight_track(data, &replacement_track) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "a noncanonical track boolean is refused");

    cna_demo_check(
        demo,
        cna_morph_target_data_ext_blend(
            data, blend_weights, UINT64_C(2), short_bytes, sizeof(short_bytes), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            short_bytes[0] == 0xA5U &&
            cna_morph_target_data_ext_blend(
                data, blend_weights, UINT64_C(2), blended, sizeof(blended), &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(32),
        "CPU blending reports its full vertex size and never partially writes");
    memcpy(position, blended, sizeof(position));
    memcpy(normal, blended + 12U, sizeof(normal));
    cna_demo_check(
        demo,
        position[0] == 2.0F && position[1] == 0.0F && position[2] == 1.0F &&
            nearly_equal(normal[0], 0.70710678F) && normal[1] == 0.0F &&
            nearly_equal(normal[2], 0.70710678F) &&
            memcmp(base + 24U, blended + 24U, 8U) == 0,
        "blending adds both positions, renormalizes the normal and preserves unrelated UV bytes");

    cna_demo_check(
        demo,
        cna_morph_target_data_ext_copy_tangent_deltas(
            data, UINT64_C(0), NULL, UINT64_C(0), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_morph_target_data_ext_set_tangent_deltas(
                data, UINT64_C(0), tangent, UINT64_C(1)) == CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_copy_tangent_deltas(
                data, UINT64_C(0), vector, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            vector[0].x == 0.5F && vector[0].z == -0.5F &&
            cna_morph_target_data_ext_set_tangent_deltas(
                data, UINT64_C(0), NULL, UINT64_C(0)) == CNA_RESULT_SUCCESS,
        "optional three-component tangent deltas attach, copy and clear independently");
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_get_recompute_flat_normals_ext(data, &recompute) ==
                CNA_RESULT_SUCCESS &&
            recompute == CNA_FALSE &&
            cna_morph_target_data_ext_set_recompute_flat_normals_ext(data, CNA_TRUE) ==
                CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_set_triangle_indices_ext(
                data, triangle, UINT64_C(3)) == CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_copy_triangle_indices_ext(
                data, indices, UINT64_C(3), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(3) && indices[0] == 0U && indices[2] == 2U,
        "flat-normal recomputation retains one complete triangle-list face");
    indices[0] = 9U;
    cna_demo_check(
        demo,
        cna_morph_target_data_ext_copy_triangle_indices_ext(
            data, indices, UINT64_C(2), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(3) && indices[0] == 9U &&
            cna_morph_target_data_ext_set_triangle_indices_ext(
                data, triangle, UINT64_C(2)) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_morph_target_data_ext_set_recompute_flat_normals_ext(
                data, UINT8_C(2)) == CNA_RESULT_INVALID_ARGUMENT,
        "short triangle transfers, incomplete faces and noncanonical flags are atomic refusals");
    return 1;
}

static int audit_track_evaluation(CnaDemo* demo)
{
    const float start[1] = {0.0F};
    const float end[1] = {1.0F};
    const float zero[1] = {0.0F};
    CNA_MorphWeightKeyframeEXTDescriptor keys[2] = {
        {0.0, start, UINT64_C(1), zero, UINT64_C(1), zero, UINT64_C(1)},
        {1.0, end, UINT64_C(1), zero, UINT64_C(1), zero, UINT64_C(1)}};
    CNA_MorphWeightTrackEXTDescriptor track = {
        keys, UINT64_C(2), CNA_FALSE, CNA_FALSE};
    float output[1] = {-1.0F};
    uint64_t count = UINT64_MAX;

    cna_demo_check(
        demo,
        cna_morph_weight_track_ext_evaluate(
            &track, 0.25, output, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) && nearly_equal(output[0], 0.25F),
        "linear track evaluation interpolates the borrowed endpoints");
    track.step_interpolation = CNA_TRUE;
    cna_demo_check(
        demo,
        cna_morph_weight_track_ext_evaluate(
            &track, 0.75, output, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            output[0] == 0.0F,
        "step evaluation holds the lower key");
    track.step_interpolation = CNA_FALSE;
    track.cubic_spline = CNA_TRUE;
    cna_demo_check(
        demo,
        cna_morph_weight_track_ext_evaluate(
            &track, 0.25, output, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            nearly_equal(output[0], 0.15625F) &&
            cna_morph_weight_track_ext_evaluate(
                &track, -1.0, output, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            output[0] == 0.0F &&
            cna_morph_weight_track_ext_evaluate(
                &track, 2.0, output, UINT64_C(1), &count) == CNA_RESULT_SUCCESS &&
            output[0] == 1.0F,
        "cubic evaluation uses tangents and clamps outside the authored interval");
    cna_demo_check(
        demo,
        cna_morph_weight_track_ext_evaluate(
            &track, NAN, output, UINT64_C(1), &count) == CNA_RESULT_INVALID_ARGUMENT,
        "non-finite evaluation time is refused");
    return 1;
}

static CNA_Result load_content(
    CNA_Handle game, const CNA_GameTime* game_time, void* context,
    CNA_CallbackError* out_error)
{
    MorphDemo* state = (MorphDemo*)context;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle part = CNA_INVALID_HANDLE;
    CNA_MorphTargetDataEXTHandle alias = CNA_INVALID_HANDLE;
    CNA_VertexBufferHandle buffer = CNA_INVALID_HANDLE;
    CNA_VertexBufferCreateInfo info;
    CNA_Bool has = CNA_TRUE;
    const float uploaded[2] = {0.5F, 0.25F};
    float copied[2] = {0.0F, 0.0F};
    uint8_t vertex[32];
    float position[3];
    uint64_t count = UINT64_MAX;
    int ok;
    (void)game_time;
    (void)out_error;
    ++state->loads;

    if (!cna_demo_check_result(
            state->demo, cna_game_get_graphics_device(game, &device),
            "borrow the device for one direct morph VertexBuffer upload")) {
        return CNA_RESULT_SUCCESS;
    }
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.vertex_count = 1;
    ok = cna_model_mesh_part_create_default(&part) == CNA_RESULT_SUCCESS &&
         cna_model_mesh_part_get_morph_target_data_ext(part, &has, &alias) ==
             CNA_RESULT_SUCCESS &&
         has == CNA_FALSE && alias == CNA_INVALID_HANDLE &&
         cna_model_mesh_part_set_morph_weights_ext(part, uploaded, UINT64_C(2)) ==
             CNA_RESULT_INVALID_STATE &&
         cna_vertex_buffer_create(device, &info, &buffer) == CNA_RESULT_SUCCESS &&
         cna_model_mesh_part_set_vertex_buffer(part, buffer) == CNA_RESULT_SUCCESS;
    cna_demo_check(
        state->demo, ok,
        "an unattached part refuses weights before receiving data and a vertex buffer");
    if (!ok) {
        return CNA_RESULT_SUCCESS;
    }
    cna_demo_check(
        state->demo,
        cna_model_mesh_part_set_morph_target_data_ext(part, state->data) ==
                CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_destroy(state->data) == CNA_RESULT_SUCCESS &&
            ((state->data = CNA_INVALID_HANDLE), 1) &&
            cna_model_mesh_part_get_morph_target_data_ext(part, &has, &alias) ==
                CNA_RESULT_SUCCESS &&
            has == CNA_TRUE && alias != CNA_INVALID_HANDLE,
        "the part retains morph data after its caller-owned handle is released");
    cna_demo_check(
        state->demo,
        cna_model_mesh_part_set_morph_weights_ext(part, uploaded, UINT64_C(2)) ==
                CNA_RESULT_SUCCESS &&
            cna_morph_target_data_ext_copy_weights(
                alias, copied, UINT64_C(2), &count) == CNA_RESULT_SUCCESS &&
            copied[0] == 0.5F && copied[1] == 0.25F &&
            cna_vertex_buffer_get_data_raw(
                buffer, UINT64_C(0), vertex, sizeof(vertex), UINT64_C(1), UINT32_C(32)) ==
                CNA_RESULT_SUCCESS,
        "part weight upload updates both retained CPU state and its attached VertexBuffer");
    memcpy(position, vertex, sizeof(position));
    cna_demo_check(
        state->demo,
        position[0] == 0.5F && position[1] == 0.0F && position[2] == 0.5F,
        "the uploaded vertex contains the independently predicted weighted position");
    cna_demo_check(
        state->demo,
        cna_morph_target_data_ext_destroy(alias) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_morph_target_data_ext(part, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_get_morph_target_data_ext(part, &has, &alias) ==
                CNA_RESULT_SUCCESS &&
            has == CNA_FALSE && alias == CNA_INVALID_HANDLE &&
            cna_model_mesh_part_set_vertex_buffer(part, CNA_INVALID_HANDLE) ==
                CNA_RESULT_SUCCESS &&
            cna_vertex_buffer_destroy(buffer) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_destroy(part) == CNA_RESULT_SUCCESS,
        "clearing the attachment releases every alias, part and buffer cleanly");
    return CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const float BaseExpected[8] = {
        0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.25F, 0.75F};
    uint8_t base[32];
    CNA_Vector3 position0[1] = {{0.0F, 0.0F, 1.0F}};
    CNA_Vector3 position1[1] = {{2.0F, 0.0F, 0.0F}};
    CNA_Vector3 normal1[1] = {{1.0F, 0.0F, 0.0F}};
    CNA_MorphTargetDeltaEXTDescriptor targets[2];
    float initial_weights[2] = {0.0F, 0.0F};
    float key0_weights[2] = {0.0F, 0.0F};
    float key1_weights[2] = {1.0F, 1.0F};
    float zero_tangents[2] = {0.0F, 0.0F};
    CNA_MorphWeightKeyframeEXTDescriptor keys[2];
    CNA_MorphTargetDataEXTDescriptor descriptor;
    CNA_MorphTargetDataEXTHandle rejected = UINT64_MAX;
    CnaDemo demo;
    MorphDemo state;
    CNA_GameCallbacks callbacks;
    CNA_GameCreateInfo create_info;
    CNA_Handle game = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    int ready;

    cna_demo_init(&demo, "c_api.graphics.morph_targets", argc, argv, 1);
    memcpy(base, BaseExpected, sizeof(base));
    targets[0] = (CNA_MorphTargetDeltaEXTDescriptor){
        position0, UINT64_C(1), NULL, UINT64_C(0)};
    targets[1] = (CNA_MorphTargetDeltaEXTDescriptor){
        position1, UINT64_C(1), normal1, UINT64_C(1)};
    keys[0] = (CNA_MorphWeightKeyframeEXTDescriptor){
        0.0, key0_weights, UINT64_C(2), zero_tangents, UINT64_C(2),
        zero_tangents, UINT64_C(2)};
    keys[1] = (CNA_MorphWeightKeyframeEXTDescriptor){
        1.0, key1_weights, UINT64_C(2), zero_tangents, UINT64_C(2),
        zero_tangents, UINT64_C(2)};
    descriptor = (CNA_MorphTargetDataEXTDescriptor){
        base, UINT64_C(32), INT32_C(32), targets, UINT64_C(2), initial_weights,
        UINT64_C(2), {keys, UINT64_C(2), CNA_FALSE, CNA_TRUE}};
    memset(&state, 0, sizeof(state));
    state.demo = &demo;
    state.data = CNA_INVALID_HANDLE;
    cna_demo_check_result(
        &demo, cna_morph_target_data_ext_create(&descriptor, &state.data),
        "deep-copy one base vertex, two targets, weights and a cubic track");
    memset(base, 0xCC, sizeof(base));
    memset(position0, 0, sizeof(position0));
    memset(position1, 0, sizeof(position1));
    memset(normal1, 0, sizeof(normal1));
    memset(initial_weights, 0xCC, sizeof(initial_weights));
    memset(key0_weights, 0xCC, sizeof(key0_weights));
    memset(key1_weights, 0xCC, sizeof(key1_weights));
    memset(zero_tangents, 0xCC, sizeof(zero_tangents));
    if (state.data != CNA_INVALID_HANDLE) {
        audit_data(&demo, state.data);
    }
    audit_track_evaluation(&demo);
    descriptor.stride = INT32_C(31);
    cna_demo_check(
        &demo,
        cna_morph_target_data_ext_create(&descriptor, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "a stride that does not divide the base byte count is refused atomically");

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.load_content = load_content;
    callbacks.context = &state;
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.target_elapsed_time_ticks = INT64_C(166667);
    create_info.callbacks = &callbacks;
    ready = cna_game_create(&create_info, &game) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_create(game, &manager) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_set_graphics_profile(
                manager, CNA_GRAPHICS_PROFILE_HI_DEF) == CNA_RESULT_SUCCESS &&
            cna_graphics_device_manager_apply_changes(manager) == CNA_RESULT_SUCCESS;
    if (cna_demo_check(&demo, ready, "create a HiDef owner for the part-upload control")) {
        cna_demo_check_result(
            &demo, cna_game_run_one_frame(game),
            "run one frame containing the retained morph upload");
        cna_demo_check(&demo, state.loads == 1, "the upload ran in exactly one load callback");
    }
    if (state.data != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_morph_target_data_ext_destroy(state.data),
            "release morph data left after an early callback exit");
    }
    if (manager != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_graphics_device_manager_destroy(manager),
            "destroy the graphics device manager");
    }
    cna_demo_check_result(&demo, cna_game_destroy(game), "destroy the owner game");
    return cna_demo_finish(&demo);
}
