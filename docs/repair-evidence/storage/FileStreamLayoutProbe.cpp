// SPDX-License-Identifier: MS-PL
// Before/after ABI evidence for a potential native-handle FileShare implementation.
#include "System/IO/FileStream.hpp"
#include <iostream>
#include <type_traits>

int main()
{
    using System::IO::FileStream;
    using System::IO::Stream;
    std::cout << "FileStream sizeof=" << sizeof(FileStream) << " alignof=" << alignof(FileStream)
              << " copy=" << std::is_copy_constructible_v<FileStream>
              << " move=" << std::is_move_constructible_v<FileStream> << '\n';
    std::cout << "Stream sizeof=" << sizeof(Stream) << " alignof=" << alignof(Stream) << '\n';
}
