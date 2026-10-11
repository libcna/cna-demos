// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Process.hpp"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstdint>
#include <iterator>
#include <system_error>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <sys/wait.h>
#else
#  include <sys/wait.h>
#endif

namespace CnaDemos
{
    std::filesystem::path ExecutableDirectory()
    {
#if defined(_WIN32)
        wchar_t buffer[MAX_PATH * 4];
        const DWORD length = GetModuleFileNameW(nullptr, buffer, static_cast<DWORD>(std::size(buffer)));
        if (length > 0 && length < std::size(buffer))
        {
            return std::filesystem::path(buffer).parent_path();
        }
#elif defined(__APPLE__)
        char buffer[4096];
        std::uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
        {
            std::error_code error;
            const auto path = std::filesystem::weakly_canonical(buffer, error);
            return (error ? std::filesystem::path(buffer) : path).parent_path();
        }
#else
        std::error_code error;
        const auto path = std::filesystem::read_symlink("/proc/self/exe", error);
        if (!error)
        {
            return path.parent_path();
        }
#endif
        return std::filesystem::current_path();
    }

    namespace
    {
        std::string Quote(const std::string& argument)
        {
#if defined(_WIN32)
            std::string quoted = "\"";
            for (const char c : argument)
            {
                if (c == '"')
                {
                    quoted += '\\';
                }
                quoted += c;
            }
            return quoted + "\"";
#else
            std::string quoted = "'";
            for (const char c : argument)
            {
                if (c == '\'')
                {
                    quoted += "'\\''";
                }
                else
                {
                    quoted += c;
                }
            }
            return quoted + "'";
#endif
        }
    }

    int RunProcess(const std::filesystem::path& program, const std::vector<std::string>& arguments)
    {
        std::string command = Quote(program.string());
        for (const std::string& argument : arguments)
        {
            command += ' ';
            command += Quote(argument);
        }
        // Children inherit stdout/stderr. Drain both C++ and C buffers before launching:
        // otherwise a child's output can split an earlier parent's buffered assertion line.
        std::cout.flush();
        std::cerr.flush();
        std::clog.flush();
        std::fflush(stdout);
        std::fflush(stderr);
#if defined(_WIN32)
        // cmd.exe strips one pair of outer quotes from the whole command line.
        command = "\"" + command + "\"";
        return std::system(command.c_str());
#else
        const int status = std::system(command.c_str());
        if (status == -1 || !WIFEXITED(status))
        {
            return -1;
        }
        return WEXITSTATUS(status);
#endif
    }
}
