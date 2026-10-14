/* SPDX-License-Identifier: MS-PL */
/* Stable renderer classifications and compile-time renderer identity through the C ABI. */
#include <CNA/C/cna.h>

#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct RendererExpectation {
    CNA_GraphicsRendererType type;
    CNA_GraphicsBackendCategory category;
    CNA_GraphicsBackendMaturity maturity;
    const char* name;
} RendererExpectation;

static const RendererExpectation Renderers[] = {
    {CNA_GRAPHICS_RENDERER_OPENGLES3, CNA_GRAPHICS_BACKEND_CATEGORY_NATIVE,
     CNA_GRAPHICS_BACKEND_MATURITY_PRODUCTION, "OPENGLES3"},
    {CNA_GRAPHICS_RENDERER_OPENGL33, CNA_GRAPHICS_BACKEND_CATEGORY_NATIVE,
     CNA_GRAPHICS_BACKEND_MATURITY_PRODUCTION, "OPENGL33"},
    {CNA_GRAPHICS_RENDERER_WEBGL2, CNA_GRAPHICS_BACKEND_CATEGORY_WEB,
     CNA_GRAPHICS_BACKEND_MATURITY_SUPPORTED, "WEBGL2"},
    {CNA_GRAPHICS_RENDERER_WEBGPU, CNA_GRAPHICS_BACKEND_CATEGORY_TRANSLATION_LAYER,
     CNA_GRAPHICS_BACKEND_MATURITY_EXPERIMENTAL, "WEBGPU"},
    {CNA_GRAPHICS_RENDERER_HEADLESS, CNA_GRAPHICS_BACKEND_CATEGORY_DIAGNOSTIC,
     CNA_GRAPHICS_BACKEND_MATURITY_SUPPORTED, "HEADLESS"},
    {CNA_GRAPHICS_RENDERER_SOFTWARE, CNA_GRAPHICS_BACKEND_CATEGORY_SOFTWARE,
     CNA_GRAPHICS_BACKEND_MATURITY_EXPERIMENTAL, "SOFTWARE"},
    {CNA_GRAPHICS_RENDERER_STUB, CNA_GRAPHICS_BACKEND_CATEGORY_DIAGNOSTIC,
     CNA_GRAPHICS_BACKEND_MATURITY_SUPPORTED, "STUB"},
    {CNA_GRAPHICS_RENDERER_DIRECTX9, CNA_GRAPHICS_BACKEND_CATEGORY_NATIVE,
     CNA_GRAPHICS_BACKEND_MATURITY_PRODUCTION, "DIRECTX9"},
    {CNA_GRAPHICS_RENDERER_SDL_GPU, CNA_GRAPHICS_BACKEND_CATEGORY_TRANSLATION_LAYER,
     CNA_GRAPHICS_BACKEND_MATURITY_SUPPORTED, "SDL_GPU"},
    {CNA_GRAPHICS_RENDERER_FNA3D, CNA_GRAPHICS_BACKEND_CATEGORY_TRANSLATION_LAYER,
     CNA_GRAPHICS_BACKEND_MATURITY_EXPERIMENTAL, "FNA3D"},
};

static const char* const CategoryNames[] = {
    "Native", "TranslationLayer", "Software", "Web", "Diagnostic"
};

static const char* const MaturityNames[] = {
    "Production", "Supported", "Experimental", "Historical", "Deprecated"
};

static int check_enum_names(void)
{
    char text[32];
    char before[32];
    uint64_t bytes = UINT64_MAX;
    uint32_t index;

    for (index = 0U; index < 5U; ++index) {
        memset(text, 0, sizeof(text));
        if (cna_graphics_backend_category_get_name_size(index, &bytes) != CNA_RESULT_SUCCESS ||
            bytes != (uint64_t)strlen(CategoryNames[index]) ||
            cna_graphics_backend_category_copy_name(index, text, sizeof(text), &bytes) !=
                CNA_RESULT_SUCCESS ||
            strcmp(text, CategoryNames[index]) != 0) {
            return 0;
        }
        memset(text, 0, sizeof(text));
        if (cna_graphics_backend_maturity_get_name_size(index, &bytes) != CNA_RESULT_SUCCESS ||
            bytes != (uint64_t)strlen(MaturityNames[index]) ||
            cna_graphics_backend_maturity_copy_name(index, text, sizeof(text), &bytes) !=
                CNA_RESULT_SUCCESS ||
            strcmp(text, MaturityNames[index]) != 0) {
            return 0;
        }
    }

    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    return cna_graphics_backend_category_copy_name(
               CNA_GRAPHICS_BACKEND_CATEGORY_TRANSLATION_LAYER, text, UINT64_C(4), &bytes) ==
               CNA_RESULT_BUFFER_TOO_SMALL &&
        bytes == UINT64_C(16) && memcmp(text, before, sizeof(text)) == 0;
}

