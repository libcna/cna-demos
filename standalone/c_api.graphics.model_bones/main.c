/* SPDX-License-Identifier: MS-PL */
/* ModelBone values, retained hierarchies and live collection views through C. */
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

static CNA_Matrix identity_matrix(void)
{
    CNA_Matrix value;
    memset(&value, 0, sizeof(value));
    value.m11 = 1.0F;
    value.m22 = 1.0F;
    value.m33 = 1.0F;
    value.m44 = 1.0F;
    return value;
}

static int matrix_is(CNA_Matrix left, CNA_Matrix right)
{
    return memcmp(&left, &right, sizeof(left)) == 0;
}

static int bone_name_is(CNA_ModelBoneHandle bone, const char* expected)
{
    char text[64];
    char short_text[64];
    char short_before[64];
    uint64_t size = UINT64_C(0);
    uint64_t copied = UINT64_C(99);
    const uint64_t expected_size = (uint64_t)strlen(expected);

    memset(text, 0, sizeof(text));
    memset(short_text, 0xA5, sizeof(short_text));
    memcpy(short_before, short_text, sizeof(short_text));
    if (cna_model_bone_get_name_byte_count(bone, &size) != CNA_RESULT_SUCCESS ||
        size != expected_size) {
        return 0;
    }
    if (expected_size == UINT64_C(0)) {
        return cna_model_bone_copy_name(bone, NULL, UINT64_C(0), &copied) ==
                   CNA_RESULT_SUCCESS &&
               copied == UINT64_C(0);
    }
    return cna_model_bone_copy_name(
               bone, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           cna_model_bone_copy_name(bone, text, expected_size, &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int release_bone(CNA_ModelBoneHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_model_bone_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_collection(CNA_ModelBoneCollectionHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_model_bone_collection_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_ModelBoneHandle default_bone = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle stale_bone = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle root = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle left = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle right = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle grandchild = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle at_view = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle found_view = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle parent_view = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle absent_view = CNA_INVALID_HANDLE;
    CNA_ModelBoneHandle retained_child = CNA_INVALID_HANDLE;
    CNA_ModelBoneCollectionHandle default_children = CNA_INVALID_HANDLE;
    CNA_ModelBoneCollectionHandle children = CNA_INVALID_HANDLE;
    CNA_ModelBoneCollectionHandle parent_children = CNA_INVALID_HANDLE;
    CNA_ModelBoneCollectionHandle empty = CNA_INVALID_HANDLE;
    CNA_ModelBoneCollectionHandle stale_collection = CNA_INVALID_HANDLE;
    CNA_Matrix identity = identity_matrix();
    CNA_Matrix transform = identity;
    CNA_Matrix observed;
    CNA_Bool flag = (CNA_Bool)9;
    uint64_t count = UINT64_C(99);
    uint64_t untouched = UINT64_C(0x1122334455667788);
    int32_t index = INT32_C(-999);
    char copied_name[] = "LeftArm";
    static const char InvalidUtf8[] = {(char)0xC3, (char)0x28, '\0'};
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.graphics.model_bones", argc, argv, 1);

    if (!cna_demo_check_result(
            &demo, cna_model_bone_create_default(&default_bone),
            "create an owned default ModelBone")) {
        goto cleanup;
    }
    memset(&observed, 0xA5, sizeof(observed));
    flag = (CNA_Bool)9;
    absent_view = UINT64_MAX;
    cna_demo_check(
        &demo,
        bone_name_is(default_bone, "") &&
            cna_model_bone_get_index(default_bone, &index) == CNA_RESULT_SUCCESS &&
            index == INT32_C(0) &&
            cna_model_bone_get_transform(default_bone, &observed) == CNA_RESULT_SUCCESS &&
            matrix_is(observed, identity) &&
            cna_model_bone_get_parent(default_bone, &flag, &absent_view) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && absent_view == CNA_INVALID_HANDLE,
        "the default bone has empty text, index zero, identity transform and no parent");
    if (cna_demo_check_result(
            &demo, cna_model_bone_get_children(default_bone, &default_children),
            "obtain the default bone's live child collection")) {
        cna_demo_check(
            &demo,
            cna_model_bone_collection_get_count(default_children, &count) == CNA_RESULT_SUCCESS &&
                count == UINT64_C(0),
            "a new default bone starts with an empty live child collection");
    }

    if (!cna_demo_check(
            &demo,
            cna_model_bone_create(INT32_C(-7), text_view("Root"), &root) ==
                    CNA_RESULT_SUCCESS &&
                cna_model_bone_create(INT32_C(42), text_view(copied_name), &left) ==
                    CNA_RESULT_SUCCESS &&
                cna_model_bone_create(INT32_C(43), text_view("RightArm"), &right) ==
                    CNA_RESULT_SUCCESS &&
                cna_model_bone_create(INT32_C(99), text_view("Finger"), &grandchild) ==
                    CNA_RESULT_SUCCESS,
            "create four named bones with signed indices preserved verbatim")) {
        goto cleanup;
    }
    copied_name[0] = 'X';
    cna_demo_check(
        &demo,
        bone_name_is(left, "LeftArm") &&
            cna_model_bone_get_index(root, &index) == CNA_RESULT_SUCCESS &&
            index == INT32_C(-7) &&
            cna_model_bone_get_index(left, &index) == CNA_RESULT_SUCCESS &&
            index == INT32_C(42),
        "bone construction copies UTF-8 text and retains exact signed indices");
    absent_view = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_model_bone_create(INT32_C(1), text_view(InvalidUtf8), &absent_view) ==
                CNA_RESULT_ENCODING &&
            absent_view == CNA_INVALID_HANDLE,
        "malformed UTF-8 is refused without publishing a bone handle");

    transform.m11 = 2.0F;
    transform.m22 = 3.0F;
    transform.m33 = 4.0F;
    transform.m41 = 5.0F;
    transform.m42 = -6.0F;
    transform.m43 = 7.0F;
    memset(&observed, 0, sizeof(observed));
    cna_demo_check(
        &demo,
        cna_model_bone_set_transform(left, transform) == CNA_RESULT_SUCCESS &&
            cna_model_bone_get_transform(left, &observed) == CNA_RESULT_SUCCESS &&
            matrix_is(observed, transform),
        "a complete nonidentity local transform round-trips exactly");

    if (!cna_demo_check_result(
            &demo, cna_model_bone_get_children(root, &children),
            "obtain a live view before adding root children")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_model_bone_collection_get_count(children, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(0) &&
            cna_model_bone_add_child(root, left) == CNA_RESULT_SUCCESS &&
            cna_model_bone_collection_get_count(children, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(1) &&
            cna_model_bone_add_child(root, right) == CNA_RESULT_SUCCESS &&
            cna_model_bone_collection_get_count(children, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "the pre-existing child collection tracks later additions in authored order");

    cna_demo_check(
        &demo,
        cna_model_bone_collection_get_at(children, UINT64_C(0), &at_view) ==
                CNA_RESULT_SUCCESS &&
            bone_name_is(at_view, "LeftArm") &&
            cna_model_bone_collection_contains(children, at_view, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_model_bone_collection_contains(children, grandchild, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "indexed lookup returns the exact contained bone rather than a value copy");

    cna_demo_check(
        &demo,
        cna_model_bone_collection_find(
            children, text_view("RightArm"), &flag, &found_view) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && bone_name_is(found_view, "RightArm") &&
            cna_model_bone_collection_find(
                children, text_view("Missing"), &flag, &absent_view) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && absent_view == CNA_INVALID_HANDLE,
        "name lookup distinguishes an owned exact match from successful absence");
    untouched = UINT64_C(0x1122334455667788);
    absent_view = UINT64_MAX;
    cna_demo_check(
        &demo,
        cna_model_bone_collection_get_at(children, count, &absent_view) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            absent_view == CNA_INVALID_HANDLE &&
            cna_model_bone_collection_get_count(children, &untouched) == CNA_RESULT_SUCCESS &&
            untouched == count,
        "the Count index is refused without disturbing the live collection");

    cna_demo_check(
        &demo,
        cna_model_bone_get_parent(left, &flag, &parent_view) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && bone_name_is(parent_view, "Root") &&
            cna_model_bone_get_children(parent_view, &parent_children) == CNA_RESULT_SUCCESS &&
            cna_model_bone_collection_contains(parent_children, left, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "a child exposes an owned parent view of the same retained hierarchy");
    cna_demo_check(
        &demo,
        cna_model_bone_add_child(left, grandchild) == CNA_RESULT_SUCCESS &&
            cna_model_bone_get_parent(grandchild, &flag, &absent_view) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE && bone_name_is(absent_view, "LeftArm"),
        "a second level links its parent and child graph consistently");
    releases_ok &= release_bone(&absent_view);
    cna_demo_check(
        &demo,
        cna_model_bone_add_child(grandchild, root) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_bone_add_child(root, root) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_model_bone_collection_get_count(children, &count) == CNA_RESULT_SUCCESS &&
            count == UINT64_C(2),
        "ancestor cycles and self-parenting are refused without changing root children");

    if (cna_demo_check_result(
            &demo, cna_model_bone_collection_create(&empty),
            "create an independent empty ModelBoneCollection")) {
        cna_demo_check(
            &demo,
            cna_model_bone_collection_get_count(empty, &count) == CNA_RESULT_SUCCESS &&
                count == UINT64_C(0) &&
                cna_model_bone_collection_contains(empty, root, &flag) == CNA_RESULT_SUCCESS &&
                flag == CNA_FALSE,
            "an independent collection is empty and contains no unrelated bone");
    }

    if (cna_demo_check_result(
            &demo,
            cna_model_bone_collection_get_at(children, UINT64_C(0), &retained_child),
            "retain one child through a stable collection view")) {
        releases_ok &= release_bone(&left);
        releases_ok &= release_bone(&root);
        releases_ok &= release_collection(&children);
        cna_demo_check(
            &demo,
            bone_name_is(retained_child, "LeftArm") &&
                cna_model_bone_get_transform(retained_child, &observed) == CNA_RESULT_SUCCESS &&
                matrix_is(observed, transform),
            "a returned bone view survives its original handles and collection view");
    }

    stale_bone = default_bone;
    cna_demo_check_result(
        &demo, cna_model_bone_destroy(default_bone),
        "release the default bone exactly once");
    default_bone = CNA_INVALID_HANDLE;
    index = INT32_C(-999);
    cna_demo_check(
        &demo,
        cna_model_bone_get_index(stale_bone, &index) == CNA_RESULT_INVALID_HANDLE &&
            index == INT32_C(-999),
        "a released bone rejects stale reads without changing output");
    stale_collection = empty;
    cna_demo_check_result(
        &demo, cna_model_bone_collection_destroy(empty),
        "release the independent collection exactly once");
    empty = CNA_INVALID_HANDLE;
    untouched = UINT64_C(0x1122334455667788);
    cna_demo_check(
        &demo,
        cna_model_bone_collection_get_count(stale_collection, &untouched) ==
                CNA_RESULT_INVALID_HANDLE &&
            untouched == UINT64_C(0x1122334455667788),
        "a released collection rejects stale reads without changing output");

cleanup:
    releases_ok &= release_collection(&empty);
    releases_ok &= release_collection(&parent_children);
    releases_ok &= release_collection(&children);
    releases_ok &= release_collection(&default_children);
    releases_ok &= release_bone(&retained_child);
    releases_ok &= release_bone(&absent_view);
    releases_ok &= release_bone(&parent_view);
    releases_ok &= release_bone(&found_view);
    releases_ok &= release_bone(&at_view);
    releases_ok &= release_bone(&grandchild);
    releases_ok &= release_bone(&right);
    releases_ok &= release_bone(&left);
    releases_ok &= release_bone(&root);
    releases_ok &= release_bone(&default_bone);
    cna_demo_check(
        &demo, releases_ok,
        "release every remaining collection and stable bone view");
    return cna_demo_finish(&demo);
}
