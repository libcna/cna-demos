// SPDX-License-Identifier: MS-PL
#include <iostream>
#include <string>
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColor.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionNormalTexture.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionTexture.hpp"
using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Graphics;
int main()
{
    int checks = 0, failed = 0;
    const auto check = [&](const std::string& actual, const std::string& expected) {
        ++checks;
        if (actual != expected) { ++failed; std::cout << "FAIL actual=" << actual << " expected=" << expected << '\n'; }
    };
    const Vector3 p(1,2,3), n = Vector3::Up;
    const Vector2 uv(4,5);
    const Color color(10,20,30,40);
    check(VertexPositionColor(p,color).ToString(),
        "{Position:{X:1 Y:2 Z:3} Color:{R:10 G:20 B:30 A:40}}");
    check(VertexPositionColorTexture(p,color,uv).ToString(),
        "{Position:{X:1 Y:2 Z:3} Color:{R:10 G:20 B:30 A:40} TextureCoordinate:{X:4 Y:5}}");
    check(VertexPositionNormalTexture(p,n,uv).ToString(),
        "{Position:{X:1 Y:2 Z:3} Normal:{X:0 Y:1 Z:0} TextureCoordinate:{X:4 Y:5}}");
    check(VertexPositionTexture(p,uv).ToString(),
        "{Position:{X:1 Y:2 Z:3} TextureCoordinate:{X:4 Y:5}}");
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed ? 1 : 0;
}
