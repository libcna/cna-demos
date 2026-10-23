// SPDX-License-Identifier: MS-PL
// Container paths stay inside the container: CNA refuses absolute paths and ".." escapes.
#include <memory>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageContainer.hpp"
#include "Microsoft/Xna/Framework/Storage/StorageDevice.hpp"
#include "System/ArgumentException.hpp"
#include "System/IAsyncResult.hpp"
#include "System/IO/Stream.hpp"

using namespace Microsoft::Xna::Framework::Storage;

namespace
{
    // Each refusal is a System::ArgumentException (ArgumentNullException for an empty name), the
    // type XNA code catches; another exception type counts as a failure (CNA-DEMOS-ISSUE-010).
    template <typename Action>
    bool Refused(CnaDemos::DemoContext& ctx, const std::string& what, Action action)
    {
        try
        {
            action();
        }
        catch (const System::ArgumentException& e)
        {
            ctx.Print("{}: refused ({})", what, e.what());
            return true;
        }
        catch (const std::exception& e)
        {
            ctx.Print("{}: refused with the wrong exception type ({})", what, e.what());
            return false;
        }
        ctx.Print("{}: accepted", what);
        return false;
    }

    void PathRules(CnaDemos::DemoContext& ctx)
    {
        StorageDevice::SetAppNameEXT("cna-demos");
        const std::unique_ptr<System::IAsyncResult> selector = StorageDevice::BeginShowSelector(nullptr, nullptr);
        const std::unique_ptr<StorageDevice> device = StorageDevice::EndShowSelector(selector.get());
        {
            const std::unique_ptr<System::IAsyncResult> opening = device->BeginOpenContainer("cna-demos paths", nullptr, nullptr);
            const std::unique_ptr<StorageContainer> container = device->EndOpenContainer(opening.get());

            // Inside the container any relative path works, including ".." that stays inside.
            container->CreateDirectory("a");
            (void)container->CreateFile("a/../inside.txt");
            ctx.Check(container->FileExists("inside.txt"), "a path that stays inside the container is fine");

            // A save file name often comes from the player (a profile name): it must not be able
            // to reach other files. XNA/FNA combined paths blindly; CNA treats a container as a
            // boundary.
            ctx.Check(Refused(ctx, "CreateFile(\"../escape.txt\")", [&] { (void)container->CreateFile("../escape.txt"); }),
                      "a path leaving the container is refused");
            ctx.Check(Refused(ctx, "OpenFile(\"/etc/passwd\")", [&] {
                          (void)container->OpenFile("/etc/passwd", System::IO::FileMode::Open);
                      }),
                      "an absolute path is refused");
            ctx.Check(Refused(ctx, "FileExists(\"\")", [&] { (void)container->FileExists(""); }),
                      "an empty file name is refused");
        }
        ctx.Check(Refused(ctx, "DeleteContainer(\"../../other-game\")", [&] { device->DeleteContainer("../../other-game"); }),
                  "DeleteContainer only deletes this program's containers");
        device->DeleteContainer("cna-demos paths");
    }
}

CNA_DEMOS_CONSOLE(PathRules, {
    .id = "storage.storage_container.path_rules",
    .title = "Paths stay inside the container",
    .description = "Shows CNA's path rules for storage: relative paths that stay inside a container work, while "
                   "paths escaping with \"..\", absolute paths, empty names and DeleteContainer outside the storage "
                   "root are refused.",
    .apis = {"StorageContainer::CreateFile", "StorageContainer::OpenFile", "StorageContainer::FileExists",
             "StorageContainer::CreateDirectory", "StorageDevice::DeleteContainer",
             "StorageDevice::BeginOpenContainer", "StorageDevice::EndOpenContainer"},
    .tags = {"storage", "security", "path", "traversal", "file"},
})
