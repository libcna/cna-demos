// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "c_api.content.cnb_importers",
    .title = "CNB image, DDS and WAV importers in C",
    .description = "A renderer-free C program imports controlled image, cube-map and looping "
                   "audio fixtures through both filesystem and memory front ends.",
    .apis = {"cna_cnb_import_image_as_texture2d", "cna_cnb_import_dds_as_texture_cube",
             "cna_cnb_decode_dds_as_texture_cube", "cna_cnb_decode_wav_as_sound_effect",
             "cna_cnb_import_wav_as_sound_effect"},
    .tags = {"c", "binding", "content", "cnb", "image", "dds", "wav", "import"},
    .needs = CnaDemos::Need::CApi,
    .frames = 1,
    .deterministic = true,
    .restrictions = "Creates private synthetic BMP, DXT1 cube DDS and PCM8 WAV files. No "
                    "ContentManager, graphics/audio device, rendering, playback or audible/visual "
                    "quality claim is involved.",
})
