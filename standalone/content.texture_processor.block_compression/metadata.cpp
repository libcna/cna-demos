// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.texture_processor.block_compression",
    .title = "Block-compress RGBA images",
    .description = "Exercises CNA's deterministic build-time BC1, BC2 and BC3 encoder, including block sizing, transparency policy, edge replication and malformed input.",
    .apis = {"CNA::Content::Pipeline::BlockCompressionFormat",
             "CNA::Content::Pipeline::BlockCompressionOptions",
             "CNA::Content::Pipeline::BlockCompressionOptions::refinementRounds",
             "CNA::Content::Pipeline::BlockCompressionOptions::alphaCutoff",
             "CNA::Content::Pipeline::BlockCompressedBlockByteCount",
             "CNA::Content::Pipeline::BlockCompressedByteCount",
             "CNA::Content::Pipeline::BlockCompressionFormatName",
             "CNA::Content::Pipeline::EncodeBlockCompressedImage",
             "CNA::Content::Pipeline::ImageHasTransparency",
             "CNA::Content::Pipeline::ImageHasPartialTransparency"},
    .tags = {"content", "pipeline", "texture", "compression", "bc1", "bc2", "bc3", "dxt"},
    .restrictions = "Build-time CPU tool in a separate process; requires CNA::ContentPipelineBuild. Exact block bytes and policy are inspected without a graphics device. Existing texture demos own runtime loading, rendering and pixel evidence.",
})
