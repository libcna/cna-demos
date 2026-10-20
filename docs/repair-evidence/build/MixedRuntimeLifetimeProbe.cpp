// SPDX-License-Identifier: MS-PL
// Link-only sidecar reference: no Game, device, service or network is started.
#include <cstdio>
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GamerServices/GamerServicesComponent.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"

namespace
{
    void ReferenceGamerServices(Microsoft::Xna::Framework::Game& game)
    {
        Microsoft::Xna::Framework::GamerServices::GamerServicesComponent component(game);
    }
}

int main()
{
    // Keep the real sidecar's runtime dependencies in the link without invoking it.
    auto* volatile reference = &ReferenceGamerServices;
    if (reference == nullptr) return 2;
    if (Microsoft::Xna::Framework::Graphics::SamplerState::PointWrap.getIsDisposedProperty())
        return 3;
    std::puts("startup reached; no game or service constructed");
    return 0;
}
