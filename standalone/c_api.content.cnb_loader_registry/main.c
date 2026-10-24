/* SPDX-License-Identifier: MS-PL */
/* Process-wide CNB loader registration, resolution, invocation and ownership through C. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct LoaderWitness {
    int calls;
    int document_usable;
    int manager_usable;
    uint32_t asset_type;
    uint64_t asset_name_size;
    char asset_name[32];
    int object_marker;
    CNA_CnbDocumentHandle borrowed_document;
    CNA_Handle borrowed_manager;
} LoaderWitness;

static CNA_StringView text_view(const char* text)
{
    CNA_StringView value;
    value.data = text;
    value.byte_length = (uint64_t)strlen(text);
    return value;
}

static CNA_Result on_load(
    void* context, CNA_CnbDocumentHandle document, CNA_Handle content_manager,
    CNA_StringView asset_name, void** out_object)
{
    LoaderWitness* witness = (LoaderWitness*)context;
    uint32_t asset_type = UINT32_C(0);
    uint64_t root_size = UINT64_C(0);
    ++witness->calls;
    witness->document_usable =
        cna_cnb_document_get_asset_type_id(document, &asset_type) == CNA_RESULT_SUCCESS;
    witness->manager_usable =
        cna_content_manager_get_root_directory_size(content_manager, &root_size) ==
        CNA_RESULT_SUCCESS;
    witness->asset_type = asset_type;
    witness->asset_name_size = asset_name.byte_length;
    if (asset_name.byte_length < sizeof(witness->asset_name)) {
        memcpy(witness->asset_name, asset_name.data, (size_t)asset_name.byte_length);
        witness->asset_name[asset_name.byte_length] = '\0';
    }
    witness->borrowed_document = document;
    witness->borrowed_manager = content_manager;
    *out_object = &witness->object_marker;
    return CNA_RESULT_SUCCESS;
}

static int registered_name_is(uint32_t asset_type, const char* expected)
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
    return cna_cnb_loader_registry_get_registered_type_name_size(asset_type, &size) ==
               CNA_RESULT_SUCCESS &&
           size == expected_size &&
           cna_cnb_loader_registry_copy_registered_type_name(
               asset_type, short_text, expected_size - UINT64_C(1), &copied) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
           copied == expected_size && memcmp(short_text, short_before, sizeof(short_text)) == 0 &&
           cna_cnb_loader_registry_copy_registered_type_name(
               asset_type, text, sizeof(text), &copied) == CNA_RESULT_SUCCESS &&
           copied == expected_size && memcmp(text, expected, (size_t)expected_size) == 0;
}

static int build_custom_document(
    uint32_t asset_type, const char* type_name, CNA_CnbDocumentHandle* out_document)
{
    static const uint8_t Payload[4] = {9U, 8U, 7U, 6U};
    CNA_CnbWriterHandle writer = CNA_INVALID_HANDLE;
    CNA_CnbChunkId chunk = UINT32_C(0);
    uint8_t image[4096];
    uint64_t image_size = UINT64_C(0);
    CNA_Result result = cna_cnb_writer_create(asset_type, UINT32_C(1), &writer);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_writer_set_metadata(
            writer, text_view(type_name), text_view("Custom/Level"));
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_make_chunk_id('G', 'A', 'M', 'E', &chunk);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_writer_add_chunk(
            writer, chunk, Payload, sizeof(Payload), CNA_CNB_CHUNK_FLAG_MANDATORY,
            UINT32_C(4));
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_writer_build(writer, image, sizeof(image), &image_size);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_document_parse(
            image, image_size, text_view("custom-memory"), NULL, out_document);
    }
    if (writer != CNA_INVALID_HANDLE) {
        (void)cna_cnb_writer_destroy(writer);
    }
    return result == CNA_RESULT_SUCCESS;
}

static int build_curve_document(CNA_CnbDocumentHandle* out_document)
{
    CNA_CurveHandle curve = CNA_INVALID_HANDLE;
    uint8_t image[4096];
    uint64_t image_size = UINT64_C(0);
    CNA_Result result = cna_curve_create(&curve);
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_encode_curve(
            curve, text_view("Curves/Empty"), image, sizeof(image), &image_size);
    }
    if (result == CNA_RESULT_SUCCESS) {
        result = cna_cnb_document_parse(
            image, image_size, text_view("curve-memory"), NULL, out_document);
    }
    if (curve != CNA_INVALID_HANDLE) {
        (void)cna_curve_destroy(curve);
    }
    return result == CNA_RESULT_SUCCESS;
}

static int release_loader(CNA_CnbLoaderHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_loader_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

static int release_document(CNA_CnbDocumentHandle* handle)
{
    CNA_Result result;
    if (*handle == CNA_INVALID_HANDLE) {
        return 1;
    }
    result = cna_cnb_document_destroy(*handle);
    *handle = CNA_INVALID_HANDLE;
    return result == CNA_RESULT_SUCCESS;
}

int main(int argc, char** argv)
{
    static const char TypeName[] = "Contoso.Game.yM7rfen420Vc";
    static const char OtherName[] = "Contoso.Game.TYRPO1kY3mv6";
    CnaDemo demo;
    LoaderWitness witness;
    CNA_PresentationParameters presentation;
    CNA_ContentManagerCreateInfo manager_info;
    CNA_Handle device = CNA_INVALID_HANDLE;
    CNA_Handle manager = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle document = CNA_INVALID_HANDLE;
    CNA_CnbDocumentHandle curve_document = CNA_INVALID_HANDLE;
    CNA_CnbLoaderHandle loader = CNA_INVALID_HANDLE;
    CNA_CnbLoaderHandle by_number = CNA_INVALID_HANDLE;
    CNA_CnbLoaderHandle builtin = CNA_INVALID_HANDLE;
    CNA_CnbLoaderHandle absent_loader = UINT64_MAX;
    uint32_t asset_type = UINT32_C(0);
    uint32_t collision_type = UINT32_C(0);
    uint32_t borrowed_type = UINT32_C(0);
    uint64_t root_size = UINT64_C(0);
    uint64_t name_size = UINT64_MAX;
    CNA_Bool state = CNA_TRUE;
    CNA_Bool removed = CNA_TRUE;
    void* object = NULL;
    char empty_name[8];
    int releases_ok = 1;

    cna_demo_init(&demo, "c_api.content.cnb_loader_registry", argc, argv, 1);
    memset(&witness, 0, sizeof(witness));
    memset(&presentation, 0, sizeof(presentation));
    if (!cna_demo_check(
            &demo,
            cna_presentation_parameters_init(&presentation) == CNA_RESULT_SUCCESS,
            "initialize presentation state for the loader's required ContentManager")) {
        goto cleanup;
    }
    presentation.back_buffer_width = INT32_C(64);
    presentation.back_buffer_height = INT32_C(64);
    presentation.is_full_screen = CNA_FALSE;
    presentation.presentation_interval = CNA_PRESENT_INTERVAL_IMMEDIATE;
    if (!cna_demo_check_result(
            &demo,
            cna_graphics_device_create(
                UINT32_C(0), CNA_GRAPHICS_PROFILE_REACH, &presentation, &device),
            "create the caller-owned device needed only by the ContentManager")) {
        goto cleanup;
    }
    memset(&manager_info, 0, sizeof(manager_info));
    manager_info.struct_size = (uint32_t)sizeof(manager_info);
    manager_info.struct_version = UINT32_C(1);
    manager_info.root_directory = text_view(".");
    if (!cna_demo_check_result(
            &demo, cna_content_manager_create_resource(device, &manager_info, &manager),
            "create the ContentManager required by the canonical loader signature")) {
        goto cleanup;
    }
    cna_demo_check_result(
        &demo, cna_cnb_loader_registry_clear(),
        "start from an empty process-wide loader registry");
    cna_demo_check(
        &demo,
        cna_cnb_asset_type_id_from_name(text_view(TypeName), &asset_type) ==
                CNA_RESULT_SUCCESS &&
            cna_cnb_asset_type_id_from_name(text_view(OtherName), &collision_type) ==
                CNA_RESULT_SUCCESS &&
            collision_type == asset_type &&
            cna_cnb_loader_registry_is_registered(asset_type, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_FALSE &&
            cna_cnb_loader_registry_find(asset_type, &state, &absent_loader) ==
                CNA_RESULT_SUCCESS &&
            state == CNA_FALSE && absent_loader == CNA_INVALID_HANDLE &&
            cna_cnb_loader_registry_get_registered_type_name_size(asset_type, &name_size) ==
                CNA_RESULT_SUCCESS &&
            name_size == UINT64_C(0) &&
            cna_cnb_loader_registry_copy_registered_type_name(
                asset_type, empty_name, sizeof(empty_name), &name_size) == CNA_RESULT_SUCCESS &&
            name_size == UINT64_C(0) &&
            cna_cnb_loader_registry_remove(asset_type, &removed) == CNA_RESULT_SUCCESS &&
            removed == CNA_FALSE,
        "a real hash-collision identity starts with ordinary empty registry answers");

    if (!cna_demo_check_result(
            &demo,
            cna_cnb_loader_registry_register(
                asset_type, text_view(TypeName), on_load, &witness),
            "register a custom loader under its hashed canonical identity")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_cnb_loader_registry_is_registered(asset_type, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_TRUE && registered_name_is(asset_type, TypeName),
        "registration state and the canonical name are exact and atomically copied");
    cna_demo_check(
        &demo,
        cna_cnb_loader_registry_register(
            asset_type, text_view(TypeName), on_load, &witness) == CNA_RESULT_SUCCESS &&
            cna_cnb_loader_registry_register(
                asset_type, text_view(OtherName), on_load, &witness) ==
                CNA_RESULT_INVALID_STATE &&
            cna_cnb_loader_registry_register(
                CNA_CNB_ASSET_TYPE_CURVE, text_view("Microsoft.Xna.Framework.Curve"),
                on_load, &witness) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_cnb_loader_registry_register(
                asset_type + UINT32_C(1), text_view(TypeName), on_load, &witness) ==
                CNA_RESULT_INVALID_ARGUMENT,
        "duplicate identity is idempotent while collisions and built-in claims are refused");

    if (!cna_demo_check(
            &demo, build_custom_document(asset_type, TypeName, &document),
            "build and parse a custom document carrying the registered identity and name")) {
        goto cleanup;
    }
    if (!cna_demo_check_result(
            &demo,
            cna_cnb_loader_registry_resolve_for_document(document, &loader),
            "resolve an owned loader copy after validating the document identity")) {
        goto cleanup;
    }
    object = NULL;
    cna_demo_check(
        &demo,
        cna_cnb_loader_invoke(
            loader, document, manager, text_view("Levels/One"), &object) ==
                CNA_RESULT_SUCCESS &&
            object == &witness.object_marker && witness.calls == 1 &&
            witness.document_usable && witness.manager_usable &&
            witness.asset_type == asset_type && witness.asset_name_size == UINT64_C(10) &&
            strcmp(witness.asset_name, "Levels/One") == 0,
        "invocation passes live borrowed handles, exact asset text and the caller's object");
    cna_demo_check(
        &demo,
        cna_cnb_document_get_asset_type_id(witness.borrowed_document, &borrowed_type) ==
                CNA_RESULT_INVALID_HANDLE &&
            cna_content_manager_get_root_directory_size(
                witness.borrowed_manager, &root_size) == CNA_RESULT_INVALID_HANDLE,
        "both callback-scoped handles expire immediately after the callback returns");
    cna_demo_check(
        &demo,
        cna_cnb_loader_registry_find(asset_type, &state, &by_number) == CNA_RESULT_SUCCESS &&
            state == CNA_TRUE && by_number != CNA_INVALID_HANDLE,
        "numeric lookup returns a second independently owned loader copy");
    cna_demo_check(
        &demo,
        cna_cnb_loader_registry_remove(asset_type, &removed) == CNA_RESULT_SUCCESS &&
            removed == CNA_TRUE &&
            cna_cnb_loader_registry_is_registered(asset_type, &state) == CNA_RESULT_SUCCESS &&
            state == CNA_FALSE &&
            ((absent_loader = UINT64_MAX),
             cna_cnb_loader_registry_resolve_for_document(document, &absent_loader)) ==
                CNA_RESULT_IO &&
            absent_loader == CNA_INVALID_HANDLE,
        "removal empties future lookup and document resolution");
    object = NULL;
    cna_demo_check(
        &demo,
        cna_cnb_loader_invoke(
            loader, document, manager, text_view(""), &object) == CNA_RESULT_SUCCESS &&
            object == &witness.object_marker &&
            cna_cnb_loader_invoke(
                by_number, document, manager, text_view(""), &object) == CNA_RESULT_SUCCESS &&
            witness.calls == 3,
        "both copied loaders remain invocable after their registry entry is removed");
    object = &witness;
    cna_demo_check(
        &demo,
        cna_cnb_loader_invoke(
            loader, document, CNA_INVALID_HANDLE, text_view(""), &object) ==
                CNA_RESULT_INVALID_HANDLE &&
            object == NULL,
        "loader invocation requires a live ContentManager and clears partial output");

    if (!cna_demo_check(
            &demo,
            cna_cnb_loader_registry_register_builtins() == CNA_RESULT_SUCCESS &&
                cna_cnb_loader_registry_register_builtins() == CNA_RESULT_SUCCESS &&
                cna_cnb_loader_registry_find(
                    CNA_CNB_ASSET_TYPE_CURVE, &state, &builtin) == CNA_RESULT_SUCCESS &&
                state == CNA_TRUE && builtin != CNA_INVALID_HANDLE &&
                build_curve_document(&curve_document),
            "idempotently register and find the neutral built-in Curve loader")) {
        goto cleanup;
    }
    object = &witness;
    cna_demo_check(
        &demo,
        cna_cnb_loader_invoke(
            builtin, curve_document, manager, text_view("Curves/Empty"), &object) ==
                CNA_RESULT_NOT_SUPPORTED &&
            object == NULL,
        "a built-in loader refuses to expose its C++ Curve object as an opaque C object");
    cna_demo_check_result(&demo, cna_cnb_loader_destroy(loader), "release the resolved loader copy");
    cna_demo_check(
        &demo,
        cna_cnb_loader_invoke(
            loader, document, manager, text_view(""), &object) == CNA_RESULT_INVALID_HANDLE,
        "the released loader copy rejects stale invocation");
    loader = CNA_INVALID_HANDLE;

cleanup:
    releases_ok &= release_loader(&builtin);
    releases_ok &= release_loader(&by_number);
    releases_ok &= release_loader(&loader);
    releases_ok &= release_document(&curve_document);
    releases_ok &= release_document(&document);
    if (cna_cnb_loader_registry_clear() != CNA_RESULT_SUCCESS) {
        releases_ok = 0;
    }
    if (manager != CNA_INVALID_HANDLE) {
        releases_ok &= cna_content_manager_destroy(manager) == CNA_RESULT_SUCCESS;
        manager = CNA_INVALID_HANDLE;
    }
    if (device != CNA_INVALID_HANDLE) {
        releases_ok &= cna_graphics_device_destroy(device) == CNA_RESULT_SUCCESS;
        device = CNA_INVALID_HANDLE;
    }
    cna_demo_check(
        &demo, releases_ok,
        "clear process-wide registrations and release every loader, document, manager and device");
    return cna_demo_finish(&demo);
}
