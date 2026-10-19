// SPDX-License-Identifier: MS-PL
// Native companion to StorageContractsOracle.cs, using the real public acquisition path.
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/IO/IOException.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

using Microsoft::Xna::Framework::Storage::StorageDevice;
using namespace System::IO;
namespace fs = std::filesystem;
namespace
{
int checks = 0, failed = 0;
void Check(bool ok, const std::string& name)
{
    ++checks;
    if (!ok) { ++failed; std::cout << "FAIL " << name << '\n'; }
}
template<class F> void Disposed(F action, const char* name)
{
    bool caught = false;
    try { action(); }
    catch (const System::ObjectDisposedException&) { caught = true; }
    catch (const std::exception& e) { std::cout << "OTHER " << name << ": " << e.what() << '\n'; }
    Check(caught, name);
}
template<class F> bool Refuses(F open)
{
    try { auto stream = open(); return false; }
    catch (const IOException&) { return true; }
}
}
int main(int argc, char** argv)
{
    std::cout << std::unitbuf;
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (argc != 2 || !xdg || fs::path(xdg) != fs::path(argv[1]) ||
        !fs::path(argv[1]).is_absolute() || fs::exists(argv[1])) return 2;
    const fs::path fixture(argv[1]);
    StorageDevice::SetAppNameEXT("StorageContractProbe");
    const fs::path root = fixture / "StorageContractProbe" / "isolated" / "AllPlayers";
    auto selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
    auto device = StorageDevice::EndShowSelector(selector.get());
    auto operation = device->BeginOpenContainer("isolated", nullptr, nullptr);
    auto c = device->EndOpenContainer(operation.get());
    if (!fs::is_directory(root)) return 2;
    { std::ofstream seed(root / "seed.bin", std::ios::binary); seed.write("abcd", 4); }
    fs::create_directory(root / "child");
    Check(c->getDisplayNameProperty() == "isolated" && !c->getIsDisposedProperty(), "live properties");
    Check(c->FileExists("seed.bin") && c->DirectoryExists("child"), "live filesystem");
    const FileAccess accesses[]{FileAccess::Read, FileAccess::Write, FileAccess::ReadWrite};
    const FileShare shares[]{FileShare::None, FileShare::Read, FileShare::Write, FileShare::ReadWrite};
    for (auto a1 : accesses) for (auto s1 : shares) for (auto a2 : accesses) for (auto s2 : shares)
    {
        const bool compatible = (static_cast<int>(s1) & static_cast<int>(a2)) == static_cast<int>(a2) &&
                                (static_cast<int>(s2) & static_cast<int>(a1)) == static_cast<int>(a1);
        auto first = c->OpenFile("seed.bin", FileMode::Open, a1, s1);
        Check(Refuses([&] { return c->OpenFile("seed.bin", FileMode::Open, a2, s2); }) != compatible,
              "sharing " + std::to_string(static_cast<int>(a1)) + "/" + std::to_string(static_cast<int>(s1)) +
              " -> " + std::to_string(static_cast<int>(a2)) + "/" + std::to_string(static_cast<int>(s2)));
    }
    {
        auto first = c->OpenFile("seed.bin", FileMode::Open);
        Check(Refuses([&] { return c->OpenFile("seed.bin", FileMode::Open, FileAccess::Read, FileShare::ReadWrite); }),
              "two-argument container open is exclusive");
    }
    {
        auto first = c->OpenFile("seed.bin", FileMode::Open, FileAccess::Read);
        Check(Refuses([&] { return c->OpenFile("seed.bin", FileMode::Open, FileAccess::Read, FileShare::ReadWrite); }),
              "three-argument container open is exclusive");
    }
    try {
        auto append = c->OpenFile("seed.bin", FileMode::Append);
        Check(!append->getCanReadProperty() && append->getCanWriteProperty() && append->getPositionProperty() == 4,
              "two-argument Append defaults to write access");
    }
    catch (const std::exception& e) {
        std::cout << "OTHER Append: " << e.what() << '\n';
        Check(false, "two-argument Append defaults to write access");
    }
    int events = 0;
    c->Disposing += [&](System::Object* sender, const System::EventArgs& args) {
        Check(sender == c.get() && &args == &System::EventArgs::Empty && c->getIsDisposedProperty(),
              "disposing event identity and state");
        ++events;
    };
    c->Dispose(); c->Dispose();
    Check(c->getIsDisposedProperty() && events == 1, "idempotent disposal");
    Disposed([&] { (void)c->getDisplayNameProperty(); }, "DisplayName");
    Disposed([&] { (void)c->getStorageDeviceProperty(); }, "StorageDevice");
    Disposed([&] { c->CreateDirectory("new"); }, "CreateDirectory");
    Disposed([&] { c->DeleteDirectory("child"); }, "DeleteDirectory");
    Disposed([&] { (void)c->DirectoryExists("child"); }, "DirectoryExists");
    Disposed([&] { (void)c->GetDirectoryNames(); }, "GetDirectoryNames");
    Disposed([&] { (void)c->GetDirectoryNames("*"); }, "GetDirectoryNames pattern");
    Disposed([&] { auto stream = c->CreateFile("new.bin"); }, "CreateFile");
    Disposed([&] { c->DeleteFile("seed.bin"); }, "DeleteFile");
    Disposed([&] { (void)c->FileExists("seed.bin"); }, "FileExists");
    Disposed([&] { (void)c->GetFileNames(); }, "GetFileNames");
    Disposed([&] { (void)c->GetFileNames("*"); }, "GetFileNames pattern");
    Disposed([&] { auto stream = c->OpenFile("seed.bin", FileMode::Open); }, "OpenFile mode");
    Disposed([&] { auto stream = c->OpenFile("seed.bin", FileMode::Open, FileAccess::Read); }, "OpenFile access");
    Disposed([&] { auto stream = c->OpenFile("seed.bin", FileMode::Open, FileAccess::Read, FileShare::Read); }, "OpenFile share");
    Disposed([&] { (void)c->FileExists(""); }, "disposed before empty path");
    Disposed([&] { c->CreateDirectory("../outside"); }, "disposed before path validation");
    Disposed([&] { (void)c->GetFileNames(""); }, "disposed before pattern validation");
    Check(fs::exists(root / "seed.bin") && fs::is_directory(root / "child") &&
          !fs::exists(root / "new.bin") && !fs::exists(root / "new"), "rejected operations preserve files");
    c.reset(); device.reset();
    // Remove only named fixtures created by this invocation; no recursive deletion.
    for (const char* name : {"seed.bin", "new.bin", "child", "new"}) fs::remove(root / name);
    fs::remove(root.parent_path() / "outside");
    fs::remove(root); fs::remove(root.parent_path()); fs::remove(root.parent_path().parent_path()); fs::remove(fixture);
    StorageDevice::SetAppNameEXT("");
    std::cout << "checks=" << checks << " failed=" << failed << '\n';
    return failed ? 1 : 0;
}
