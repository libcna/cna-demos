// SPDX-License-Identifier: MS-PL
// A second strong definition is observable without deliberately corrupting memory.
int runtimeInitializations = 0;
struct RuntimeInitializer
{
    RuntimeInitializer() { ++runtimeInitializations; }
};
RuntimeInitializer runtimeInitializer;
#ifdef PROBE_CYCLE
extern int peerValue();
int coreValue() { return runtimeInitializations + peerValue(); }
#else
int coreValue() { return runtimeInitializations; }
#endif
