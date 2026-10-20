// SPDX-License-Identifier: MS-PL
// ISSUE-013: reserved pipeline bookkeeping is not a runtime content asset.
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace fs = std::filesystem;
using Microsoft::Xna::Framework::Content::ContentManager;
namespace
{
    int checks = 0, failed = 0;
    void Check(bool condition, const char* label)
    {
        ++checks;
        if (!condition) ++failed;
        std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
    }
    struct Root
    {
        fs::path path = fs::temp_directory_path() /
            ("cna-repair-manifest-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Root() { if (!fs::create_directory(path)) throw std::runtime_error("Private fixture directory already exists"); }
        ~Root() { std::error_code error; fs::remove_all(path, error); }
        void Write(const std::string& relative)
        {
            const auto target = path / relative;
            fs::create_directories(target.parent_path());
            std::ofstream file;
            file.exceptions(std::ios::failbit | std::ios::badbit);
            file.open(target, std::ios::binary);
            file << "fixture";
        }
    };
    std::set<std::string> Names(ContentManager& manager)
    {
        std::set<std::string> names;
        for (const auto& entry : manager.GetContentManifest()) names.insert(entry.relativePath);
        return names;
    }
}
int main()
{
    try
    {
        Root root;
        for (const std::string name : {".cna-content.lock", ".cna-content-manifest.json",
             "nested/.cna-content.lock", "nested/.cna-content-manifest.json",
             ".cna-contentproj/cna-buildcontent.json", "nested/.cna-contentproj/cache.bin",
             "good.png", ".private.png", "user.lock", ".cna-content.lock.png",
             ".cna-content-manifest.json.cnj", ".cna-contentproj-extra/visible.png"}) root.Write(name);
        ContentManager manager(nullptr, root.path.string());
        const auto names = Names(manager);
        Check(!names.contains(".cna-content"), "root lock excluded");
        Check(!names.contains(".cna-content-manifest"), "root build manifest excluded");
        Check(!names.contains("nested/.cna-content"), "nested lock excluded");
        Check(!names.contains("nested/.cna-content-manifest"), "nested build manifest excluded");
        Check(!names.contains(".cna-contentproj/cna-buildcontent"), "project intermediate subtree excluded");
        Check(!names.contains("nested/.cna-contentproj/cache"), "nested project intermediate subtree excluded");
        Check(names.contains("good"), "ordinary native asset retained");
        Check(names.contains(".private"), "unrelated hidden asset retained");
        Check(names.contains("user"), "unrelated lock-extension asset retained");
        Check(names.contains(".cna-content.lock"), "lock-prefixed asset retained");
        Check(names.contains(".cna-content-manifest.json"), "manifest-prefixed CNJ asset retained");
        Check(names.contains(".cna-contentproj-extra/visible"), "intermediate-prefix directory asset retained");
        root.Write("late.cnj");
        root.Write("new/.cna-content.lock");
        Check(Names(manager) == names, "snapshot remains cached before refresh");
        manager.RefreshContentManifest();
        const auto refreshed = Names(manager);
        Check(refreshed.contains("late"), "refresh adds new real asset");
        Check(!refreshed.contains("new/.cna-content"), "refresh still excludes new bookkeeping");
        const std::set<std::string> wanted{"good", ".private", "user", ".cna-content.lock",
            ".cna-content-manifest.json", ".cna-contentproj-extra/visible", "late"};
        Check(refreshed == wanted, "exact refreshed inventory has only intended assets");
        std::cout << "checks=" << checks << " failed=" << failed << '\n';
        return failed ? 1 : 0;
    }
    catch (const std::exception& error) { std::cerr << "Unexpected fixture error: " << error.what() << '\n'; return 2; }
}
