// SPDX-License-Identifier: MS-PL
#include <cstdio>
extern int frameworkValue();
extern int objectValue();
int main()
{
    const int framework = frameworkValue();
    const int object = objectValue();
    std::printf("framework=%d object=%d expected=1\n", framework, object);
    return framework == 1 && object == 1 ? 0 : 1;
}
