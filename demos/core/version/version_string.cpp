// SPDX-License-Identifier: MS-PL
// Reading the CNA version a program was compiled against.
#include "CNA/Version.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    void VersionString(CnaDemos::DemoContext& ctx)
    {
        // Everything in CNA/Version.hpp is constexpr: it describes the CNA headers this
        // program was compiled with.
        ctx.Print("CNA::getVersionString()  = {}", CNA::getVersionString());
        ctx.Print("major.minor.patch        = {}.{}.{}", CNA::getVersionMajor(), CNA::getVersionMinor(),
                  CNA::getVersionPatch());
        ctx.Print("pre-release              = {}", CNA::isPreReleaseVersion() ? "yes" : "no");

        ctx.Check(!CNA::getVersionString().empty(), "the version string is not empty");
        ctx.Check(CNA::getVersionString().starts_with(std::to_string(CNA::getVersionMajor()) + "."),
                  "the version string starts with the major version");
    }
}

CNA_DEMOS_CONSOLE(VersionString, {
    .id = "core.version.version_string",
    .title = "CNA version",
    .description = "Prints the CNA version the program was compiled against, as a string and as numeric "
                   "components.",
    .apis = {"CNA::getVersionString", "CNA::getVersionMajor", "CNA::getVersionMinor", "CNA::getVersionPatch",
             "CNA::isPreReleaseVersion"},
    .tags = {"version"},
})
