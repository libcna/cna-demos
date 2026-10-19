// SPDX-License-Identifier: MS-PL
// The model is verified by ColorFixedPointOracle.cs against genuine XNA 4.0.
// Standalone pre-fix witness; do not change its expected values to match CNA.
#include "Microsoft/Xna/Framework/Color.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

using Microsoft::Xna::Framework::Color;

namespace
{
int LerpFactor(float amount)
{
    const float scaled = amount * 65536.0f;
    if (std::isnan(scaled) || scaled < 0) return 0;
    if (scaled >= 65536) return 65536;
    const float floor = std::floor(scaled);
    const int integer = static_cast<int>(floor);
    return integer + (scaled - floor > 0.5f || (scaled - floor == 0.5f && integer % 2 != 0));
}
std::array<int, 4> Channels(const Color& c)
{
    return {static_cast<int>(c.getRProperty()), static_cast<int>(c.getGProperty()),
            static_cast<int>(c.getBProperty()), static_cast<int>(c.getAProperty())};
}
Color LerpModel(const Color& a, const Color& b, float amount)
{
    const auto first = Channels(a), second = Channels(b);
    const int factor = LerpFactor(amount);
    std::array<int, 4> channels{};
    for (std::size_t i = 0; i < channels.size(); ++i)
        channels[i] = first[i] + ((second[i] - first[i]) * factor >> 16);
    return Color(channels[0], channels[1], channels[2], channels[3]);
}
Color MultiplyModel(const Color& color, float scale)
{
    const float scaled = scale * 65536.0f;
    const std::uint64_t factor = std::isnan(scaled) || scaled < 0 ? 0 :
        scaled > 16777215.0f ? 16777215 : static_cast<std::uint64_t>(scaled);
    auto channels = Channels(color);
    for (int& channel : channels)
        channel = static_cast<int>(std::min<std::uint64_t>(255, (channel * factor) >> 16));
    return Color(channels[0], channels[1], channels[2], channels[3]);
}
}

int main()
{
    const std::array colors{Color(0,0,0,0), Color(255,255,255,255), Color(1,2,3,4),
        Color(10,20,30,40), Color(110,220,130,240), Color(0,1,254,255),
        Color(255,254,1,0), Color(127,128,129,130), Color(23,137,91,253)};
    const float amounts[]{std::numeric_limits<float>::quiet_NaN(),
        -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(),
        -1, -0.0f, 0, std::numeric_limits<float>::denorm_min(), 1.0f/131072, 3.0f/131072,
        0.1f, 0.5f, 0.99999f, 1, 1.1f, 2, 255, 256, 1e20f};
    int checks = 0, failed = 0;
    const auto check = [&](const Color& actual, const Color& expected, const char* operation, float amount) {
        ++checks;
        if (actual != expected) {
            ++failed;
            std::cout << "FAIL " << operation << " factor=" << amount
                      << " actual=" << actual.ToString() << " expected=" << expected.ToString() << '\n';
        }
    };
    for (const auto& a : colors) for (const float amount : amounts) {
        check(Color::Multiply(a, amount), MultiplyModel(a, amount), "Multiply", amount);
        check(a * amount, MultiplyModel(a, amount), "operator", amount);
        for (const auto& b : colors)
            check(Color::Lerp(a, b, amount), LerpModel(a, b, amount), "Lerp", amount);
    }
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed == 0 ? 0 : 1;
}
