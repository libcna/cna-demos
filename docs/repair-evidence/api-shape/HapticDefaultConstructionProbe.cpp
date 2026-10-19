// SPDX-License-Identifier: MS-PL
// ISSUE-009: public-header consumer, deliberately no platform implementation include.
#include "CNA/Input/HapticDevice.hpp"
int main()
{
    CNA::Input::HapticDevice device;
    return device.IsOpenEXT() ? 1 : 0;
}
