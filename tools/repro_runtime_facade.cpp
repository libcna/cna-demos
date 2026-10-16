// SPDX-License-Identifier: MS-PL
// Minimal link-time reproducer for CNA-DEMOS-ISSUE-150.
#include "CNA/Misc.hpp"

int main()
{
    CNA::Runtime runtime;
    CNA::RuntimeOptions options;
    options.enableContent = false;
    if (!runtime.Initialize(options)) return 1;
    const bool enabled = runtime.IsGraphicsEnabled() || runtime.IsAudioEnabled() || runtime.IsInputEnabled();
    runtime.Shutdown();
    return enabled ? 0 : 2;
}
