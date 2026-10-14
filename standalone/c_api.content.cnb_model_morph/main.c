/* SPDX-License-Identifier: MS-PL */
/* CNB model morph targets, defaults and weight timelines through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t expected)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_CnbModelDataHandle model = CNA_INVALID_HANDLE;
    CNA_CnbModelPartInfo part;
    CNA_CnbMorphInfo morph;
    CNA_CnbMorphWeightKeyInfo key_info;
    float position_deltas[6] = {0.1F, 0.0F, 0.0F, 0.0F, 0.2F, 0.0F};
    float normal_deltas[6] = {0.0F, 0.0F, 0.3F, 0.0F, 0.0F, 0.4F};
    float weights[2] = {0.25F, 0.75F};
    float key_weights[2] = {0.5F, 0.5F};
    float in_tangents[2] = {-0.1F, -0.2F};
    float out_tangents[2] = {0.1F, 0.2F};
    float output[16];
    uint64_t index = UINT64_C(0);
    uint64_t count = UINT64_C(0);
    CNA_Bool present = CNA_FALSE;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_model_morph", argc, argv, 1);

    if (!cna_demo_check_result(
            &demo, cna_cnb_model_create(&model),
            "create the neutral model that will own the morph data")) {
        goto cleanup;
    }
    memset(&part, 0, sizeof(part));
    part.struct_size = (uint32_t)sizeof(part);
    part.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    part.vertex_stride = UINT32_C(12);
    part.vertex_count = UINT32_C(2);
    part.index_element_size = UINT32_C(2);
    part.primitive_topology = UINT32_C(4);
    cna_demo_check_result(
        &demo,
        cna_cnb_model_add_part(
            model, &part, text_view("morphable"), text_view(""), &index),
        "append one two-vertex part addressed by the morph routes");
    cna_demo_check(
        &demo,
        index == UINT64_C(0) &&
            cna_cnb_model_has_morph(model, index, &present) == CNA_RESULT_SUCCESS &&
            present == CNA_FALSE,
        "a new part reports ordinary morph absence");
    memset(&morph, 0, sizeof(morph));
    morph.struct_size = (uint32_t)sizeof(morph);
    morph.struct_version = CNA_CNB_MORPH_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_morph(model, UINT64_C(0), &morph) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_model_add_morph_target(model, UINT64_C(0), &index) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "morph access and target creation require morph state first");

    memset(&morph, 0, sizeof(morph));
    morph.struct_size = (uint32_t)sizeof(morph);
    morph.struct_version = CNA_CNB_MORPH_INFO_STRUCT_VERSION;
    morph.vertex_count = UINT32_C(2);
    morph.recompute_flat_normals = CNA_TRUE;
    morph.weight_track_cubic_spline = CNA_TRUE;
    cna_demo_check_result(
        &demo, cna_cnb_model_set_morph(model, UINT64_C(0), &morph),
        "attach two-vertex cubic morph state to the part");
    cna_demo_check(
        &demo,
        cna_cnb_model_add_morph_target(model, UINT64_C(0), &index) == CNA_RESULT_SUCCESS &&
            index == UINT64_C(0) &&
            cna_cnb_model_add_morph_target(model, UINT64_C(0), &index) == CNA_RESULT_SUCCESS &&
            index == UINT64_C(1),
        "append two independently indexed empty morph targets");
    cna_demo_check(
        &demo,
        cna_cnb_model_set_morph_target_deltas(
            model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_DELTA_POSITION,
            position_deltas, UINT64_C(6)) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_set_morph_target_deltas(
                model, UINT64_C(0), UINT64_C(1), CNA_CNB_MORPH_DELTA_NORMAL,
                normal_deltas, UINT64_C(6)) == CNA_RESULT_SUCCESS,
        "set copied position and normal delta streams on separate targets");
    memset(position_deltas, 0, sizeof(position_deltas));
    memset(normal_deltas, 0, sizeof(normal_deltas));
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_morph_target_deltas(
            model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_DELTA_POSITION,
            output, UINT64_C(5), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(6) &&
            cna_cnb_model_copy_morph_target_deltas(
                model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_DELTA_POSITION,
                output, UINT64_C(16), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(6) && output[0] == 0.1F && output[4] == 0.2F,
        "delta copies are atomic and prove ownership after caller mutation");
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_morph_target_deltas(
            model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_DELTA_TANGENT,
            output, UINT64_C(16), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "an unset target stream is an exact empty value");

    cna_demo_check_result(
        &demo,
        cna_cnb_model_set_morph_weights(model, UINT64_C(0), weights, UINT64_C(2)),
        "set one copied default weight per morph target");
    memset(weights, 0, sizeof(weights));
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_morph_weights(
            model, UINT64_C(0), output, UINT64_C(1), &count) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            count == UINT64_C(2) &&
            cna_cnb_model_copy_morph_weights(
                model, UINT64_C(0), output, UINT64_C(16), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && output[0] == 0.25F && output[1] == 0.75F,
        "default weight copies are atomic and independent of caller storage");

    cna_demo_check(
        &demo,
        cna_cnb_model_add_morph_weight_key(
            model, UINT64_C(0), 1.5, key_weights, UINT64_C(2),
            in_tangents, UINT64_C(2), out_tangents, UINT64_C(2), &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append one cubic weight key with copied weights and both tangent streams");
    memset(key_weights, 0, sizeof(key_weights));
    memset(in_tangents, 0, sizeof(in_tangents));
    memset(out_tangents, 0, sizeof(out_tangents));
    memset(&key_info, 0, sizeof(key_info));
    key_info.struct_size = (uint32_t)sizeof(key_info);
    key_info.struct_version = CNA_CNB_MORPH_WEIGHT_KEY_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_morph_weight_key(
            model, UINT64_C(0), UINT64_C(0), &key_info) == CNA_RESULT_SUCCESS &&
            key_info.time_seconds == 1.5 && key_info.weight_count == UINT64_C(2) &&
            key_info.in_tangent_count == UINT64_C(2) &&
            key_info.out_tangent_count == UINT64_C(2),
        "the versioned key record reports exact time and stream lengths");
    memset(output, 0, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_morph_weight_key_values(
            model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_KEY_WEIGHTS,
            output, UINT64_C(16), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && output[0] == 0.5F && output[1] == 0.5F &&
            cna_cnb_model_copy_morph_weight_key_values(
                model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_KEY_IN_TANGENT,
                output, UINT64_C(16), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && output[0] == -0.1F && output[1] == -0.2F &&
            cna_cnb_model_copy_morph_weight_key_values(
                model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_KEY_OUT_TANGENT,
                output, UINT64_C(16), &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) && output[0] == 0.1F && output[1] == 0.2F,
        "all three key streams return their distinct copied values");

    memset(&morph, 0, sizeof(morph));
    morph.struct_size = (uint32_t)sizeof(morph);
    morph.struct_version = CNA_CNB_MORPH_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_morph(model, UINT64_C(0), &morph) == CNA_RESULT_SUCCESS &&
            morph.vertex_count == UINT32_C(2) && morph.target_count == UINT64_C(2) &&
            morph.weight_count == UINT64_C(2) &&
            morph.weight_track_key_count == UINT64_C(1) &&
            morph.recompute_flat_normals == CNA_TRUE &&
            morph.weight_track_step_interpolation == CNA_FALSE &&
            morph.weight_track_cubic_spline == CNA_TRUE &&
            bytes_are(morph.reserved2, sizeof(morph.reserved2), 0U),
        "live morph info derives its target, weight and key counts from owned streams");
    morph.recompute_flat_normals = CNA_FALSE;
    morph.weight_track_cubic_spline = CNA_FALSE;
    morph.weight_track_step_interpolation = CNA_TRUE;
    cna_demo_check(
        &demo,
        cna_cnb_model_set_morph(model, UINT64_C(0), &morph) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_get_morph(model, UINT64_C(0), &morph) == CNA_RESULT_SUCCESS &&
            morph.target_count == UINT64_C(2) && morph.weight_count == UINT64_C(2) &&
            morph.weight_track_key_count == UINT64_C(1) &&
            morph.recompute_flat_normals == CNA_FALSE &&
            morph.weight_track_step_interpolation == CNA_TRUE &&
            morph.weight_track_cubic_spline == CNA_FALSE,
        "replacing morph flags preserves every independently built stream and count");
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_morph_target_deltas(
            model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_DELTA_MAXIMUM + UINT32_C(1),
            output, UINT64_C(16), &count) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_model_copy_morph_weight_key_values(
                model, UINT64_C(0), UINT64_C(0), CNA_CNB_MORPH_KEY_MAXIMUM + UINT32_C(1),
                output, UINT64_C(16), &count) == CNA_RESULT_INVALID_ARGUMENT,
        "unknown delta and key stream selectors are argument errors");

    cna_demo_check(
        &demo,
        cna_cnb_model_clear_morph(model, UINT64_C(0)) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_has_morph(model, UINT64_C(0), &present) == CNA_RESULT_SUCCESS &&
            present == CNA_FALSE &&
            cna_cnb_model_clear_morph(model, UINT64_C(0)) == CNA_RESULT_SUCCESS,
        "clear removes the whole morph graph and remains idempotent");
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_morph_weights(
            model, UINT64_C(0), output, UINT64_C(16), &count) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "removed morph streams reject access rather than appearing as empty data");

cleanup:
    if (model != CNA_INVALID_HANDLE) {
        releases_ok &= cna_cnb_model_destroy(model) == CNA_RESULT_SUCCESS;
        model = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo, releases_ok,
        "release the model and all nested morph data exactly once");
    return cna_demo_finish(&demo);
}
