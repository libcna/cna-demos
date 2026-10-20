// SPDX-License-Identifier: MS-PL
// Same twenty path checks as TitlePathsOracle.cs, through the real native API.
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <typeinfo>

using namespace Microsoft::Xna::Framework;
namespace fs = std::filesystem;
namespace
{
int checks = 0, failed = 0;
void Check(bool ok, const std::string& name)
{
    ++checks;
    if (!ok) { ++failed; std::cout << "FAIL " << name << '\n'; }
}
template<class T> void Reject(const std::string& path)
{
    bool caught = false;
    try { auto stream = TitleContainer::OpenStream(path); }
    catch (const std::exception& e) { caught = typeid(e) == typeid(T); }
    Check(caught, "exception category path=" + path);
}
}
int main(int argc, char** argv)
{
    if (argc != 2 || fs::exists(argv[1])) return 2;
    const fs::path root = fs::absolute(argv[1]), cwd = fs::current_path();
    const std::string oldTitle = TitleLocation::Path(), name = root.filename().string();
    fs::create_directories(root / "child");
    { std::ofstream seed(root / "seed.bin", std::ios::binary); seed.write("\1\2\3\4", 4); }
    TitleLocation::setPathProperty(root.parent_path().string());
    fs::current_path(root / "child");
    for (const auto& relative : {name + "/seed.bin", name + "\\seed.bin",
                                name + "/./seed.bin", name + "/child/../seed.bin"})
    {
        auto stream = TitleContainer::OpenStream(relative);
        Check(stream->getLengthProperty() == 4 && stream->ReadByte() == 1 &&
              stream->getCanReadProperty() && stream->getCanSeekProperty() &&
              !stream->getCanWriteProperty(), "relative content/control " + relative);
    }
    Reject<System::ArgumentNullException>("");
    Reject<System::ArgumentException>((root / "seed.bin").string());
    Reject<System::ArgumentException>("/rooted.bin");
    Reject<System::ArgumentException>("\\rooted.bin");
    Reject<System::ArgumentException>("C:drive-relative.bin");
    Reject<System::ArgumentException>("../outside.bin");
    for (char bad : {':', '*', '?', '"', '<', '>', '|'})
        Reject<System::ArgumentException>(name + "/bad" + bad + ".bin");
    Reject<System::IO::FileNotFoundException>(name + "/missing.bin");
    Reject<System::IO::FileNotFoundException>(name + "/missing/child.bin");
    Reject<System::InvalidOperationException>(name + "/child");
    fs::current_path(cwd);
    TitleLocation::setPathProperty(oldTitle);
    fs::remove(root / "seed.bin"); fs::remove(root / "child"); fs::remove(root);
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed ? 1 : 0;
}
