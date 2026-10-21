# Sprite texture binding (ISSUE-003)

## Current evidence boundary (2026-10-21)

**Confirmed by actual baseline61003 after156 commit7097e1684:** strict compile
2210ms, six original tests on each SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU. Every
renderer gives the same fiveFAIL/onePASS: deferred/immediate publication, both
sorted final bindings and bound SetData refusal fail; empty-batch preservation
passes. Runs78/387/338/909ms, frozen inputs unchanged, peak415399936bytes and all
memory events0. The observation driver exits0 because it captured the expected
reproduction; the test processes each exit1 and all failures remain in XML/audit.
No003 production change or closure. All previous prepared/unrun wording below
describes the state before this baseline, not its current validation status.

Read-only source audit at CNA `2af199aad4d11053a7549bee01f9efff272cd9e0`,
with the separately documented uncommitted011/082/156 candidates present.
No production edit, new runtime pass or issue closure is claimed here.
`SpriteTextureBindingTests.cpp` contains six prepared, **uncompiled/unrun**
public-state witnesses. Run them unchanged after the current native98308 and
the prepared132-case external156 comparison finish; no overlapping heavy job.
`build-probe/repair-sprite-texture-binding-before.mjs` is syntax/transformation
checked, **UNRUN**. It reuses the actual native consumer compile flags and links
the current external library, then records all six original outcomes on each of
SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU. It requires the132-case observation audit to
be complete and sources/artifacts unchanged; it does not treat that audit's
separate C callback failures as accepted regressions.

The historical inventory's null binding is still explained by current source:

- `SpriteBatch::pushSprite` retains only `shared_ptr<ITextureRenderer>` in each
  queued `SpriteInfo`, not a safely tracked public `Texture2D` identity.
- `flushSingle` sends the retained renderer to its sprite renderer, without
  assigning `GraphicsDevice::getTexturesProperty()`.
- `drawThroughStockEffect` similarly overrides `GpuDrawParams::texture0` after
  applying each effect pass, but does not publish the corresponding public slot.
- `TextureCollection::operator()` already provides disposed/device/render-target
  validation. `Texture::ThrowIfDataTransferResourceInUseEXT` relies on the public
  slot; silently using only the renderer bypasses that observable binding.

## Genuine XNA source evidence

The existing pinned local XNA assembly decompilation is:
`../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/`
`Microsoft.Xna.Framework.Graphics/SpriteBatch.cs`.
Its `RenderBatch` assigns `_parent.Textures[0] = texture` immediately before
`PlatformRenderBatch`, once per custom-effect pass and after that pass's Apply;
the default-effect branch also assigns it. `SortSprites` orders the queue before
rendering, so the final slot follows rendered order, not original submission order.
This is source evidence, **not a new genuine-XNA GPU runtime execution**.

The six prepared witnesses cover deferred publication timing, immediate per-draw
publication, empty-batch preservation, both depth-sort directions with distinct
depths, and refusal/restoration of SetData around explicit unbinding.
They do not yet establish custom-effect pass ordering or wrapper lifetime safety.

## Required lifetime audit before implementing

Do not add an unguarded `Texture2D*` to the queue. Current SpriteBatch explicitly
retains renderer ownership so destroying a temporary before End is safe; tests
around `SpriteBatchTests.cpp`'s retained-texture renderer seam protect that route.
Current End also documents a deliberate disposal-before-End difference from XNA.
The inherited011 shared content identity changes must remain isolated.

A repair must preserve safe destruction/move handling, distinguish an explicitly
disposed live wrapper from one whose C++ lifetime ended, and avoid leaving a
dangling public slot. Inspect the current texture detach hooks, resource identity
and tests before selecting the smallest correct representation. Add focused
lifetime/sanitizer and custom-effect-order controls alongside the public-state
regression. Do not claim those controls are covered by the six prepared cases.
