/* SPDX-License-Identifier: MS-PL */
/* Type-reader registry, placeholder and caller callback behavior through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

#define READER_NAME "CNA.Demos.MemberReader"
#define TARGET_NAME "CNA.Demos.MemberValue"

static const char ContainerName[] = "C API Type Reader Registry";
static const char AssetFile[] = "empty.bin";

typedef struct ReaderState {
    int create_calls;
    int read_calls;
    int destroy_calls;
    int borrowed_usable;
    int existing_was_null;
    int refuse_read;
    int object;
    CNA_ContentReaderHandle borrowed;
} ReaderState;

static CNA_StringView view(const char* text)
{
    CNA_StringView result;
    result.data = text;
    result.byte_length = (uint64_t)strlen(text);
    return result;
}

static CNA_Result create_reader(void* context, void** out_reader_context)
{
    ReaderState* state = (ReaderState*)context;
    ++state->create_calls;
    *out_reader_context = context;
    return CNA_RESULT_SUCCESS;
}

static CNA_Result read_value(
    void* reader_context,
    CNA_ContentReaderHandle input,
    void* existing_object,
    void** out_object)
{
    ReaderState* state = (ReaderState*)reader_context;
    int32_t version = -1;
    ++state->read_calls;
    state->borrowed = input;
    state->borrowed_usable =
        cna_content_reader_get_version(input, &version) == CNA_RESULT_SUCCESS && version == 7 &&
        cna_content_reader_destroy(input) == CNA_RESULT_INVALID_STATE;
    state->existing_was_null = existing_object == NULL;
    if (state->refuse_read) {
        return CNA_RESULT_IO;
    }
    *out_object = &state->object;
    return CNA_RESULT_SUCCESS;
}

static void destroy_reader(void* reader_context)
{
    ++((ReaderState*)reader_context)->destroy_calls;
}

static CNA_ContentTypeReaderCallbacks callbacks_for(ReaderState* state)
{
    CNA_ContentTypeReaderCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.struct_size = (uint32_t)sizeof(callbacks);
    callbacks.struct_version = UINT32_C(1);
    callbacks.target_type_name = view(TARGET_NAME);
    callbacks.type_version = INT32_C(3);
    callbacks.can_deserialize_into_existing_object = CNA_FALSE;
    callbacks.create = create_reader;
    callbacks.read = read_value;
    callbacks.destroy = destroy_reader;
    callbacks.context = state;
    return callbacks;
}

int main(int argc, char** argv)
{
    static const char EffectReaderName[] = "Microsoft.Xna.Framework.Content.EffectReader";
    ReaderState state;
    CNA_ContentTypeReaderCallbacks callbacks;
    CNA_StorageDeviceHandle device = CNA_INVALID_HANDLE;
    CNA_StorageContainerHandle container = CNA_INVALID_HANDLE;
    CNA_StorageStreamHandle stream = CNA_INVALID_HANDLE;
    CNA_ContentReaderHandle reader = CNA_INVALID_HANDLE;
    CNA_ContentTypeReaderHandle placeholder = CNA_INVALID_HANDLE;
    CNA_ContentTypeReaderHandle type_reader = CNA_INVALID_HANDLE;
    CNA_ContentTypeReaderHandle second = CNA_INVALID_HANDLE;
    CNA_ContentTypeReaderHandle rejected_reader = UINT64_C(99);
    CNA_Handle registration = CNA_INVALID_HANDLE;
    CNA_Handle duplicate = UINT64_C(99);
    CNA_Handle replacement = CNA_INVALID_HANDLE;
    CNA_ContentReaderCreateInfo create_info;
    CNA_Bool flag = CNA_TRUE;
    CNA_Bool has_value = CNA_FALSE;
    CnaDemo demo;
    uint64_t bytes = 0U;
    int32_t version = -1;
    char name[64];
    char before[64];

    cna_demo_init(&demo, "c_api.content.type_reader_registry", argc, argv, 0);
    memset(&state, 0, sizeof(state));
    state.object = 42;
    callbacks = callbacks_for(&state);

    cna_demo_check(
        &demo,
        CNA_UNSUPPORTED_CONTENT_READER_REASON_COMPILED_PLATFORM_SHADER_BYTECODE == UINT32_C(0),
        "the published unsupported-reader reason keeps its fixed identity");
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_clear_type_creators() == CNA_RESULT_SUCCESS &&
            cna_content_register_known_unsupported_xnb_readers() == CNA_RESULT_SUCCESS &&
            cna_content_register_known_unsupported_xnb_readers() == CNA_RESULT_SUCCESS,
        "the isolated registry clears and its currently empty unsupported hook is idempotent");
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_get_is_registered(view(EffectReaderName), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_content_type_reader_manager_create_reader(view("CNA.Demos.Missing"), &rejected_reader) ==
                CNA_RESULT_NOT_SUPPORTED &&
            rejected_reader == CNA_INVALID_HANDLE,
        "the empty hook adds no EffectReader and an unknown name returns no reader");

    cna_demo_check(
        &demo,
        cna_known_unsupported_content_type_reader_create(
            view("CNA.Demos.Placeholder"), UINT32_C(99), &rejected_reader) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            rejected_reader == CNA_INVALID_HANDLE,
        "placeholder construction rejects an unknown reason atomically");
    if (!cna_demo_check_result(
            &demo,
            cna_known_unsupported_content_type_reader_create(
                view("CNA.Demos.Placeholder"),
                CNA_UNSUPPORTED_CONTENT_READER_REASON_COMPILED_PLATFORM_SHADER_BYTECODE,
                &placeholder),
            "create an explicit recognized-but-unsupported placeholder")) {
        goto cleanup;
    }
    memset(name, 0xA5, sizeof(name));
    cna_demo_check(
        &demo,
        cna_content_type_reader_get_target_type_name_size(placeholder, &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == strlen("CNA.Demos.Placeholder") &&
            cna_content_type_reader_copy_target_type_name(placeholder, name, bytes, &bytes) ==
                CNA_RESULT_SUCCESS &&
            memcmp(name, "CNA.Demos.Placeholder", (size_t)bytes) == 0 &&
            (unsigned char)name[bytes] == 0xA5U,
        "a placeholder exposes its exact unterminated target type name");
    memcpy(before, name, sizeof(name));
    cna_demo_check(
        &demo,
        cna_content_type_reader_copy_target_type_name(placeholder, name, bytes - 1U, &bytes) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            memcmp(name, before, sizeof(name)) == 0,
        "a short target-name destination is rejected without a partial write");
    cna_demo_check(
        &demo,
        cna_content_type_reader_get_type_version(placeholder, &version) == CNA_RESULT_SUCCESS &&
            version == 0 &&
            cna_content_type_reader_supports_version(placeholder, 0, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_content_type_reader_supports_version(placeholder, 1, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "the placeholder reports version zero and accepts only that serialized version");
    cna_demo_check(
        &demo,
        cna_content_type_reader_get_can_deserialize_into_existing_object(placeholder, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_content_type_reader_initialize(placeholder) == CNA_RESULT_SUCCESS,
        "the placeholder refuses existing objects and initializes successfully");

    cna_demo_check(
        &demo,
        cna_storage_set_app_name_ext(view("cna-demos-type-reader-registry")) == CNA_RESULT_SUCCESS &&
            cna_storage_device_show_selector(NULL, NULL, &device) == CNA_RESULT_SUCCESS,
        "select isolated storage for the callback-scoped reader");
    if (device == CNA_INVALID_HANDLE) {
        goto cleanup;
    }
    cna_demo_check_result(
        &demo, cna_storage_device_delete_container(device, view(ContainerName)),
        "remove any previous registry fixture");
    if (!cna_demo_check_result(
            &demo, cna_storage_container_open(device, view(ContainerName), NULL, NULL, &container),
            "open a fresh registry fixture") ||
        !cna_demo_check_result(
            &demo, cna_storage_container_create_file(container, view(AssetFile), &stream),
            "create an empty reader stream")) {
        goto cleanup;
    }
    cna_demo_check_result(&demo, cna_storage_stream_close(stream),
                          "close the authored empty stream");
    stream = CNA_INVALID_HANDLE;
    if (!cna_demo_check_result(
            &demo,
            cna_storage_container_open_file(container, view(AssetFile), CNA_FILE_MODE_OPEN, &stream),
            "reopen the empty stream for content reading")) {
        goto cleanup;
    }
    memset(&create_info, 0, sizeof(create_info));
    create_info.struct_size = (uint32_t)sizeof(create_info);
    create_info.struct_version = UINT32_C(1);
    create_info.stream = stream;
    create_info.asset_name = view("registry");
    create_info.version = 7;
    create_info.platform = (uint8_t)'w';
    if (!cna_demo_check_result(
            &demo, cna_content_reader_create(&create_info, &reader),
            "create the content reader supplied to type-reader callbacks")) {
        goto cleanup;
    }
    cna_demo_check(
        &demo,
        cna_content_type_reader_read_untyped(placeholder, reader, &has_value) == CNA_RESULT_IO,
        "the explicit unsupported placeholder refuses an untyped read");
    cna_demo_check(
        &demo,
        cna_content_type_reader_destroy(placeholder) == CNA_RESULT_SUCCESS &&
            cna_content_type_reader_destroy(placeholder) == CNA_RESULT_INVALID_HANDLE,
        "the placeholder handle releases exactly once");
    placeholder = CNA_INVALID_HANDLE;

    {
        CNA_ContentTypeReaderCallbacks malformed = callbacks;
        malformed.create = NULL;
        cna_demo_check(
            &demo,
            cna_content_type_reader_manager_register(view(READER_NAME), &malformed, &duplicate) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                duplicate == CNA_INVALID_HANDLE && state.create_calls == 0,
            "registration rejects a missing required factory without invoking callbacks");
    }
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_get_is_registered(view(READER_NAME), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_content_type_reader_manager_register(view(READER_NAME), &callbacks, &registration) ==
                CNA_RESULT_SUCCESS &&
            registration != CNA_INVALID_HANDLE &&
            cna_content_type_reader_manager_get_is_registered(view(READER_NAME), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE,
        "registration makes the caller-owned factory discoverable by canonical name");
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_register(view(READER_NAME), &callbacks, &duplicate) ==
                CNA_RESULT_INVALID_STATE &&
            duplicate == CNA_INVALID_HANDLE,
        "a live canonical name refuses a shadowing registration");
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_create_reader(view(READER_NAME), &type_reader) ==
                CNA_RESULT_SUCCESS &&
            cna_content_type_reader_manager_create_reader(view(READER_NAME), &second) ==
                CNA_RESULT_SUCCESS &&
            type_reader != second && state.create_calls == 2,
        "each registry lookup creates a fresh owned reader instance");
    cna_demo_check(
        &demo,
        cna_content_type_reader_destroy(second) == CNA_RESULT_SUCCESS && state.destroy_calls == 1,
        "destroying one instance releases exactly its callback context");
    second = CNA_INVALID_HANDLE;
    memset(name, 0, sizeof(name));
    cna_demo_check(
        &demo,
        cna_content_type_reader_copy_target_type_name(type_reader, name, sizeof(name), &bytes) ==
                CNA_RESULT_SUCCESS &&
            bytes == strlen(TARGET_NAME) && strcmp(name, TARGET_NAME) == 0 &&
            cna_content_type_reader_get_type_version(type_reader, &version) == CNA_RESULT_SUCCESS &&
            version == 3,
        "the caller-supplied instance exposes its registered target and type version");
    cna_demo_check(
        &demo,
        cna_content_type_reader_supports_version(type_reader, 3, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_TRUE &&
            cna_content_type_reader_supports_version(type_reader, 2, &flag) == CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE &&
            cna_content_type_reader_get_can_deserialize_into_existing_object(type_reader, &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE && cna_content_type_reader_initialize(type_reader) == CNA_RESULT_SUCCESS,
        "the caller-supplied instance enforces its registered version and initialization contract");
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_unregister(registration) == CNA_RESULT_SUCCESS &&
            cna_content_type_reader_manager_unregister(registration) == CNA_RESULT_INVALID_HANDLE &&
            cna_content_type_reader_manager_get_is_registered(view(READER_NAME), &flag) ==
                CNA_RESULT_SUCCESS &&
            flag == CNA_FALSE,
        "unregistration withdraws the name and releases its handle exactly once");
    registration = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_content_type_reader_read_untyped(type_reader, reader, &has_value) == CNA_RESULT_SUCCESS &&
            has_value == CNA_TRUE && state.read_calls == 1 && state.borrowed_usable == 1 &&
            state.existing_was_null == 1,
        "an existing instance survives unregistration and receives a usable callback-scoped reader");
    cna_demo_check(
        &demo,
        cna_content_reader_get_version(state.borrowed, &version) == CNA_RESULT_INVALID_HANDLE,
        "the borrowed reader handle is invalid immediately after the callback returns");
    state.refuse_read = 1;
    cna_demo_check(
        &demo,
        cna_content_type_reader_read_untyped(type_reader, reader, &has_value) == CNA_RESULT_IO &&
            state.read_calls == 2,
        "a caller read-callback failure propagates instead of producing an object");
    state.refuse_read = 0;
    cna_demo_check(
        &demo,
        cna_content_type_reader_destroy(type_reader) == CNA_RESULT_SUCCESS &&
            state.destroy_calls == 2,
        "destroying the surviving instance runs its cleanup callback once");
    type_reader = CNA_INVALID_HANDLE;
    cna_demo_check(
        &demo,
        cna_content_type_reader_manager_create_reader(view(READER_NAME), &rejected_reader) ==
                CNA_RESULT_NOT_SUPPORTED &&
            rejected_reader == CNA_INVALID_HANDLE &&
            cna_content_type_reader_manager_register(view(READER_NAME), &callbacks, &replacement) ==
                CNA_RESULT_SUCCESS &&
            cna_content_type_reader_manager_unregister(replacement) == CNA_RESULT_SUCCESS,
        "an unregistered name is unresolved and immediately available for a replacement factory");
    replacement = CNA_INVALID_HANDLE;

cleanup:
    if (second != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_type_reader_destroy(second),
                              "destroy a remaining second type reader");
    }
    if (type_reader != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_type_reader_destroy(type_reader),
                              "destroy a remaining type reader");
    }
    if (placeholder != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_type_reader_destroy(placeholder),
                              "destroy a remaining placeholder reader");
    }
    if (registration != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_type_reader_manager_unregister(registration),
                              "withdraw a remaining reader registration");
    }
    if (replacement != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_type_reader_manager_unregister(replacement),
                              "withdraw a remaining replacement registration");
    }
    if (reader != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_content_reader_destroy(reader),
                              "destroy the callback input reader");
        reader = CNA_INVALID_HANDLE;
    }
    if (stream != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_stream_close(stream),
                              "release the callback input stream handle");
    }
    if (container != CNA_INVALID_HANDLE) {
        cna_demo_check_result(&demo, cna_storage_container_destroy(container),
                              "destroy the registry fixture container handle");
    }
    if (device != CNA_INVALID_HANDLE) {
        cna_demo_check_result(
            &demo, cna_storage_device_delete_container(device, view(ContainerName)),
            "delete the registry fixture container tree");
        cna_demo_check_result(&demo, cna_storage_device_destroy(device),
                              "destroy the selected storage device");
    }
    cna_demo_check_result(&demo, cna_content_type_reader_manager_clear_type_creators(),
                          "leave the process type-reader registry empty");
    return cna_demo_finish(&demo);
}
