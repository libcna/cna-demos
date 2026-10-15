// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "platform.window.assembly_title",
    .title = "Declare the default window title",
    .description = "Registers a product title with AssemblyTitleAttributeEXT, overrides it through "
                   "SetAssemblyTitleEXT before game construction, and verifies the real GameWindow starts "
                   "with that title.",
    .apis = {"CNA::AssemblyTitleAttributeEXT", "CNA::SetAssemblyTitleEXT", "CNA::GetAssemblyTitleEXT",
             "Game::Window", "GameWindow::Title"},
    .tags = {"window", "title", "assembly", "metadata"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
    .restrictions = "Standalone: the assembly title is process-wide and must be declared before the game creates "
                    "its platform window.",
})
