/* SPDX-License-Identifier: MS-PL */
/* Neutral CNB model flags, bones, parts and materials through C. */
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

static int bytes_are(const uint8_t* bytes, size_t count, uint8_t value)
{
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != value) {
            return 0;
        }
    }
    return 1;
}

static void identity(float* matrix, float diagonal)
{
    memset(matrix, 0, 16U * sizeof(float));
    matrix[0] = diagonal;
    matrix[5] = diagonal;
    matrix[10] = diagonal;
    matrix[15] = 1.0F;
}

static int release_model(CNA_CnbModelDataHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_model_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const char* const TextureNames[8] = {
        "albedo", "overlay", "normal", "metallic_roughness",
        "emissive", "occlusion", "specular", "specular_color"};
    CnaDemo demo;
    CNA_CnbModelDataHandle model = CNA_INVALID_HANDLE;
    CNA_CnbModelDataHandle stale_model = CNA_INVALID_HANDLE;
    CNA_CnbModelInfo model_info;
    CNA_CnbModelBone bone;
    CNA_CnbModelPartInfo part;
    CNA_CnbMaterialInfo material;
    float transform[16];
    uint8_t vertex_bytes[36];
    uint8_t index_bytes[6] = {0U, 0U, 1U, 0U, 2U, 0U};
    uint8_t output[128];
    char text[128];
    uint64_t index = UINT64_C(0);
    uint64_t size = UINT64_C(0);
    uint32_t byte_index;
    int values_ok = 1;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_model_core", argc, argv, 1);

    cna_demo_check(
        &demo,
        sizeof(CNA_CnbEffectKind) == sizeof(uint32_t) &&
            sizeof(CNA_CnbMaterialTextureSlot) == sizeof(uint32_t) &&
            CNA_CNB_EFFECT_KIND_BASIC == UINT32_C(0) &&
            CNA_CNB_EFFECT_KIND_EXTERNAL == UINT32_C(5) &&
            CNA_CNB_MATERIAL_TEXTURE_BASE_COLOR == UINT32_C(0) &&
            CNA_CNB_MATERIAL_TEXTURE_SPECULAR_COLOR == UINT32_C(7) &&
            CNA_CNB_TEXTURE_SLOT_COUNT == UINT32_C(7) &&
            CNA_CNB_NO_INDEX == UINT32_C(0xFFFFFFFF),
        "model effect, texture-slot and no-index identities retain their wire values");
    if (!cna_demo_check_result(
            &demo, cna_cnb_model_create(&model),
            "create one empty neutral model description")) {
        goto cleanup;
    }
    memset(&model_info, 0, sizeof(model_info));
    model_info.struct_size = (uint32_t)sizeof(model_info);
    model_info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_info(model, &model_info) == CNA_RESULT_SUCCESS &&
            model_info.bone_count == UINT64_C(0) && model_info.part_count == UINT64_C(0) &&
            model_info.mesh_count == UINT64_C(0) &&
            model_info.animation_count == UINT64_C(0) &&
            model_info.light_count == UINT64_C(0) && model_info.has_skeleton == CNA_FALSE &&
            model_info.applies_gltf_lighting_policy == CNA_FALSE &&
            model_info.has_bone_hierarchy == CNA_FALSE && model_info.reserved == UINT8_C(0),
        "a new model publishes empty counts and canonical false flags");
    cna_demo_check(
        &demo,
        cna_cnb_model_set_flags(model, CNA_TRUE, CNA_TRUE) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_set_flags(model, (CNA_Bool)9, CNA_TRUE) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "model flags accept canonical booleans and refuse a noncanonical value");

    identity(transform, 1.0F);
    cna_demo_check(
        &demo,
        cna_cnb_model_add_bone(model, text_view("root"), INT32_C(-1), transform, &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append a named root bone with an identity local transform");
    identity(transform, 2.0F);
    cna_demo_check(
        &demo,
        cna_cnb_model_add_bone(model, text_view("child"), INT32_C(0), transform, &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(1),
        "append a child bone whose parent is the root index");
    memset(transform, 0, sizeof(transform));
    memset(&bone, 0, sizeof(bone));
    bone.struct_size = (uint32_t)sizeof(bone);
    bone.struct_version = CNA_CNB_MODEL_BONE_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_bone(model, UINT64_C(1), &bone) == CNA_RESULT_SUCCESS &&
            bone.parent == INT32_C(0) && bone.reserved == UINT32_C(0) &&
            bone.transform[0] == 2.0F && bone.transform[5] == 2.0F &&
            bone.transform[10] == 2.0F && bone.transform[15] == 1.0F,
        "bone values own their transform copy and preserve the parent relationship");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_model_get_bone_name_size(model, UINT64_C(1), &size) == CNA_RESULT_SUCCESS &&
            size == UINT64_C(5) &&
            cna_cnb_model_copy_bone_name(
                model, UINT64_C(1), text, size - UINT64_C(1), &index) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            index == size && bytes_are((const uint8_t*)text, sizeof(text), 0xA5U) &&
            cna_cnb_model_copy_bone_name(
                model, UINT64_C(1), text, sizeof(text), &index) == CNA_RESULT_SUCCESS &&
            index == size && memcmp(text, "child", 5U) == 0,
        "bone names report and copy exact UTF-8 bytes atomically");

    memset(&part, 0, sizeof(part));
    part.struct_size = (uint32_t)sizeof(part);
    part.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    part.vertex_stride = UINT32_C(12);
    part.vertex_count = UINT32_C(3);
    part.index_count = UINT32_C(3);
    part.index_element_size = UINT32_C(2);
    part.primitive_topology = UINT32_C(4);
    part.primitive_count = UINT32_C(1);
    part.effect_kind = CNA_CNB_EFFECT_KIND_EXTERNAL;
    part.vertex_color_enabled = CNA_TRUE;
    cna_demo_check(
        &demo,
        cna_cnb_model_add_part(
            model, &part, text_view("triangle"), text_view("effects/water"), &index) ==
                CNA_RESULT_SUCCESS &&
            index == UINT64_C(0),
        "append a named triangle part with an external effect reference");
    for (byte_index = UINT32_C(0); byte_index < sizeof(vertex_bytes); ++byte_index) {
        vertex_bytes[byte_index] = (uint8_t)(byte_index + UINT32_C(1));
    }
    cna_demo_check(
        &demo,
        cna_cnb_model_set_part_vertex_bytes(
            model, UINT64_C(0), vertex_bytes, sizeof(vertex_bytes)) == CNA_RESULT_SUCCESS &&
            cna_cnb_model_set_part_index_bytes(
                model, UINT64_C(0), index_bytes, sizeof(index_bytes)) == CNA_RESULT_SUCCESS,
        "set copied vertex and sixteen-bit index payloads for the part");
    memset(vertex_bytes, 0, sizeof(vertex_bytes));
    memset(index_bytes, 0, sizeof(index_bytes));
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_part_vertex_bytes(
            model, UINT64_C(0), output, UINT64_C(35), &size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            size == UINT64_C(36) && bytes_are(output, sizeof(output), 0xA5U) &&
            cna_cnb_model_copy_part_vertex_bytes(
                model, UINT64_C(0), output, sizeof(output), &size) == CNA_RESULT_SUCCESS &&
            size == UINT64_C(36) && output[0] == UINT8_C(1) && output[35] == UINT8_C(36),
        "vertex bytes are owned and short copies are atomic");
    memset(output, 0xA5, sizeof(output));
    cna_demo_check(
        &demo,
        cna_cnb_model_copy_part_index_bytes(
            model, UINT64_C(0), output, UINT64_C(5), &size) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            size == UINT64_C(6) && bytes_are(output, sizeof(output), 0xA5U) &&
            cna_cnb_model_copy_part_index_bytes(
                model, UINT64_C(0), output, sizeof(output), &size) == CNA_RESULT_SUCCESS &&
            size == UINT64_C(6) && output[0] == UINT8_C(0) && output[2] == UINT8_C(1) &&
            output[4] == UINT8_C(2),
        "index bytes are owned and short copies are atomic");

    part.unlit = CNA_TRUE;
    cna_demo_check(
        &demo,
        cna_cnb_model_set_part(model, UINT64_C(0), &part) == CNA_RESULT_SUCCESS,
        "replace numeric part state without replacing its owned names or bytes");
    memset(&part, 0, sizeof(part));
    part.struct_size = (uint32_t)sizeof(part);
    part.struct_version = CNA_CNB_MODEL_PART_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_part(model, UINT64_C(0), &part) == CNA_RESULT_SUCCESS &&
            part.vertex_stride == UINT32_C(12) && part.vertex_count == UINT32_C(3) &&
            part.index_count == UINT32_C(3) && part.index_element_size == UINT32_C(2) &&
            part.primitive_topology == UINT32_C(4) && part.primitive_count == UINT32_C(1) &&
            part.effect_kind == CNA_CNB_EFFECT_KIND_EXTERNAL &&
            part.vertex_color_enabled == CNA_TRUE && part.unlit == CNA_TRUE &&
            bytes_are(part.reserved, sizeof(part.reserved), 0U),
        "the versioned part value returns every authored numeric field");
    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        cna_cnb_model_get_part_name_size(model, UINT64_C(0), &size) == CNA_RESULT_SUCCESS &&
            size == UINT64_C(8) &&
            cna_cnb_model_copy_part_name(
                model, UINT64_C(0), text, sizeof(text), &index) == CNA_RESULT_SUCCESS &&
            index == size && memcmp(text, "triangle", 8U) == 0 &&
            cna_cnb_model_get_part_external_effect_size(
                model, UINT64_C(0), &size) == CNA_RESULT_SUCCESS &&
            size == UINT64_C(13) &&
            cna_cnb_model_copy_part_external_effect(
                model, UINT64_C(0), text, sizeof(text), &index) == CNA_RESULT_SUCCESS &&
            index == size && memcmp(text, "effects/water", 13U) == 0,
        "part and external-effect names survive numeric replacement independently");

    memset(&material, 0, sizeof(material));
    material.struct_size = (uint32_t)sizeof(material);
    material.struct_version = CNA_CNB_MATERIAL_INFO_STRUCT_VERSION;
    material.base_color_factor[0] = 0.25F;
    material.base_color_factor[1] = 0.5F;
    material.base_color_factor[2] = 0.75F;
    material.base_color_factor[3] = 1.0F;
    material.emissive_factor[0] = 0.125F;
    material.specular_color_factor[0] = 0.875F;
    material.metallic_factor = 0.4F;
    material.roughness_factor = 0.6F;
    material.ior = 1.33F;
    material.specular_factor = 0.9F;
    material.normal_scale = 1.5F;
    material.occlusion_strength = 0.8F;
    material.alpha_cutoff = 0.25F;
    material.alpha_mode = UINT32_C(2);
    material.double_sided = CNA_TRUE;
    cna_demo_check_result(
        &demo, cna_cnb_model_set_material(model, UINT64_C(0), &material),
        "replace the part's complete numeric material state");
    memset(&material, 0, sizeof(material));
    material.struct_size = (uint32_t)sizeof(material);
    material.struct_version = CNA_CNB_MATERIAL_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_material(model, UINT64_C(0), &material) == CNA_RESULT_SUCCESS &&
            material.base_color_factor[0] == 0.25F &&
            material.base_color_factor[3] == 1.0F && material.emissive_factor[0] == 0.125F &&
            material.specular_color_factor[0] == 0.875F &&
            material.metallic_factor == 0.4F && material.roughness_factor == 0.6F &&
            material.ior == 1.33F && material.specular_factor == 0.9F &&
            material.normal_scale == 1.5F && material.occlusion_strength == 0.8F &&
            material.alpha_cutoff == 0.25F && material.alpha_mode == UINT32_C(2) &&
            material.double_sided == CNA_TRUE &&
            bytes_are(material.reserved, sizeof(material.reserved), 0U),
        "the material returns every scalar, vector, enum and boolean field exactly");

    for (index = UINT64_C(0); index <= CNA_CNB_MATERIAL_TEXTURE_MAXIMUM; ++index) {
        if (cna_cnb_model_set_material_texture(
                model, UINT64_C(0), (CNA_CnbMaterialTextureSlot)index,
                text_view(TextureNames[index])) != CNA_RESULT_SUCCESS) {
            values_ok = 0;
        }
    }
    cna_demo_check(
        &demo, values_ok,
        "set all eight independently named material texture slots");
    values_ok = 1;
    for (index = UINT64_C(0); index <= CNA_CNB_MATERIAL_TEXTURE_MAXIMUM; ++index) {
        const size_t expected = strlen(TextureNames[index]);
        memset(text, 0xA5, sizeof(text));
        if (cna_cnb_model_get_material_texture_size(
                model, UINT64_C(0), (CNA_CnbMaterialTextureSlot)index, &size) !=
                CNA_RESULT_SUCCESS ||
            size != expected ||
            cna_cnb_model_copy_material_texture(
                model, UINT64_C(0), (CNA_CnbMaterialTextureSlot)index,
                text, sizeof(text), &size) != CNA_RESULT_SUCCESS ||
            size != expected || memcmp(text, TextureNames[index], expected) != 0) {
            values_ok = 0;
        }
    }
    cna_demo_check(
        &demo, values_ok,
        "all eight texture asset names retain their distinct order and exact bytes");

    values_ok = 1;
    for (index = UINT64_C(0); index < CNA_CNB_TEXTURE_SLOT_COUNT; ++index) {
        CNA_CnbTextureTransform uv;
        CNA_CnbSamplerState sampler;
        uv.offset_x = (float)index;
        uv.offset_y = (float)index + 0.5F;
        uv.scale_x = 2.0F;
        uv.scale_y = 3.0F;
        uv.rotation = 0.25F;
        memset(&sampler, 0, sizeof(sampler));
        sampler.filter = (uint32_t)index;
        sampler.address_u = UINT32_C(1);
        sampler.address_v = UINT32_C(2);
        sampler.declared = CNA_TRUE;
        if (cna_cnb_model_set_material_texture_coordinate_set(
                model, UINT64_C(0), index, (uint8_t)(index % UINT64_C(2))) !=
                CNA_RESULT_SUCCESS ||
            cna_cnb_model_set_material_texture_transform(
                model, UINT64_C(0), index, &uv) != CNA_RESULT_SUCCESS ||
            cna_cnb_model_set_material_sampler(
                model, UINT64_C(0), index, &sampler) != CNA_RESULT_SUCCESS) {
            values_ok = 0;
        }
    }
    cna_demo_check(
        &demo, values_ok,
        "set all seven importer slots without confusing them with the eight name slots");
    values_ok = 1;
    for (index = UINT64_C(0); index < CNA_CNB_TEXTURE_SLOT_COUNT; ++index) {
        uint8_t coordinate_set = UINT8_C(9);
        CNA_CnbTextureTransform uv;
        CNA_CnbSamplerState sampler;
        memset(&uv, 0, sizeof(uv));
        memset(&sampler, 0, sizeof(sampler));
        if (cna_cnb_model_get_material_texture_coordinate_set(
                model, UINT64_C(0), index, &coordinate_set) != CNA_RESULT_SUCCESS ||
            coordinate_set != (uint8_t)(index % UINT64_C(2)) ||
            cna_cnb_model_get_material_texture_transform(
                model, UINT64_C(0), index, &uv) != CNA_RESULT_SUCCESS ||
            uv.offset_x != (float)index || uv.offset_y != (float)index + 0.5F ||
            uv.scale_x != 2.0F || uv.scale_y != 3.0F || uv.rotation != 0.25F ||
            cna_cnb_model_get_material_sampler(
                model, UINT64_C(0), index, &sampler) != CNA_RESULT_SUCCESS ||
            sampler.filter != (uint32_t)index || sampler.address_u != UINT32_C(1) ||
            sampler.address_v != UINT32_C(2) || sampler.declared != CNA_TRUE ||
            !bytes_are(sampler.reserved, sizeof(sampler.reserved), 0U)) {
            values_ok = 0;
        }
    }
    cna_demo_check(
        &demo, values_ok,
        "all seven coordinate-set, UV-transform and sampler values round-trip independently");
    cna_demo_check(
        &demo,
        cna_cnb_model_get_material_texture_size(
            model, UINT64_C(0), CNA_CNB_MATERIAL_TEXTURE_MAXIMUM + UINT32_C(1), &size) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_model_set_material_texture_coordinate_set(
                model, UINT64_C(0), CNA_CNB_TEXTURE_SLOT_COUNT, UINT8_C(0)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_model_get_bone(model, UINT64_C(9), &bone) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_model_get_part(model, UINT64_C(9), &part) == CNA_RESULT_INVALID_ARGUMENT,
        "name-slot, importer-slot, bone and part bounds are argument errors");

    memset(&model_info, 0, sizeof(model_info));
    model_info.struct_size = (uint32_t)sizeof(model_info);
    model_info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_info(model, &model_info) == CNA_RESULT_SUCCESS &&
            model_info.bone_count == UINT64_C(2) && model_info.part_count == UINT64_C(1) &&
            model_info.mesh_count == UINT64_C(0) &&
            model_info.animation_count == UINT64_C(0) &&
            model_info.light_count == UINT64_C(0) && model_info.has_skeleton == CNA_FALSE &&
            model_info.applies_gltf_lighting_policy == CNA_TRUE &&
            model_info.has_bone_hierarchy == CNA_TRUE,
        "live model info aggregates the authored graph and independent content flags");
    stale_model = model;
    cna_demo_check_result(&demo, cna_cnb_model_destroy(model), "release the neutral model graph");
    model = CNA_INVALID_HANDLE;
    memset(&model_info, 0, sizeof(model_info));
    model_info.struct_size = (uint32_t)sizeof(model_info);
    model_info.struct_version = CNA_CNB_MODEL_INFO_STRUCT_VERSION;
    cna_demo_check(
        &demo,
        cna_cnb_model_get_info(stale_model, &model_info) == CNA_RESULT_INVALID_HANDLE,
        "the released model rejects stale graph access");

cleanup:
    releases_ok &= release_model(&model);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining model handle exactly once");
    return cna_demo_finish(&demo);
}
