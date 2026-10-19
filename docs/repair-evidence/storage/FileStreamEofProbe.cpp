// SPDX-License-Identifier: MS-PL
#include "System/IO/FileStream.hpp"
#include "System/IO/SeekOrigin.hpp"
#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace System::IO;
namespace
{
int checks = 0, failed = 0;
void Check(bool ok, const char* name)
{
    ++checks;
    if (!ok) { ++failed; std::cout << "FAIL " << name << '\n'; }
}
}
int main(int argc, char** argv)
{
    if (argc != 2 || std::filesystem::exists(argv[1])) return 2;
    {
        std::ofstream seed(argv[1], std::ios::binary);
        if (!seed) return 2;
        const char bytes[]{1, 2, 3, 4};
        seed.write(bytes, 4);
    }
    {
        FileStream file(argv[1], FileMode::Open, FileAccess::ReadWrite);
        std::array<SharpRuntime::bytecs, 8> buffer{77,77,77,77,77,77,77,77};
        Check(file.Read(buffer.data(),1,6)==4,"short read count");
        Check(buffer[0]==77 && buffer[1]==1 && buffer[2]==2 && buffer[3]==3 &&
              buffer[4]==4 && buffer[5]==77 && buffer[6]==77 && buffer[7]==77,"short read data and guards");
        Check(file.getPositionProperty()==4,"position after short read");
        Check(file.Read(buffer.data(),1,2)==0,"repeated EOF count");
        Check(file.getPositionProperty()==4,"position after repeated EOF");
        try { Check(file.Seek(-1,SeekOrigin::Current)==3,"relative seek after EOF"); }
        catch (const std::exception&) { Check(false,"relative seek threw"); }
        Check(file.Seek(0,SeekOrigin::Begin)==0,"explicit rewind control");
        Check(file.Read(buffer.data(),1,4)==4 && buffer[4]==4,"full reread control");
        Check(file.Read(buffer.data(),1,1)==0,"EOF before write");
        file.WriteByte(9);
        Check(file.getPositionProperty()==5,"write after EOF advances");
        Check(file.getLengthProperty()==5,"write after EOF extends");
        file.setPositionProperty(0);
        Check(file.Read(buffer.data(),1,6)==5 && buffer[5]==9,"write after EOF persists");
        Check(file.getPositionProperty()==5,"final short read position");
    }
    std::filesystem::remove(argv[1]); // Only the exact new probe fixture created above.
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed==0?0:1;
}
