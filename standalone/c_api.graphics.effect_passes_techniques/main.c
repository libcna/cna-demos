/* SPDX-License-Identifier: MS-PL */
/*
 * Effect passes and techniques from C: ownerless values expose copied names, stable identities,
 * reflected indices, P0/default structure and annotations. Standalone collections retain stable
 * element views through growth and their descendants remain usable after parent release.
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

static CNA_Result create_annotation(const char* name, CNA_EffectAnnotationHandle* out_annotation)
{
    CNA_EffectAnnotationCreateInfo info;
    memset(&info, 0, sizeof(info));
    info.struct_size = (uint32_t)sizeof(info);
    info.struct_version = UINT32_C(1);
    info.name = text_view(name);
    info.row_count = 1;
    info.column_count = 1;
    info.parameter_class = CNA_EFFECT_PARAMETER_CLASS_SCALAR;
    info.parameter_type = CNA_EFFECT_PARAMETER_TYPE_SINGLE;
    return cna_effect_annotation_create(&info, out_annotation);
}

static void demonstrate_standalone_values(CnaDemo* demo)
{
    CNA_EffectTechniqueHandle default_technique = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle named_technique = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle reflected_empty = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle reflected_default = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle p0 = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle standalone = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle indexed = CNA_INVALID_HANDLE;
    CNA_EffectPassCollectionHandle default_passes = CNA_INVALID_HANDLE;
    CNA_EffectPassCollectionHandle named_passes = CNA_INVALID_HANDLE;
    CNA_EffectPassCollectionHandle reflected_empty_passes = CNA_INVALID_HANDLE;
    CNA_EffectPassCollectionHandle reflected_default_passes = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationCollectionHandle technique_annotations = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationCollectionHandle pass_annotations = CNA_INVALID_HANDLE;
    CNA_EffectAnnotationHandle annotation = CNA_INVALID_HANDLE;
    CNA_Result result;
    uint64_t count = UINT64_C(0);
    uint64_t bytes = UINT64_C(0);
    uint64_t default_identity = UINT64_C(0);
    uint64_t named_identity = UINT64_C(0);
    uint64_t empty_identity = UINT64_C(0);
    uint64_t reflected_identity = UINT64_C(0);
    uint32_t pass_index = UINT32_MAX;
    uint32_t technique_index = UINT32_MAX;
    char technique_name[] = "Lighting";
    char pass_name[] = "Standalone";
    char copied[16];

    if (!cna_demo_check_result(
            demo, cna_effect_technique_create_default(&default_technique),
            "create the native default ownerless technique")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            demo, cna_effect_technique_create_named(
                       text_view(technique_name), &named_technique),
            "create a named ownerless technique with its canonical P0 pass")) {
        goto cleanup;
    }
    memset(technique_name, '#', sizeof(technique_name) - 1U);

    memset(copied, 0x5A, sizeof(copied));
    cna_demo_check(
        demo,
        cna_effect_technique_get_name_byte_count(default_technique, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(0) &&
            cna_effect_technique_get_name_byte_count(named_technique, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(8) &&
            cna_effect_technique_copy_name(named_technique, copied, UINT64_C(7), &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == UINT64_C(8) && bytes_are(copied, sizeof(copied), 0x5A),
        "default and named techniques report copied names and atomic short-buffer refusal");
    cna_demo_check(
        demo,
        cna_effect_technique_copy_name(named_technique, copied, UINT64_C(8), &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(copied, "Lighting", 8U) == 0 && (unsigned char)copied[8] == 0x5A,
        "an exact technique-name copy preserves the caller's following sentinel byte");

    cna_demo_check(
        demo,
        cna_effect_technique_get_identity(default_technique, &default_identity) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(named_technique, &named_identity) ==
                CNA_RESULT_SUCCESS &&
            default_identity != UINT64_C(0) && named_identity != UINT64_C(0) &&
            default_identity != named_identity,
        "independently constructed techniques expose distinct nonzero stable identities");
    cna_demo_check(
        demo,
        cna_effect_technique_get_index_ext(default_technique, &technique_index) ==
                CNA_RESULT_SUCCESS &&
            technique_index == UINT32_C(0) &&
            cna_effect_technique_get_index_ext(named_technique, &technique_index) ==
                CNA_RESULT_SUCCESS &&
            technique_index == UINT32_C(0),
        "ordinary ownerless techniques report the documented zero runtime index");

    result = cna_effect_technique_get_passes(default_technique, &default_passes);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_get_passes(named_technique, &named_passes);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(default_passes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_pass_collection_get_count(named_passes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "the default technique starts empty while a named technique owns one canonical pass");

    memset(copied, 0x5A, sizeof(copied));
    cna_demo_check(
        demo,
        cna_effect_pass_collection_get_at(named_passes, UINT64_C(0), &p0) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_pass_get_name_byte_count(p0, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(2) &&
            cna_effect_pass_copy_name(p0, copied, UINT64_C(2), &bytes) == CNA_RESULT_SUCCESS &&
            memcmp(copied, "P0", 2U) == 0 && (unsigned char)copied[2] == 0x5A &&
            cna_effect_pass_get_index_ext(p0, &pass_index) == CNA_RESULT_SUCCESS &&
            pass_index == UINT32_C(0) && cna_effect_pass_apply(p0) == CNA_RESULT_SUCCESS,
        "the canonical P0 pass reports index zero and performs its ownerless successful no-op apply");

    result = cna_effect_technique_get_annotations(named_technique, &technique_annotations);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_pass_get_annotations(p0, &pass_annotations);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = create_annotation("TechniqueTag", &annotation);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_annotation_collection_add(technique_annotations, annotation);
    }
    if (annotation != CNA_INVALID_HANDLE) {
        const CNA_Result destroy_result = cna_effect_annotation_destroy(annotation);
        if (result == CNA_RESULT_SUCCESS) {
            result = destroy_result;
        }
        if (destroy_result == CNA_RESULT_SUCCESS) {
            annotation = CNA_INVALID_HANDLE;
        }
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = create_annotation("PassTag", &annotation);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_annotation_collection_add(pass_annotations, annotation);
    }
    if (annotation != CNA_INVALID_HANDLE) {
        const CNA_Result destroy_result = cna_effect_annotation_destroy(annotation);
        if (result == CNA_RESULT_SUCCESS) {
            result = destroy_result;
        }
        if (destroy_result == CNA_RESULT_SUCCESS) {
            annotation = CNA_INVALID_HANDLE;
        }
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_collection_get_count(technique_annotations, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_effect_annotation_collection_get_count(pass_annotations, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "technique and pass annotation views retain separate copied reflection values");

    result = cna_effect_pass_create(text_view(pass_name), named_identity, &standalone);
    memset(pass_name, '#', sizeof(pass_name) - 1U);
    memset(copied, 0x5A, sizeof(copied));
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_get_name_byte_count(standalone, &bytes) == CNA_RESULT_SUCCESS &&
            bytes == UINT64_C(10) &&
            cna_effect_pass_copy_name(standalone, copied, UINT64_C(9), &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            bytes == UINT64_C(10) && bytes_are(copied, sizeof(copied), 0x5A),
        "a standalone pass copies its name and refuses a short destination atomically");
    cna_demo_check(
        demo,
        cna_effect_pass_copy_name(standalone, copied, UINT64_C(10), &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(copied, "Standalone", 10U) == 0 &&
            cna_effect_pass_get_index_ext(standalone, &pass_index) == CNA_RESULT_SUCCESS &&
            pass_index == UINT32_C(0) && cna_effect_pass_apply(standalone) == CNA_RESULT_SUCCESS,
        "the ordinary standalone pass reports index zero and applies as an ownerless no-op");

    cna_demo_check(
        demo,
        cna_effect_pass_create_indexed_ext(
            text_view("Indexed"), named_identity, UINT32_C(7), &indexed) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_pass_get_index_ext(indexed, &pass_index) == CNA_RESULT_SUCCESS &&
            pass_index == UINT32_C(7) && cna_effect_pass_apply(indexed) == CNA_RESULT_SUCCESS,
        "an indexed standalone pass preserves its independent reflected runtime index");

    result = cna_effect_technique_create_reflected_ext(
        text_view("EmptyReflected"), UINT32_C(3), CNA_FALSE, &reflected_empty);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_create_reflected_ext(
            text_view("DefaultReflected"), UINT32_C(5), CNA_TRUE, &reflected_default);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_index_ext(reflected_empty, &technique_index) ==
                CNA_RESULT_SUCCESS &&
            technique_index == UINT32_C(3) &&
            cna_effect_technique_get_index_ext(reflected_default, &technique_index) ==
                CNA_RESULT_SUCCESS &&
            technique_index == UINT32_C(5) &&
            cna_effect_technique_get_identity(reflected_empty, &empty_identity) ==
                CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(reflected_default, &reflected_identity) ==
                CNA_RESULT_SUCCESS &&
            empty_identity != UINT64_C(0) && reflected_identity != UINT64_C(0) &&
            empty_identity != reflected_identity,
        "reflected techniques preserve authored indices while retaining unique identities");
    result = cna_effect_technique_get_passes(reflected_empty, &reflected_empty_passes);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_get_passes(
            reflected_default, &reflected_default_passes);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(reflected_empty_passes, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_pass_collection_get_count(reflected_default_passes, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "reflected construction independently selects an empty or canonical-P0 pass list");

    result = cna_effect_technique_destroy(named_technique);
    if (result == CNA_RESULT_SUCCESS) {
        named_technique = CNA_INVALID_HANDLE;
    }
    if (p0 != CNA_INVALID_HANDLE) {
        const CNA_Result destroy_result = cna_effect_pass_destroy(p0);
        if (result == CNA_RESULT_SUCCESS) {
            result = destroy_result;
        }
        if (destroy_result == CNA_RESULT_SUCCESS) {
            p0 = CNA_INVALID_HANDLE;
        }
    }
    if (named_passes != CNA_INVALID_HANDLE) {
        const CNA_Result destroy_result = cna_effect_pass_collection_destroy(named_passes);
        if (result == CNA_RESULT_SUCCESS) {
            result = destroy_result;
        }
        if (destroy_result == CNA_RESULT_SUCCESS) {
            named_passes = CNA_INVALID_HANDLE;
        }
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_annotation_collection_get_count(technique_annotations, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_effect_annotation_collection_get_count(pass_annotations, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "owned annotation views remain usable after their technique, pass and pass view are released");

cleanup:
    if (annotation != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_destroy(annotation);
    }
    if (technique_annotations != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_collection_destroy(technique_annotations);
    }
    if (pass_annotations != CNA_INVALID_HANDLE) {
        (void)cna_effect_annotation_collection_destroy(pass_annotations);
    }
    if (p0 != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(p0);
    }
    if (standalone != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(standalone);
    }
    if (indexed != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(indexed);
    }
    if (default_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(default_passes);
    }
    if (named_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(named_passes);
    }
    if (reflected_empty_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(reflected_empty_passes);
    }
    if (reflected_default_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(reflected_default_passes);
    }
    if (default_technique != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(default_technique);
    }
    if (named_technique != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(named_technique);
    }
    if (reflected_empty != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(reflected_empty);
    }
    if (reflected_default != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(reflected_default);
    }
}

static void demonstrate_collections(CnaDemo* demo)
{
    CNA_EffectPassCollectionHandle passes = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueCollectionHandle techniques = CNA_INVALID_HANDLE;
    CNA_EffectPassCollectionHandle nested_passes = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle first_pass = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle indexed_pass = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle found_pass = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle nested_pass = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle first_technique = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle indexed_technique = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle found_technique = CNA_INVALID_HANDLE;
    CNA_EffectPassHandle missing_pass = CNA_INVALID_HANDLE;
    CNA_EffectTechniqueHandle missing_technique = CNA_INVALID_HANDLE;
    CNA_Result result;
    CNA_Bool found = CNA_FALSE;
    uint64_t count = UINT64_C(0);
    uint64_t first_identity = UINT64_C(0);
    uint64_t indexed_identity = UINT64_C(0);
    uint64_t found_identity = UINT64_C(0);
    char pass_name[] = "FirstPass";
    char technique_name[] = "FirstTechnique";
    int index;

    result = cna_effect_pass_collection_create(&passes);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_collection_create(&techniques);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(passes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_effect_technique_collection_get_count(techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(0),
        "independent pass and technique collections start empty");
    if (result != CNA_RESULT_SUCCESS) {
        goto cleanup;
    }

    result = cna_effect_pass_collection_add_create(
        passes, text_view(pass_name), UINT64_C(11), &first_pass);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_collection_add_named(
            techniques, text_view(technique_name), &first_technique);
    }
    memset(pass_name, '#', sizeof(pass_name) - 1U);
    memset(technique_name, '#', sizeof(technique_name) - 1U);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_get_identity(first_technique, &first_identity);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && first_identity != UINT64_C(0),
        "collection add calls copy both names and return stable owned element views");

    for (index = 0; result == CNA_RESULT_SUCCESS && index < 32; ++index) {
        CNA_EffectPassHandle added_pass = CNA_INVALID_HANDLE;
        CNA_EffectTechniqueHandle added_technique = CNA_INVALID_HANDLE;
        result = cna_effect_pass_collection_add_create(
            passes, text_view("ExtraPass"), (uint64_t)index, &added_pass);
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_effect_technique_collection_add_named(
                techniques, text_view("ExtraTechnique"), &added_technique);
        }
        if (added_pass != CNA_INVALID_HANDLE) {
            const CNA_Result destroy_result = cna_effect_pass_destroy(added_pass);
            if (result == CNA_RESULT_SUCCESS) {
                result = destroy_result;
            }
        }
        if (added_technique != CNA_INVALID_HANDLE) {
            const CNA_Result destroy_result = cna_effect_technique_destroy(added_technique);
            if (result == CNA_RESULT_SUCCESS) {
                result = destroy_result;
            }
        }
    }
    {
        CNA_EffectTechniqueHandle default_element = CNA_INVALID_HANDLE;
        if (result == CNA_RESULT_SUCCESS) {
            result = cna_effect_technique_collection_add_default(
                techniques, &default_element);
        }
        if (default_element != CNA_INVALID_HANDLE) {
            const CNA_Result destroy_result = cna_effect_technique_destroy(default_element);
            if (result == CNA_RESULT_SUCCESS) {
                result = destroy_result;
            }
        }
    }
    indexed_identity = UINT64_C(0);
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(passes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(33) &&
            cna_effect_technique_collection_get_count(techniques, &count) ==
                CNA_RESULT_SUCCESS &&
            count == UINT64_C(34) && cna_effect_pass_apply(first_pass) == CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(first_technique, &indexed_identity) ==
                CNA_RESULT_SUCCESS &&
            indexed_identity == first_identity,
        "first pass and technique views remain stable after substantial collection growth");

    found = CNA_FALSE;
    result = cna_effect_pass_collection_get_at(passes, UINT64_C(0), &indexed_pass);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_pass_collection_find(
            passes, text_view("FirstPass"), &found, &found_pass);
    }
    if (result == CNA_RESULT_SUCCESS && found == CNA_TRUE) {
        result = cna_effect_technique_collection_get_at(
            techniques, UINT64_C(0), &indexed_technique);
    }
    found = CNA_FALSE;
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_collection_find(
            techniques, text_view("FirstTechnique"), &found, &found_technique);
    }
    if (result == CNA_RESULT_SUCCESS && found == CNA_TRUE) {
        result = cna_effect_technique_get_identity(indexed_technique, &indexed_identity);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_get_identity(found_technique, &found_identity);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && found == CNA_TRUE &&
            cna_effect_pass_apply(indexed_pass) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(found_pass) == CNA_RESULT_SUCCESS &&
            indexed_identity == first_identity && found_identity == first_identity,
        "index and exact-name lookups publish owned views of the same collection elements");

    found = CNA_TRUE;
    missing_pass = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_effect_pass_collection_find(
            passes, text_view("firstpass"), &found, &missing_pass) == CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && missing_pass == CNA_INVALID_HANDLE,
        "pass lookup is case-sensitive and absence publishes no handle");
    found = CNA_TRUE;
    missing_technique = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_effect_technique_collection_find(
            techniques, text_view("firsttechnique"), &found, &missing_technique) ==
                CNA_RESULT_SUCCESS &&
            found == CNA_FALSE && missing_technique == CNA_INVALID_HANDLE,
        "technique lookup is case-sensitive and absence publishes no handle");
    missing_pass = UINT64_MAX;
    missing_technique = UINT64_MAX;
    cna_demo_check(
        demo,
        cna_effect_pass_collection_get_at(passes, UINT64_C(33), &missing_pass) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            missing_pass == CNA_INVALID_HANDLE &&
            cna_effect_technique_collection_get_at(
                techniques, UINT64_C(34), &missing_technique) == CNA_RESULT_INVALID_ARGUMENT &&
            missing_technique == CNA_INVALID_HANDLE,
        "both collection upper bounds are refused before publishing a handle");
    missing_pass = CNA_INVALID_HANDLE;
    missing_technique = CNA_INVALID_HANDLE;

    result = cna_effect_technique_get_passes(first_technique, &nested_passes);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_pass_collection_get_at(nested_passes, UINT64_C(0), &nested_pass);
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && cna_effect_pass_apply(nested_pass) == CNA_RESULT_SUCCESS,
        "a technique element exposes its canonical P0 descendant through an owned collection view");

    result = cna_effect_pass_collection_destroy(passes);
    if (result == CNA_RESULT_SUCCESS) {
        passes = CNA_INVALID_HANDLE;
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_effect_technique_collection_destroy(techniques);
        if (result == CNA_RESULT_SUCCESS) {
            techniques = CNA_INVALID_HANDLE;
        }
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && cna_effect_pass_apply(first_pass) == CNA_RESULT_SUCCESS &&
            cna_effect_pass_apply(found_pass) == CNA_RESULT_SUCCESS &&
            cna_effect_technique_get_identity(first_technique, &indexed_identity) ==
                CNA_RESULT_SUCCESS &&
            indexed_identity == first_identity &&
            cna_effect_pass_collection_get_count(nested_passes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1),
        "element and nested views remain usable after both root collections are destroyed");

    result = cna_effect_technique_destroy(first_technique);
    if (result == CNA_RESULT_SUCCESS) {
        first_technique = CNA_INVALID_HANDLE;
    }
    if (indexed_technique != CNA_INVALID_HANDLE) {
        const CNA_Result destroy_result = cna_effect_technique_destroy(indexed_technique);
        if (result == CNA_RESULT_SUCCESS) {
            result = destroy_result;
        }
        if (destroy_result == CNA_RESULT_SUCCESS) {
            indexed_technique = CNA_INVALID_HANDLE;
        }
    }
    if (found_technique != CNA_INVALID_HANDLE) {
        const CNA_Result destroy_result = cna_effect_technique_destroy(found_technique);
        if (result == CNA_RESULT_SUCCESS) {
            result = destroy_result;
        }
        if (destroy_result == CNA_RESULT_SUCCESS) {
            found_technique = CNA_INVALID_HANDLE;
        }
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_collection_get_count(nested_passes, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) && cna_effect_pass_apply(nested_pass) == CNA_RESULT_SUCCESS,
        "the nested pass collection and P0 survive destruction of every technique-element view");

    result = cna_effect_pass_collection_destroy(nested_passes);
    if (result == CNA_RESULT_SUCCESS) {
        nested_passes = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS && cna_effect_pass_apply(nested_pass) == CNA_RESULT_SUCCESS,
        "the owned P0 descendant remains usable after its nested collection view is destroyed");

    missing_pass = first_pass;
    result = cna_effect_pass_destroy(first_pass);
    if (result == CNA_RESULT_SUCCESS) {
        first_pass = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        demo,
        result == CNA_RESULT_SUCCESS &&
            cna_effect_pass_destroy(missing_pass) == CNA_RESULT_INVALID_HANDLE,
        "a pass element view releases exactly once and then becomes stale");
    missing_pass = CNA_INVALID_HANDLE;

cleanup:
    if (missing_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(missing_pass);
    }
    if (missing_technique != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(missing_technique);
    }
    if (nested_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(nested_pass);
    }
    if (nested_passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(nested_passes);
    }
    if (indexed_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(indexed_pass);
    }
    if (found_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(found_pass);
    }
    if (first_pass != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_destroy(first_pass);
    }
    if (indexed_technique != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(indexed_technique);
    }
    if (found_technique != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(found_technique);
    }
    if (first_technique != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_destroy(first_technique);
    }
    if (passes != CNA_INVALID_HANDLE) {
        (void)cna_effect_pass_collection_destroy(passes);
    }
    if (techniques != CNA_INVALID_HANDLE) {
        (void)cna_effect_technique_collection_destroy(techniques);
    }
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    cna_demo_init(&demo, "c_api.graphics.effect_passes_techniques", argc, argv, 0);
    demonstrate_standalone_values(&demo);
    demonstrate_collections(&demo);
    return cna_demo_finish(&demo);
}
