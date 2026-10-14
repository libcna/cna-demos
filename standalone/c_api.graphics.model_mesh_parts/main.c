/* SPDX-License-Identifier: MS-PL */
/* Standalone ModelMeshPart state, sampler slots and retained collection views through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static int part_values_are(
    CNA_ModelMeshPartHandle part, int32_t vertices, int32_t primitives,
    int32_t start, int32_t offset, CNA_ModelMeshPartTag tag)
{
    int32_t value = INT32_C(0);
    CNA_ModelMeshPartTag observed_tag = UINT64_C(0);
    return cna_model_mesh_part_get_num_vertices(part, &value) == CNA_RESULT_SUCCESS &&
           value == vertices &&
           cna_model_mesh_part_get_primitive_count(part, &value) == CNA_RESULT_SUCCESS &&
           value == primitives &&
           cna_model_mesh_part_get_start_index(part, &value) == CNA_RESULT_SUCCESS &&
           value == start &&
           cna_model_mesh_part_get_vertex_offset(part, &value) == CNA_RESULT_SUCCESS &&
           value == offset && cna_model_mesh_part_get_tag(part, &observed_tag) ==
               CNA_RESULT_SUCCESS &&
           observed_tag == tag;
}

static int sampler_is(CNA_SamplerState left, CNA_SamplerState right)
{
    return left.struct_size == right.struct_size &&
           left.struct_version == right.struct_version && left.address_u == right.address_u &&
           left.address_v == right.address_v && left.address_w == right.address_w &&
           left.filter == right.filter && left.max_anisotropy == right.max_anisotropy &&
           left.max_mip_level == right.max_mip_level &&
           left.mip_map_level_of_detail_bias == right.mip_map_level_of_detail_bias &&
           left.reserved == right.reserved;
}

static int release_part(CNA_ModelMeshPartHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_model_mesh_part_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_collection(CNA_ModelMeshPartCollectionHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_model_mesh_part_collection_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_ModelMeshPartHandle defaults = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle authored = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle alias = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle second_alias = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle rejected = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle stale_part = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartHandle parts[2];
    CNA_ModelMeshPartCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartCollectionHandle empty = CNA_INVALID_HANDLE;
    CNA_ModelMeshPartCollectionHandle stale_collection = CNA_INVALID_HANDLE;
    CNA_SamplerState default_sampler;
    CNA_SamplerState point_sampler;
    CNA_SamplerState wrap_sampler;
    CNA_SamplerState observed_sampler;
    CNA_SamplerState malformed_sampler;
    CNA_PrimitiveType topology = UINT32_MAX;
    uint64_t count = UINT64_C(99);
    int32_t scalar = INT32_C(0);
    uint32_t slot;
    int slots_ok = 1;
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.graphics.model_mesh_parts", argc, argv, 1);

    if (!cna_demo_check_result(
            &demo, cna_model_mesh_part_create_default(&defaults),
            "create an owned default ModelMeshPart")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        part_values_are(
            defaults, INT32_C(0), INT32_C(0), INT32_C(0), INT32_C(0), UINT64_C(0)),
        "the default part starts with zero scalar state and an empty C tag");
    cna_demo_check(
        &demo,
        cna_model_mesh_part_set_num_vertices(defaults, INT32_MIN) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_primitive_count(defaults, INT32_C(-7)) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_start_index(defaults, INT32_MAX) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_vertex_offset(defaults, INT32_C(-9)) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_set_tag(defaults, UINT64_MAX) == CNA_RESULT_SUCCESS &&
            part_values_are(
                defaults, INT32_MIN, INT32_C(-7), INT32_MAX, INT32_C(-9), UINT64_MAX),
        "every mutable signed scalar and the opaque tag preserve their complete bit range");

    if (!cna_demo_check_result(
            &demo,
            cna_model_mesh_part_create(
                CNA_INVALID_HANDLE, CNA_INVALID_HANDLE, INT32_C(11), INT32_C(12),
                INT32_C(13), INT32_C(14), &authored),
            "construct a part with explicit scalar state and no GPU resources")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        part_values_are(
            authored, INT32_C(11), INT32_C(12), INT32_C(13), INT32_C(14), UINT64_C(0)),
        "the explicit constructor retains all four independent scalar values");

    cna_demo_check(
        &demo,
        cna_model_mesh_part_get_primitive_type_ext(defaults, &topology) == CNA_RESULT_SUCCESS &&
            topology == CNA_PRIMITIVE_TRIANGLE_LIST,
        "a new part describes triangle-list indices by default");
    for (topology = CNA_PRIMITIVE_TRIANGLE_LIST;
         topology <= CNA_PRIMITIVE_POINT_LIST_EXT; ++topology) {
        CNA_PrimitiveType observed = UINT32_MAX;
        if (cna_model_mesh_part_set_primitive_type_ext(defaults, topology) !=
                CNA_RESULT_SUCCESS ||
            cna_model_mesh_part_get_primitive_type_ext(defaults, &observed) !=
                CNA_RESULT_SUCCESS ||
            observed != topology) {
            slots_ok = 0;
        }
    }
    cna_demo_check(
        &demo, slots_ok,
        "every published primitive topology identity round-trips through part state");
    topology = UINT32_MAX;
    cna_demo_check(
        &demo,
        cna_model_mesh_part_set_primitive_type_ext(
            defaults, CNA_PRIMITIVE_POINT_LIST_EXT + UINT32_C(1)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_mesh_part_get_primitive_type_ext(defaults, &topology) ==
                CNA_RESULT_SUCCESS &&
            topology == CNA_PRIMITIVE_POINT_LIST_EXT,
        "an undefined topology is refused without changing the previous value");

    memset(&default_sampler, 0, sizeof(default_sampler));
    memset(&point_sampler, 0, sizeof(point_sampler));
    memset(&wrap_sampler, 0, sizeof(wrap_sampler));
    cna_demo_check(
        &demo,
        cna_sampler_state_init(CNA_SAMPLER_STATE_PRESET_DEFAULT, &default_sampler) ==
                CNA_RESULT_SUCCESS &&
            cna_sampler_state_init(CNA_SAMPLER_STATE_PRESET_POINT_CLAMP, &point_sampler) ==
                CNA_RESULT_SUCCESS &&
            cna_sampler_state_init(CNA_SAMPLER_STATE_PRESET_LINEAR_WRAP, &wrap_sampler) ==
                CNA_RESULT_SUCCESS,
        "prepare three complete versioned sampler-state values");
    slots_ok = 1;
    for (slot = UINT32_C(0); slot <= CNA_PBR_TEXTURE_MAXIMUM; ++slot) {
        const CNA_SamplerState* desired = (slot & UINT32_C(1)) == UINT32_C(0)
                                              ? &point_sampler
                                              : &wrap_sampler;
        memset(&observed_sampler, 0, sizeof(observed_sampler));
        observed_sampler.struct_size = (uint32_t)sizeof(observed_sampler);
        observed_sampler.struct_version = UINT32_C(1);
        if (cna_model_mesh_part_get_sampler_state_ext(defaults, slot, &observed_sampler) !=
                CNA_RESULT_SUCCESS ||
            !sampler_is(observed_sampler, default_sampler) ||
            cna_model_mesh_part_set_sampler_state_ext(defaults, slot, desired) !=
                CNA_RESULT_SUCCESS) {
            slots_ok = 0;
            continue;
        }
        memset(&observed_sampler, 0, sizeof(observed_sampler));
        observed_sampler.struct_size = (uint32_t)sizeof(observed_sampler);
        observed_sampler.struct_version = UINT32_C(1);
        if (cna_model_mesh_part_get_sampler_state_ext(defaults, slot, &observed_sampler) !=
                CNA_RESULT_SUCCESS ||
            !sampler_is(observed_sampler, *desired)) {
            slots_ok = 0;
        }
    }
    cna_demo_check(
        &demo, slots_ok,
        "all seven material and specular texture slots start default and retain distinct presets");
    point_sampler.address_u = CNA_TEXTURE_ADDRESS_WRAP;
    memset(&observed_sampler, 0, sizeof(observed_sampler));
    observed_sampler.struct_size = (uint32_t)sizeof(observed_sampler);
    observed_sampler.struct_version = UINT32_C(1);
    cna_demo_check(
        &demo,
        cna_model_mesh_part_get_sampler_state_ext(
            defaults, CNA_PBR_TEXTURE_BASE_COLOR, &observed_sampler) == CNA_RESULT_SUCCESS &&
            observed_sampler.address_u == CNA_TEXTURE_ADDRESS_CLAMP,
        "sampler assignment copies the caller value instead of retaining its descriptor");
    malformed_sampler = wrap_sampler;
    malformed_sampler.struct_version = UINT32_C(2);
    cna_demo_check(
        &demo,
        cna_model_mesh_part_set_sampler_state_ext(
            defaults, CNA_PBR_TEXTURE_BASE_COLOR, &malformed_sampler) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_mesh_part_set_sampler_state_ext(
                defaults, CNA_PBR_TEXTURE_MAXIMUM + UINT32_C(1), &wrap_sampler) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_mesh_part_get_sampler_state_ext(
                defaults, CNA_PBR_TEXTURE_BASE_COLOR, &observed_sampler) ==
                CNA_RESULT_SUCCESS &&
            observed_sampler.address_u == CNA_TEXTURE_ADDRESS_CLAMP,
        "bad sampler versions and undefined slots are refused without changing prior state");

    parts[0] = defaults;
    parts[1] = authored;
    if (!cna_demo_check_result(
            &demo, cna_model_mesh_part_collection_create(parts, UINT64_C(2), &collection),
            "create a retained two-part collection snapshot")) {
        goto cleanup;
    }
    parts[0] = CNA_INVALID_HANDLE;
    parts[1] = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_model_mesh_part_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2) &&
            cna_model_mesh_part_collection_get_at(collection, UINT64_C(0), &alias) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_collection_get_at(collection, UINT64_C(1), &second_alias) ==
                CNA_RESULT_SUCCESS &&
            part_values_are(
                alias, INT32_MIN, INT32_C(-7), INT32_MAX, INT32_C(-9), UINT64_MAX) &&
            part_values_are(
                second_alias, INT32_C(11), INT32_C(12), INT32_C(13), INT32_C(14), UINT64_C(0)),
        "the collection keeps ordered stable views independent of its caller array");
    cna_demo_check(
        &demo,
        cna_model_mesh_part_set_num_vertices(alias, INT32_C(1234)) == CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_get_num_vertices(defaults, &scalar) == CNA_RESULT_SUCCESS &&
            scalar == INT32_C(1234),
        "a collection view aliases the same live part state as its original handle");
    rejected = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_model_mesh_part_collection_get_at(collection, count, &rejected) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "the collection refuses its Count index without publishing a part view");

    cna_demo_check(
        &demo,
        cna_model_mesh_part_collection_create(NULL, UINT64_C(0), &empty) ==
                CNA_RESULT_SUCCESS &&
            cna_model_mesh_part_collection_get_count(empty, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            ((rejected = UINT64_MAX),
             cna_model_mesh_part_collection_create(NULL, UINT64_C(1), &rejected)) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected == CNA_INVALID_HANDLE,
        "an explicit empty snapshot succeeds while a missing nonempty input is refused");

    releases_ok &= release_part(&defaults);
    releases_ok &= release_part(&authored);
    releases_ok &= release_collection(&collection);
    cna_demo_check(
        &demo,
        part_values_are(
            alias, INT32_C(1234), INT32_C(-7), INT32_MAX, INT32_C(-9), UINT64_MAX) &&
            part_values_are(
                second_alias, INT32_C(11), INT32_C(12), INT32_C(13), INT32_C(14), UINT64_C(0)),
        "stable part views survive their originals and retained collection");

    stale_part = alias;
    cna_demo_check_result(
        &demo, cna_model_mesh_part_destroy(alias),
        "release one stable part view exactly once");
    alias = CNA_INVALID_HANDLE;
    scalar = INT32_C(-999);
    cna_demo_check(
        &demo,
        cna_model_mesh_part_get_num_vertices(stale_part, &scalar) ==
                CNA_RESULT_INVALID_HANDLE &&
            scalar == INT32_C(-999),
        "a released part rejects stale reads without changing output");
    stale_collection = empty;
    cna_demo_check_result(
        &demo, cna_model_mesh_part_collection_destroy(empty),
        "release the empty collection exactly once");
    empty = CNA_INVALID_HANDLE;
    count = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_model_mesh_part_collection_get_count(stale_collection, &count) ==
                CNA_RESULT_INVALID_HANDLE &&
            count == UINT64_C(0x1122334455667788),
        "a released part collection rejects stale reads without changing output");

cleanup:
    releases_ok &= release_collection(&empty);
    releases_ok &= release_collection(&collection);
    releases_ok &= release_part(&rejected);
    releases_ok &= release_part(&second_alias);
    releases_ok &= release_part(&alias);
    releases_ok &= release_part(&authored);
    releases_ok &= release_part(&defaults);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining collection and stable part view");
    return cna_demo_finish(&demo);
}
