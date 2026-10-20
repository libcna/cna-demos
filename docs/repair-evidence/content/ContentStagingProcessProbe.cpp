// SPDX-License-Identifier: MS-PL
// ISSUE-057: real staging owners, scavengers and OS-lock lifetime in separate processes.
#include "CnaContentStaging.hpp"

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <map>
#include <thread>

namespace
{
    struct Results
    {
        unsigned owners = 0, scans = 0, errors = 0, removed = 0, active = 0, inspected = 0;
        std::map<std::string, unsigned> diagnostics;

        void Observe(const CNA::Tools::ContentStagingScavengeResult& result)
        {
            ++scans;
            removed += static_cast<unsigned>(result.removedDirectories);
            active += static_cast<unsigned>(result.activeDirectories);
            inspected += static_cast<unsigned>(result.scannedEntries);
            for (const auto& diagnostic : result.diagnostics)
            {
                const auto separator = diagnostic.find(": ");
                ++diagnostics[separator == std::string::npos ? diagnostic : diagnostic.substr(separator + 2)];
            }
        }

        int Finish() const
        {
            unsigned count = 0;
            for (const auto& [message, occurrences] : diagnostics) { count += occurrences; }
            std::cout << "owners=" << owners << " scans=" << scans << " errors=" << errors
                      << " diagnostics=" << count << " removed=" << removed << " active=" << active
                      << " inspected=" << inspected << '\n';
            for (const auto& [message, occurrences] : diagnostics)
            {
                std::cout << "diagnostic[" << occurrences << "]: " << message << '\n';
            }
            return errors || count ? 1 : 0;
        }
    };

    std::string Ready()
    {
        std::cout << "READY\n" << std::flush;
        std::string command;
        if (!std::getline(std::cin, command)) { throw std::runtime_error("missing controller command"); }
        return command;
    }

    bool IndependentlyLocked(const std::filesystem::path& marker)
    {
#if defined(_WIN32)
        HANDLE handle = ::CreateFileW(marker.c_str(), GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                     FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        if (handle == INVALID_HANDLE_VALUE) { throw std::runtime_error("cannot inspect OS lock"); }
        OVERLAPPED position{};
        const bool acquired = ::LockFileEx(
            handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &position);
        const DWORD error = ::GetLastError();
        ::CloseHandle(handle);
        if (!acquired && error != ERROR_LOCK_VIOLATION) { throw std::runtime_error("unexpected OS lock error"); }
#else
        const int handle = ::open(marker.c_str(), O_RDWR | O_CLOEXEC | O_NOFOLLOW);
        if (handle < 0) { throw std::runtime_error("cannot inspect OS lock"); }
        const bool acquired = ::flock(handle, LOCK_EX | LOCK_NB) == 0;
        const int error = errno;
        ::close(handle);
        if (!acquired && error != EWOULDBLOCK && error != EAGAIN)
        {
            throw std::runtime_error("unexpected OS lock error");
        }
#endif
        return !acquired;
    }
}

int main(int argc, char** argv)
{
    if (argc != 3) { return 2; }
    try
    {
        const std::string mode(argv[1]);
        const std::filesystem::path parent(argv[2]);
        if (!parent.is_absolute() || !std::filesystem::is_directory(parent)) { return 2; }
        if (mode == "parent")
        {
            CNA::Tools::ContentStagingDetail::ParentLifecycleLock lock;
            std::string reason;
            if (!lock.Acquire(parent, reason)) { throw std::runtime_error(reason); }
            // A second handle in this process must be excluded too.
            if (!IndependentlyLocked(parent / CNA::Tools::ContentStagingParentLeaseFile))
            {
                throw std::runtime_error("same-process independent handle bypassed the lock");
            }
            if (Ready() != "ABANDON") { return 2; }
            std::_Exit(0); // Deliberately bypass C++ destructors; the OS must release the lock.
        }
        if (mode == "try")
        {
            std::cout << "locked=" << IndependentlyLocked(parent / CNA::Tools::ContentStagingParentLeaseFile) << '\n';
            return 0;
        }
        if (mode == "make-symlink")
        {
            std::error_code error;
            const auto marker = parent / CNA::Tools::ContentStagingParentLeaseFile;
            std::filesystem::create_symlink(parent.parent_path() / "keep", marker, error);
            if (error)
            {
                std::cout << "unsupported=" << error.value() << ":" << error.message() << '\n';
                return 77;
            }
            if (!std::filesystem::is_symlink(std::filesystem::symlink_status(marker)))
            {
                throw std::runtime_error("successful native symlink creation did not produce a symlink");
            }
            std::cout << "native_symlink=1\n";
            return 0;
        }
        if (mode == "old-owner")
        {
            CNA::Tools::ContentBuildStagingDirectory staging(parent, 1);
            if (Ready() != "ABANDON") { return 2; }
            std::_Exit(0); // Leave a valid old tree, without running its destructor.
        }
        Results results;
        if (mode == "once")
        {
            CNA::Tools::ContentBuildStagingDirectory staging(parent);
            results.Observe(staging.ScavengeResult());
            ++results.owners;
            return results.Finish();
        }
        if (mode == "inspect")
        {
            results.Observe(CNA::Tools::ScavengeContentStagingDirectories(parent));
            return results.Finish();
        }
        if (mode == "owners")
        {
            if (Ready() != "START") { return 2; }
            for (unsigned index = 0; index < 1000; ++index)
            {
                try
                {
                    CNA::Tools::ContentBuildStagingDirectory staging(parent);
                    results.Observe(staging.ScavengeResult());
                    if (!std::filesystem::is_regular_file(staging.Path() / CNA::Tools::ContentStagingMetadataFile) ||
                        !std::filesystem::is_regular_file(staging.Path() / CNA::Tools::ContentStagingLeaseFile))
                    {
                        ++results.errors;
                    }
                    ++results.owners;
                    std::this_thread::yield();
                }
                catch (const std::exception&) { ++results.errors; }
            }
            return results.Finish();
        }
        if (mode == "scan")
        {
            if (Ready() != "START") { return 2; }
            std::atomic<bool> stop{false};
            std::atomic<bool> invalidCommand{false};
            std::thread controller([&]
            {
                std::string command;
                if (!std::getline(std::cin, command) || command != "STOP") { invalidCommand = true; }
                stop = true;
            });
            do
            {
                results.Observe(CNA::Tools::ScavengeContentStagingDirectories(parent));
            } while (!stop.load());
            controller.join();
            if (invalidCommand.load()) { ++results.errors; }
            return results.Finish();
        }
        return 2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "ERROR " << error.what() << '\n';
        return 2;
    }
}
