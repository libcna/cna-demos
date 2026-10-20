// SPDX-License-Identifier: MS-PL
// Diagnostic-only source probe. Uses the unchanged CNA tool staging implementation.
#include "CnaContentStaging.hpp"
#include <atomic>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    const std::filesystem::path parent(argv[1]);
    // The driver creates an empty private parent. Never scan the shared production staging root.
    if (!std::filesystem::is_directory(parent) || !std::filesystem::is_empty(parent)) return 2;
    std::atomic<bool> ready{false}, done{false};
    std::mutex resultMutex;
    std::map<std::string, int> diagnostics;
    std::vector<std::string> examples;
    int owners = 0, scans = 0, ownerErrors = 0;
    const auto record = [&](const CNA::Tools::ContentStagingScavengeResult& result)
    {
        std::lock_guard lock(resultMutex);
        for (const auto& diagnostic : result.diagnostics)
        {
            const auto separator = diagnostic.find(": ");
            ++diagnostics[separator == std::string::npos ? diagnostic : diagnostic.substr(separator + 2)];
            if (examples.size() < 12) examples.push_back(diagnostic);
        }
        if (result.removedDirectories != 0) ++ownerErrors;
    };
    std::thread owner([&]
    {
        ready = true;
        for (int i = 0; i < 1000; ++i)
        {
            try
            {
                CNA::Tools::ContentBuildStagingDirectory staging(parent);
                record(staging.ScavengeResult());
                if (!std::filesystem::is_regular_file(staging.Path() / CNA::Tools::ContentStagingMetadataFile))
                {
                    std::lock_guard lock(resultMutex); ++ownerErrors;
                }
                ++owners;
                std::this_thread::yield();
            }
            catch (const std::exception& error)
            {
                std::lock_guard lock(resultMutex); ++ownerErrors;
                if (examples.size() < 12) examples.push_back(std::string("owner exception: ") + error.what());
            }
        }
        done = true;
    });
    while (!ready) std::this_thread::yield();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (!done && std::chrono::steady_clock::now() < deadline)
    {
        record(CNA::Tools::ScavengeContentStagingDirectories(parent));
        ++scans;
    }
    owner.join();
    std::cout << "owners=" << owners << " scans=" << scans << " owner_errors=" << ownerErrors
              << " parent_empty=" << std::filesystem::is_empty(parent) << '\n';
    for (const auto& [message, count] : diagnostics) std::cout << "diagnostic[" << count << "]: " << message << '\n';
    for (const auto& message : examples) std::cout << "example: " << message << '\n';
    const bool reproduced = diagnostics.contains("owner metadata is absent or is not a regular file");
    std::cout << "exact_owner_metadata_warning_reproduced=" << reproduced << '\n';
    // Exit1 intentionally records the reproduced diagnostic, not a compiler/probe crash.
    return ownerErrors ? 2 : (reproduced ? 1 : 0);
}
