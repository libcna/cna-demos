# Changelog

All notable changes to cna-demos. The format follows [Keep a Changelog](https://keepachangelog.com/);
versions follow Semantic Versioning (pre-1.0: a minor version may change the CLI or metadata).

## [Unreleased]

### Added
- Project foundation: CMake integration with a CNA source checkout (`CNA_ROOT_DIR`, presets
  `default`, `windowless`, `linux-multi`, `linux-full`), Ms-PL licence, documentation set.
- Demo framework: self-registering registry, `DemoInfo` metadata, console/Game/standalone demo
  kinds, `DemoGame` base class (need checks, automated frame limit, Escape to exit, screenshots),
  `DemoContext` (output, checks, seed), runner with PASSED/FAILED/SKIPPED/NOT RUN results.
- `cna-demos` executable: interactive terminal browser and CLI (`--list`, `--categories`,
  `--search`, `--info`, `--run-category`, `--run-tag`, `--run-all`, `--smoke`, `--catalog-json`,
  `--list-ids`, `--validate`, `--renderers`, `--renderer`, `--frames`, `--seed`, `--screenshot`,
  `--report`).
- CTest integration: CLI tests and per-demo tests discovered at ctest time.
- Coverage report generator (`tools/coverage_report.py`) and deterministic asset generator
  (`tools/generate_assets.py`).
- Pilot demos in seven categories (math, core, graphics, content, audio, input, platform),
  including one standalone demo.
- Math demos: Vector2/Vector3/Vector4 (constants, arithmetic, dot, cross, normalize, distance,
  lerp, smooth step, Catmull-Rom, Hermite, barycentric, clamp/min/max, reflect, transforms,
  by-reference overloads, equality/ToString, homogeneous coordinates), Matrix (identity, SRT,
  axis rotations, axis-angle, yaw/pitch/roll, look-at, perspective, orthographic, invert,
  transpose, determinant, decompose, world/direction properties, billboards, reflection/shadow,
  element-wise operations, quaternion transform), Quaternion (axis-angle, matrix conversion,
  concatenate, slerp/lerp, inverse/conjugate, normalize), MathHelper (angles, scalar helpers,
  curves).
- `--log-level` option (CNA's logger defaults to `warn` in cna-demos).
- `tools/validate.sh`: build, validate, ctest, smoke reports and coverage in one command.
- `CnaDemos/MathChecks.hpp`: tolerance helpers for matrix/quaternion checks.
- Math demos: Point, Rectangle (edges, contains, intersect/union, inflate/offset), BoundingBox,
  BoundingSphere, BoundingFrustum culling, Plane, Ray, Curve (evaluate, tangents, loop types, step
  continuity, key collection), Color (construction, premultiplied alpha, lerp/vectors, packed
  value); packed vector formats (compact colour, half precision, signed normalized, integer,
  wide unorm, IPackedVector).
- `knownIssue` metadata: demos that fail because of a recorded CNA defect are expected failures in
  batch runs and CTest (`WILL_FAIL`), and an unexpected pass is reported.
- Game lifecycle demos (25): method order, Exit/Exiting, RunOneFrame, host-driven RunFrameEXT,
  fixed/variable time step, IsRunningSlowly, SuppressDraw, GameComponent, DrawableGameComponent,
  UpdateOrder/DrawOrder, component collection events, GameServiceContainer, back-buffer size,
  PreparingDeviceSettings, device events, Reach/HiDef profile, vertical sync, full screen (manual),
  window title/client bounds/resizing, IsMouseVisible/InactiveSleepTime, LaunchParameters,
  TitleContainer.
- SpriteBatch demos (14): destination/source rectangles, sprite-sheet animation, tint and fade,
  rotation/origin, scale, SpriteEffects flips, layer depth and sort modes, blend states, straight-alpha
  loose PNGs (NonPremultiplied), point/linear sampling, wrap tiling, transform-matrix camera.
- Graphics device demos (12): ClearOptions (and the error for a missing plane), split screen with
  viewports (XNA 4.0's Clear ignores the viewport), Viewport properties and Project/Unproject
  picking, scissor rectangle, PresentationParameters, GetBackBufferData (whole/region),
  SupportsCapability and the capability report (CNAEXT), GraphicsAdapter display modes and
  format queries, GraphicsResource lifetime events (ResourceCreated/Destroyed, Name, Tag).
- `CnaDemos/GraphicsNames.hpp`: names of graphics enums for demo output.
- Texture2D demos (7): procedural SetData, rectangle SetData on a canvas (and XNA's rule that a
  texture set on the device cannot be written), per-pixel collision with GetData, mipmap levels,
  16-bit surface formats, FromStream (as stored, fitted, zoomed), SaveAsPng/SaveAsJpeg round trips.
- SpriteBatch demos (5): SpriteSortMode::Texture, Immediate mode with device states changed between
  Draws (blend state; sampler state is CNA-DEMOS-ISSUE-004), several batches per frame, Begin/End
  rules.
- Compiled content: `content/` is built at build time by CNA's `cna-content` (`cna_add_content`)
  into `bin/assets`; `CNA_DEMOS_CONTENT_EXECUTABLE` names a host tool for cross builds.
- Original pixel font (`tools/pixel_font.py`), generated as an XNA font-texture sheet and compiled
  with `CNA.FontTextureProcessor` into `fonts/pixel_font`.
- SpriteFont demos (7): DrawString, MeasureString (centring, alignment, line rules), rotated/scaled/
  mirrored text, Spacing/LineSpacing, Characters/DefaultCharacter, StringBuilder text, and a font
  built in code with the CNAEXT SpriteFont constructor.
- CNA-DEMOS-ISSUE-003 (SpriteBatch leaves Textures[0] unset) and CNA-DEMOS-ISSUE-004 (Immediate
  SpriteBatch ignores sampler states set between Draws) recorded.
- Screenshots of a demo running under Reach are reported as unavailable instead of failing the run.
- `tools/contact_sheet.py`: captures Game demos on a renderer into one labelled image for review.
- `deterministic` now means "identical capture on every machine"; GameTime-driven animation is
  excluded (catch-up Updates make game time at frame N machine-dependent).
- The runner calls `Dispose()` after `Run()` (XNA's `using` pattern), so demos' `UnloadContent`
  runs while the demo object is alive.
- CNA-DEMOS-ISSUE-002 recorded: `Curve::Evaluate` with Step keys compares the absolute position
  with 1.0.
