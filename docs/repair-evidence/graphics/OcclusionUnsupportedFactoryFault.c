/* SPDX-License-Identifier: MS-PL */
/* Linux-only negative harness: on an unsupported renderer, a dishonest successful factory
 * must make the C demo FAIL rather than let its declared-capability branch hide the error. */
#include <CNA/C/graphics_device.h>

CNA_Result cna_occlusion_query_create(
    CNA_Handle graphics_device, CNA_OcclusionQueryHandle* out_occlusion_query)
{
    (void)graphics_device;
    (void)out_occlusion_query;
    return CNA_RESULT_SUCCESS;
}
