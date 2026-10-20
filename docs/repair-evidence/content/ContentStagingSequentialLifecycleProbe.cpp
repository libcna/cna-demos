// SPDX-License-Identifier: MS-PL
// ISSUE-057: preserve the original sequential control under the documented
// persistent-parent-lock contract. The historical empty-parent fixture remains unchanged.
#include "CnaContentStaging.hpp"
#include <iostream>

namespace
{
    bool ExactParentCleanup(const std::filesystem::path& parent)
    {
        std::size_t entries = 0;
        for (const auto& entry : std::filesystem::directory_iterator(parent))
        {
            ++entries;
            if (entry.path().filename() != CNA::Tools::ContentStagingParentLeaseFile)
                return false;
        }
        const auto marker = parent / CNA::Tools::ContentStagingParentLeaseFile;
        if (entries != 1 ||
            !std::filesystem::is_regular_file(std::filesystem::symlink_status(marker)) ||
            std::filesystem::file_size(marker) != 0 ||
            std::filesystem::hard_link_count(marker) != 1)
            return false;
#if !defined(_WIN32)
        struct stat info{};
        if (::lstat(marker.c_str(), &info) != 0 || info.st_uid != ::geteuid())
            return false;
#endif
        return true;
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) return 2;
    const std::filesystem::path parent(argv[1]);
    if (!std::filesystem::is_directory(parent) || !std::filesystem::is_empty(parent)) return 2;
    int owners = 0, errors = 0, diagnostics = 0, exactCleanups = 0;
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
        if (closed.matchingCandidates != 0 || closed.removedDirectories != 0) ++errors;
        if (ExactParentCleanup(parent)) ++exactCleanups;
        else ++errors;
    }
    std::cout << "owners=" << owners << " sequential_scan_errors=" << errors
              << " diagnostics=" << diagnostics << " exact_parent_cleanups=" << exactCleanups << '\n';
    return errors || diagnostics || exactCleanups != 1000 ? 1 : 0;
}
