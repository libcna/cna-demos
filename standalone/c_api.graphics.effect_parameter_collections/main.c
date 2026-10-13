/* SPDX-License-Identifier: MS-PL */
/*
 * Effect parameter collections from C: collection growth keeps stable mutable element views;
 * exact name and semantic lookups share element state; nested element, structure-member and
 * annotation collection views own their parents for as long as their descendants need them.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

static CNA_StringView text_view(const char* text)
{
    CNA_StringView view;
    view.data = text;
    view.byte_length = (uint64_t)strlen(text);
    return view;
}

static CNA_EffectParameterCreateInfo parameter_info(const char* name, const char* semantic)
{
    CNA_EffectParameterCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.name = text_view(name);
    info.semantic = text_view(semantic);
    info.row_count = 1;
    info.column_count = 1;
    info.parameter_class = CNA_EFFECT_PARAMETER_CLASS_SCALAR;
    info.parameter_type = CNA_EFFECT_PARAMETER_TYPE_INT32;
    return info;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_EffectParameterCollectionHandle collection = CNA_INVALID_HANDLE;
    CNA_EffectParameterCollectionHandle elements = CNA_INVALID_HANDLE;
    CNA_EffectParameterCollectionHandle members = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationCollectionHandle annotations = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle first = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle second = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle indexed = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle named = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle semantic = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle element = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle member = CNA_INVALID_HANDLE;
    CNA_EffectParameterHandle missing = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationHandle annotation = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationHandle annotation_copy = CNA_INVALID_HANDLE;
    CNA_EffectParameterCreateInfo create_info;
    CNA_EffectAnnotationCreateInfo annotation_info;
    CNA_Result result;
    CNA_Bool found = CNA_FALSE;
    uint64_t count = UINT64_C(0);
    uint64_t copied = UINT64_C(0);
    int32_t value = 0;
    float annotation_value = 3.25f;
    float copied_annotation_value = 0.0f;
    char first_name[] = "First";
    char first_semantic[] = "POSITION";
    char copied_name[6] = {0};
    int index;

    cna_demo_init(&demo, "c_api.graphics.effect_parameter_collections", argc, argv, 0);

    if (!cna_demo_check_result(
            &demo, cna_effect_parameter_collection_create(&collection),
            "create an independently owned empty effect-parameter collection")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_effect_parameter_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "the newly created collection starts empty");

    create_info = parameter_info(first_name, first_semantic);
    result = cna_effect_parameter_collection_add_create(collection, &create_info, &first);
    memset(first_name, '#', sizeof(first_name) - 1U);
    memset(first_semantic, '#', sizeof(first_semantic) - 1U);
    create_info = parameter_info("Second", "Position");
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_collection_add_create(collection, &create_info, &second) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_parameter_copy_name(first, copied_name, UINT64_C(5), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == UINT64_C(5) && memcmp(copied_name, "First", 5U) == 0,
        "add-create copies metadata and returns stable mutable element views in insertion order");

    value = 41;
    result = cna_effect_parameter_set_value(first, CNA_EFFECT_VALUE_INT32, &value);
    for (index = 0; result == CNA_RESULT_SUCCESS && index < 32; ++index) {
        CNA_EffectParameterHandle added = CNA_INVALID_HANDLE;
        create_info = parameter_info("Extra", "EXTRA");
        create_info.row_count = index + 2;
        result = cna_effect_parameter_collection_add_create(collection, &create_info, &added);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_effect_parameter_destroy(added);
        }
    }
    value = 0;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_collection_get_count(collection, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(34) &&
            cna_effect_parameter_get_value(first, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 41,
        "the first returned view stays valid and keeps its value after substantial collection growth");

    found = CNA_FALSE;
    cna_demo_check(
        &demo,
        cna_effect_parameter_collection_get_at(collection, UINT64_C(0), &indexed) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_parameter_collection_find_name(
                collection, text_view("First"), &found, &named) == CNA_RESULT_SUCCESS &&
            found == CNA_TRUE &&
            cna_effect_parameter_collection_find_semantic(
                collection, text_view("POSITION"), &found, &semantic) == CNA_RESULT_SUCCESS &&
            found == CNA_TRUE,
        "index, exact-name and exact-semantic lookup each return an owned view of the first element");

    value = 77;
    result = cna_effect_parameter_set_value(indexed, CNA_EFFECT_VALUE_INT32, &value);
    value = 0;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(first, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 77 &&
            cna_effect_parameter_get_value(named, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 77 &&
            cna_effect_parameter_get_value(semantic, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 77,
        "all collection views share the same mutable parameter state rather than copied values");

    found = CNA_TRUE;
    missing = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_effect_parameter_collection_find_name(
            collection, text_view("first"), &found, &missing) == CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && missing == CNA_INVALID_HANDLE,
        "name lookup is case-sensitive and absence publishes no parameter handle");
    /* ISSUE-068: the C header says "exact" although the C++ contract ignores semantic case. */
    found = CNA_FALSE;
    missing = UINT64_MAX;
    value = 0;
    result = cna_effect_parameter_collection_find_semantic(
        collection, text_view("position"), &found, &missing);
    if (result == CNA_RESULT_SUCCESS && found == CNA_TRUE) {
        result = cna_effect_parameter_get_value(missing, CNA_EFFECT_VALUE_INT32, &value);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_destroy(missing);
        if (result == CNA_RESULT_SUCCESS) {
            missing = CNA_INVALID_HANDLE;
        }
    }
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS && found == CNA_TRUE && value == 77,
        "semantic lookup ignores case and returns the first matching element");
    missing = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_effect_parameter_collection_get_at(collection, count, &missing) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            missing == CNA_INVALID_HANDLE,
        "the upper collection bound is refused before publishing a parameter handle");
    missing = CNA_INVALID_HANDLE;

    cna_demo_check(
        &demo,
        cna_effect_parameter_get_elements(first, &elements) == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_structure_members(first, &members) == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_annotations(first, &annotations) == CNA_RESULT_SUCCESS &&
            elements != CNA_INVALID_HANDLE && members != CNA_INVALID_HANDLE &&
            annotations != CNA_INVALID_HANDLE,
        "a parameter publishes separate owned element, structure-member and annotation views");
    cna_demo_check(
        &demo,
        cna_effect_parameter_collection_get_count(elements, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_parameter_collection_get_count(members, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_annotation_collection_get_count(annotations, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "all three nested collections start independently empty");

    create_info = parameter_info("Element0", "ELEMENT");
    result = cna_effect_parameter_collection_add_create(elements, &create_info, &element);
    create_info = parameter_info("Member0", "MEMBER");
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_collection_add_create(members, &create_info, &member);
    }
    value = 101;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_set_value(element, CNA_EFFECT_VALUE_INT32, &value);
    }
    value = 202;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_set_value(member, CNA_EFFECT_VALUE_INT32, &value);
    }
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_collection_get_count(elements, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_effect_parameter_collection_get_count(members, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "element and structure-member views mutate distinct one-item parameter collections");

    memset(&annotation_info, 0, sizeof(annotation_info));
    annotation_info.struct_size = (uint32_t)sizeof(annotation_info);
    annotation_info.struct_version = UINT32_C(1);
    annotation_info.name = text_view("NestedTag");
    annotation_info.semantic = text_view("META");
    annotation_info.row_count = 1;
    annotation_info.column_count = 1;
    annotation_info.parameter_class = CNA_EFFECT_PARAMETER_CLASS_SCALAR;
    annotation_info.parameter_type = CNA_EFFECT_PARAMETER_TYPE_SINGLE;
    annotation_info.data = &annotation_value;
    annotation_info.data_count = UINT64_C(1);
    result = cna_effect_annotation_create(&annotation_info, &annotation);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_annotation_collection_add(annotations, annotation);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_annotation_destroy(annotation);
        annotation = CNA_INVALID_HANDLE;
    }
    found = CNA_FALSE;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_collection_get_count(annotations, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_effect_annotation_collection_find(
                annotations, text_view("NestedTag"), &found, &annotation_copy) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_TRUE &&
            cna_effect_annotation_get_value_single(annotation_copy, &copied_annotation_value) ==
                CNA_RESULT_SUCCESS &&
            copied_annotation_value == 3.25f,
        "the parameter's annotation view stores a copied value with an independent owned lookup result");

    result = cna_effect_parameter_collection_destroy(collection);
    if (result == CNA_RESULT_SUCCESS) {
        collection = CNA_INVALID_HANDLE;
    }
    value = 88;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_set_value(semantic, CNA_EFFECT_VALUE_INT32, &value);
    }
    value = 0;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(named, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 88 &&
            cna_effect_parameter_collection_get_count(elements, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_effect_annotation_collection_get_count(annotations, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "owned element and nested collection views remain usable after the root collection is destroyed");

    result = cna_effect_parameter_destroy(first);
    first = CNA_INVALID_HANDLE;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_destroy(second);
        second = CNA_INVALID_HANDLE;
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_destroy(indexed);
        indexed = CNA_INVALID_HANDLE;
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_destroy(named);
        named = CNA_INVALID_HANDLE;
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_destroy(semantic);
        semantic = CNA_INVALID_HANDLE;
    }
    value = 0;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(element, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 101 &&
            cna_effect_parameter_get_value(member, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 202 &&
            cna_effect_annotation_get_value_single(annotation_copy, &copied_annotation_value) ==
                CNA_RESULT_SUCCESS &&
            copied_annotation_value == 3.25f,
        "nested parameter and annotation children survive destruction of every root element view");

    result = cna_effect_parameter_collection_destroy(elements);
    elements = CNA_INVALID_HANDLE;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_collection_destroy(members);
        members = CNA_INVALID_HANDLE;
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_annotation_collection_destroy(annotations);
        annotations = CNA_INVALID_HANDLE;
    }
    value = 303;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_parameter_set_value(element, CNA_EFFECT_VALUE_INT32, &value);
    }
    value = 0;
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_get_value(element, CNA_EFFECT_VALUE_INT32, &value) ==
                CNA_RESULT_SUCCESS &&
            value == 303 &&
            cna_effect_annotation_get_value_single(annotation_copy, &copied_annotation_value) ==
                CNA_RESULT_SUCCESS &&
            copied_annotation_value == 3.25f,
        "owned descendants remain valid after all three nested collection views are destroyed");

    missing = element;
    result = cna_effect_parameter_destroy(element);
    if (result == CNA_RESULT_SUCCESS) {
        element = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_parameter_destroy(missing) == CNA_RESULT_INVALID_HANDLE,
        "a nested parameter view releases exactly once and then becomes stale");
    missing = CNA_INVALID_HANDLE;
    result = cna_effect_parameter_destroy(member);
    if (result == CNA_RESULT_SUCCESS) {
        member = CNA_INVALID_HANDLE;
    }
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS, "the structure-member view releases cleanly");
    result = cna_effect_annotation_destroy(annotation_copy);
    if (result == CNA_RESULT_SUCCESS) {
        annotation_copy = CNA_INVALID_HANDLE;
    }
    cna_demo_check(&demo, result == CNA_RESULT_SUCCESS, "the nested annotation copy releases cleanly");

cleanup:
    if (missing != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(missing);
    }
    if (annotation_copy != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(annotation_copy);
    }
    if (annotation != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(annotation);
    }
    if (element != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(element);
    }
    if (member != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(member);
    }
    if (annotations != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_collection_destroy(annotations);
    }
    if (elements != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_collection_destroy(elements);
    }
    if (members != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_collection_destroy(members);
    }
    if (semantic != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(semantic);
    }
    if (named != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(named);
    }
    if (indexed != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(indexed);
    }
    if (second != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(second);
    }
    if (first != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_destroy(first);
    }
    if (collection != CNA_INVALID_HANDLE) {
        (void)cna_effect_parameter_collection_destroy(collection);
    }
    return cna_demo_finish(&demo);
}
