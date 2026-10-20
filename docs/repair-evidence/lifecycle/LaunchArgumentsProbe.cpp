// SPDX-License-Identifier: MS-PL
// Invoke with: --windowed /empty: /mixed:a:b /x:first /x:second plain:value
#include "Microsoft/Xna/Framework/LaunchParameters.hpp"
#include "System/ArgumentException.hpp"
#include <iostream>

using Microsoft::Xna::Framework::LaunchParameters;
int main(int argc, char**)
{
    if (argc != 7) return 2;
    int checks = 0, failures = 0;
    const auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failures; std::cout << "FAIL " << name << '\n'; }
    };
    LaunchParameters parsed;
    const auto value = [&](const char* key, const char* expected) {
        return parsed.ContainsKey(key) && parsed.at(key) == expected;
    };
    check(parsed.size() == 5, "process arguments exclude executable and keep first duplicate");
    check(value("windowed", ""), "colonless process flag");
    check(value("empty", ""), "empty process value");
    check(value("mixed", "a:b"), "first colon only");
    check(value("x", "first"), "first parsed duplicate");
    check(value("plain", "value"), "flagless process parameter");
    LaunchParameters explicitArgs(std::vector<std::string>{"first"});
    check(explicitArgs.size() == 1 && explicitArgs.ContainsKey("first") && explicitArgs.at("first").empty(),
          "explicit vector does not discard first parameter");
    bool duplicate = false;
    try { parsed.Add("x", "replacement"); }
    catch (const System::ArgumentException&) { duplicate = true; }
    check(duplicate && value("x", "first") && parsed.size() == 5, "direct duplicate Add is rejected atomically");
    std::cout << "checks=" << checks << " failed=" << failures << '\n';
    return failures ? 1 : 0;
}
