// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "content.audio.xma_encoder_service",
    .title = "Drive the build-time XMA encoder seam",
    .description = "Exercises CNA's unavailable, attached and external-process XMA encoder services, "
                   "including exact PCM requests, quality and loop propagation, XMA2 result adoption, "
                   "argument substitution and deterministic backend failures.",
    .apis = {"CNA::Content::Pipeline::XmaEncodeQuality",
             "CNA::Content::Pipeline::XmaEncodeQualityName",
             "CNA::Content::Pipeline::XmaEncodeRequest",
             "CNA::Content::Pipeline::XmaEncodeRequest::pcm",
             "CNA::Content::Pipeline::XmaEncodeRequest::channelCount",
             "CNA::Content::Pipeline::XmaEncodeRequest::sampleRate",
             "CNA::Content::Pipeline::XmaEncodeRequest::bitsPerSample",
             "CNA::Content::Pipeline::XmaEncodeRequest::loopStart",
             "CNA::Content::Pipeline::XmaEncodeRequest::loopLength",
             "CNA::Content::Pipeline::XmaEncodeRequest::quality",
             "CNA::Content::Pipeline::XmaEncodeResult",
             "CNA::Content::Pipeline::XmaEncodeResult::data",
             "CNA::Content::Pipeline::XmaEncodeResult::formatBlock",
             "CNA::Content::Pipeline::XmaEncodeResult::loopStart",
             "CNA::Content::Pipeline::XmaEncodeResult::loopLength",
             "CNA::Content::Pipeline::XmaEncodeResult::diagnostics",
             "CNA::Content::Pipeline::XmaEncodeResult::succeeded",
             "CNA::Content::Pipeline::XmaEncoderIdentity",
             "CNA::Content::Pipeline::XmaEncoderIdentity::backend",
             "CNA::Content::Pipeline::XmaEncoderIdentity::version",
             "CNA::Content::Pipeline::XmaEncoderIdentity::ToString",
             "CNA::Content::Pipeline::XmaEncoderUnavailableSentence",
             "CNA::Content::Pipeline::XmaEncoderService::Identity",
             "CNA::Content::Pipeline::XmaEncoderService::Available",
             "CNA::Content::Pipeline::XmaEncoderService::UnavailableReason",
             "CNA::Content::Pipeline::XmaEncoderService::Encode",
             "CNA::Content::Pipeline::MakeUnavailableXmaEncoder",
             "CNA::Content::Pipeline::ExternalXmaEncoderOptions",
             "CNA::Content::Pipeline::ExternalXmaEncoderOptions::executable",
             "CNA::Content::Pipeline::ExternalXmaEncoderOptions::launcher",
             "CNA::Content::Pipeline::ExternalXmaEncoderOptions::arguments",
             "CNA::Content::Pipeline::MakeExternalXmaEncoder",
             "CNA::Content::Pipeline::SetBuildXmaEncoder",
             "CNA::Content::Pipeline::BuildXmaEncoder",
             "Microsoft::Xna::Framework::Content::Pipeline::Audio::AudioContent::ConvertFormat",
             "Microsoft::Xna::Framework::Content::Pipeline::Audio::ConversionFormat::Xma"},
    .tags = {"content", "pipeline", "audio", "xma", "encoder", "xna", "cnaext"},
    .restrictions = "Build-time CPU workflow in a separate process; requires CNA::ContentPipelineBuild. "
                    "The external-process probe relaunches this demo as a deterministic contract stub; "
                    "it emits XMA2-shaped test bytes but is not an XMA encoder. No genuine XMA encoding, "
                    "Xbox playback, audio-device output, listening test or genuine-XNA comparison is claimed.",
})