static int check_renderer_table(void)
{
    CNA_GraphicsBackendCategory category = UINT32_MAX;
    CNA_GraphicsBackendMaturity maturity = UINT32_MAX;
    size_t index;

    for (index = 0U; index < sizeof(Renderers) / sizeof(Renderers[0]); ++index) {
        if (cna_graphics_backend_get_category(Renderers[index].type, &category) !=
                CNA_RESULT_SUCCESS ||
            category != Renderers[index].category ||
            cna_graphics_backend_get_maturity(Renderers[index].type, &maturity) !=
                CNA_RESULT_SUCCESS ||
            maturity != Renderers[index].maturity) {
            return 0;
        }
    }
    return 1;
}

static const RendererExpectation* find_renderer(CNA_GraphicsRendererType type)
{
    size_t index;
    for (index = 0U; index < sizeof(Renderers) / sizeof(Renderers[0]); ++index) {
        if (Renderers[index].type == type) {
            return &Renderers[index];
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    CNA_GraphicsRendererType current = CNA_GRAPHICS_RENDERER_UNKNOWN;
    CNA_GraphicsBackendCategory category = UINT32_MAX;
    CNA_GraphicsBackendMaturity maturity = UINT32_MAX;
    const RendererExpectation* expected = 0;
    char text[32];
    char before[32];
    uint64_t bytes = UINT64_MAX;
    uint64_t copied = UINT64_MAX;

    cna_demo_init(&demo, "c_api.platform.renderer_metadata", argc, argv, 1);

    cna_demo_check(
        &demo,
        CNA_GRAPHICS_BACKEND_CATEGORY_NATIVE == 0U &&
            CNA_GRAPHICS_BACKEND_CATEGORY_DIAGNOSTIC == 4U &&
            CNA_GRAPHICS_BACKEND_MATURITY_PRODUCTION == 0U &&
            CNA_GRAPHICS_BACKEND_MATURITY_DEPRECATED == 4U,
        "backend category and maturity identities retain their fixed public ordinals");
    cna_demo_check(&demo, check_enum_names(),
                   "all category and maturity identities have exact stable names and atomic copies");
    cna_demo_check(&demo, check_renderer_table(),
                   "every public renderer has its documented implementation category and maturity");
    cna_demo_check(
        &demo,
        cna_graphics_backend_get_category(CNA_GRAPHICS_RENDERER_UNKNOWN, &category) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_get_maturity(UINT32_C(1), &maturity) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_get_category(CNA_GRAPHICS_RENDERER_HEADLESS, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_get_maturity(CNA_GRAPHICS_RENDERER_HEADLESS, 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_category_get_name_size(UINT32_C(5), &bytes) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_maturity_copy_name(
                UINT32_C(5), text, sizeof(text), &bytes) == CNA_RESULT_INVALID_ARGUMENT,
        "unknown, retired and malformed classification/name requests are refused");

    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_current_type(&current) == CNA_RESULT_SUCCESS &&
            (expected = find_renderer(current)) != 0 &&
            cna_graphics_backend_get_current_category(&category) == CNA_RESULT_SUCCESS &&
            category == expected->category &&
            cna_graphics_backend_get_current_maturity(&maturity) == CNA_RESULT_SUCCESS &&
            maturity == expected->maturity,
        "the compiled-in renderer identity agrees with its current category and maturity");

    memset(text, 0xA5, sizeof(text));
    cna_demo_check(
        &demo,
        expected != 0 &&
            cna_graphics_renderer_get_current_name_size(&bytes) == CNA_RESULT_SUCCESS &&
            bytes == (uint64_t)strlen(expected->name) &&
            cna_graphics_renderer_copy_current_name(text, sizeof(text), &copied) ==
                CNA_RESULT_SUCCESS &&
            copied == bytes && memcmp(text, expected->name, (size_t)bytes) == 0 &&
            (unsigned char)text[bytes] == 0xA5U,
        "the compiled-in renderer name is copied as exact unterminated build-option text");

    memset(text, 0xA5, sizeof(text));
    memcpy(before, text, sizeof(text));
    cna_demo_check(
        &demo,
        cna_graphics_renderer_copy_current_name(text, bytes - 1U, &copied) ==
                CNA_RESULT_BUFFER_TOO_SMALL &&
            copied == bytes && memcmp(text, before, sizeof(text)) == 0,
        "a short compiled-renderer name destination reports its need without a partial write");

    cna_demo_check(
        &demo,
        cna_graphics_renderer_get_current_type(0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_get_current_name_size(0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_renderer_copy_current_name(text, sizeof(text), 0) ==
                CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_get_current_category(0) == CNA_RESULT_INVALID_ARGUMENT &&
            cna_graphics_backend_get_current_maturity(0) == CNA_RESULT_INVALID_ARGUMENT,
        "compiled-renderer queries refuse null outputs");

    return cna_demo_finish(&demo);
}
