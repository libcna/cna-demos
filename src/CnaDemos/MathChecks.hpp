// SPDX-License-Identifier: MS-PL
#pragma once

// Tolerance helpers for checks in math demos. They only measure differences; every operation a
// demo is about is still called directly on the CNA types in the demo itself.

#include <algorithm>
#include <cmath>

#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"

namespace CnaDemos
{
    /** Largest absolute difference between corresponding elements of two matrices. */
    [[nodiscard]] inline float MaxDifference(const Microsoft::Xna::Framework::Matrix& a,
                                             const Microsoft::Xna::Framework::Matrix& b)
    {
        const float pairs[16][2] = {
            {a.M11, b.M11}, {a.M12, b.M12}, {a.M13, b.M13}, {a.M14, b.M14},
            {a.M21, b.M21}, {a.M22, b.M22}, {a.M23, b.M23}, {a.M24, b.M24},
            {a.M31, b.M31}, {a.M32, b.M32}, {a.M33, b.M33}, {a.M34, b.M34},
            {a.M41, b.M41}, {a.M42, b.M42}, {a.M43, b.M43}, {a.M44, b.M44},
        };
        float largest = 0.0f;
        for (const auto& pair : pairs)
        {
            largest = std::max(largest, std::abs(pair[0] - pair[1]));
        }
        return largest;
    }

    /**
     * Angle-insensitive difference between two rotations: 0 when they are the same rotation.
     * q and -q describe the same rotation, so the sign is ignored.
     */
    [[nodiscard]] inline float RotationDifference(const Microsoft::Xna::Framework::Quaternion& a,
                                                  const Microsoft::Xna::Framework::Quaternion& b)
    {
        return 1.0f - std::abs(Microsoft::Xna::Framework::Quaternion::Dot(a, b));
    }
}
