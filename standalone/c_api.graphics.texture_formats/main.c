/* SPDX-License-Identifier: MS-PL */
/*
 * Renderer-independent texture format rules from C: every public SurfaceFormat reports its
 * compression footprint, storage-unit byte size and pixel-store alignment. The same table drives
 * element-width validation, while the base Texture contract accepts only Color.
 *
 * Arguments: [--automated] [--frames N] [--screenshot PATH] (ignored: nothing is drawn).
 * Exit code 0 passed, 1 failed.
 */
#include <CNA/C/cna.h>

#include "CnaDemos/CApiDemo.h"

enum { FORMAT_COUNT = 27 };

int main(int argc, char** argv)
{
    static const int32_t BlockSizes[FORMAT_COUNT] = {
        1, 1, 1, 1, 16, 16, 16, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 16, 16, 16, 1, 1
    };
    static const int32_t FormatSizes[FORMAT_COUNT] = {
        4, 2, 2, 2, 8, 16, 16, 2, 4, 4, 4, 8, 1, 4,
        8, 16, 2, 4, 8, 8, 4, 4, 16, 16, 16, 1, 2
    };
    CnaDemo demo;
    int block_sizes_match = 1;
    int format_sizes_match = 1;
    int alignments_match = 1;
    int transfer_widths_match = 1;
    int base_contract_matches = 1;
    uint32_t format;

    cna_demo_init(&demo, "c_api.graphics.texture_formats", argc, argv, 0);

    for (format = CNA_SURFACE_FORMAT_COLOR; format <= CNA_SURFACE_FORMAT_USHORT_EXT; ++format) {
        int32_t block_size = -1;
        int32_t format_size = -1;
        int32_t alignment = -1;
        const int32_t expected_alignment = FormatSizes[format] < 8 ? FormatSizes[format] : 8;
        block_sizes_match = block_sizes_match &&
            cna_texture_get_block_size_squared(format, &block_size) == CNA_RESULT_SUCCESS &&
            block_size == BlockSizes[format];
        format_sizes_match = format_sizes_match &&
            cna_texture_get_format_size(format, &format_size) == CNA_RESULT_SUCCESS &&
            format_size == FormatSizes[format];
        alignments_match = alignments_match &&
            cna_texture_get_pixel_store_alignment(format, &alignment) == CNA_RESULT_SUCCESS &&
            alignment == expected_alignment;
        transfer_widths_match = transfer_widths_match &&
            cna_texture_validate_get_data_format(format, 1) == CNA_RESULT_SUCCESS &&
            cna_texture_validate_get_data_format(format, FormatSizes[format]) == CNA_RESULT_SUCCESS &&
            cna_texture_validate_get_data_format(format, FormatSizes[format] + 1) ==
                CNA_RESULT_INVALID_ARGUMENT;
        base_contract_matches = base_contract_matches &&
            cna_texture_validate_format(format) ==
                (format == CNA_SURFACE_FORMAT_COLOR ? CNA_RESULT_SUCCESS : CNA_RESULT_NOT_SUPPORTED);
    }

    cna_demo_check(&demo, block_sizes_match,
                   "all 27 formats report the exact compressed-block texel footprint");
    cna_demo_check(&demo, format_sizes_match,
                   "all 27 formats report their exact texel or compression-block byte size");
    cna_demo_check(&demo, alignments_match,
                   "pixel-store alignment follows each format unit size up to eight bytes");
    cna_demo_check(&demo, transfer_widths_match,
                   "transfer element widths must divide each format unit exactly");
    cna_demo_check(&demo, base_contract_matches,
                   "the renderer-independent base Texture contract accepts only Color");

    {
        int32_t untouched = 77;
        cna_demo_check(
            &demo,
            cna_texture_get_block_size_squared(UINT32_MAX, &untouched) == CNA_RESULT_INVALID_ARGUMENT &&
                untouched == 77 &&
                cna_texture_get_format_size(UINT32_MAX, &untouched) == CNA_RESULT_INVALID_ARGUMENT &&
                untouched == 77 &&
                cna_texture_get_pixel_store_alignment(UINT32_MAX, &untouched) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                untouched == 77 &&
                cna_texture_validate_get_data_format(CNA_SURFACE_FORMAT_COLOR, 0) ==
                    CNA_RESULT_INVALID_ARGUMENT &&
                cna_texture_validate_format(UINT32_MAX) == CNA_RESULT_INVALID_ARGUMENT,
            "invalid identities and nonpositive element widths are refused without output mutation");
    }

    return cna_demo_finish(&demo);
}
