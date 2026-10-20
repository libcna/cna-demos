// SPDX-License-Identifier: MS-PL
#include "CnaContentStaging.hpp"
#include <iostream>
int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    const std::filesystem::path parent(argv[1]);
    if (!std::filesystem::is_directory(parent) || !std::filesystem::is_empty(parent)) return 2;
    int owners = 0, errors = 0, diagnostics = 0;
    for (int i = 0; i < 1000; ++i)
    {
        {
            CNA::Tools::ContentBuildStagingDirectory staging(parent);
            diagnostics += static_cast<int>(staging.ScavengeResult().diagnostics.size());
            const auto active = CNA::Tools::ScavengeContentStagingDirectories(parent);
            diagnostics += static_cast<int>(active.diagnostics.size());
            if (active.matchingCandidates != 1 || active.recentDirectories != 1 || active.removedDirectories != 0) ++errors;
            ++owners;
        }
        const auto closed = CNA::Tools::ScavengeContentStagingDirectories(parent);
        diagnostics += static_cast<int>(closed.diagnostics.size());
        if (closed.matchingCandidates != 0 || !std::filesystem::is_empty(parent)) ++errors;
    }
    std::cout << "owners=" << owners << " sequential_scan_errors=" << errors
              << " diagnostics=" << diagnostics << " parent_empty=" << std::filesystem::is_empty(parent) << '\n';
    return errors || diagnostics ? 1 : 0;
}
