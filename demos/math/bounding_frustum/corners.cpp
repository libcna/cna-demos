// SPDX-License-Identifier: MS-PL
// BoundingFrustum corner order, output storage and short-buffer refusal.
#include <stdexcept>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void BoundingFrustumCorners(CnaDemos::DemoContext& ctx)
    {
        const BoundingFrustum identity(Matrix::getIdentityProperty());
        const std::vector<Vector3> expected = {
            Vector3(-1.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f),
            Vector3(1.0f, -1.0f, 0.0f), Vector3(-1.0f, -1.0f, 0.0f),
            Vector3(-1.0f, 1.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f),
            Vector3(1.0f, -1.0f, 1.0f), Vector3(-1.0f, -1.0f, 1.0f)};

        const std::vector<Vector3> corners = identity.GetCorners();
        ctx.Check(BoundingFrustum::CornerCount == 8 &&
                      corners.size() == static_cast<std::size_t>(BoundingFrustum::CornerCount),
                  "CornerCount and GetCorners expose exactly eight points");
        ctx.Check(corners == expected,
                  "the value form returns XNA's near-clockwise then far-clockwise corner order");

        const Vector3 guardA(78.0f, 88.0f, 98.0f);
        const Vector3 guardB(79.0f, 89.0f, 99.0f);
        std::vector<Vector3> output(10, Vector3(70.0f, 80.0f, 90.0f));
        output[8] = guardA;
        output[9] = guardB;
        identity.GetCorners(output);
        ctx.Check(std::vector<Vector3>(output.begin(), output.begin() + 8) == expected &&
                      output[8] == guardA && output[9] == guardB,
                  "the output form overwrites exactly eight corners and preserves trailing storage");

        std::vector<Vector3> shortOutput(7, Vector3(70.0f, 80.0f, 90.0f));
        const std::vector<Vector3> before = shortOutput;
        bool canonicalException = false;
        try
        {
            identity.GetCorners(shortOutput);
        }
        catch (const System::ArgumentOutOfRangeException&)
        {
            canonicalException = true;
        }
        catch (const std::out_of_range&)
        {
        }
        ctx.Check(canonicalException && shortOutput == before,
                  "a short output is refused atomically with XNA's ArgumentOutOfRangeException");
    }
}

CNA_DEMOS_CONSOLE(BoundingFrustumCorners, {
    .id = "math.bounding_frustum.corners",
    .title = "Bounding-frustum corners",
    .description = "Checks BoundingFrustum's exact eight-corner order, output storage and guards, and the canonical "
                   "atomic short-buffer refusal.",
    .apis = {"BoundingFrustum::GetCorners", "BoundingFrustum::CornerCount"},
    .tags = {"frustum", "corners", "output", "validation"},
    .restrictions = "CNA-DEMOS-ISSUE-134 records std::out_of_range replacing XNA's ArgumentOutOfRangeException.",
    .knownIssue = "CNA-DEMOS-ISSUE-134",
})
