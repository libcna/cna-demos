// SPDX-License-Identifier: MS-PL
// Treating any packed format the same way through the IPackedVector interface.
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Bgra4444.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/HalfVector4.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/IPackedVector.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/NormalizedByte4.hpp"
#include "Microsoft/Xna/Framework/Graphics/PackedVector/Rgba1010102.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics::PackedVector;

namespace
{
    void PackedVectorInterface(CnaDemos::DemoContext& ctx)
    {
        // Code that converts pixel data can work on IPackedVector without knowing the format.
        std::vector<std::pair<std::string, std::unique_ptr<IPackedVector>>> formats;
        formats.emplace_back("Bgra4444", std::make_unique<Bgra4444>());
        formats.emplace_back("Rgba1010102", std::make_unique<Rgba1010102>());
        formats.emplace_back("NormalizedByte4", std::make_unique<NormalizedByte4>());
        formats.emplace_back("HalfVector4", std::make_unique<HalfVector4>());

        const Vector4 value(0.5f, 0.25f, 0.75f, 1.0f);
        for (auto& [name, packed] : formats)
        {
            packed->PackFromVector4(value);
            const Vector4 back = packed->ToVector4();
            ctx.Print("{:<16} {}", name, back.ToString());
            ctx.CheckNear(Vector4::Distance(back, value), 0.0, 0.05, name + " round-trips the value approximately");
        }
    }
}

CNA_DEMOS_CONSOLE(PackedVectorInterface, {
    .id = "graphics.packed_vector.ipacked_vector",
    .title = "The IPackedVector interface",
    .description = "Converts one value through several packed formats using only the IPackedVector interface: "
                   "PackFromVector4 and ToVector4.",
    .apis = {"IPackedVector::PackFromVector4", "IPackedVector::ToVector4"},
    .tags = {"packed", "interface", "generic"},
})
