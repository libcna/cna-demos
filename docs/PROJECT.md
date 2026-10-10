# The idea behind CNA Demos

## What it is

CNA Demos is a collection of **small executable demonstrations covering CNA functionality at a
much finer granularity than traditional samples**. Each demo shows one capability -- clear the
screen, rotate a sprite, load a PNG, play a sound, read the keyboard -- in a program short enough
to read in a minute, with a stable ID, machine-readable metadata, and a defined way to run it
unattended.

The collection is expected to grow to thousands of demos, because CNA's public surface is large:
the documented XNA 4.0 runtime alone has 331 types and over 3,600 members, and CNA adds its own
extensions (renderer selection, PBR effects, glTF, CNB/CNJ content, devices, diagnostics, a C API).
The goal is not a number. The goal is that **every meaningful CNA capability has a minimal,
executable, discoverable demonstration showing a developer how to use it.**

## What it is for

The same demo serves several purposes at once:

| Purpose | How the collection serves it |
|---|---|
| Learnability | a developer finds "how do I do X in CNA?" as a 30-line program that runs |
| Discoverability | category tree, search over titles/descriptions/tags/APIs, a JSON catalog |
| Documentation | metadata is structured enough to generate API example pages later |
| API confidence | each demo uses the real CNA API of the checked-out version, and is built and run |
| Regression detection | automated demos carry checks; every demo is a CTest test |
| Renderer validation | one process per renderer; the same demo runs on SOFTWARE, OpenGL ES 3, Vulkan, ... |
| Maintainability of CNA | gaps and bugs surface as failing demos and are recorded in `CNA_ISSUES.md` |

## How it differs from neighbouring things

| | Purpose | Granularity | Audience |
|---|---|---|---|
| **CNA unit tests** (`CnaTests`, GoogleTest, inside CNA) | prove every member behaves per the XNA spec | one assertion per behaviour, no user-facing program | CNA maintainers |
| **CNA integration/regression tests** (pixel oracles, renderer contract tests) | prove renderers agree with references | scenes engineered for comparison | CNA maintainers |
| **cna-samples** (114 ports of the XNA Game Studio samples) | show complete techniques and small games | one sample = many features | game developers learning techniques |
| **cna-examples** (one catalog app, ~250 screens) | an interactive tour of CNA features | screens inside one shared app framework | people exploring CNA |
| **cna-template** | a starting point for a new game | one game skeleton | new projects |
| **CNA Demos** | executable documentation, one concept per program | **one concept, one ID, one file** | developers looking up a capability; CI; CNA maintainers checking coverage |

A demo is not a test: it is written to be *read*, and its checks verify only what it shows. A
demo is not a sample: it deliberately avoids combining features. Where a sample demonstrates a
technique built from many APIs, a demo isolates one of those APIs.

## Principles

1. **Real API only.** Every demo is based on CNA's headers at the checked-out revision -- never on
   assumptions from XNA, MonoGame, FNA or SDL. If CNA lacks a feature, the demo does not exist
   and the gap is recorded.
2. **One concept per demo.** The CNA call the demo teaches stays visible in the demo's source.
   The framework removes boilerplate; it never wraps CNA in a second API.
3. **Stable identity.** IDs are permanent, textual, hierarchical. Numbers are menu positions.
4. **Honest results.** Compiled is not run; run is not visually verified. SKIPPED means a
   documented requirement is missing, never "it failed so we skipped it".
5. **Scale without magic.** Adding a demo is adding a file. No generated registry, no central list,
   no Python needed to build.
6. **Long-lived.** The repository is always buildable, the handoff is always current, and every
   decision is written down where the next maintainer will look.

## Scope boundaries

* cna-demos does **not** develop CNA features. It may fix an obvious, low-risk CNA bug in CNA's own
  repository, separately and with validation; anything else is documented in `CNA_ISSUES.md`.
* cna-demos does **not** copy CNA samples. It may demonstrate one aspect a sample also uses.
* Network services, credentials and hardware-specific features are demonstrated where CNA has
  them, but classified honestly (manual) when they cannot run unattended.
