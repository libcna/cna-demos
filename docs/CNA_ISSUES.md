# CNA issues found by cna-demos

One purpose of cna-demos is to expose real CNA problems. A demo that reveals one does **not**
work around it silently: the problem is minimised, recorded here, and the demo either keeps
failing visibly or is marked with the issue ID in its `restrictions` metadata.

Each entry records the affected CNA area, the demo(s) involved, reproduction steps, observed
and expected behaviour, the environment, and the investigation status. IDs are permanent
(`CNA-DEMOS-ISSUE-NNN`); a fixed issue stays listed under "Resolved".

CNA revision used unless stated otherwise: `10a4ef1cb4cd832f761536d77588000104947f8d`
(CNA 0.1.0, "chore(release): prepare v0.1.0").

---

## Open

### CNA-DEMOS-ISSUE-123 -- Vector3 Dot and Cross lose XNA's wide product cancellation

- **Area/API:** `Microsoft::Xna::Framework::Vector3::Dot` and `Vector3::Cross`, including their
  output overloads.
- **Demo/reproduction:** run `math.vector3.dot` and `math.vector3.cross`. Dot
  `(1e20,1e20,1)` with `(1e20,-1e20,1)`, cross an all-`1e20` vector with itself, then cross
  `(1e20,1e20,1)` with `(1e20,1e20,2)`. Ordinary angles, projections, perpendiculars, triangle
  normals and output aliasing remain adjacent positive controls.
- **Observed:** CNA narrows each multiplication before addition/subtraction. The Dot products
  become positive and negative infinity and combine to NaN instead of cancelling before the final
  `+1`. Cross likewise subtracts equal infinities: the parallel result has three NaN components,
  and the partially huge result loses its otherwise exact zero Z component. Dot retains exactly one
  expected failure among six controls; Cross retains two among eight on each renderer label.
- **Expected/reference:** genuine Windows XNA 4.0 returns exactly `1.0f` through both Dot forms,
  exact `Vector3::Zero` through both parallel Cross forms, and `(1e20,-1e20,0)` for the partially
  huge Cross. Its 32-bit x87 evaluation retains the finite products beyond float range until each
  sum/difference is complete, then narrows once.
- **Cause/suggested repair:** compute each Dot accumulation and Cross product difference in a wider
  intermediate and narrow only its final scalar/component, following the already measured approach
  used by CNA's Vector3 matrix transforms. Keep Cross inputs cached so output aliasing remains safe.
- **Evidence/status:** open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux; genuine XNA ran from the installed Windows
  XNA 4.0 assembly under Wine. Evidence:`/tmp/checkpoint-1055-vector3-product-range/*`. The clean
  warning-free three-job build, both exact known-issue CTests, metadata695/0 and self-tests5/5 plus4/4
  pass; ten runs per demo and renderer label reproduce only the three documented assertions
  with empty stderr. No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-122 -- Vector2/3 component bounds differ at reversed ranges and signed-zero ties

- **Area/API:** `Microsoft::Xna::Framework::Vector2` and `Vector3` `Clamp`, `Min` and `Max`,
  including their output overloads.
- **Demo/reproduction:** run `math.vector2.component_bounds` and `math.vector3.clamp_min_max`.
  Clamp a mixed value between per-component minimum10 and maximum0, then call Min and Max with
  all-positive/all-negative zero vectors in both orders. NaN bounds are adjacent controls;
  `math.vector3.arithmetic` and `math.vector3.reflect` separately retain the matching Vector3
  division and reflection boundaries.
- **Observed:** CNA resolves the reversed Clamp interval to its maximum `(0,0)` rather than its
  minimum `(10,10)`. CNA Min and Max also preserve their first operand when equal zeros differ only
  by sign, opposite XNA's second-operand result. These are the only expected failures on each
  tested renderer label: two among seven controls in Vector2 and two among six in Vector3.
- **Expected/reference:** genuine Windows XNA 4.0 resolves both reversed components to the supplied
  minimum in value and output Clamp forms. Both Min and Max select their second operand when the
  comparison is equal: positive-then-negative order therefore returns negative zero, and reversing
  the operands returns positive zero. Value and output forms agree. XNA and CNA match on the three
  distinct NaN Clamp placements and every division/reflection control.
- **Cause/suggested repair:** CNA's shared `ClampScalar` composes `std::max(value,min)` then
  `std::min(...,max)`, which returns the maximum for reversed bounds; XNA caps against maximum first
  and then floors against minimum. CNA's direct `std::min`/`std::max` preserve the first operand on
  equality, whereas XNA's ordered comparisons fall through to the second. Preserve those orders
  explicitly rather than relying on the standard algorithms' tie policy. The duplicated vector
  implementations expose the same behavior and should be repaired consistently.
- **Evidence/status:** open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux; genuine XNA ran from the installed Windows
  XNA 4.0 assembly under Wine. Evidence:`/tmp/checkpoint-1050-vector2-component-bounds/*` and
  `/tmp/checkpoint-1052-vector3-identity-bounds/*`. Clean warning-free three-job builds, exact
  known-issue CTests, metadata695/0 and self-tests5/5 plus4/4 pass; ten runs per demo and renderer
  label reproduce only the documented assertions with empty stderr. No CNA source was modified and
  no upstream message was sent.

### CNA-DEMOS-ISSUE-121 -- Vector2/3 range transforms reject XNA's negative-length no-op

- **Area/API:** range overloads of `Microsoft::Xna::Framework::Vector2` and `Vector3` `Transform`
  for Matrix and Quaternion plus `TransformNormal` for Matrix.
- **Demo/reproduction:** run `math.vector2.transform` and `math.vector3.transform_array`. Apply each
  range overload with valid zero source/destination indices and `length=-1` to a guarded destination
  array. Adjacent controls cover full arrays, nonempty subranges, both directions of same-array
  overlap, ordinary invalid source/destination windows and zero length at one-past-end indices.
- **Observed:** CNA throws `std::out_of_range` for all three negative-length calls before writing.
  Each array workflow retains exactly this grouped expected failure: one among sixteen Vector2 and
  ten Vector3 controls. Every other value, alias, array, overlap, guard and refusal result matches
  the XNA reference.
- **Expected/reference:** genuine Windows XNA 4.0 accepts `length=-1` for all three overload
  families as an empty range and leaves every destination element unchanged. It also accepts zero
  length with both indices exactly one past the array ends, while rejecting the separately tested
  negative source index and overlong positive source/destination windows before any write.
- **Cause/suggested repair:** the duplicated Vector2/3 local `CheckArrayRange` explicitly rejects
  `length < 0`, whereas XNA reaches a zero-iteration loop for the observed valid-index calls.
  Preserve the positive-window validation and the one-past-end zero case while matching the
  observed negative-length no-op. Vector4 contains the same duplicated check and should be audited
  before changing its public behavior.
- **Evidence/status:** open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux; genuine XNA ran from the installed Windows
  XNA 4.0 assembly under Wine. Evidence:`/tmp/checkpoint-1049-vector2-transform-arrays/*` and
  `/tmp/checkpoint-1054-vector3-transform-arrays/*`. Warning-free three-job builds, exact known-
  issue CTests, metadata695/0 and self-tests5/5 plus4/4 pass; ten runs per demo and renderer label
  reproduce only the one grouped assertion with empty stderr. No CNA source was modified and no
  upstream message was sent.

### CNA-DEMOS-ISSUE-120 -- Equal signed-zero Vector2/3 and Quaternion values have unequal hashes

- **Area/API:** `Microsoft::Xna::Framework::Vector2`, `Vector3` and `Quaternion` typed/boxed/operator
  equality and `GetHashCode`.
- **Demo/reproduction:** run `math.vector2.toolbox_2d`, `math.vector3.equality_and_string` and
  `math.quaternion.value_identity`. Compare UnitX/Identity with otherwise identical values
  containing `-0.0f`, then compare each all-positive-zero value with a negative-zero component.
  Ordinary construction and equality/hash pairs are adjacent positive controls.
- **Observed:** CNA correctly treats positive and negative zero as equal through every tested
  equality form, but hashes their raw IEEE-754 bits. Quaternion Identity hashes to `1065353216`
  while its signed-X-zero equal hashes to `-1082130432`; zero hashes to0 while its signed-W-zero
  equal hashes to `-2147483648`. Vector2 exhibits the equivalent UnitX/Y-zero and origin/X-zero
  mismatches, and Vector3 does the same for UnitX/Y-zero and origin/X-zero. Each demo retains exactly
  one expected hash failure, among five Quaternion, fourteen Vector2 and seven Vector3 controls.
- **Expected/reference:** genuine Windows XNA 4.0 treats all six pairs as equal and gives each pair
  equal hashes: `1065353216` for both tested unit values and0 for both zero values in every family.
  The equality/hash invariant therefore holds regardless of zero's sign bit.
- **Cause/suggested repair:** CNA's repeated `FloatHash` helpers copy each float's raw bits, while
  float equality equates both zeros. Canonicalize either zero sign to the same hash before combining
  components and apply the rule consistently to every affected value type.
- **Evidence/status:** open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux; genuine XNA ran from the installed Windows
  XNA 4.0 assembly under Wine. Evidence:`/tmp/checkpoint-1044-quaternion-signed-zero-hash/*` and
  `/tmp/checkpoint-1047-vector2-signed-zero-hash/*` and
  `/tmp/checkpoint-1052-vector3-identity-bounds/*`. Warning-free three-job builds, exact known-issue
  CTests, metadata695/0 and self-tests5/5 plus4/4 pass; ten runs per demo and renderer label
  reproduce only each demo's one hash assertion with empty stderr. No CNA source was modified and
  no upstream message was sent.

### CNA-DEMOS-ISSUE-119 -- Quaternion inverse and division lose extreme finite magnitudes

- **Area/API:** `Microsoft::Xna::Framework::Quaternion::Inverse` and `Quaternion::Divide`, value
  and output overloads.
- **Demo/reproduction:** run `math.quaternion.inverse_and_conjugate` and
  `math.quaternion.double_cover`. Invert quaternions with a sole X component of `1e20` and `1e-20`,
  then divide Identity by the same values. Ordinary unit/nonunit, output-overload, identity and
  zero controls distinguish the extreme-range defect from each formula's normal behavior.
- **Observed:** Inverse maps the huge input to signed zero instead of `-1e-20`; for the tiny input
  it returns `-infinity` in X and NaN in the three zero components instead of finite `-1e20` and
  zeros. Divide likewise loses the huge quotient and produces nonfinite components for the tiny
  divisor. Each demo retains exactly two expected failures: eleven Inverse controls and24 Divide
  controls on ten runs per renderer label.
- **Expected/reference:** genuine Windows XNA 4.0 returns `(-1e-20,0,0,0)` and
  `(-1e20,0,0,0)` respectively for both Inverse and Identity/Divide, including their output forms.
  It agrees with CNA for ordinary nonunit values, identity behavior and zero inverse/divisor
  classifications.
- **Cause/suggested repair:** Both CNA implementations independently accumulate squared length in
  float. The huge square overflows, making the reciprocal zero; the tiny square underflows enough
  that its reciprocal overflows. A shared scaled norm or sufficient intermediate range should
  preserve finite reciprocal magnitudes in both APIs.
- **Evidence/status:** open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux; genuine XNA ran from the installed Windows
  XNA 4.0 assembly under Wine. Evidence:`/tmp/checkpoint-1037-quaternion-inverse-boundaries/*` and
  `/tmp/checkpoint-1042-quaternion-divide-boundaries/*`. Exact known-issue CTests, metadata693/0 and
  self-tests5/5 plus4/4 pass. No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-118 -- Vector and Quaternion magnitude operations lose extreme finite ranges

- **Area/API:** `Microsoft::Xna::Framework::Quaternion::Length`/`Normalize` and
  `Vector2`/`Vector3`/`Vector4` `Length`/`Distance`/`Normalize`, including
  static/member/output forms.
- **Demo/reproduction:** run `math.quaternion.normalize`, `math.vector2.length`,
  `math.vector3.normalize`, `math.vector3.distance` and `math.vector4.magnitude_range`. Measure
  sole-component `1e20` and `1e-30` values, normalize the same finite inputs and measure
  value/output Distance from zero. Ordinary magnitude/normalization, zero and separately rounded
  squared-magnitude controls distinguish the range defect from normal behavior.
- **Observed:** CNA returns infinity for every tested `Length(1e20)` path and zero for every
  `Length(1e-30)` path. Quaternion normalization loses the huge value. Every Vector2, Vector3 and
  Vector4 normalization form loses both extremes, while all three vector Distance paths likewise
  return infinity or zero. Quaternion retains three expected failures among eleven controls;
  Vector2 retains six among eighteen; Vector3 normalization retains four among fourteen and
  distance two among eight; Vector4 retains six among fourteen. The separately returned
  `LengthSquared` and `DistanceSquared` intentionally overflow to infinity or underflow to zero
  and match XNA.
- **Expected/reference:** genuine Windows XNA 4.0 returns the original finite magnitudes from
  Length and vector Distance even when their squared counterparts are infinity or zero. It
  normalizes each sole positive component to UnitX in every tested public form. XNA and CNA both
  return NaN components when normalizing zero.
- **Suggested repair/scope:** calculate square-root magnitudes with sufficient intermediate range
  or use a scaled norm before returning Length/Distance or a normalization reciprocal. The repeated
  direct float squared-sum implementations expose the same range defect across these value types.
  Open against pinned CNA `d23e139de9e5c15511b6be745de290896a69ee37` on Linux. Genuine XNA ran
  from the installed Windows XNA 4.0 assembly under Wine; no renderer or visual claim is involved.
- **Evidence/status:** `/tmp/checkpoint-1036-quaternion-normalize-boundaries/*`,
  `/tmp/checkpoint-1043-quaternion-length-range/*` and
  `/tmp/checkpoint-1046-vector2-magnitude-range/*` and
  `/tmp/checkpoint-1051-vector3-magnitude-range/*` and
  `/tmp/checkpoint-1057-vector4-magnitude-range/*`; exact known-issue CTests, metadata696/0 and
  self-tests5/5 plus4/4 pass. The earlier Quaternion exact-equality tiny assertion was a corrected
  over-strict demo oracle, not a second CNA issue. No CNA source was modified and no upstream
  message was sent.

### CNA-DEMOS-ISSUE-117 -- Perspective accepts Pi despite XNA's strict field-of-view bound

- **Area/API:** `Microsoft::Xna::Framework::Matrix::CreatePerspectiveFieldOfView`.
- **Demo/reproduction:** run `math.matrix.perspective`. Alongside valid equivalent-frustum and
  depth controls, call the value overload with `fieldOfView = MathHelper::Pi`, aspect1 and positive
  ordered near/far planes. Zero FOV and all depth-plane refusal controls remain positive controls.
- **Observed:** CNA accepts Pi and returns a matrix. This is the sole expected failure among twelve
  checks on ten SOFTWARE and ten OPENGLES3-labelled runs; stdout is stable and stderr empty.
- **Expected/reference:** genuine Windows XNA 4.0 throws `ArgumentOutOfRangeException` with
  `ParamName=fieldOfView` for both zero and exactly `MathHelper.Pi`. It accepts zero/negative
  aspect, zero width/height and collapsed off-centre spans as IEEE-754 inputs, and propagates NaN
  FOV; CNA matches all those adjacent distinctions and all near/far refusals.
- **Cause observed:** CNA checks `fieldOfView >= 3.141593f`. That rounded literal is greater than
  the public float `MathHelper::Pi`, leaving the exact documented/XNA upper endpoint accepted.
  Comparing against the public Pi value would close the observed one-float gap.
- **Evidence/status:** open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux. Genuine XNA ran from the installed Windows
  XNA 4.0 assembly under Wine. Evidence:`/tmp/checkpoint-1030-matrix-perspective-boundaries/*`.
  Exact known-issue CTest, metadata693/0 and self-tests5/5 plus4/4 pass. No CNA source was modified
  and no upstream message was sent.

### CNA-DEMOS-ISSUE-116 -- Singular affine Matrix inversion loses XNA's finite components

- **Area/API:** `Microsoft::Xna::Framework::Matrix::Invert`, value and by-reference overloads.
- **Demo/reproduction:** run `math.matrix.invert`. Invert
  `Scale(1,0,1) * Translation(7,8,9)` and inspect all sixteen public components. The demo also
  retains an ordinary round trip and adds zero-matrix and `1e-20` nonzero-scale controls.
- **Observed:** CNA returns `M22=+infinity` and `M42=-infinity`; the other fourteen values are NaN.
  This is the sole expected failure among six checks on ten SOFTWARE and ten OPENGLES3-labelled
  runs. The zero matrix produces sixteen NaNs and the tiny nonzero scale correctly yields `1e20`.
- **Expected/reference:** genuine Windows XNA 4.0 returns finite `M22=8` and `M42=-8` for the same
  singular affine input, with the other fourteen values NaN. Both overloads have the same
  component classification (ordinary equality is false because NaN is unequal to itself).
- **Investigation/scope:** CNA's XNA-rounding implementation keeps the reciprocal determinant as
  `double`; at this exact-zero singular boundary its later products yield infinities instead of
  XNA's two finite values. Open against pinned CNA
  `d23e139de9e5c15511b6be745de290896a69ee37` on Linux. The reference ran through the installed
  Windows XNA 4.0 assembly under Wine. No rendering or visual claim is involved.
- **Evidence/status:** `/tmp/checkpoint-1028-matrix-invert-xna/*`; exact known-issue CTest,
  metadata693/0 and tool self-tests5/5 plus4/4 pass. No CNA source was modified and no upstream
  message was sent.

### CNA-DEMOS-ISSUE-115 -- Matrix decomposition loses XNA reflection and zero-scale results

- **Area/API:** `Microsoft::Xna::Framework::Matrix::Decompose`.
- **Demo/reproduction:** run `math.matrix.decompose`. It retains the ordinary affine control and
  axis-aligned quarter turns, then decomposes `Scale(-2,3,4) * RotationY(0.7) * Translation(7,8,9)`
  and `Scale(1,0,1) * Translation(7,8,9)`. The reflected result is also recomposed.
- **Observed in CNA:** the reflection reports positive scale `(2,3,4)` and a rotation that cannot
  rebuild the source matrix (maximum component difference `3.5296845`). The flattened transform
  reports failure and identity rotation. These are exactly the demo's three expected failures on
  ten SOFTWARE and ten OPENGLES3-labelled runs; the other six controls pass and stderr is empty.
- **Expected/reference:** a genuine Windows XNA 4.0 probe reports success with scale `(2,3,-4)`
  for the reflection and recomposes it exactly. The flattened transform also reports success,
  preserves scale `(1,0,1)` and translation `(7,8,9)`, and returns a quaternion with four NaN
  components. The ordinary and all three quarter-turn controls agree.
- **Cause/scope:** CNA follows FNA's row-product sign heuristic, whose affine zero entries prevent
  it from retaining this reflection, and explicitly returns false when any extracted scale is
  near zero. This is an inherited compatibility difference, not a demo oracle error. Open against
  clean pinned CNA `d23e139de9e5c15511b6be745de290896a69ee37` on Linux. The XNA reference ran
  under Wine from the installed Windows XNA 4.0 assembly; no renderer or visual claim is involved.
- **Evidence/status:** `/tmp/checkpoint-1027-matrix-decompose-xna/*` contains the C# probe, assembly
  hash, XNA output, repeated CNA outputs and hashes. The exact known-issue CTest, metadata693/0 and
  both tool self-test suites pass. No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-114 -- Releasing a nonterminal SoundEffect loop drops its authored tail

- **Area/API:** `Microsoft::Xna::Framework::Audio::SoundEffectInstance::Stop(false)`, authored
  SoundEffect loop regions and the CNA mixer track end/loop state.
- **Demo/reproduction:** run `audio.sound_effect.loop_region` with dummy audio. Its generated mono
  PCM contains a 0.25 s intro, a 0.25 s loop region and a separate 0.75 s tail. Let the loop run
  beyond the complete 1.25 s buffer duration, pause it, call `Stop(false)`, resume and inspect state
  after 0.5 s. The remaining loop pass is at most 0.25 s, while the tail alone is 0.75 s.
- **Observed:** the instance is already Stopped at the half-second discriminator. Ten SOFTWARE and
  ten OPENGLES3-labelled runs reproduce exactly this one failed assertion; the other six controls
  pass and stderr is empty. Pause/release preserves the configured loop property, the released
  track reaches Stopped, immediate Stop still works after replay, and invalid loop coordinates are
  still refused.
- **Expected/contract:** `Stop(false)` exits the loop and allows the current sound to finish. FNA
  forwards it to `FAudioSourceVoice_ExitLoop`, so after the current loop pass playback continues
  from the authored loop end through the buffer tail. The instance must still be Playing after
  0.5 s and then stop naturally after the 0.75 s tail.
- **Cause observed in source:** CNA implements an authored loop end by setting the mixer track's
  `maxFrame` to `loopStart + loopLength`; the mixer treats that value as the whole pass/track end.
  `Stop(false)` only changes `loopsRemaining` to zero. The next read therefore ends at `maxFrame`
  and has no state that can expose the real audio end or enter the post-loop frames.
- **Demo correction:** the first expanded fixture reused the old exact-end invalid length, which
  became valid after adding the tail and caused a second failed assertion. Changing the refusal
  probe to one sample beyond the new complete buffer restored all three invalid-coordinate checks.
  This was a demo bug; the retained tail failure is independent and reproducible.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`, using SDL dummy audio on Linux. State and timing are
  observed; no audible sample-content, click-free transition, hardware-device or renderer claim is
  made. Evidence: `/tmp/checkpoint-1017-loop-tail-*`; the corrected unmarked demo and source audit
  SHA-256 values are `a9b78da5da33a8e934b2077a1e87bfba956febc6ce99e16f1390c2d73ba45ca9`
  and `555186469793313cdce5f5c501a3058e516227b4a09a43fc7cd66cf70da97232`.
  No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-113 -- A second OcclusionQuery Begin ignores the active-query contract

- **Area/API:** `Microsoft::Xna::Framework::Graphics::OcclusionQuery::Begin`, its shared public
  lifecycle state and the Software/EasyGL native query implementations.
- **Demo/reproduction:** run `graphics.occlusion_query.visible_pixels`. Create two queries on the
  same HiDef device, call `Begin` on the owner, then call `Begin` on the contender before ending the
  owner. Dispose the contender before ending and disposing the owner, so an incorrectly accepted
  native attempt cannot end or leak into the later visibility queries.
- **Observed:** the contender's `Begin` returns normally on both SOFTWARE and OPENGLES3. The demo's
  new assertion is the sole failure; its other12 controls pass, including safe cleanup, active-query
  disposal and completion of later hidden/open measurements. OPENGLES3 stderr remains empty even
  though the second GL begin targets an already active occlusion-query target.
- **Expected/contract:** the public header says `Begin` throws `System::InvalidOperationException`
  when a query is already active. The second object's call should be refused before its wrapper or
  native renderer enters a Begin/End pair, while the owner's pair remains usable.
- **Cause observed in source:** `OcclusionQuery` tracks `isInBeginEndPair_` only on each object and
  unconditionally records a successful begin after calling the renderer. The renderer method
  returns `void`, so Software's failed `TryActivateOcclusionQuery` result cannot reach the wrapper.
  EasyGL invokes the native begin before `try_emplace` leaves the existing per-context owner in its
  map; neither path raises the documented public exception.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`, reproduced on SOFTWARE and Mesa/X11/Xvfb OPENGLES3.
  No OPENGL33, physical-GPU, WebGPU or cross-device claim is made. Evidence:
  `/tmp/checkpoint-1012-occlusion-distinct-begin/*`; the unmarked demo and exact source audit have
  SHA-256 `4e8a962f85f555bacd6abf384fdc3ca37f906c44b82e73d55917ca54c9a2eced` and
  `4ce838aaffa7b5622e9401cdf260f13969347768123ba7e6ac6a8f5770848761`. No CNA source was modified
  and no upstream message was sent.

### CNA-DEMOS-ISSUE-112 -- Secondary EasyGL SetRenderTarget runs without its device context

- **Area/API:** `GraphicsDevice::SetRenderTarget`/`SetRenderTargets`, EasyGL renderer-context
  leasing and multiple simultaneously live `GraphicsDevice` instances.
- **Demo/reproduction:** run `graphics.render_target.render_target2d` with OPENGLES3. The game-owned
  primary device stays live while the demo creates a second 32x32 HiDef device and a 20x10
  `RenderTarget2D` owned by it. Outside a Game frame, call the secondary device's
  `SetRenderTarget`, clear LimeGreen, unbind and read the target's origin. The same sequence passes
  on SOFTWARE. Cross-device move controls then use explicit texture transfer so they remain
  independent positive controls.
- **Observed:** OPENGLES3 reads transparent black `{0,0,0,0}` both before and after the initial
  diagnostic variant moved the target; SOFTWARE reads exact LimeGreen. The final focused workflow
  fails only the pre-move secondary binding assertion. Its other16 controls pass, including
  cross-device owner/shape transfer, detachment of the old primary texture slot, transfer of the
  secondary public render-target binding, retained payload and post-unbind writes. Ten runs per
  renderer reproduce17/17 SOFTWARE passes and the same single OPENGLES3 failure with empty stderr.
- **Expected:** binding an owned target through a `GraphicsDevice` must execute against that
  device's renderer context. Its immediately following `Clear` already leases the correct context,
  so unbinding and readback should return the requested LimeGreen pixel just as they do for the
  primary device and SOFTWARE.
- **Cause observed in source:** `GraphicsDevice::Clear`, `Present` and `Reset` acquire a renderer
  thread-context lease. `GraphicsDevice::SetRenderTargets` calls
  `renderer_->SetRenderTargets(...)` directly, and EasyGL's `SetRenderTarget2D` also performs its
  framebuffer work without `EnsureCallingThreadContext` or a lease. Outside a frame there is no
  secondary-device frame lease to supply the missing current context, so the public binding state
  changes while the intended EasyGL framebuffer does not become the clear destination.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`, reproduced under Mesa/X11/Xvfb with OPENGLES3.
  No OPENGL33, physical-GPU, WebGPU or secondary-device presentation claim is made. This occurs
  before device destruction and is distinct from ISSUE-086's later primary-device blackout.
  Evidence: `/tmp/checkpoint-1010-render-target-cross-device/*`; the diagnostic source and context
  audit have SHA-256 `17130e03d200cfdcb0e7f3fcd7f3744f092241c58be934f71ff70551e694482e`
  and `7a2cfc151a43ead9fc54e24ff18133dfc76d095a00e38836da0c2fa6b2220464`.
  No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-111 -- Direct XNA XNB writer ignores its build environment platform and profile

- **Area/API:** `CNA::Content::Pipeline::RegisterXnaXnbOutput`, specifically the registered
  canonical writer's environment-aware `ContentTypeWriter::Write` overload.
- **Demo/reproduction:** `content.custom.xml_xnb_pipeline` creates a default XNB compiler, adds its
  existing `VitalWriter`, and directly calls `RegisterXnaXnbOutput` with container options that
  retain the default Windows/Reach target while requesting LZX compression. It resolves the
  resulting `CnaDemos.XmlVital` writer and calls the environment overload with Windows Phone,
  HiDef and a private output directory. The emitted XNB is loaded through `VitalReader` as a
  positive control.
- **Observed:** the XNB is valid, compressed and restores the exact value63, but its header reports
  `Windows/Reach`; the demo prints that exact pair and retains one labeled failed assertion. Ten
  serial SOFTWARE and ten serial OPENGLES3-labelled runs each pass26 controls and fail only this
  assertion plus established ISSUE-090. Their normalized stdout SHA-256 is
  `f043f692ac5ad687634516a2ecdbdb7ed2cf3d4fa142c8946f7cf5b0e6e8e695`; normalized stderr SHA-256
  is `d4de1e8a60e84e94cd4430995e7b9d160df8c9e3010e6bd39aa0fdc4cec57dad`.
- **Expected/contract:** the public `RegisterXnaXnbOutput` declaration says its registration
  options configure the container while platform and profile come from the build environment at
  write time. The environment overload should map `targetPlatform` and `targetProfile` into the
  compiler request, while retaining registration-time compression, reader-name and limit options.
- **Cause observed in source:** `XnaObjectXnbWriter::Write(..., environment)` forwards only
  `environment.outputDirectory`. It chooses `CompileOptions::targetPlatform` from
  `options_.platform` and `targetProfile` from `options_.graphicsProfile`. This also contradicts
  the implementation comment that calls platform/profile build properties.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The same demo proves complete direct writer
  registration, collision preservation, idempotence, stable identity/schema, compression and
  runtime loading. Its end-to-end CLI path remains correct because the registry factory receives
  command-line platform/profile in the registration options. No CNA source was modified, no
  upstream message was sent and no genuine-XNA, GPU or visual behavior is claimed. Evidence:
  `/tmp/checkpoint-994-authoritative-*`.
- **Additional evidence:** Focused995 adds five passing `RegisterXnaXmlSourceContentPipeline`
  controls around the same writer. The current demo passes31/33 controls, retaining only
  ISSUE-090/111. Ten serial runs per renderer label share normalized stdout SHA-256
  `2be8ac174e57aa7e88203f9a6fc49adcb3064b25343b3cffc24b3e8e71b944d7` and stderr SHA-256
  `df194fd20910f4cf1206a8a06e8a4429fa6194cb7ada323069f3b791395e6d89`.

### CNA-DEMOS-ISSUE-110 -- Direct uncompressed-chunk helper bypasses its size contract

- **Area/API:** `CNA::Content::Cnb::DecompressCnbChunk` with
  `CnbCompression::None`.
- **Demo/reproduction:** run `content.custom.cnb_read_limits`. The passing diagnostic supplies
  4096 stored bytes, declares 4097 uncompressed bytes, sets the maximum uncompressed size to one
  byte, and calls the public helper directly.
- **Observed:** the helper returns a 4096-byte copy successfully. Its `None` branch returns before
  comparing the declared size with the stored size or checking the configured ceiling.
- **Expected/contract:** the public header says the input must expand to exactly
  `uncompressedSize`, that this attacker-controlled value is checked against
  `maxUncompressedSize` before allocation, and that either violation throws
  `ContentLoadException`. Those requirements should apply to the no-compression path too.
- **Scope/status:** `CnbDocument` first enforces equal stored/logical sizes and all configured
  limits while validating its table of contents, and it does not call the helper for uncompressed
  chunks. Ordinary container parsing therefore remains bounded. A direct caller can nevertheless
  get a second copy larger than its stated ceiling, contrary to the public helper contract. The
  exact header/implementation excerpt is
  `/tmp/checkpoint-979-cnb-infrastructure/decompress-contract-and-source.txt`, SHA-256
  `994a2ae3717617eb1f5b8c101fada0e70f70dad5e06d734028a44951b5f74615`. Open against clean
  isolated CNA `d23e139de9e5c15511b6be745de290896a69ee37`; no CNA source was modified and no upstream
  message was sent. The issue is a passing diagnostic rather than a runtime `knownIssue` marker.

### CNA-DEMOS-ISSUE-109 -- Model encoder emits values its paired decoder refuses

- **Area/API:** `CNA::Content::Cnb::EncodeModelToCnb` and `DecodeModelFromCnb`, schema 1.
- **Demo/reproduction:** run `content.model.pipeline_contract`. The demo first round-trips a
  complete valid `CnbModelData`, then makes five independent copies and changes exactly one public
  field in each: bone parent `-2`, mesh parent `-2`, a 16-bit index equal to `vertexCount`, material
  `alphaMode` 3, or texture-coordinate set 2. For every copy it calls the public encoder, parses the
  returned container successfully, and passes it to the public decoder.
- **Observed:** `EncodeModelToCnb` returns bytes for all five inconsistent values.
  `CnbDocument::Parse` accepts each structurally valid container, then `DecodeModelFromCnb` refuses
  it under its documented schema checks. The same encoder separately rejects later/self bone
  parents, byte-count disagreement, an incorrect derived primitive count, malformed morph stream,
  out-of-range mesh part and mismatched skeleton arrays.
- **Expected/contract:** the encoder documents `ContentLoadException` for an internally
  inconsistent model and explicitly names an out-of-range index. A successful result from the
  schema encoder should be accepted by the paired schema decoder under the same default limits.
  Validate negative parent sentinels, decoded index bounds and the decoder's material ordinals
  before returning bytes, while preserving the existing positive relationship checks.
- **Cause observed in source:** bone and mesh parent checks only enter their range branches for
  nonnegative values; the encoder validates index byte length but not decoded index values; and
  `WriteMaterial` serializes `alphaMode` and coordinate sets without applying the bounds enforced
  by `ReadMaterial`. These are five instances of one producer/consumer validation asymmetry.
- **Evidence/status:** the complete source/contract excerpt is
  `/tmp/checkpoint-972-model/model-codec-contract-and-validation.txt`, SHA-256
  `3824b21b5fa5eb1a28a706a1c632adca97590a287be367e07dd35e43cef2de68`. Ten SOFTWARE and ten
  OPENGLES3-labelled direct runs pass all 25 demo controls with byte-identical stdout SHA-256
  `ba4f54743fcdbe157d5bb52875cc7bc7cbc100e2cf18222504f5d214b0794c8b` and empty stderr. The issue
  is recorded as a passing diagnostic control rather than an expected demo failure. Open against
  clean isolated CNA `d23e139de9e5c15511b6be745de290896a69ee37`; no CNA source was modified,
  no upstream message was sent, and no malformed file from outside the public encoder is needed.

### CNA-DEMOS-ISSUE-108 -- Character-region expansion silently ignores an inverted range

- **Area/API:** `CNA::Content::Pipeline::ExpandCharacterRegions`, declared in
  `SpriteFontContentPipeline.hpp`.
- **Demo/reproduction:** run `content.sprite_font.pipeline_contract`. The standalone build-time
  demo parses a valid `.spritefont`, then copies the public `FontDescription` value and replaces
  its regions with the directly constructible `FontCharacterRegion{u'Z', u'A'}`, clears the
  unrelated fallback character, and calls `ExpandCharacterRegions`.
- **Observed:** expansion returns an empty vector. Its unsigned loop starts at `Z`, tests it
  against the smaller `A`, and performs no iteration. The XML parser rejects the same ordering,
  but the public expansion function accepts a `FontDescription` assembled by any caller.
- **Expected/contract:** the public declaration explicitly documents a `std::runtime_error` for
  an inverted region. The function should validate every public input region before expanding it,
  independent of whether that value originated in the built-in XML parser.
- **Scope/status:** the passing demo labels and asserts the exact current behavior so the rest of
  the 44-declaration source-pipeline audit remains executable; its metadata names this issue rather
  than hiding the discrepancy. Malformed XML and a character set above the 8192-glyph limit are
  separately refused as documented. Open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`; no CNA source was modified, no upstream message was
  sent, and no genuine-XNA behavior is claimed for direct construction of CNA's extension type.

### CNA-DEMOS-ISSUE-107 -- XNB output digest documentation gives an impossible example

- **Area/API:** `CNA::Content::Pipeline::XnbOutputOptionsDigest`, declared in
  `XnbOutputContentPipeline.hpp`.
- **Demo/reproduction:** run `content.xnb.output_pipeline`. The public function returns `w5run`
  for the documented defaults and `d5h4p` for DesktopGL/version5/HiDef/LZ4/portable names. The
  latter digest also appears in the resolved writer identity and drives an actual deterministic
  container whose header is `d`, version5 and flags `0x41`.
- **Observed:** the header documents a human-readable result “such as `w5rn-x`”. The implementation
  always appends exactly five fields with no separators: one platform byte, one decimal version,
  one profile byte (`r`/`h`), one compression byte (`u`/`x`/`4`) and one reader-style byte
  (`n`/`p`). It therefore cannot produce the documented six-character hyphenated example.
- **Expected:** the example should use a value the public function can return, such as default
  `w5run`, or explicitly document the five field positions without an example that suggests a
  different grammar.
- **Cause/source evidence:** the implementation in `XnbOutputContentPipeline.cpp` appends the five
  values directly and the same digest is incorporated into both writer and codec versions. This
  is a stale public comment, not a byte-output or incremental-invalidation defect.
- **Scope/status:** documentation-only, so the demo remains a passing workflow and has no runtime
  `knownIssue` marker. Open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`; no CNA source was modified and no upstream message
  was sent.

### CNA-DEMOS-ISSUE-106 -- Pre-built content compiler reports every service option as an FX option

- **Area/API:** `CNA::Content::Pipeline::ContentCompilerOptions` and the pre-built-registry
  `RunContentCompiler` overload.
- **Demo/reproduction:** run `content.custom.incremental_compiler`. It first supplies a configured
  registry with `--fx-compiler`, then repeats the same build with only
  `--xma-encoder unused-encoder`. Both calls are captured before source discovery and enter no
  importer, processor or writer.
- **Observed:** both correctly return command-line status 2 and direct the caller to the
  `ContentPipelineRegistryFactory` overload. The XMA-only call nevertheless says only that
  `--fx-compiler and --fx-compiler-launcher` select a build-time service and never names XMA.
  Source review shows the same hard-coded diagnostic is also used when `fontDirectories` or
  `buildConfiguration` make the options non-default.
- **Expected:** the refusal should name the supplied option, or generically name every
  registry-affecting service/configuration option. A caller selecting an XMA encoder, font search
  directory or build configuration should not be told that they selected an FX compiler.
- **Cause observed in source:** the overload compares `ContentCompilerOptions` with its defaults.
  Its equality operator intentionally includes effect and XMA executable/launcher settings, XMA
  arguments, font directories and build configuration while excluding coordinator-owned XNB
  container settings. The failure text was written only for the original two FX fields and was
  not broadened with the public options structure.
- **Scope/status:** the refusal itself is correct and prevents silent option loss; this is a
  user-facing diagnostic/documentation defect, so the demo records it in a passing observation
  rather than becoming an expected-failure demo. Open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. Relevant public contract and implementation are in
  `/tmp/checkpoint-960-compiler-options-source.txt`, SHA-256
  `cc2385482db71984b5d78ef6171bb06381a59ce6b7537eab442364e30d258c15`; direct stderr is retained
  in `/tmp/checkpoint-960-compiler-options.err`. No CNA source was modified and no upstream
  message was sent.

### CNA-DEMOS-ISSUE-105 -- A sole named-only importer becomes the convention route

- **Area/API:** `CNA::Content::Pipeline::ContentImporter::SelectedByNameOnly` and
  `ContentPipelineRegistry::ResolveImporter`.
- **Demo/reproduction:** run `content.sound_effect.pipeline_components`. Register the built-in
  SoundEffect pipeline with a compressed decoder, then resolve an `.mp3` without an explicit
  importer name. `CompressedSoundImporter` is the only component declaring that extension and its
  public `SelectedByNameOnly()` result is true. The demo separately proves that the same component
  resolves and builds successfully when explicitly named.
- **Observed:** default resolution returns `CNA.CompressedSoundImporter` instead of reporting that
  no ordinary importer is available. Fourteen other controls pass, including the descriptor
  contract, the complete real-WAV route, deterministic CNB output, contextual parameter failure,
  explicit compressed callback success, and missing/unsuccessful decoder failures. The final
  named-only exclusion control fails under the renderer-independent pipeline code.
- **Expected/contract:** the base-class documentation says a named-only importer is left out of
  default resolution, still runs when a build names it, and never becomes the answer to which
  importer reads an extension on its own. `CompressedSoundImporter` documents the same rule so a
  convention build of compressed audio remains on its ordinary song route.
- **Cause observed in source:** `ResolveImporter` filters `SelectedByNameOnly()` candidates only
  when more than one importer declares the extension. With exactly one candidate it forwards an
  empty explicit name to the generic route resolver, which selects that sole candidate without
  consulting `SelectedByNameOnly()`. The equivalent processor resolver has the same conditional
  shape and should be audited with the repair.
- **Suggested repair:** always form the eligible default candidate set by excluding named-only
  components, then require exactly one eligible candidate. Preserve explicit-name resolution.
- **Evidence/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. Contract and implementation excerpts are
  `/tmp/checkpoint-951-named-only-importer-source.txt`, SHA-256
  `eac283ca9615ae578d490836551e7bb8a768c0679bed6de952db2863339bc38d`. No CNA source was
  modified, no upstream message was sent, and the demo names this issue as an expected failure.

### CNA-DEMOS-ISSUE-104 -- STUB does not provide the promised WarnAndStub Texture3D null object

- **Area/API:** `CNA::Unsupported3DGraphicsCallBehavior`,
  `GraphicsDevice::{Set,Get}Unsupported3DGraphicsCallBehavior`, and `Texture3D` on the STUB
  renderer.
- **Demo/reproduction:** run `graphics.device.supports_capability --automated --renderer STUB`.
  The fresh HiDef device reports `GraphicsCapability::ThreeD == false` and the default `Throw`
  policy correctly refuses a valid 1x1x1 volume. Select `WarnAndStub`, create two otherwise valid
  1x1x1 Color volumes, upload one red texel to the first, read it back, and capture CNA warnings.
- **Observed:** policy set/get, the unchanged capability answer and argument validation all work.
  Both volume constructors return, but the first `SetData` throws `System::NotSupportedException`:
  `Texture3D::SetDataPointerEXT: this graphics renderer creates no volume texture resource`.
  No RENDER warning naming `CreateTexture3D` is emitted. The demo therefore has five passing and
  two failing controls on STUB, reproduced byte-for-byte in ten runs with stdout SHA-256
  `dce912aeca46cd7c617f6b234398a44f15fb60cf4d109ce1b62707e52a08bde9` and empty stderr.
- **Expected/contract:** the public policy says `WarnAndStub` logs one warning per unsupported
  renderer operation and returns usable no-op resources whose later SetData/GetData calls remain
  safe. Two creations of the same unsupported operation should produce one RENDER warning; the
  null object should accept the upload and return deterministic zero/transparent data.
- **Cause observed in source:** `Texture3D` deliberately bypasses its capability refusal under
  `WarnAndStub`, then asks the renderer for storage. STUB does not override `CreateTexture3D`, so
  it inherits `IGraphicsRenderer`'s unconditional `nullptr` result. That path neither invokes
  `HandleUnsupported3DCall` nor constructs `NoOpTexture3DRenderer`; the public wrapper survives
  construction with a null renderer and refuses the first transfer.
- **Controls/scope:** ten SOFTWARE and ten OPENGLES3/Xvfb runs pass all five applicable controls
  with stable stdout hashes `e0cef59accec93d66bbc0938db50c781d849da30b493dab8df410103ec17ecd2`
  and `de1d45804b6b0eabfa8d56e97ecce3dc37143eee6e74b6d389ce7d84775355c7`,
  respectively, and empty stderr. This runtime claim is limited to Texture3D on STUB; other
  unsupported factories and draws require their own audit.
- **Evidence/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. Public contract and implementation excerpts are
  `/tmp/checkpoint-944-unsupported3d/source.txt`, SHA-256
  `42afcf412220b921f89ce61ce56f7bce194542bd66f94d1c71911f62e48d8c91`.
  No CNA source was modified, no workaround was added, and no upstream message or visual-quality
  claim was made.

### CNA-DEMOS-ISSUE-103 -- C content loaders discard canonical `ContentLoadException` identity

- **Area/API:** the C exception-detail contract in `CNA/C/core.h` and the typed
  `cna_content_manager_load_*` routes, observed through
  `cna_content_manager_load_texture2d`.
- **Demo/reproduction:** `c_api.content.content_manager` changes its live manager root from
  `assets` to `assets/textures`, unloads the cached `textures/checker` value, then requests that
  logical name again. The native `ContentManager::Load<Texture2D>` path cannot open the resulting
  location and throws its ordinary `Microsoft::Xna::Framework::Content::ContentLoadException`.
  Query `cna_error_get_last_info`, `cna_error_get_last_exception_type_size_ext` and the matching
  copy route immediately after the failed load.
- **Observed:** the route correctly returns `CNA_RESULT_IO`; the thread diagnostic keeps result IO,
  category IO and its nonempty message, but reports zero exception-type bytes. Both SOFTWARE and
  OPENGLES3/Xvfb print `missing load exception type: <none>` and fail only the exact canonical-type
  check among54 controls.
- **Expected/contract:** `core.h` says the exception-type query names the canonical exception behind
  the most recent failure so a managed binding can rethrow it, and explicitly uses
  `Microsoft.Xna.Framework.Content.ContentLoadException` as an example. This failure came from that
  native exception rather than a C-layer argument or handle check, so the type must survive as that
  exact name. It carries no argument parameter.
- **Cause observed in source:** `ContentManagerLoadTexture2d` catches `ContentLoadException` inside
  the route body and calls `IoError(exception.what())`. The outer `CallWithExceptionBarrier` sees a
  returned result rather than the exception, so `NoteTranslatedException` never records its dynamic
  type. The same catch-and-`IoError` pattern appears in the sound-effect, cube, font, Effect and
  Model content loaders; only Texture2D is claimed by runtime evidence here.
- **Suggested repair:** preserve the caught canonical exception metadata when translating the
  intended IO result, or let the common exception barrier perform that translation. Keep the
  existing result/category/message and all output-handle clearing contracts.
- **Evidence/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. Ten SOFTWARE and ten OPENGLES3/Xvfb runs reproduce
  exactly one failure and53 passes with byte-identical stdout SHA-256
  `960cf6f99b002990d67441e387e954a8eeb6a79249356308e9183aabc1edb348` and empty stderr. The exact
  known-issue CTest passes ten repetitions. Contract/source excerpts are
  `/tmp/checkpoint-925-content-exception-source.txt`, SHA-256
  `97cff773d99c041cb9fc457399f1451b781b61358ebf43beb7346013da26ca94`. No CNA source was modified
  and no upstream message or manual visual-quality claim was made.

### CNA-DEMOS-ISSUE-102 -- Oversized shadow cascade count overruns fixed draw-parameter arrays

- **Area/API:** `ShadowCascadeStateEXT::Count` and `setShadowCascadesEXT` on `BasicEffect`,
  `SkinnedEffect`, `PbrEffect` and `SkinnedPbrEffect`.
- **Discovery:** the Focused918 audit first completes safe maximum-count rendering with Count4 and
  all four selected slots. Source review then follows the same public value into each effect's draw
  parameter construction. The undefined path is deliberately not invoked by a demo.
- **Observed source behavior:** all four setters copy an arbitrary `Count` unchanged. When shadows
  are enabled and Count is positive, every effect loops `c < Count`, reads
  `WorldToAtlas[c]`/`SplitDistance[c]` and writes `cascadeMatricesColMajor[c * 16 + i]` plus
  `cascadeSplits[c]`. Both public source arrays and renderer destination arrays have capacity four.
  The EasyGL, SDL-GPU and WebGPU backends clamp the count to four only after this copy, so Count5
  has already performed out-of-bounds reads and writes.
- **Contract:** `ShadowCascadeStateEXT::kMaxCascades` is explicitly the largest receiver count and
  both arrays use that bound. The public comments do not grant undefined behavior for an oversized
  Count. A public value crossing the boundary must not corrupt memory.
- **Expected repair:** validate or clamp Count before any indexed copy, consistently in the shared
  effect path; preferably make the setter reject values outside `0..kMaxCascades` and preserve the
  preceding valid state. Add unit coverage for every effect family so the backend's later clamp
  cannot mask a regression.
- **Evidence/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The collected public type, four effect and fixed
  renderer-structure excerpts are `/tmp/checkpoint-918-shadow-count-source.txt`, SHA-256
  `93baa280e929bff3a999e8eb90556ea7c05915e075a90f064523e932e1709346`. No CNA source was modified,
  no unsafe runtime call was made and no upstream message was sent. This source-diagnosed memory-
  safety issue adds no runtime knownIssue marker.

### CNA-DEMOS-ISSUE-101 -- Video URI resolution and deferred file failures diverge from FNA

- **Area/API:** `Video::FromUriEXT`, the trusted-metadata Video constructor and
  `VideoPlayer::Play`.
- **Demo:** `media.video.uri_and_metadata`.
- **Reproduction:** use the generated local `track_matrix.mkv` through a plain absolute path, its
  absolute `file:///` URI and a title-relative URI while the process current directory points
  elsewhere. Also pass a non-file HTTPS URI. Finally construct a Video with trusted metadata and a
  nonexistent local path, then call Play. Exception checks accept only the exact System type.
- **Observed:** the plain absolute path succeeds. CNA treats the `file:///` and HTTPS strings as
  literal filesystem paths and throws FileNotFoundException; the relative string is resolved below
  the process current directory instead of TitleLocation and also fails. Trusted construction
  correctly does no I/O, but Play silently returns in Stopped/null-Video state when decoder open
  fails rather than reporting the missing file. A separate real-file metadata mismatch does throw
  InvalidOperationException and closes atomically as documented.
- **Expected/reference:** FNA's `Video.FromUriEXT(Uri, GraphicsDevice)` converts an absolute local
  file URI to `LocalPath`, combines a relative URI with `TitleLocation.Path`, and rejects a
  non-file absolute URI with InvalidOperationException. Its trusted/XNB constructor deliberately
  postpones file access, while codec Play throws FileNotFoundException when opening the referenced
  file fails. See FNA's public
  [`Video.cs`](https://github.com/FNA-XNA/FNA/blob/master/src/Media/Video/Video.cs) and
  [`VideoPlayerTheora.cs`](https://github.com/FNA-XNA/FNA/blob/master/src/Media/Video/VideoPlayerTheora.cs).
- **Cause observed in source:** CNA changes the factory parameter from Uri to `std::string` and
  forwards it unchanged to the raw-file constructor; no URI parsing or TitleLocation combination
  occurs. `VideoPlayer::OpenDecoder` clears its decoder and Video pointer when `Open` returns false,
  then `Play` treats the null decoder as an ordinary early return.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction is local and deterministic; the
  HTTPS spelling is classified without making a network request. No CNA source was modified and no
  upstream message was sent.

### CNA-DEMOS-ISSUE-100 -- Detached Video track preferences are ignored by VideoPlayer::Play

- **Area/API:** `Video::SetVideoTrackEXT`, the symmetric `Video::SetAudioTrackEXT`, and
  `VideoPlayer::Play`.
- **Demo:** `media.video_player.track_selection`.
- **Reproduction:** create a raw Video over the generated `track_matrix.mkv`, whose two conforming
  160x90 YUV420 video streams are solid red and blue and whose two mono audio streams share a
  48 kHz wave format. Call both Video-level selectors with index one before attaching that Video to
  a fresh VideoPlayer, then Play and read the first frame through public Texture2D data.
- **Observed:** the first frame is red, from default video stream zero, rather than blue stream one.
  The same fixture proves that player-level preselection before Play does select blue stream one,
  and that calling the Video setters while attached forwards without stopping playback. The
  audio preference has the same disconnected storage path in source, but the demo deliberately
  claims no audible or sample-rate observation.
- **Expected/reference:** FNA's official extension contract defines these methods on Video, says
  they select tracks in a multi-track file, and specifically recommends calling them before
  `VideoPlayer.Play` for a clean selection. The fixture obeys its stated same-format constraints.
  See [FNA Extensions: SetAudioTrackEXT, SetVideoTrackEXT](https://fna-xna.github.io/docs/5%3A-FNA-Extensions/#setaudiotrackext-setvideotrackext).
- **Cause observed in source:** each Video setter stores its index in `audioTrack_` or
  `videoTrack_` and forwards only when `parent_` is already non-null. `VideoPlayer::OpenDecoder`
  applies only the player's separate fields and never reads the Video's stored preferences, so
  detached selections cannot reach the decoder.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. Runtime evidence is exact public pixel data for the
  video selector; source inspection establishes the symmetric disconnected audio field without
  upgrading it to runtime proof. No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-099 -- VideoPlayer validation and lifecycle contracts diverge from XNA

- **Area/API:** `VideoPlayer::Play`, `GetTexture`, `Volume`, `Stop`, `Dispose`, `State`,
  `PlayPosition`, `IsLooped`, `IsMuted` and their post-disposal behavior.
- **Demo:** `media.video_player.pause_and_loop`; its singular `knownIssue` field names this issue.
- **Reproduction:** on a fresh player, call `GetTexture`, pass a null `Video` to `Play`, and assign
  `Volume` values below zero and above one. Play a real compiled video, stop a second player, and
  compare its `Video` identity with the object it played. Dispose the active first player, compare
  its last `Video`, then read `State` and `PlayPosition` and assign `IsLooped`, `IsMuted` and
  `Volume`. Every exception check accepts only the exact documented System exception type.
- **Observed:** pinned CNA returns null from `GetTexture`, silently ignores `Play(nullptr)`, and
  clamps the two invalid volumes to zero and one. `Stop` and `Dispose` clear the observable Video
  pointer. After disposal, both state getters and all three configuration setters remain callable.
  The focused demo therefore fails six grouped contract checks while its 21 controls pass,
  including real decode, pause/resume/loop transport, monotonic frame generation, raw-Video
  metadata and safe C++ destruction of a Video that still has a dependent player.
- **Expected/reference:** Windows XNA 4.0 throws `InvalidOperationException` from `GetTexture`
  without an active video, `ArgumentNullException("video")` from `Play(nullptr)`, and
  `ArgumentOutOfRangeException("value")` without mutating Volume for values outside `[0,1]`.
  `Stop` and `Dispose` retain `activeVideo`. `State` and `PlayPosition` getters and the setters of
  `IsLooped`, `IsMuted` and `Volume` call `ThrowIfDisposed`; their three getters and `Video` remain
  readable. This was checked directly in the installed Windows
  `Microsoft.Xna.Framework.Video.dll`, SHA-256
  `17538b1ca9d48a993e2cd88c96b436df08e7abb4aec5d4758eb21feb580d6e06`.
- **Cause observed in source:** CNA has explicit early returns for null `Play` and no-active-video
  `GetTexture`, clamps Volume with `std::clamp`, and `CloseDecoder` unconditionally nulls `video_`.
  The two state getters and three configuration setters do not call `CheckDisposed`. The guarded
  transport, texture and CNAEXT track-selection methods correctly throw after disposal.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction uses public state and exact
  exception types. Frame pixels are drawn through the established path, but no screenshot, visual
  quality or audible result is claimed. No CNA source was modified and no upstream message was
  sent.

### CNA-DEMOS-ISSUE-098 -- Disposed media library types do not enforce their access guards

- **Area/API:** `Picture`, `PictureAlbum`, `Song`, `Album`, `Artist`, `Genre`, `Playlist`, all seven
  media collection types and `MediaLibrary`; their ordinary post-disposal operations.
- **Demos:** `media.media_library.save_picture`, `media.media_library.music_index` and
  `media.song.properties`. Their singular `knownIssue` fields name this issue; the pinned
  save-picture baseline also retains ISSUE-041.
- **Reproduction:** create and cross-index a real PNG in verified private Linux/XDG folders, retain
  live peer objects for comparisons, then dispose one Picture and its containing PictureAlbum
  twice. First prove the flags are set idempotently and sibling objects remain live. Invoke twelve
  Picture operations (five properties, both stream methods, both Equals overloads, equality,
  GetHashCode and ToString) and nine corresponding PictureAlbum operations (four properties, both
  Equals overloads, equality, GetHashCode and ToString), accepting only exact
  `System::ObjectDisposedException`.
- **Music-entity follow-up:** `music_index` performs the parallel operation after first proving
  repeated disposal and sibling isolation. It checks 13 Album operations (six properties, both
  no-art image methods, both Equals overloads, equality, hash and text) plus eight operations each
  on Artist, Genre and Playlist (three properties, both Equals overloads, equality, hash and text).
  Live mirror objects make every comparison valid. The image calls specifically establish that
  disposal must take precedence over the otherwise expected missing-art InvalidOperationException.
- **Collection/library follow-up:** dispose the populated Song, Album, Artist, Genre and Playlist
  collections twice, retain independent entity pointers, and require exact ObjectDisposedException
  from Count and Item on each collection. Separately dispose a fully indexed MediaLibrary twice,
  prove its previously retained collection and entity views remain live, then invoke all nine
  library properties, GetPictureFromToken and both SavePicture overloads. The picture operations
  use one-byte inputs and the already verified private XDG root so an unguarded implementation has
  deterministic, isolated side effects.
- **Remaining-family follow-up:** dispose a caller-created Song twice while retaining two live peer
  Songs, then invoke all ten ordinary properties plus both Equals overloads, both operators, hash
  and text. The picture workflow similarly disposes populated PictureCollection and
  PictureAlbumCollection views twice, retains their borrowed entries, then invokes Count and Item.
- **Observed:** both renderer-labelled runs report `Picture 0/12, PictureAlbum 0/9`; every grouped
  refusal assertion fails. The other 31 of 34 controls pass except the separately established
  ISSUE-041 short-read assertion, for three total failures. Streams returned by the disposed
  Picture remain caller-owned and are deleted by the probe; no invalid pointer use is involved.
- **Music-entity observation:** both labels report `Album 0/13, Artist 0/8, Genre 0/8, Playlist
  0/8`. Album image calls yield the live object's missing-art path rather than the disposal refusal.
- **Collection/library observation:** all five collections report `0/2`. Count remains readable as
  zero after CNA clears the view, while Item throws ArgumentOutOfRangeException rather than the
  required disposal exception. MediaLibrary reports `0/12`: every property remains readable,
  token lookup executes and both picture saves complete. The retained child graph correctly stays
  live. Together with the four entity groups, `music_index` has 29 passing controls and ten grouped
  ISSUE-098 failures without changing its stable ID.
- **Remaining-family observation:** Song reports `0/16`; every property and value operation remains
  usable. Both picture collections report `0/2`: Count returns zero and Item takes the wrong bounds
  path. The save-picture workflow keeps 31 passing controls and now has four grouped ISSUE-098
  failures in addition to the separate ISSUE-041 failure.
- **Expected/reference:** Windows XNA 4.0 checks disposal before Count, Item and GetEnumerator on
  every collection, before all nine MediaLibrary properties and before each of its three picture
  methods. All seven entity types' ordinary members have the same rule. Only IsDisposed itself
  remains queryable; repeated Dispose stays safe, and disposing the library must not cascade into
  previously obtained child views. This was checked directly in the installed Windows assembly
  whose SHA-256 is
  `38e7093f52d7474bbc6256906519781a1210d7da50a1c667b52716fcf49ca130`.
- **Cause observed in source:** all seven CNA entities and MediaLibrary only set `isDisposed_ = true`;
  the seven collections clear their pointer arrays and set the flag. None of their ordinary methods
  checks that flag. The non-owning implementation explains the retained child lifetime but does
  not satisfy the public post-disposal operation contract.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`; the relevant implementation files are unchanged at
  the later sibling checkout observed during this audit. All three demos' renderer streams are
  byte-identical; each exact known-issue SOFTWARE CTest passes20 consecutive runs, and all four
  checkpoints have clean full incremental builds and standard validators. Evidence:
  `/tmp/checkpoint-881-*`, `/tmp/checkpoint-884-*`, `/tmp/checkpoint-885-*` and
  `/tmp/checkpoint-886-*`. No CNA source was modified and no upstream message was sent. This is
  exact exception/state/stream-lifetime evidence; no screenshot or visual judgment is involved.

### CNA-DEMOS-ISSUE-097 -- Nearby CurveKey replacement can violate collection ordering

- **Area/API:** `CurveKeyCollection::setItemProperty`, the writable `Item` property and the
  collection's ascending-position invariant.
- **Reproduction:** `math.curve.key_collection` adds keys at positions zero and one half of
  `MathHelper::MachineEpsilonFloat`, then replaces index zero with a key at three quarters of that
  epsilon. The replacement position is greater than the following key, while remaining within
  one machine epsilon of the old position.
- **Observed:** the setter leaves the replacement at index zero. The resulting positions are
  descending, with `0.75 * epsilon` before `0.5 * epsilon`. SOFTWARE and OPENGLES3-labelled runs
  each pass the other twenty controls and fail only the ascending-order invariant. Ordinary
  duplicate insertion is stable, exact-position replacement stays in place, a larger position
  change is reinserted correctly, and invalid indices throw without mutation.
- **Cause observed in source:** `setItemProperty` compares the old and replacement positions with
  `MathHelper::WithinEpsilon`. It assigns in place when that comparison succeeds and performs the
  sorted remove-and-`Add` path otherwise. The in-place branch does not compare the new position to
  either neighbour. This can contradict the public header's explicit promise to preserve position
  ordering.
- **Expected:** every successful indexed replacement must leave positions in ascending order.
  Exact-position replacement may remain in place; any changed position should be inserted through
  the same sorted path as `Add`, or otherwise checked against both neighbours.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction is renderer-independent and uses
  public collection values only. Evidence:`/tmp/checkpoint-860-curve-key-collection-*`. No CNA
  source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-096 -- Partial SkinningData root-prefix arrays are silently ignored

- **Area/API:** `SkinningData::SkeletonRootPrefix` and `AnimationPlayer` construction/update.
- **Reproduction:** `graphics.model.animation_player_validation` constructs a two-bone skeleton
  whose complete two-entry prefix places its root at X=4 and whose allowed empty prefix places it
  at X=1. It then supplies a nonempty one-entry prefix with `BoneCount == 2`, both to a new player
  and by mutating the borrowed SkinningData before an existing player's next `Update`.
- **Observed:** neither operation throws. The one supplied prefix is discarded and the entire
  skeleton uses identity scene context. The demo's other nine controls pass: required array-size
  and parent-order errors are refused, complete/empty prefix behavior is exact, mutations of the
  borrowed required arrays are revalidated, restored data recovers, and edits through the borrowed
  clip reference affect and restore the sampled pose. SOFTWARE and OPENGLES3 each fail only the
  combined partial-prefix invariant.
- **Cause observed in source:** `AnimationPlayer::RecomputeTransforms` validates
  `SkeletonHierarchy`, `BindPose` and `InverseBindPose` against `BoneCount`, but not
  `SkeletonRootPrefix`. It computes `hasPrefix` as exact size equality and otherwise takes the
  all-identity branch. This correctly handles the documented empty form but gives every other
  malformed size the same silent meaning.
- **Expected:** `SkeletonRootPrefix` may be empty, meaning all identity, or contain exactly
  `BoneCount` entries. A nonempty mismatched array must raise `ArgumentException` during
  construction and any later recomputation, like the other parallel skeleton arrays, rather than
  silently moving the skeleton into another coordinate space.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. This renderer-independent reproduction runs under
  both renderer selections and uses only public numeric transform arrays; no drawing, screenshot
  or visual judgment is involved. Evidence:`/tmp/checkpoint-856-animation-player-validation-*`.
  No CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-095 -- Multi-skin glTF channels retained by another palette are reported lost

- **Area/API:** loose glTF `ContentManager::Load<Model>`, multiple `ModelSkinEXT` palettes,
  `SkinningData::AnimationClips` and aggregate `GltfImportReportEXT` animation diagnostics.
- **Reproduction:** `graphics.model.multi_skin_animation_report` loads an original two-skin glTF
  with one `SplitMotion` source animation. One translation channel targets `LeftRig`'s only joint
  and the other targets `RightRig`'s only joint. Both named SkinningData objects retain a one-track
  JointPalette clip, and two AnimationPlayers reach the authored final Y translations +0.2 and
  -0.3. The report correctly distinguishes one source animation from two retained palette clips.
- **Observed:** the Model report also contains two separate `animation-targets-unavailable`
  DroppedData warnings, each with count one. `WarningCount` and `DroppedFeatureCount` are therefore
  both two and `AnythingLost()` is true, although every authored channel is publicly retained and
  playable by one of the Model's palettes. No `rigid-animation-dropped-on-skinned-model`
  diagnostic is present. SOFTWARE and OPENGLES3/Xvfb each pass the other eight of nine controls
  and fail only the no-false-loss invariant.
- **Cause observed in source:** `ContentModelLoaders` calls `ExtractClips` independently for each
  skeleton and immediately appends each palette-local `AnimationReportEXT` to the one Model report.
  `ExtractClips` correctly skips and counts channels outside its current joint map, but the
  aggregate path never reconciles those skips with tracks retained by another skin. In contrast,
  the later rigid-track accounting explicitly tests every retained skeleton before calling a
  scene-node track lost.
- **Expected:** aggregate Model diagnostics must report a source channel unavailable only when no
  retained palette can carry its target. Palette-local skips may remain internal details, but they
  must not produce Model-level dropped-data counts for motion available through another
  `ModelSkinEXT::Data`.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction is deterministic on both exercised
  renderers and uses public clip data and numeric player poses; no screenshot or visual judgment
  is involved. Evidence:`/tmp/checkpoint-854-multi-skin-animation-*`. No CNA source was modified
  and no upstream message was sent.

### CNA-DEMOS-ISSUE-094 -- Retained glTF morph animation is reported as unsupported and lost

- **Area/API:** loose glTF `ContentManager::Load<Model>`, `MorphTargetDataEXT::WeightTrack` and
  `GltfImportReportEXT` animation diagnostics.
- **Reproduction:** `graphics.model.morph_animation_import` loads the same original two-target
  `Morph` asset through compiled CNB and loose glTF routes. Both parts retain the same four-key
  weight track. Evaluating it at 1.5 seconds yields weights `(1, 0.5)`, both carriers compute the
  same vertex bytes, and `SetMorphWeightsEXT` uploads those exact bytes on the loose path.
- **Observed:** the loose Model nevertheless reports `animation-paths-unsupported` count one and
  `animations-empty` count one. Both are warnings with loss kinds, so `AnythingLost()` is true and
  `DroppedFeatureCount` is two for a weight channel that remains available and playable through
  the public morph carrier. The compiled format's ordinary import report is empty. SOFTWARE and
  OPENGLES3/Xvfb each pass the other six of seven controls and fail only the no-false-loss
  invariant.
- **Cause observed in source:** `ExtractMorphWeightTrack` correctly extracts the weights channel
  and `ContentModelLoaders` attaches it to every relevant `MorphTargetDataEXT`. Independently,
  `ExtractSceneNodeClips` sends the same channel through `GatherChannels`, whose bone-clip carrier
  understands only translation, rotation and scale. It counts `weights` as an unsupported path,
  leaves the bone clip empty and appends both loss diagnostics. The final report never reconciles
  that bone-clip result with the successfully retained morph track.
- **Expected:** a weights channel retained in `MorphTargetDataEXT::WeightTrack` is supported by the
  returned Model and must not contribute unsupported-path, empty-animation or dropped-feature
  loss. The report may identify the separate morph carrier as information; it must not tell an
  application that authored motion was lost when the public playback data is present.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction uses exact CPU output and real
  VertexBuffer readback on both exercised renderers; no visual judgment is involved. Evidence:
  `/tmp/checkpoint-852-morph-animation-*`. No CNA source was modified and no upstream message was
  sent.

### CNA-DEMOS-ISSUE-093 -- Unknown glTF animation interpolation silently becomes LINEAR

- **Area/API:** loose glTF `ContentManager::Load<Model>`, animation sampler validation,
  `ModelAnimationsEXT` and `GltfImportReportEXT`.
- **Reproduction:** `graphics.model.animation_validation` loads an original otherwise valid glTF
  whose one animation sampler declares `interpolation: "CATMULLROM"`. glTF 2.0 permits only
  `LINEAR`, `STEP` and `CUBICSPLINE`. Adjacent fixtures prove that the same public loader refuses a
  decreasing input domain with CNA's actionable ordering diagnostic and an output accessor shorter
  than its input through structural validation.
- **Observed:** loading the unknown mode succeeds without a warning. CNA exposes a normal
  `UnknownInterpolation` scene-node clip with two keys and one-second duration, and sampling it at
  0.5 seconds produces translation X=0.2, the LINEAR midpoint. SOFTWARE and OPENGLES3/Xvfb each
  pass the other four of five controls and fail only the required-refusal assertion.
- **Cause observed in source:** `cgltf_parse_json_animation_sampler` writes the enum only for the
  three recognised strings. The zero-initialised field otherwise retains enum value zero, which is
  `cgltf_interpolation_type_linear`; `cgltf_validate` validates animation output counts but does not
  reject the unknown string. CNA's `ValidateGltfEXT` adds no interpolation-token check, and
  `LoadChannel` consequently sees an ordinary LINEAR sampler. The original invalid token is no
  longer available to the importer or report.
- **Expected:** a sampler declaring any other interpolation string is invalid glTF and must reach
  the caller as `ContentLoadException`, rather than changing authored motion silently. If CNA ever
  chooses an explicit compatibility policy, it must at least preserve a structured invalid-source
  diagnostic; the current unreported LINEAR substitution is not such a policy.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction is deterministic on both exercised
  renderers and needs no rendering. Evidence:`/tmp/checkpoint-850-animation-validation-*`. No CNA
  source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-092 -- An empty selected glTF scene resurrects an unselected mesh

- **Area/API:** loose glTF `ContentManager::Load<Model>`, default-scene selection and
  `GltfImportReportEXT`.
- **Reproduction:** `graphics.model.node_instances` loads a document whose explicit default is
  scene zero, an empty scene. Scene one alone references `ExcludedNode`, which places
  `ExcludedShape`. The demo expects the empty selected scene to yield no importable Model geometry
  and therefore a `ContentLoadException`; a second control records the exact leaked shape when the
  defect is present.
- **Observed:** loading succeeds. CNA returns one synthetic root, no `ExcludedNode`, and one
  `ExcludedShape` ModelMesh parented to that root. The public report consequently says
  `NodeCount == 0`, `MeshInstanceCount == 1` and `DistinctMeshCount == 1`. SOFTWARE and
  OPENGLES3/Xvfb each pass the other 23 of 24 controls and fail only the selected-empty-scene
  assertion.
- **Cause observed in source:** `BuildSceneGraph` correctly traverses only the selected empty
  scene, so `CollectMeshGroups` initially produces no groups. Its final fallback then imports every
  `data->meshes` entry whenever `groups.empty()`, without requiring `data->scenes_count == 0`.
  That condition is broader than CNA's documented no-scenes fallback and bypasses the preceding
  reachability filter. The GLTF-143 plan explicitly says the fallback must not fire for a file
  that has scenes.
- **Expected:** a file with scenes imports only nodes reachable from its selected scene. If that
  scene has no mesh placements, meshes referenced only from another scene stay excluded; the
  current Model loader should then report that there are no mesh primitives to import rather than
  fabricate an identity-root placement.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction is deterministic on both exercised
  renderers and requires no rendering to observe. Evidence:`/tmp/checkpoint-847-empty-scene-*`
  and `/tmp/checkpoint-848-node-graph-*`. No CNA source was modified and no upstream message was
  sent.

### CNA-DEMOS-ISSUE-091 -- C ImageBasedLight validity accepts live non-texture handles

- **Area/API:** `CNA_ImageBasedLightEXT` and `cna_image_based_light_ext_is_valid` in
  `graphics_ext.h`.
- **Reproduction:** `c_api.graphics.image_based_light_values` initializes the versioned value,
  proves its empty/default, mip-count, negative/NaN-intensity and incomplete-resource states, then
  creates two real TextureCube handles and one Texture2D handle. The complete typed bundle becomes
  valid and destroying one texture makes it invalid again. A separate bundle places the live Game
  handle in all three texture fields and asks the same public validity route to reject it.
- **Observed:** the wrong-kind bundle is reported valid. Both SOFTWARE and OPENGLES3/Xvfb pass the
  other 24 of 25 controls and fail only the exact non-texture-handle assertion; their child stdout
  is otherwise identical and stderr is empty.
- **Cause observed in source:** `ImageBasedLightExtIsValid` calls the runtime handle table's
  `GetKind` for each field but accepts every successfully resolved kind. Its own comment says a
  handle must be a live texture, while the corresponding C++ value has two `TextureCube*` fields
  and one `Texture2D*` field. The C adapter never compares the returned kinds with those required
  resource types.
- **Expected:** irradiance and prefiltered-specular fields accept only live TextureCube handles,
  and the BRDF lookup field accepts only a live Texture2D handle. Other live handle kinds must
  produce `out_valid = CNA_FALSE` without becoming an argument error.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction uses actual public C handles and is
  deterministic on both exercised renderers. Evidence:`/tmp/checkpoint-817-ibl-*`. No CNA source
  was modified, no upstream message was sent, and no PBR draw or visual result is claimed.

### CNA-DEMOS-ISSUE-090 -- ContentTypeWriter Initialize deadlocks when retrieving a nested writer

- **Area/API:** `ContentCompiler::AddTypeWriter`, `ContentTypeWriter::Initialize` and
  `ContentCompiler::GetTypeWriter` in the build-time XNB compiler façade.
- **Reproduction:** `content.custom.xml_xnb_pipeline` registers a focused writer whose
  `Initialize(ContentCompiler&)` follows the documented contract by calling `GetTypeWriter` for
  the built-in string writer it intends to cache. The attempt runs on a worker thread with a
  one-second watchdog so the demo reports the failure instead of hanging its test process.
- **Observed:** `AddTypeWriter` never returns and the watchdog records the one failed invariant.
  The demo first completes thirty-one positive controls through the same public pipeline and focused
  compiler probes: its main
  writer is initialized, resolves a nested custom writer later from `Write`, receives Windows
  Phone/HiDef target context, declines requested LZX compression, while a default writer accepts
  requested LZX and produces a compressed payload that the runtime restores; it emits ordinary,
  typed/raw, dynamically dispatched, shared-resource and external-reference values, loads them through game
  readers, including ordinary/shared/external nulls, skips an unchanged source and rebuilds an
  edited one; raw null, missing dynamic and foreign explicit writers are refused at their intended
  boundaries; its compiler inventory/freeze and direct XNB-registration contracts also pass.
  SOFTWARE and OPENGLES3 each pass31/33 controls; the second failure is independently labeled
  ISSUE-111. Their outputs and intended diagnostics are identical after normalizing the private
  temporary path.
- **Cause observed in source:** `ContentCompiler::AddWriter` acquires its non-recursive `mutex_`
  before invoking `facade->Initialize(*this)`. `GetTypeWriter` immediately tries to acquire the
  same mutex, so the same thread waits on a lock it already owns. CNA's public documentation says
  `Initialize` is called to retrieve and cache nested type writers, making this a direct contract
  failure rather than unsupported re-entrancy invented by the demo.
- **Expected:** a writer's `Initialize` callback can query already registered writers from the
  supplied compiler and complete registration. Registration should preserve its duplicate/frozen
  checks without holding a non-recursive lock across a callback documented to re-enter the
  compiler.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The reproduction is deterministic,
  build-time-only and independent of graphics. The demo manually removes its private files before
  starting the deadlock probe and flushes its result before terminating the focused process, since
  the blocked thread cannot be joined. Evidence: `/tmp/checkpoint-773-xnb-writer-*`. No CNA source
  was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-089 -- Intermediate serializers never receive their Initialize callback

- **Area/API:** `ContentTypeSerializer::Initialize`, `IntermediateSerializer`, custom serializer
  registration and the namespace-scanning pass.
- **Reproduction:** register the custom `TypedEnvelopeSerializer`. Its `Initialize` override records
  the call and caches the described Attack serializer from the supplied IntermediateSerializer.
  Serialize an envelope whose `ScanChildren` override reports that Attack as a dynamic child, then
  deserialize the emitted document.
- **Observed:** `ScanChildren`, Serialize and Deserialize all run, the scan reserves the
  `CnaDemos` namespace, and the nested `<Encounter Type="CnaDemos:Attack">` round-trips exactly.
  `Initialize` is never called, so the serializer must perform a fallback lookup during Serialize.
  SOFTWARE and OPENGLES3/Xvfb each pass16 of18 child controls and fail this invariant plus the
  independent established ISSUE-088 invariant; stdout is byte-identical and stderr is empty.
- **Cause observed in source:** `ContentTypeSerializerBase::InvokeInitialize` exists and the public
  `Initialize` documentation says it retrieves and caches nested serializers. A source-wide search
  finds no production caller. `RegisterTypeSerializer` installs the object without initializing it,
  and `SerializeObject`/`DeserializeObject` construct a pass but never invoke initialization.
- **Expected:** each registered custom serializer must receive `Initialize` with the active
  IntermediateSerializer before child scanning or serialization/deserialization, with lifecycle
  semantics that permit its documented nested-serializer cache. The demo's scan fallback is only
  diagnostic; it does not make the failed callback pass.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The issue is deterministic, build-time-only and
  independent of graphics. Evidence: `/tmp/checkpoint-770-intermediate-scan-*`. No CNA source was
  modified and no upstream message was sent. ISSUE-088 remains the demo's primary knownIssue
  marker.

### CNA-DEMOS-ISSUE-088 -- Conflicting intermediate XML shortcuts create a non-round-trippable serializer registration

- **Area/API:** `IntermediateSerializer::AddTypeSerializer`, `RegisterTypeSerializer`,
  `FindTypeSerializer` and intermediate XML `Type` shortcut resolution.
- **Reproduction:** register `DiceRollSerializer` for `CnaDemos.DiceRoll` with XML shortcut `dice`,
  then register `CoinFlipSerializer` for the distinct `CnaDemos.CoinFlip` type with the same
  shortcut. Serialize a CoinFlip root and immediately deserialize it as CoinFlip.
- **Observed:** the second registration returns successfully and installs CoinFlip under its C++
  type and full name, but the `dice` name remains mapped to DiceRoll. CoinFlip serialization emits
  `<Asset Type="dice">3c</Asset>`; deserialization resolves that type as DiceRoll and refuses it as
  unrelated to CoinFlip. Both renderer selections reproduce exactly one failed invariant among ten
  controls; the nine other controls pass and child stderr is empty.
- **Cause observed in source:** `RegisterTypeSerializer` checks only an existing target type. It
  moves a new distinct-type serializer into registry ownership and installs its type/carrier before
  using unchecked `std::map::emplace` calls for the full name and XML shortcut. A pre-existing name
  silently keeps its old owner, leaving the returned serializer only partly registered.
- **Expected:** a name collision must be rejected before registry mutation, or registration must
  otherwise preserve a complete mapping whose own emitted XML resolves back to that serializer.
  Re-registering the same target type may continue returning the first serializer, as the focused
  control demonstrates.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. The issue is deterministic, build-time-only and
  independent of graphics. Evidence: `/tmp/checkpoint-766-intermediate-registration-*`. No CNA
  source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-087 -- Disposable XNB shared resources bypass the ReadAsset recorder

- **Area/API:** `ContentManager::ReadAsset`, `ContentReader::ReadSharedResources`,
  `ContentReader::InnerReadObjectAny` and disposal recording for XNB object graphs.
- **Reproduction:** make the existing shared graph's `Item` implement `IDisposable`, expose
  protected `ReadAsset<std::shared_ptr<Party>>` through a derived manager and supply a recorder.
  The authored graph contains one typed inline Item and two Items in the shared-resource table.
- **Observed:** the recorder receives the typed inline Item exactly once but receives neither
  shared-table Item. The complete graph still decodes correctly: both root aliases, the forward
  edge, weak back edge and all deferred fixups pass. SOFTWARE and OPENGLES3/Xvfb each pass16/17
  controls and fail only the exact three-resource recorder assertion.
- **Cause observed in source:** typed `InnerReadObject<T>` calls `RecordDisposable(result)`, while
  `ReadSharedResources` constructs every table value through `InnerReadObjectAny`. That type-erased
  route deliberately omits `RecordDisposable`, so a disposable value cannot reach either the
  supplied callback or the manager fallback. The reference ContentReader performs its runtime
  IDisposable check even when the static read type is object.
- **Expected:** the supplied ReadAsset callback receives every IDisposable owned by the asset,
  including values materialized from the shared-resource table. Alias fixups must not record the
  same shared object again; each of the two table objects should appear once alongside the inline
  object.
- **Scope/status:** open against clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`. This is distinct from ISSUE-011: a nonempty caller
  recorder is present and demonstrably receives the typed inline object. Explicit cleanup follows
  the assertion. No graphics, screenshot or manual inspection is involved. Evidence:
  `/tmp/checkpoint-759-shared-resources-*`. No CNA source was modified and no upstream message was
  sent.

### CNA-DEMOS-ISSUE-086 -- Destroying a secondary EasyGL device blacks out the primary device

- **Area/API:** multiple `GraphicsDevice` lifetimes under OPENGLES3/EasyGL, especially
  `GraphicsDevice::Dispose`, renderer context leasing and EasyGL resource-registry teardown.
- **Reproduction:** while the game-owned primary HiDef device remains alive, construct a second
  32x32 HiDef device. Create and transfer cube resources on both devices, copy-assign the primary
  wrapper from the secondary resource, move the actively bound secondary wrapper, unbind it and
  explicitly dispose the secondary device. Then clear and draw the primary device's ordinary
  Texture2D atlas through SpriteBatch and read the back buffer. The extended
  `graphics.texture_cube.face_transfers` performs this sequence. The existing
  `graphics.texture3d.volume_data` independently creates and destroys secondary Reach and HiDef
  devices before drawing its atlas.
- **Observed:** all four cube lifecycle controls pass: cross-device copy assignment reports the
  secondary owner and exact data, the old primary slot is detached, later source edits are shared,
  move construction transfers the active binding, and secondary disposal invalidates both
  registered wrappers atomically. The following primary OPENGLES3 frame is nevertheless completely
  black. Both established atlas checks fail, and a captured 800x480 frame contains only black
  pixels. The Texture3D demo still reports its 25 non-framebuffer controls as passing, but its
  diagnostic screenshot after the same two-device pattern is also completely black. SOFTWARE
  passes all 12 extended cube checks and renders the expected atlas.
- **Expected:** destroying one graphics device must not prevent a different live device from
  clearing, drawing, presenting or returning its framebuffer. The primary's resources and renderer
  remain owned by the primary device.
- **Source investigation:** `GraphicsDevice::destroyNativeResources` resets only the disposing
  device's renderer/presenter/window. EasyGL has per-device platform contexts and frame/resource
  leases that promise to make the requested context current and restore the previous binding.
  `EasyGLRenderer` also registers each per-device resource registry with meta-gl. Despite those
  boundaries, the next primary frame produces no visible clear, text or texture output and emits
  no diagnostic. The exact shared context/registry state invalidated by secondary teardown remains
  to be isolated inside CNA; the demo does not restore or recreate it.
- **Scope/status:** open against the clean isolated CNA `d23e139de9e5c15511b6be745de290896a69ee37`
  used by `linux-full`. Reproduced under Mesa/X11/Xvfb with OPENGLES3; no physical-GPU, OPENGL33,
  Vulkan or WebGPU claim is made. `graphics.texture_cube.face_transfers` is marked only for
  OPENGLES3. Evidence: `/tmp/checkpoint-748-texture-cube-OPENGLES3.*`,
  `/tmp/checkpoint-748-failed-gl.png` and `/tmp/checkpoint-748-texture3d-gl.png`. No CNA source was
  modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-085 -- Offline leaderboard writers cannot create or persist stream columns

- **Area/API:** `PropertyDictionary::GetValueStream`, `LeaderboardWriter::GetLeaderboard` and the
  offline leaderboard store in `modules/gamer-services`.
- **Reproduction:** run `gamer_services.leaderboard.paging` with its four isolated local profiles.
  For each writer entry, request a new `Replay` stream column, write three bytes and then read the
  board back after `Rating` commits the row.
- **Observed:** `GetValueStream("Replay")` throws `std::out_of_range` from the missing map key for
  every offline writer. The demo catches that refusal so its other typed columns continue through
  the real store; the reader consequently has no `Replay` value. Both SOFTWARE and OPENGLES3/X11
  execute17 controls, pass the16 typed, lookup, paging and lifecycle controls, and fail only the
  stream round-trip assertion with byte-identical stdout and empty stderr.
- **Cause observed in source:** `LeaderboardWriter::BindServiceEntry` sets the private
  `PropertyDictionary::createsStreams_` flag only for service-backed entries. The offline
  `GetLeaderboard` path installs its rating persistence hook but leaves the flag false, so
  `GetValueStream` treats a missing stream key as a read and calls `map::at`. Additionally,
  offline `ColumnsToJson` explicitly skips `Stream*`, so enabling creation alone would still not
  fulfill the documented persistence contract.
- **Expected:** a leaderboard writer's columns should create a writable stream on first access and
  persist its contents with the entry, as the public PropertyDictionary header states; reader
  entries should expose the stored read-only bytes.
- **Scope/status:** open against clean external CNA `f698d0f548`. The same workflow proves exact
  offline persistence for int32, int64, float, double, DateTime, string, TimeSpan and
  LeaderboardOutcome columns. Evidence:`/tmp/checkpoint-704-property-dictionary-*`. No CNA source
  was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-084 -- PacketReader documentation and example still describe the fixed Color mismatch

- **Area/API:** `PacketReader::ReadColor`, `PacketWriter::Write(Color)`, the public
  `PacketReader.hpp` contract and `modules/net/examples/demo_packet_roundtrip`.
- **Reproduction:** run `net.packet.send_and_receive` with two local profiles. The demo writes
  `Color::Orange` through PacketWriter, sends the packet through a real local System Link session
  and reads it through PacketReader before reading the following double, vectors, quaternion and
  matrix. Compare this with the public header and the bundled roundtrip example.
- **Observed:** the implementation reads four byte channels and the complete 141-byte packet
  round-trips exactly on SOFTWARE and OPENGLES3. CNA's public header still says ReadColor reads four
  floats, is not the inverse of Write(Color), and preserves that behavior for fidelity. The bundled
  example likewise expects the former 4-byte/16-byte mismatch and treats its exception as a quirk.
  CNA's own `BINDFIX-022` record says that mismatch was fixed.
- **Expected:** public documentation and the bundled example should describe and exercise the
  current inverse four-byte behavior, so users are not told to avoid a working API or to expect an
  exception that no longer occurs.
- **Scope/status:** open documentation/example defect against clean external CNA `f698d0f548`.
  Runtime color and every following typed read pass; no runtime knownIssue marker is needed. Both
  renderer runs have byte-identical stdout and empty stderr. Evidence:`/tmp/checkpoint-685-*`. No
  CNA source was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-083 -- Adapter back-buffer query reports zero samples before a multisampled device is created

- **Area/API:** `GraphicsAdapter::QueryBackBufferFormat`,
  `GraphicsDeviceManager::PreferMultiSampling` and the applied
  `PresentationParameters::MultiSampleCount`.
- **Reproduction:** in `game.graphics_device_manager.back_buffer_size`, request Bgr565, the
  default Depth24 and multisampling through GraphicsDeviceManager. Before inspecting the created
  device, query its adapter for the same profile, formats and eight samples; compare all selected
  values with the device's stored presentation parameters.
- **Observed:** on SOFTWARE the query selects `Color`, `Depth24`, 0 samples while device creation
  applies `Color`, `Depth24`, 4 samples. On OPENGLES3 the same query returns 0 while creation
  applies 8 samples. The format and depth answers agree, and disabling multisampling through
  `ApplyChanges` writes the expected zero to the device.
- **Cause observed in source:** `GraphicsAdapter::QueryBackBufferFormat` can clamp samples only
  through the selected renderer descriptor's `adapterQueries.clampMultiSampleCount` callback.
  The SOFTWARE descriptor has no adapter queries, and OPENGLES3 supplies only a render-target
  format predicate. Their live renderer instances nevertheless implement and apply supported
  multisample counts. CNA's own current handoff independently records the same query-versus-reset
  discrepancy for the surviving renderers.
- **Expected:** the public format query's selected multisample count should predict the count the
  same adapter/profile/settings combination will apply, or the API must expose a documented
  inability to answer before device creation rather than returning a contradictory valid value.
- **Positive controls/scope:** manager preference getters retain Bgr565/true; the adapter-selected
  Color and Depth24 match the device; applied multisampling is greater than one; the later
  Color/false transition reaches Color/zero. No edge-quality, screenshot or visual result is
  claimed.
- **Status:** open against the runtime built from external CNA `73991b01f7`; the sibling checkout
  also has six unrelated renderer/build documentation paths, none edited here. Both renderer runs
  fail only the query-agreement control with empty stderr. Initial diagnostics are
  `/tmp/checkpoint-660-diagnostic*`; final marked evidence is `/tmp/checkpoint-660-*`. No CNA source
  was modified and no upstream message was sent.

### CNA-DEMOS-ISSUE-082 -- Disposing an open SpriteBatch leaves its device batch active

- **Area/API:** `SpriteBatch::Dispose(bool)`, `SpriteBatch::Begin/End` and the per-device
  `spriteBeginCount_` / `spriteImmediateBeginCount_` coordination in
  `modules/graphics/src/Xna/SpriteBatch.cpp`.
- **Reproduction:** create two SpriteBatch values on one device, begin a deferred batch on the
  first, dispose it without End, then begin an Immediate batch on the second. The extended
  `graphics.sprite_batch.begin_end_rules` performs the same sequence after its established
  Begin/End refusal controls.
- **Observed:** Dispose raises Disposing once, clears the queue, publishes IsDisposed and correctly
  makes subsequent Begin, End and Draw calls throw ObjectDisposedException. The unrelated
  Immediate Begin nevertheless throws InvalidOperationException claiming another batch remains
  active. SOFTWARE and OPENGLES3 execute the same fourteen checks: thirteen positive controls pass
  and only the active-accounting assertion fails.
- **Cause observed in source:** successful Begin increments the GraphicsDevice counters.
  `SpriteBatch::End` has a `releaseDeviceAccounting` path, but `Dispose(bool)` only clears the
  renderer, queue and `begun` flag before delegating to GraphicsResource. Once disposed, End cannot
  be called to release the leaked count.
- **Expected:** disposing an open batch should retire the accounting established by its successful
  Begin so it cannot poison other SpriteBatch values on the device. Public Dispose may abandon the
  disposed batch's queued work, but unrelated batching must remain usable.
- **Positive controls/scope:** all older sequencing refusals pass. Drawing an explicitly disposed
  Texture2D throws ObjectDisposedException before queueing, after which the same deferred batch
  accepts a valid Draw and End. A deferred command also retains the
  renderer payload of a temporary Texture2D whose C++ wrapper leaves scope before End and produces
  exact orange/black framebuffer samples on both renderers. This is programmatic readback; no
  screenshot or visual inspection was performed. Explicitly disposing a texture after it has been
  queued, renderer End failures and device teardown are not claimed. Cross-device resource
  rejection is demonstrated separately by `graphics.sprite_batch.cross_device_texture`.
- **Status:** open and reconfirmed against clean test-only CNA `d23e139de`; runtime marker applies
  to both exercised renderers. No CNA source was modified and no upstream message was sent.
  Current evidence: `/tmp/checkpoint-730-disposed-texture-*`.

### CNA-DEMOS-ISSUE-081 -- ModelMesh construction omits pre-attached part effects

- **Area/API:** `ModelMesh` constructors and `ModelMeshPart::setEffectProperty` in
  `modules/graphics/src/Xna`, plus `cna_model_mesh_create[_named]`,
  `cna_model_mesh_get_effects` and the parallel retained-effect list in
  `modules/c-api/src/CnaCApiModels.cpp`.
- **Reproduction:** create a real BasicEffect and a drawable ModelMeshPart, attach the effect to
  the standalone part, create a ModelMesh from that part and inspect the returned live
  ModelEffectCollection before changing the part again.
- **Observed:** the part still returns the exact attached effect and `ModelMesh::Draw` uses it,
  but `Effects` has count zero. Both native ModelMesh constructors assign each part's parent after
  its effect was set and never seed `effects_`; the C adapter likewise leaves
  `MeshResource::effects` empty. Detaching and reattaching through the public setter after
  construction synchronizes both collections and drawing remains correct.
- **Expected:** a newly constructed mesh's `Effects` should contain each distinct non-null effect
  already used by its parts, just as it does after those same assignments are made while the part
  belongs to the mesh.
- **Positive controls/status:** buffer/effect retention, post-construction setter synchronization,
  duplicate collection mutation, stable mesh views, parent ownership and exact green/black
  framebuffer probes all pass. Open renderer-independent construction/state defect reproduced on
  both SOFTWARE and OPENGLES3 by the same stable C API library. The passing demo names the issue
  in its check and `restrictions`; no CNA source was modified or upstream message sent. Evidence:
  `checkpoint-623-c-model-meshes-focused-*-direct.*`.

### CNA-DEMOS-ISSUE-080 -- C avatar animation info claims built-in clips have no timeline

- **Area/API:** `CNA_AvatarAnimationInfo` documentation in
  `modules/c-api/include/CNA/C/gamer_services.h`; the canonical `AvatarAnimation` implementation
  and `cna_avatar_animation_*` adapters.
- **Reproduction:** create each defined `CNA_AVATAR_ANIMATION_PRESET_*`, call
  `cna_avatar_animation_get_info`, then sample Wave at zero and1.2 seconds and inspect its right-
  elbow transform.
- **Observed:** all31 presets report71 bones and real lengths from2.5 to10 seconds. Wave's elbow
  changes substantially at1.2 seconds; direct position, clamped/looping update and Celebration's
  keyed facial expression all work. This agrees with the canonical C++ header, implementation and
  tests.
- **Expected:** the C info documentation currently says a built-in preset has no timeline and
  `length_ticks` stays zero until a real clip is loaded. It should describe the built-in original
  CNA clips and their real timeline, bone and expression sampling.
- **Scope/status:** passing C-header documentation mismatch. The renderer-free demo validates CPU
  animation data only and needs no runtime knownIssue marker. Both focused renderer-labelled direct
  runs pass17 byte-identical controls with empty stderr. No CNA source was modified or upstream
  message sent. Evidence:`checkpoint-599-c-avatar-animations-focused-*-direct.*`.

### CNA-DEMOS-ISSUE-079 -- Anonymous leaderboard entry clears an output promised unchanged

- **Area/API:** `cna_leaderboard_entry_get_gamer` in
  `modules/c-api/include/CNA/C/gamer_services.h` and generated `Leaderboards.inc`.
- **Reproduction:** create a leaderboard entry with `CNA_INVALID_HANDLE` as its optional gamer,
  initialize `out_gamer` to a nonzero sentinel and call `cna_leaderboard_entry_get_gamer`.
- **Observed:** the call succeeds with `out_has_gamer == CNA_FALSE` and replaces the sentinel with
  `CNA_INVALID_HANDLE`. The generated wrapper unconditionally clears `*out_gamer` before the helper
  discovers that the entry has no gamer.
- **Expected:** the public header explicitly says the gamer output "is left exactly as the caller
  set it" when the presence flag is false. Either preserve it for the ordinary absent result or
  document deterministic invalid-handle initialization, matching the wrapper and the wider C
  output policy.
- **Positive controls:** gamer-backed borrowing, exact entry info, rating mutation/hook behavior,
  equality, live mutable columns, alias retention and stale-handle refusal all pass. Both focused
  renderer-labelled direct runs pass all23 controls with byte-identical output and empty stderr.
- **Scope/status:** passing public-header documentation mismatch; the demo asserts the current
  deterministic result and needs no runtime knownIssue marker. No CNA source was modified or
  upstream message sent. Evidence:`checkpoint-596-c-leaderboard-entries-focused-*-direct.*`;
  runtime uses the last completed CNA libraries while current source contains the same generated
  assignment.

### CNA-DEMOS-ISSUE-078 -- Missing C achievement key escapes as an internal failure

- **Area/API:** `cna_achievement_collection_get_by_key` in
  `modules/c-api/include/CNA/C/gamer_services.h`, its adapter in
  `modules/c-api/src/CnaCApiAchievements.cpp`, and the exception barrier in
  `modules/c-api/src/CnaCApiDetail.hpp`.
- **Reproduction:** create one synthetic achievement, copy it into an achievement collection,
  initialize `out_achievement` to a nonzero sentinel, then request an absent key.
- **Observed:** the route clears the output to `CNA_INVALID_HANDLE` but returns
  `CNA_RESULT_INTERNAL`. The canonical string indexer throws `System::IndexOutOfRangeException`;
  that type reaches the barrier's generic `std::exception` arm instead of one of its argument/range
  translations.
- **Expected:** the route-specific public contract explicitly promises
  `CNA_RESULT_INVALID_ARGUMENT` when no achievement has the key. Preserve the deterministic cleared
  output and translate this canonical exception to that result.
- **Scope/status:** passing result-contract mismatch; the demo asserts the current deterministic
  result and names this issue, so it needs no additional runtime knownIssue marker. Both focused
  renderer-labelled direct runs otherwise have identical output and empty stderr. No CNA source was
  modified or upstream message sent. Evidence:
  `checkpoint-589-c-achievement-collection-focused-*-direct.*`; runtime uses the last completed CNA
  libraries while current source contains the same exception path.

### CNA-DEMOS-ISSUE-077 -- Mutable C achievement collection reports read-only

- **Area/API:** `cna_achievement_collection_get_is_read_only` and the Add/Insert/Remove/RemoveAt/
  Clear routes in `modules/c-api/include/CNA/C/gamer_services.h`; canonical implementation in
  `modules/gamer-services/src/Xna/AchievementCollection.cpp`.
- **Reproduction:** create a two-value collection, query `get_is_read_only`, then add and insert
  copied values, remove them by value and index, and clear the collection.
- **Observed:** `get_is_read_only` succeeds and returns `CNA_TRUE`; every public mutator also
  succeeds and changes the collection exactly as requested. The native property is hard-coded true
  while its methods modify the backing vector.
- **Expected:** the C header defines this property as whether the collection refuses modification.
  A collection whose public mutators succeed must report `CNA_FALSE`, or the mutators must
  consistently refuse and the C surface/documentation must describe a read-only collection.
- **Positive controls:** count, indexed/key lookup, equality membership, bounds, atomic shifted
  copying, all mutations and independent ownership behave consistently. Both focused renderer-
  labelled runs have identical output and empty stderr, with29/30 controls passing.
- **Scope/status:** open renderer-independent CNA implementation/contract defect. The focused C demo
  retains the one expected failed assertion and is marked `CNA-DEMOS-ISSUE-077`. No CNA source was
  modified or upstream message sent. Evidence:
  `checkpoint-589-c-achievement-collection-focused-*-direct.*`; runtime uses the last completed CNA
  libraries while current source contains the same hard-coded property.

### CNA-DEMOS-ISSUE-076 -- Absent signed-in player lookup contradicts its output contract

- **Area/API:** `cna_gamer_get_signed_in_gamer_at_player_index` in
  `modules/c-api/include/CNA/C/gamer_services.h` and generated `Gamers.inc`.
- **Reproduction:** create a valid signed-in gamer for player two but leave the process collection
  empty, initialize `out_gamer` to a nonzero sentinel, then query player two. Repeat after publishing
  player two while querying the unoccupied player-one slot.
- **Observed:** both calls succeed with `out_has_gamer == CNA_FALSE` and replace the sentinel with
  `CNA_INVALID_HANDLE`. The generated wrapper unconditionally assigns `*out_gamer` to the invalid
  handle before the implementation determines whether the player slot is occupied.
- **Expected:** the public header explicitly says `out_gamer` "is left exactly as the caller set it"
  when `out_has_gamer` is false. Either preserve it for the ordinary absent result or update the
  contract to promise deterministic invalid-handle initialization, matching the generated wrapper
  and the broader C output policy.
- **Scope/status:** passing public-header documentation mismatch; the demo follows the current
  deterministic invalid-handle behavior and needs no runtime knownIssue marker. Both focused
  renderer-labelled runs pass all29 controls with identical output and empty stderr. No CNA source
  was modified or upstream message sent. Evidence:
  `checkpoint-587-c-signed-in-state-focused-*-direct.*`; runtime uses the last completed CNA
  libraries while current source contains the same unconditional generated assignment.

### CNA-DEMOS-ISSUE-075 -- C offset packet receive overwrites bytes outside the requested range

- **Area/API:** `cna_local_network_gamer_receive_data_at` in
  `modules/c-api/include/CNA/C/net_sessions.h` and its adapter in
  `modules/c-api/src/CnaCApiNetSessions.cpp`.
- **Reproduction:** create a socket-free Local session, inject a four-byte packet into its local
  gamer, fill a twelve-byte destination with `0xA5`, then receive the packet at offset two. The
  route succeeds, reports four bytes and places the exact payload at indices two through five.
- **Observed:** the adapter also changes every byte outside that range to zero. It allocates a new
  zero-initialized vector whose size is the caller's complete capacity, asks the canonical overload
  to write at the offset, then copies the entire vector over the caller's destination. Both the
  two-byte prefix and six-byte suffix are therefore overwritten.
- **Expected:** the API describes the offset as the position within the destination where writing
  begins. A successful four-byte receive at offset two should change only indices two through five,
  as the canonical array overload does; unrelated caller bytes should remain intact.
- **Positive controls:** an undersized offset request is refused without consuming the queued
  packet; the accepted call returns the exact payload and sender; plain-buffer and PacketReader
  receives are exact; all ownership cleanup succeeds. Both renderer-labelled direct runs have
  identical output and empty stderr, with27/28 checks passing.
- **Suggested repair/status:** copy only the returned payload range from the temporary vector into
  `destination + offset`, or preserve the caller buffer before invoking the canonical overload.
  Open CNA implementation defect; `c_api.net.local_gamer_packets` retains the single expected
  failure and is marked `CNA-DEMOS-ISSUE-075`. No CNA source was modified or upstream message sent.
  Evidence:`checkpoint-583-c-local-packets-focused-*-direct.*`; runtime uses the last completed CNA
  libraries while unrelated renderer-retirement work remains in flight.

### CNA-DEMOS-ISSUE-074 -- C assembly-title route misstates malformed-text result

- **Area/API:** `cna_assembly_set_title_ext` in
  `modules/c-api/include/CNA/C/core_ext.h`.
- **Observed contract conflict:** the route-specific return documentation lists
  `CNA_RESULT_INVALID_ARGUMENT` for a malformed string view. An otherwise valid five-byte view
  containing an embedded NUL returns `CNA_RESULT_ENCODING` and leaves the existing title intact.
  The generated adapter explicitly uses `ReadText(..., EmbeddedNul::Refused)`, whose documented
  result set includes `ENCODING`; CNA's general `STRINGS_AND_BUFFERS.md` likewise says semantic
  text values reject embedded U+0000 with `CNA_RESULT_ENCODING`.
- **Runtime evidence:** `c_api.platform.host_identity` first installs and exactly copies an
  authored title, submits `{'b','a','d','\0','x'}`, observes `ENCODING`, and copies the unchanged
  authored bytes before restoring the previous process title. Both focused direct renderer
  selections pass with identical output and empty stderr after correcting the demo's initial
  expectation. Evidence:`checkpoint-556-c-platform-host-identity-focused-*-direct.*`.
- **Expected:** list `CNA_RESULT_ENCODING` for malformed UTF-8 or forbidden embedded NUL text,
  while retaining `CNA_RESULT_INVALID_ARGUMENT` for an invalid pointer/count pair. The
  implementation and shared text contract should stay as-is.
- **Scope/status:** passing public-header documentation mismatch; no runtime knownIssue marker is
  needed. No CNA source was modified or upstream message sent. Direct runtime uses the last
  completed CNA libraries while unrelated external renderer-retirement work remains in flight.

### CNA-DEMOS-ISSUE-073 -- Refused live-3D pan setter changes the public pan value

- **Area/API:** `SoundEffectInstance::setPanProperty` and the C
  `cna_sound_effect_instance_set_pan`/`cna_sound_effect_instance_get_info` routes.
- **Reproduction:** create a mono `SoundEffect`, create an instance, call `Apply3D` before `Play`,
  start playback, then request direct pan0.5. The setter correctly refuses the incompatible mode
  with `CNA_RESULT_INVALID_STATE`; immediately read the instance snapshot.
- **Observed:** the snapshot reports pan0.5 even though the setter failed. The canonical setter
  stores `Pan_ = pan` before checking the latched live-3D state and throwing. The real spatial mixer
  still selects its separate `spatialPan_`, so this is a public property/failed-operation side
  effect rather than an audible-output observation.
- **Expected:** validate the live-3D state before assigning `Pan_`, so a refused setter leaves the
  public property unchanged at its prior value (zero in the focused reproduction).
- **Scope/status:** open CNA implementation defect. The focused C demo retains one expected failing
  assertion and continues through cleanup; no CNA source was modified or upstream message sent.
  Direct runtime currently uses the last completed CNA libraries from `b2ded99da9`; project-built
  evidence awaits completion of the unrelated in-progress Vulkan retirement. Both direct renderer
  selections pass21 of22 controls with identical stdout and empty stderr; evidence is archived as
  `checkpoint-530-c-spatial-state-focused-*-direct.*`.

### CNA-DEMOS-ISSUE-072 -- C SoundEffectInstance snapshot promises pitch clamping

- **Area/API:** `CNA_SoundEffectInstanceInfo.pitch` and
  `cna_sound_effect_instance_set_pitch` in `modules/c-api/include/CNA/C/audio.h`.
- **Observed contract conflict:** the public snapshot field says it reports the pitch "after CNA
  clamping to `[-1, 1]`", and the setter only documents a finite input. Passing2.0 actually returns
  `CNA_RESULT_INVALID_ARGUMENT`; setting1.0 succeeds and the snapshot returns1.0. The generated C
  adapter checks finiteness and then calls `SoundEffectInstance::setPitchProperty`, whose canonical
  implementation deliberately throws for every value outside the closed interval. CNA's pure-C
  `AudioSmoke.c` also explicitly requires rejection of2.0 and describes that as XNA behavior.
- **Runtime evidence:** `c_api.audio.sound_effect_instance_state` range-checks2.0, then configures
  pitch1.0 with volume, pan and looping. The complete property snapshot and all later Play/Pause/
  Resume/release/replay controls pass in five repeated SOFTWARE direct runs and one OPENGLES3
  direct run with byte-identical output and empty stderr.
- **Expected:** document pitch as range-checked to `[-1, 1]` and list out-of-range input among the
  setter's invalid-argument outcomes. The implementation and its canonical test should stay as-is.
- **Scope/status:** passing public-header documentation mismatch; no runtime knownIssue marker is
  needed. No CNA source was modified or upstream message sent. Direct runtime currently uses the
  last completed CNA libraries from `b2ded99da9`; project-built evidence awaits completion of the
  unrelated in-progress Vulkan retirement. Open; direct evidence is archived as
  `checkpoint-529-c-sound-instance-state-focused-*-direct.*`.

### CNA-DEMOS-ISSUE-071 -- C PBR texture getter documents five of its seven slots

- **Area/API:** `cna_pbr_effect_get_texture` in
  `modules/c-api/include/CNA/C/effects.h`.
- **Observed contract conflict:** its `slot` parameter says "One of the five `CNA_PBR_TEXTURE_*`
  identities." The same public header defines seven identities from base color0 through specular
  color6, makes6 `CNA_PBR_TEXTURE_MAXIMUM`, and describes the coordinate/transform routes as
  accepting all of them. The generated texture getter/setter accept every identity through6.
- **Runtime evidence:** `c_api.graphics.pbr_texture_slots` gets and sets exact retained handles in
  all seven slots, independently round-trips each slot's UV selector and transform, and confirms the
  three color-carrying sRGB identities. Two textures survive cloning and source destruction through
  five core and two specular slots respectively, then release after their last slot clears. All34
  controls pass identically under SOFTWARE and OPENGLES3 with empty stderr.
- **Expected:** say seven defined slot identities, or refer to the inclusive named range, so the
  public C contract includes its two specular extension slots.
- **Scope/status:** passing public-header documentation mismatch; no runtime knownIssue marker is
  needed. No CNA source was modified or upstream message sent. Observed at clean CNA `a6bb279d47`;
  focused evidence is archived as `checkpoint-517-c-pbr-textures-focused-*`. Open.

### CNA-DEMOS-ISSUE-070 -- PBR clones lose vertex-color and alpha-coverage state

- **Area/API:** `PbrEffect::Clone`, `SkinnedPbrEffect::Clone`, and their shared C routes in
  `modules/c-api/include/CNA/C/effects.h`.
- **Runtime reproduction:** create a C `PbrEffect`, enable vertex color and double-sided rendering,
  select blend alpha mode and set cutoff0.125, then clone it through `cna_effect_clone`, destroy the
  source and query the clone through the same public PBR routes.
- **Observed:** four focused controls fail under both SOFTWARE and OPENGLES3. The clone reports the
  constructor defaults: vertex color false, opaque alpha mode, cutoff0.5 and double-sided false.
  Core metallic-roughness/dielectric values and the separately stored output-encoding switch all
  survive the same clone and source destruction, and the clone applies successfully.
- **Expected:** `Clone()` creates an independent effect with its current public material state, as
  the existing clone behavior and C route imply. These four draw-affecting properties must survive
  like the other material fields.
- **Source evidence:** both copy constructors explicitly copy the core and extension factors,
  texture metadata and `encodeOutputToSrgb_`, but omit `VertexColorEnabledEXT`, `alphaMode_`,
  `alphaCutoff_` and `doubleSided_`. The two implementations have the same omission. Existing CNA
  clone tests cover several copied fields but do not set or assert these four.
- **Scope/status:** renderer-independent runtime defect; `c_api.graphics.pbr_effect_material_state`
  retains exactly the four failed assertions and is marked with this issue. No PBR draw, BRDF pixel
  or visual result is claimed. No CNA source was modified or upstream message sent. Observed at
  clean CNA `a6bb279d47`; focused evidence is archived as
  `checkpoint-516-c-pbr-material-focused-*`. Open.

### CNA-DEMOS-ISSUE-069 -- C Effect clone comment overpromises concrete-type preservation

- **Area/API:** `cna_effect_clone` in `modules/c-api/include/CNA/C/effects.h`.
- **Observed contract conflict:** the public C comment says the result is "an owned clone of the
  same concrete native type." `EffectMaterial` deliberately has no `Clone` override, so its
  inherited canonical `Effect::Clone()` returns an independent base `Effect`. CNA's public
  software-renderer documentation and dedicated C++ tests identify this as recovered Microsoft XNA
  and FNA behavior; changing the runtime to preserve `EffectMaterial` would be a regression.
- **Runtime evidence:** `c_api.graphics.effect_concrete_types` creates an `EffectMaterial`, clones
  it through the public C route and observes the exact base type name
  `Microsoft.Xna.Framework.Graphics.Effect`; the clone applies successfully but both
  EffectMaterial-only ownership routes refuse it. In the same run, `SpriteEffect.Clone()` preserves
  its concrete type and exact stock-sprite identity. All22 controls pass identically under SOFTWARE
  and OPENGLES3 with empty stderr.
- **Expected:** document the independent clone and the `EffectMaterial` exception instead of
  promising concrete-type preservation for every effect. No runtime change is indicated.
- **Scope/status:** passing public-header documentation mismatch; no runtime knownIssue marker is
  needed. No CNA source was modified or upstream message sent. Observed at clean CNA `a6bb279d47`.
  Open; focused evidence will be archived as
  `checkpoint-510-c-effect-concrete-types-focused-*`.

### CNA-DEMOS-ISSUE-068 -- C effect-parameter semantic lookup misstates case matching

- **Area/API:** `cna_effect_parameter_collection_find_semantic` in
  `modules/c-api/include/CNA/C/effects.h`.
- **Observed contract conflict:** the public C comment calls its input an "Exact UTF-8 semantic",
  but the route delegates to `EffectParameterCollection::GetParameterBySemantic`. The public C++
  header documents case-insensitive matching, its implementation uses `OrdinalIgnoreCase`, and its
  tests require mixed-case matching plus first-insertion wins for semantic duplicates.
- **Runtime evidence:** `c_api.graphics.effect_parameter_collections` stores `POSITION` followed by
  `Position`; a lookup for lowercase `position` returns the first parameter and its current value77
  under both tested renderer selections. Name lookup in the same collection remains case-sensitive.
- **Expected:** describe the C route as ordinal case-insensitive and state that the first matching
  parameter is returned, aligning the public binding contract with the behavior it deliberately
  exposes. No runtime change is indicated.
- **Scope/status:** passing public-header documentation mismatch; no runtime knownIssue marker is
  needed. No CNA source was modified or upstream message sent. Observed at clean CNA `fbd8e64a3`.
  Open; focused evidence will be archived as
  `checkpoint-507-c-effect-parameter-collections-focused-*`.

### CNA-DEMOS-ISSUE-067 -- C display-color-space routes expose unnamed integer ordinals

- **Area/API:** `cna_graphics_device_get_display_color_space_ext`,
  `cna_graphics_device_set_display_color_space_ext` and
  `cna_graphics_device_supports_display_color_space_ext` in
  `modules/c-api/include/CNA/C/graphics_device.h`.
- **Observed:** all three public C signatures use plain `uint32_t`. The C headers define no
  `CNA_DisplayColorSpace` identity type, no sRGB/scRGB/HDR10 constants and no prose mapping from
  names to ordinals. The implementation privately accepts0..2 by casting to the C++
  `CNA::DisplayColorSpace` enum, so a C caller cannot name a valid nondefault request from the
  public C contract alone.
- **Runtime evidence:** `c_api.graphics.device_capabilities` locally names the three implementation
  ordinals solely to exercise the otherwise unreachable contract. SOFTWARE and OPENGLES3 both
  report current sRGB0, support `1/0/0` for sRGB/scRGB/HDR10, preserve the original state through
  all support probes and rejected changes, and atomically reject ordinal3. All24 controls pass.
- **Expected:** expose a fixed-width `CNA_DisplayColorSpace` typedef plus named sRGB, scRGB and
  HDR10 constants, use that type in the three declarations, and document its defined range.
- **Scope/status:** public C API completeness/documentation defect; runtime behavior is coherent, so
  no expected-failure marker is needed. No CNA source was modified or upstream message sent.
  Observed at clean CNA `f37f8d184`; focused evidence is archived as
  `checkpoint-504-c-device-capabilities-focused-*`. Open.

### CNA-DEMOS-ISSUE-066 -- SOFTWARE ignores GraphicsDevice's direct blend-enable toggle

- **Area/API:** `GraphicsDevice::SetBlendEnabled`,
  `cna_graphics_device_set_blend_enabled` and the SOFTWARE renderer.
- **Observed:** after applying Opaque state, enabling blending and drawing two identical
  half-alpha instances over black, SOFTWARE reads back the unchanged source pixel
  `(255,0,0,128)`. Disabling blending produces the same RGB result. OPENGLES3 instead reads the
  expected accumulated red value near192, then full red after blending is disabled. All other
  bound/user/indexed/instanced, current-effect and depth-toggle controls pass on both renderers.
- **Source evidence:** `SoftwareRenderer::SetBlendEnabled(bool)` in
  `modules/renderers/software/src/SoftwareRenderer2DState.cpp` has an empty body. The EasyGL
  implementation changes native blending and installs source-alpha/inverse-source-alpha factors;
  the public GraphicsDevice method and C route forward directly to these renderer methods.
- **Expected:** enabling blending on SOFTWARE must affect subsequent 3D draws consistently with
  the renderer-neutral GraphicsDevice contract; disabling it must restore overwrite behavior.
- **Scope/status:** renderer-specific runtime defect. `c_api.graphics.draw_routes` retains the
  failed pixel assertion with `knownIssueRenderers={"SOFTWARE"}`; OPENGLES3 must continue to pass.
  No CNA source was modified or upstream message sent. Observed at clean CNA `f37f8d184`; focused
  evidence is archived as `checkpoint-503-c-draw-routes-focused-*`. Open.

### CNA-DEMOS-ISSUE-065 -- C GraphicsDevice reset declaration is separated from its documentation

- **Area/API:** `cna_graphics_device_reset` in
  `modules/c-api/include/CNA/C/graphics_device.h`.
- **Observed:** the complete documentation block for resetting with current presentation parameters
  and the resetting/reset event order appears before the separate, complete documentation block for
  `cna_graphics_device_create`. The create declaration, destroy documentation and destroy
  declaration then intervene before the bare `cna_graphics_device_reset` declaration. A reader or
  documentation generator cannot associate the reset contract with the reset declaration.
- **Runtime/source evidence:** `c_api.graphics.owned_device` calls the bare-declared route on a
  caller-owned device and observes SUCCESS plus the synchronous resetting-then-reset callback pair
  under both SOFTWARE and OPENGLES3. The generated implementation calls canonical `Reset()`; the
  behavior matches the displaced comment. All20 demo controls pass identically with empty stderr.
- **Expected:** place the reset declaration immediately after its existing documentation block, or
  move that block immediately above the existing declaration after destroy. No runtime change is
  indicated by this evidence.
- **Scope/status:** passing public-header documentation-placement defect; no runtime knownIssue
  marker is needed. No CNA source was modified or upstream message sent. Observed at clean CNA
  `d7eff66ee`; focused evidence is archived as `checkpoint-502-c-owned-device-focused-*`. Open.

### CNA-DEMOS-ISSUE-064 -- C Viewport default comment promises the wrong maximum depth

- **Area/API:** `cna_viewport_init` and its public comment in
  `modules/c-api/include/CNA/C/graphics_device.h`.
- **Observed contract conflict:** the comment says the default initializer writes zero position
  and size, `min_depth` zero and `max_depth` one. The generated route constructs the canonical
  `Viewport()` value, whose six fields are all zero. CNA's own `GraphicsDeviceSmoke.c` explicitly
  requires `max_depth == 0` and explains that XNA has no parameterless Viewport constructor, so
  `default(Viewport)` is the all-zero value; only the bounds constructors install depth0..1.
- **Runtime evidence:** `c_api.graphics.viewport` seeds every field with seven, calls the route and
  observes six zero fields under both SOFTWARE and OPENGLES3. Its separate parameterized controls
  observe depth0..1, so the two contracts cannot be confused. All19 demo controls pass identically
  with empty stderr, and the focused61-test regression is green.
- **Expected:** change the public C comment to promise `max_depth` zero for the default value and
  reserve the0..1 statement for the two parameterized constructors. The current implementation
  and tests match XNA value semantics; changing runtime behavior to match the stale sentence would
  reintroduce the old contradiction described by CNA's own test.
- **Scope/status:** passing documentation mismatch; no runtime knownIssue marker is needed. No CNA
  source was modified or upstream message sent. Observed at clean CNA `d7eff66ee`; focused evidence
  is archived as `checkpoint-498-c-viewport-focused-*`. Open.

### CNA-DEMOS-ISSUE-063 -- C synthetic-camera comments promise an unreachable Closed state

- **Area/API:** `cna_camera_create_with_test_backend_ext`, `cna_camera_set_test_state_ext`,
  `cna_camera_set_test_frame_ext` and their comments in
  `modules/c-api/include/CNA/C/devices.h`.
- **Observed contract conflict:** the creation comment says the synthetic camera starts closed,
  and the frame-publishing comment says a null pixel pointer clears the frame and closes the
  camera. Actual creation reports `CNA_CAMERA_STATE_OPENING` with zero dimensions; clearing a
  frame restores the same Opening state. Passing the defined `CNA_CAMERA_STATE_CLOSED` identity to
  the documented state setter returns `CNA_RESULT_INVALID_ARGUMENT`, as does NotSupported.
- **Runtime/source evidence:** `c_api.devices.camera_frames` observes Opening at creation, then
  round-trips Denied, Lost and Opening, proves Closed/NotSupported/undefined states are refused
  without mutation, publishes and acquires an exact synthetic frame, and observes Opening again
  after clearing it. All23 controls pass under SOFTWARE and OPENGLES3 with identical output and
  empty stderr. `CameraTestState` initializes to Opening; `CameraSetTestStateExt` explicitly
  refuses Closed/NotSupported; the null-frame branch explicitly assigns Opening. CNA's own pure-C
  smoke expects the same behavior and explains that the platform now hands out only opened devices.
- **Expected:** update all three public comments to describe the current platform-opened model and
  list the two defined but rejected identities, or restore the documented Closed lifecycle and
  make it reachable consistently. The current words and behavior cannot both be correct.
- **Scope/status:** passing documentation mismatch; no runtime knownIssue marker is needed. The
  real `cna_camera_create` route is deliberately not invoked, so no camera is opened and no
  permission prompt is claimed. No CNA source was modified or upstream message sent. Open; focused
  evidence is archived as `checkpoint-488-c-camera-frames-focused-*`.

### CNA-DEMOS-ISSUE-062 -- Disposed accelerometer and gyroscope retain readable CurrentValue

- **Area/API:** `cna_accelerometer_get_current_value`, `cna_gyroscope_get_current_value`, their
  disposal contracts in `modules/c-api/include/CNA/C/sensors.h`, and
  `Microsoft::Devices::Sensors::SensorBase<T>::getCurrentValueProperty()`.
- **Reproduction:** create each sensor on the game thread, use the documented supported/started
  test switches, inject one deterministic reading, read it successfully, stop and unsubscribe,
  then dispose the sensor. A second Dispose correctly returns `CNA_RESULT_INVALID_STATE`. Before
  destroying the still-owned handle, call its CurrentValue route again.
- **Observed:** both disposed CurrentValue calls return `CNA_RESULT_SUCCESS` and copy the last
  synthetic reading. The new `c_api.devices.sensor_lifecycle` child therefore has exactly two
  failed checks out of40 under both SOFTWARE and OPENGLES3; its other38 controls pass, both stderr
  streams are empty and the complete outputs are identical. Compass and Motion return
  `INVALID_STATE` in the same workflow after their test
  backends are removed; this entry makes no claim about which internal condition produces those
  two refusals.
- **Expected:** both routes document `CNA_RESULT_INVALID_STATE` for a disposed sensor. The
  Accelerometer disposal section is stronger: every other route is said to report the same
  use-after-disposal failure. A retained owned handle must therefore refuse CurrentValue instead
  of exposing cached data.
- **Cause observed in source:** the shared `SensorBase<T>::getCurrentValueProperty()` locks the
  object and checks only `isSupported_`; it never checks `disposed_`. The synthetic support switch
  leaves `isSupported_` true through Accelerometer/Gyroscope disposal, so the cached value remains
  readable. Start/Stop and the second Dispose have separate explicit disposal guards and correctly
  refuse the calls.
- **Suggested repair/status:** make the shared property enforce disposed state before support/data
  access, then retain the existing unsupported-sensor error for live objects. Also audit other
  shared getters against the header's broad post-disposal statement. No CNA source was modified or
  upstream message sent. Open; focused catalog evidence is archived as
  `checkpoint-483-c-sensor-lifecycle-focused-*`.

### CNA-DEMOS-ISSUE-061 -- C default mouse-cursor documentation contradicts the Arrow constructor

- **Area/API:** `cna_mouse_cursor_create_ext`,
  `modules/c-api/include/CNA/C/input_cursor.h`, `CnaCApiInput.cpp` and the public C++
  `MouseCursor.hpp`/`MouseCursor.cpp`.
- **Observed contract conflict:** the C header says this route creates an empty cursor that wraps
  no native cursor and that setting it changes nothing. The route actually allocates
  `MouseCursor()`. That public C++ constructor is documented as creating the default Arrow and its
  implementation leaves `systemShape_` at `ShapeArrow`, `isCustom_` false and `isDisposed_` false;
  `Mouse::SetCursor` therefore selects the platform Arrow.
- **Runtime/source evidence:** the extended `c_api.input.keyboard_mouse` creates the owned default,
  installs it successfully under both renderer selections, disposes it twice and destroys its
  handle. Separate controls show that a borrowed stock Arrow survives Dispose and that a
  texture-derived cursor can be installed before restoring the stock Arrow. The run does not use
  pixel or pointer-shape inspection, so the Arrow identity is established by the direct public
  constructor/setter source path rather than a visual claim.
- **Expected:** describe the C route as an owned default Arrow cursor, matching the public C++
  constructor, or change the implementation and the underlying cursor model if a distinct empty
  cursor is really intended. The current API has no separate empty shape identity.
- **Scope/status:** passing documentation mismatch; cursor creation, installation and lifetime
  succeed, so no runtime knownIssue marker is needed. Focused source hashes, exact child controls
  and both renderer logs are archived under `checkpoint-476-c-input-mouse-cursor-focused-*`.
  Complete476 preserves every prior parent outcome and38/42 failures while only this child grows
  15->32; evidence is under `checkpoint-476-c-input-mouse-cursor-*`. No CNA file was edited and no
  upstream message was sent. Open.

### CNA-DEMOS-ISSUE-060 -- Raised C touch-event documentation contradicts its gesture-only implementation

- **Area/API:** `cna_touch_panel_raise_touch_event_ext`,
  `modules/c-api/include/CNA/C/input_touch.h`, generated `Input.inc`, `TouchPanel.cpp` and
  `SdlInputBridge.cpp`.
- **Observed contract conflict:** one public-header paragraph says the route feeds "gesture
  detection and the event-driven touch map, not the slot array," then says a raised event never
  appears in the snapshot. Those statements cannot both describe CNA's current `GetState`
  fallback, which reads the event-driven map when the slot array is empty.
- **Implementation and runtime evidence:** the generated C route calls only
  `TouchPanel::INTERNAL_onTouchEvent`, which scales the normalized coordinates and feeds the
  gesture detector. The real SDL bridge separately calls `INTERNAL_setTouchState` before that
  gesture call; the C route does not. The extended `c_api.input.gesture_queue` workflow raises a
  deterministic FreeDrag, reads its exact 640x480-scaled position/delta, then observes an empty
  `CNA_TouchState`. This agrees with the latter sentence and CNA's pure-C smoke expectation.
- **Expected:** describe the route as gesture-processing input only, or make it update the
  event-driven state map as the first sentence promises. If behavior changes, clarify identifier,
  pressure and previous-location semantics rather than silently treating the gesture event as a
  complete platform touch event.
- **Scope/status:** documentation mismatch; no renderer, physical-device or gesture-timing defect
  is claimed. The passing demo records actual behavior and needs no runtime knownIssue marker. Both
  focused renderer runs pass64 identical controls with empty stderr; source hashes and the exact
  31->64 comparison are archived under `checkpoint-476-c-input-touch-scaling-focused-*`. No CNA
  file was edited and no upstream message was sent. Complete476 preserves every prior parent and
  unrelated child result while this demo alone grows31->64 controls; evidence:
  `checkpoint-476-c-input-touch-scaling-*`. Open.

### CNA-DEMOS-ISSUE-059 -- Empty MediaQueue active-Song output contradicts its C documentation

- **Area/API:** `cna_media_queue_get_active_song`,
  `modules/c-api/include/CNA/C/media_player.h` and generated `MediaPlayer.inc`.
- **Reproduction:** after the existing C media demo stops playback and safely clears the global
  queue, initialize `out_song` with a still-valid shared Song handle and `out_available` with
  `CNA_TRUE`, then call `cna_media_queue_get_active_song`. Repeat after deliberately storing the
  documented unchecked active index 7 in the empty queue. Both SOFTWARE and OPENGLES3 runs use
  the same public C route; neither dereferences the unavailable result.
- **Observed:** both calls return `CNA_RESULT_SUCCESS`, set availability to `CNA_FALSE` and replace
  the valid sentinel with `CNA_INVALID_HANDLE`. All53demo checks pass identically under both
  renderers with empty stderr. The independently held sentinel Song remains valid and is released
  once through its original handle. Generated `MediaPlayer.inc` explicitly assigns
  `CNA_INVALID_HANDLE` before delegating to the queue lookup.
- **Expected/documented:** the public header says `out_song` receives an owned handle when active
  and is "untouched otherwise." An empty queue is the ordinary unavailable case, so overwriting a
  valid caller value contradicts that statement. Either the route must preserve the output or,
  consistently with its generated implementation and other handle-output routes, the header must
  document initialization to `CNA_INVALID_HANDLE` when unavailable.
- **Controls/scope:** `cna_media_queue_get_at` independently returns INVALID_ARGUMENT for index
  zero on the empty queue and initializes its output to invalid; its header makes no untouched
  output promise. The active-index setter stores 7 exactly as documented, active lookup remains
  unavailable, and the demo restores -1 before cleanup. Collection disposal, idempotence and
  retained-Song lifetime also pass. This is a documentation/implementation mismatch, not a
  renderer, playback, ownership or memory-safety failure. ISSUE-058's unsafe replay is never
  invoked.
- **Evidence/environment:** Linux x86_64, GCC14.2/CMake3.31.6 linux-full, CNA clean at
  `eff31310179168fb4f47258e6226a5234c2140d0`. Exact two-renderer logs, output lists, focused
  review, targeted29-test log, observed CNA sources/hashes and watched3909-input build evidence
  are archived under `build/linux-full/reports/checkpoint-474-c-media-bounds-focused-*`.
- **Status/handling:** open documentation defect. No CNA source was edited and no upstream message
  was sent. The passing demo names the behavior it actually observes and needs no runtime
  knownIssue marker; revisit the wording or expectation after CNA resolves the contract. Complete
  checkpoint474-C-media-lifecycle-bounds preserves all prior full-smoke outcomes and the exact
  seven-check extension under both renderers; evidence: `checkpoint-474-c-media-bounds-*`.

### CNA-DEMOS-ISSUE-058 -- Replaying the active MediaQueue Song uses it after freeing it

- **Area/API:** `MediaPlayer::Play(Song*)`, `MediaPlayer::Queue`, and
  `MediaQueue::ActiveSong`; `modules/media/src/Xna/MediaPlayer.cpp` and `MediaQueue.cpp`.
- **Reproduction:** a private public-API probe appends a caller-created Song to the process-wide
  queue, selects index zero, obtains the valid pointer returned by `ActiveSong`, and passes that
  pointer to `MediaPlayer::Play`. A separate process passes an independently caller-owned Song
  with the same 128-character name and audio path. The probe is linked with AddressSanitizer to
  the unchanged linux-full `libcna.so`; leak detection is disabled because this is a bounded
  input-alias lifetime check, not a leak audit. Both use SDL dummy audio and the same original
  `rising.wav` fixture.
- **Observed:** the caller-owned control copies the name into the queue and exits zero. The
  queue-owned case reaches `Play` with a valid Song, then aborts with an ASan heap-use-after-free.
  The freeing stack is `MediaPlayer::Play` -> `MediaQueue::Clear`; the subsequent read is
  `MediaPlayer::LoadSong` copying `Song::getNameProperty`. The failing process never reaches its
  post-Play check. This is a native memory-lifetime defect, not an acoustic-output discrepancy.
- **Expected:** a Song returned by the public active-queue property is a valid input to the public
  single-Song Play overload. Replacing the queue must snapshot or copy that input before deleting
  its owning entry. The overload has no documented exclusion for the current queue entry.
- **Source mechanism:** `Play(Song*)` saves a previous pointer, clears the queue, then calls
  `LoadSong(song)` and later `PlaySong(song)`. `MediaQueue` owns its entries with `unique_ptr`, so
  Clear destroys the object before either later call can read it. Merely moving the first name/
  handle read before Clear would leave the later `PlaySong(song)` use invalid; a repair needs a
  stable copy or must operate on the newly enqueued entry throughout.
- **Evidence/environment:** Linux x86_64, GCC 14.2, CNA clean at
  `eff31310179168fb4f47258e6226a5234c2140d0`. The caller control exits0; the queue case exits
  by SIGABRT (`-6`) with the exact ASan allocation/free/read stacks. Probe source, build command,
  executable hash, stdout/stderr/exits, source hashes and reviewed result JSON are archived under
  `build/linux-full/reports/checkpoint-473-hierarchy-*queue-replay*`. All36selected project
  binaries stayed unchanged through compile and both runs. This is selected-artifact evidence,
  not an atomic dependency snapshot.
- **Status/handling:** open. No CNA source was edited and no upstream message was sent. The normal
  smoke catalog does not invoke undefined behavior merely to retain a failing assertion; this is
  tracked as a probe-backed issue without a runtime knownIssue marker. Existing MediaPlayer queue
  demos remain valid because they pass caller-owned Songs/collections. Revisit a safe regression
  demo after CNA repairs the lifetime order.

### CNA-DEMOS-ISSUE-057 -- Content staging scavenger warns during normal concurrent lifecycle

- **Area:** CNA content tools, `tools/common/CnaContentStaging.hpp`, consumed by `cna-content`
  and the content-project build. Build-time diagnostic race; no runtime demo knownIssue marker.
- **Original observation:** full470-query, demo source `a699a2d`, completed all seven scheduled
  build operations, then `tools/validate.sh` stopped at its unchanged warning gate before
  metadata validation, CTest or smoke. Content-project output included
  `error: warning: content staging scavenger: cna_content_stage_v1_306788_18dd6c565a238173_0:
  owner metadata is absent or is not a regular file`. Both wrapper exits were1.
  The actual candidate under `/tmp/cna_content_staging_v1` was absent when later inspected.
- **Independent reproduction:** compile `/tmp/cna-staging-concurrency-probe.cpp` with GCC14.2,
  C++23, `-O2 -pthread -I../cna/tools/common`. It directly uses the unchanged CNA tool header.
  One thread creates/destroys1,000 `ContentBuildStagingDirectory` objects in an initially empty
  private parent; another repeatedly calls `ScavengeContentStagingDirectories` there. No malformed
  staging fixture is fabricated and the production staging root is never scanned by the probe.
  Run it only with a fresh private parent; the archived Python driver creates one per trial.
- **Observed:** all three bounded trials reproduce the exact missing-owner-metadata diagnostic,
  respectively3,175/3,079/3,089 times. Each completes1,000 owners with zero owner errors, zero
  reported removals and an empty private parent afterwards. Other observed race diagnostics
  concern vanished directory validation and incomplete/unreadable metadata. Each probe exits1
  deliberately for the reproduced condition, with empty stderr. A sequential control creates
  another1,000 owners and scans while each complete owner is alive and after destruction:
  zero scan errors, zero diagnostics, final parent empty, exit0.
- **Expected:** overlapping normal CNA staging lifecycles should not produce malformed-owner
  warnings merely because another builder is publishing or removing its private staging state.
  Conservative cleanup must remain safe; suspicious unrelated directories should still be
  preserved and reported as appropriate. A warning-free successful consumer build should not
  depend on the scheduling of independent content-tool invocations.
- **Mechanism and limits:** enumeration, directory validation, metadata stat/size/read and lease
  handling are separate filesystem operations. Creation exposes the candidate before its lease
  and atomic metadata publication; destruction removes the metadata/directory. The scanner reads
  metadata before checking age or claiming the lease. The isolated probe establishes diagnostics
  during legitimate concurrent lifecycle operations. The original build's precise interleaving,
  including creation versus teardown, was not observed. No asset corruption or unsafe removal
  is claimed. The older checkpoint437 refusal involved the related vanished-directory message;
  its original logs remain separate and are not retroactively a reproduction of this exact run.
- **Evidence/environment:** Linux x86_64, GCC14.2, CNA tools observed at clean
  `eff31310179168fb4f47258e6226a5234c2140d0`; relevant header snapshots and hashes are archived.
  Original build/refusal/exits and post-refusal selected artifacts:
  `build/linux-full/reports/checkpoint-470-query-initial-staging-refusal-*`.
  Probe sources, executables, original streams, compiler logs, private-root identities and review:
  `build/linux-full/reports/checkpoint-470-query-staging-probe-*`.
  All35selected shipped binaries and four existing assets remained unchanged through the probes.
  This is a source/selected-artifact observation, not an atomic full dependency snapshot.
- **Recurrence on471-punctual:** initial focused build22225, demo HEADc173c11 plus the new
  uncommitted punctual receiver, compiles/links33scheduled operations but exits1 at the
  unchanged diagnostic gate (CMake exit0, gate1). The content-project step reports the same
  missing-owner condition for `cna_content_stage_v1_477930_18dd5445705a3d88_0`. No new demo
  metadata/native/CTest/smoke run follows that refusal. Both relevant tool headers are byte-
  identical to the original independent probe; CNA remains cleaneff3131017 and all3909watched
  build inputs stay unchanged. The exact candidate and its owner file are absent at later
  inspection; creation versus teardown is still unobserved. Original build, source/input
  observations, headers and post-build artifact hashes are preserved separately under
  `build/linux-full/reports/checkpoint-471-punctual-initial-staging-refusal-*`. No directory
  is removed, warning suppressed or CNA source changed. The fresh watched consistency retry
  completes7scheduled operations with CMake0/gate0 and all3909inputs unchanged. Focused
  SW/GL and27targeted CTests then pass (GL8pixels, exactSWskip, actual image inspected).
  This permits native validation but does not resolve the staging race.
- **Recurrence on Focused873:** the first SOFTWARE and OPENGLES3-labelled
  `content.custom.multi_artifact_publication` runs were launched simultaneously while adding a
  separate-process output-lease control. Both programs passed20/20, but the SOFTWARE stderr gained
  the exact `owner metadata is absent or is not a regular file` warning for another live
  `cna_content_stage_v1_*` directory. Immediate serial reruns passed20/20 with matching normalized
  streams and only the eight deliberately exercised compiler diagnostics. This is a real compiler
  workflow recurrence at CNA `d23e139de9e5c15511b6be745de290896a69ee37`, rather than the earlier
  direct header probe; no artifact loss was observed. Evidence:
  `/tmp/checkpoint-873-independent-process-lease-{software,opengles3}{,-serial}.*`.
- **Recurrence on Focused878:** the first full `-j3` build succeeds, but the content-project step
  reports `not a validated current-user staging directory` for a `cna_content_stage_v1_*`
  candidate. This is another conservative validation outcome from the same enumeration/lifecycle
  window; the candidate is absent after the build and no asset loss is observed. An immediate
  unchanged `-j3` retry is warning-free. The retry log is
  `/tmp/checkpoint-878-build-retry.log`; the initial diagnostic was observed in the interactive
  build output and was not redirected to a file.
- **Recurrence on Focused993:** the first `-j3` build compiles and links the bridge-context
  extension, but the content-project step reports `owner metadata is absent or is not a regular
  file` for `cna_content_stage_v1_23_18dc22105b79a260_0`. The candidate is absent on later
  inspection, no artifact loss is observed, and an immediate unchanged `-j3` retry is warning-free.
  The diagnostic was observed in the interactive build output and was not redirected to a file.
- **Recurrence on Focused994:** one of twenty deliberately parallel renderer-labelled direct runs
  completed with its expected26/28 result but added the same missing-owner warning for
  `cna_content_stage_v1_16_18dc3fc3c769aa47_0`. All twenty subsequent serial repetitions produced
  one identical normalized stderr hash without the warning. No artifact loss was observed.
- **Handling:** keep the warning gate and original failure visible, retry the unchanged documented
  full validation after recording the defect, and retain this issue even if scheduling permits a
  clean retry. The eleven OcclusionQuery controls already pass on both renderers independently.
  No sibling source was changed, unknown staging directory deleted or upstream message sent.

### CNA-DEMOS-ISSUE-056 -- File stream position becomes -1 after a short EOF read

- **Area:** sharp-runtime `System::IO::FileStream::Read/getPositionProperty`, exposed through
  CNA StorageContainer file streams and `cna_storage_stream_get_position`.
- **Reproduction:** the extension of `c_api.storage.save_game` is independently
  compiled as `/tmp/cna-storage-eof-probe` using existing C target inputs. It seeks to byte16
  of the original24-byte save and requests12bytes into physically allocated guarded storage.
  A second read at EOF and a position query follow. A separate `/tmp/cna-canonical-eof-probe`
  uses direct FileStream with a unique4-byte temporary file, requesting6bytes, then querying
  position after both short and empty reads. Sources/builders/runners are archived below.
- **Observed:** C returns SUCCESS with position-1 instead of24. Exactly one of31child checks
  fails; all23original controls plus seven additions pass, including exact tail bytes, untouched
  guards, empty EOF output, rewind/full reread, parent release guards and stale-handle output.
  Both requested renderer environments agree. The direct FileStream probe reads4bytes and
  reports-1after both reads instead of4, while every data/guard/rewind/reread control passes.
  Both programs return1 specifically for the defect; stderr is empty. No Game/rendering claim.
- **Expected:** an open file stream remains positioned at the actual end after a short/empty
  EOF read; its position getter must not publish an internal std::fstream failure sentinel as
  a successful byte offset. The C adapter should retain the same canonical stream semantics.
- **Investigation:** FileStream::Read uses fstream.read/gcount and leaves EOF/fail state set;
  getPositionProperty directly calls tellg. CNA's generated C getter forwards that value with
  SUCCESS. Direct reproduction isolates the underlying stream path from the C adapter and
  rules out the demo's source/output buffers. Seek from Begin clears native state and restores
  successful reading; that positive control does not weaken the failing position assertion.
- **Environment/evidence:** Linux x86_64/GCC14.2, existing linux-full runtime artifacts from
  complete466-transfer (CNA59598bfac), observed clean sharp-runtime c20373bae. Later source
  review observes CNA714f9f76a; no atomic input/source snapshot is claimed. All35selected
  shipped binaries remain unchanged through probe builds/runs. Immutable stdout/stderr/JSON,
  direct/C sources, helper scripts, source hashes and exact23+8child review are in
  `build/linux-full/reports/checkpoint-466-transfer-eof-*`. No sibling source changes or
  upstream messages. The catalog extension now retains the failing assertion with knownIssue metadata; its native
  build, both focused selections and relevant CTest WILL_FAIL contract pass as documented
  in checkpoint-467-focused-* (30positive child checks and only the confirmed position failure). The later read-only extension preserves this failure and adds seven positive controls
  (37passes/one failure,38total) on both requested selections, in checkpoint-467-scene-focused-*.
  Reproduce via `cna-demos c_api.storage.save_game --automated --renderer SOFTWARE` after building.

### CNA-DEMOS-ISSUE-001 -- `libcna.so` fails to link with CMake 3.28 (duplicate symbols)

- **Area:** build system -- CNA's shared runtime library (`CNA_SHARED_LIBRARY`,
  `cmake/SharedRuntimeLibrary.cmake`).
- **Demos:** none specifically; it blocks building any consumer.
- **Environment:** Ubuntu 24.04, CMake 3.28.3 (distribution package), GCC 14.2, lld (CNA's
  default linker choice), multi-renderer build
  `OPENGLES3;OPENGL33;SDL_RENDERER;VULKAN;SDL_GPU;SOFTWARE;HEADLESS;STUB`,
  `CNA_CNAEXT=ON`, `CNA_DEVICES=ON`, CNA added with `add_subdirectory()`.
- **Reproduction:**
  1. Configure cna-demos with `/usr/bin/cmake` 3.28.3 and `-DCNA_SHARED_LIBRARY=ON`
     (CNA's own default on Linux with CMake >= 3.27).
  2. `cmake --build <dir> --target cna-demos`.
- **Observed:** the `CNA/libcna.so` link fails with hundreds of
  `ld.lld: error: duplicate symbol: ...` (first ones: `CNA::Internal::Xnb::XnbAssemblyQualifier`,
  `CNA::Internal::Audio::DecodeMsAdpcm`, `CNA::Internal::Media::MediaLibraryIndex::...`,
  `Microsoft::Xna::Framework::Audio::SoundBank::...`). The generated link line lists
  `libcna_runtime.a` and `libcna_content.a` twice inside the first `--whole-archive` group, and
  `libcna_media.a`/`libcna_audio.a` and the input/graphics/renderer archives twice inside later
  groups -- the repetition CMake emits for static libraries in a dependency cycle. Inside
  `--whole-archive` every repetition re-adds every object, hence the duplicates.
- **Expected:** CNA builds on every CMake version its configuration admits. `CNA_SHARED_LIBRARY`
  defaults to ON for CMake >= 3.27, and `cmake_minimum_required` is 3.20.
- **Cross-check:** the identical configuration with CMake 4.4.4 (`pip install cmake`) generates a
  link line with every archive exactly once and links successfully.
- **Likely cause (not verified):** CMake 3.29 changed link-line de-duplication (policy CMP0156),
  so 3.27/3.28 still repeat cyclic static libraries. Which exact CMake version is the first good
  one, and whether CNA's default option set (CNAEXT/DEVICES off, single renderer) also
  reproduces on 3.28, has not been tested.
- **Workaround in cna-demos:** `cmake/CnaDemosCna.cmake` sets `CNA_SHARED_LIBRARY=OFF` (static
  CNA) when CMake is older than 3.29 and the user did not choose a value. With CMake >= 3.29 CNA's
  default is kept. This is a build-mode choice, not a change to CNA.
- **Suggested upstream fix:** raise the `CNA_SHARED_LIBRARY` support gate in CNA's
  `CMakeLists.txt` to the first CMake that de-duplicates correctly, or de-duplicate the
  WHOLE_ARCHIVE list in `cmake/SharedRuntimeLibrary.cmake` so cycle repetition cannot occur.
- **Status:** documented; not yet reported upstream.

### CNA-DEMOS-ISSUE-002 -- `Curve::Evaluate` with Step keys compares the absolute position with 1.0

- **Area:** math -- `Microsoft::Xna::Framework::Curve` (`modules/math/src/Curve.cpp`, the
  `CurveContinuity::Step` branch of the in-range evaluation, around line 288).
- **Demo:** `math.curve.step_continuity` (marked `knownIssue`).
- **Reproduction:**
  ```cpp
  Curve c;
  c.getKeysProperty().Add(CurveKey(1.0f, 10.0f, 0.0f, 0.0f, CurveContinuity::Step));
  c.getKeysProperty().Add(CurveKey(2.0f, 20.0f, 0.0f, 0.0f, CurveContinuity::Step));
  c.Evaluate(1.5f);   // returns 20
  ```
- **Observed:** 20 -- the *next* key's value. The implementation returns `next.Value` whenever
  `position >= 1.0f` and `prev.Value` otherwise, i.e. it compares the absolute curve position with
  1.0 instead of the position's fraction within the segment. Segments that lie entirely below
  position 1.0 behave as documented (keys at 0 and 0.5: `Evaluate(0.25)` returns the first key's
  value), which is why the problem only appears for curves whose keys extend past 1.0.
- **Expected:** 10. The XNA 4.0 documentation of `CurveContinuity.Step` (xna4-spec,
  `Microsoft.Xna.Framework/CurveContinuity.xml`): *"Interpolation cannot be used between this
  CurveKey and the next. Specifying a position between the two points returns this point."*
- **Environment:** platform-independent (pure math); CNA 0.1.0.
- **Notes:** the code has the shape of FNA's `Curve.GetCurvePosition`, which CNA ports; FNA's source
  was not available in this environment to confirm it carries the same test. Whether genuine XNA 4.0
  (its IL) behaves like its documentation has **not** been verified here either; CNA's policy is
  that XNA's measured behaviour wins over FNA. CNA has no unit test that evaluates a Step curve.
- **Status:** open; minimal reproduction above; not reported upstream yet.

### CNA-DEMOS-ISSUE-003 -- `SpriteBatch` does not leave its texture in `GraphicsDevice::Textures[0]`

- **Area:** graphics -- `Microsoft::Xna::Framework::Graphics::SpriteBatch`
  (`modules/graphics/src/Xna/SpriteBatch.cpp`): sprites are submitted through the renderer's own
  sprite path, which never assigns the device's `TextureCollection`.
- **Demo:** `graphics.texture2d.set_data_rectangle` follows the XNA rule (it unsets `Textures[0]`
  before `SetData`) and checks that `SetData` on a bound texture throws -- which CNA implements --
  but it cannot show the situation XNA programmers actually run into.
- **Reproduction:**
  ```cpp
  spriteBatch.Begin();
  spriteBatch.Draw(canvas, Vector2::Zero, Color::White);
  spriteBatch.End();
  device.getTexturesProperty()[0];             // nullptr in CNA
  canvas.SetData(pixels.data(), count);       // succeeds in CNA
  ```
- **Observed:** `Textures[0]` is null after `End` (SOFTWARE and OPENGLES3), so a following
  `SetData` on the last drawn texture succeeds.
- **Expected:** XNA 4.0's SpriteBatch assigns `GraphicsDevice.Textures[0]` for every batch it
  draws (CNA's own comment in `SpriteBatch.cpp` says "XNA assigns Textures[0] after the pass"), so
  after `End` the last sprite texture is still set on the device and `SetData` on it throws
  `InvalidOperationException` ("You may not call SetData on a resource while it is actively set on
  the GraphicsDevice") until the program unsets it. CNA's `Texture::ThrowIfDataTransferResourceInUseEXT`
  implements that throw for textures set explicitly.
- **Impact:** low. XNA programs keep working on CNA (they already unset the texture). Programs
  written against CNA can call `SetData` where XNA would throw, and code that reads
  `Textures[0]` after drawing sprites sees null.
- **Environment:** CNA 0.1.0, Linux, SOFTWARE and OPENGLES3 renderers.
- **Status:** open; observed behaviour above; not reported upstream yet.

### CNA-DEMOS-ISSUE-004 -- Immediate `SpriteBatch` ignores sampler states set between `Draw` calls

- **Area:** graphics -- `SpriteBatch` with `SpriteSortMode::Immediate`
  (`modules/graphics/src/Xna/SpriteBatch.cpp`, `Begin`): the batch's `SamplerState` is passed to the
  sprite renderer once, in `Begin` (`renderer_->SetSamplerState(...)`), and `flushSingle` draws
  each sprite with it without consulting `GraphicsDevice::SamplerStates`.
- **Demo:** `graphics.sprite_batch.immediate_mode.sampler_state` (marked `knownIssue`); its sibling
  `graphics.sprite_batch.immediate_mode.blend_state` passes.
- **Reproduction:**
  ```cpp
  spriteBatch.Begin(SpriteSortMode::Immediate, &BlendState::Opaque, &SamplerState::PointClamp,
                    nullptr, nullptr);
  spriteBatch.Draw(checker, Rectangle(20, 40, 370, 370), Color::White);
  device.getSamplerStatesProperty()(0, SamplerState::LinearClamp);
  spriteBatch.Draw(checker, Rectangle(410, 40, 370, 370), Color::White);   // still point-sampled
  spriteBatch.End();
  ```
- **Observed:** both sprites are point-sampled (only the checker's two colours along a row) on
  SOFTWARE and OPENGLES3. A `BlendState` set on the device between the same two calls *is*
  honoured, so the problem is specific to sampler state.
- **Expected:** XNA 4.0, `SpriteSortMode.Immediate` (xna4-spec `SpriteSortMode.xml`): *"Begin will
  apply new graphics device settings, and sprites will be drawn within each Draw call."* Each Draw
  therefore draws with the device's current state, so the second sprite is filtered linearly --
  changing states or effect parameters between sprites is what Immediate mode is for.
- **Environment:** CNA 0.1.0, Linux, SOFTWARE and OPENGLES3 renderers.
- **Status:** open; minimal reproduction above; not reported upstream yet.

### CNA-DEMOS-ISSUE-005 -- `Color` is polymorphic, so XNA-style custom vertex structs silently break

- **Area:** math/graphics -- `Microsoft::Xna::Framework::Color` derives from
  `PackedVector::IPackedVectorT<UInt32>`, whose `getPackedValueProperty`/`setPackedValueProperty`
  are virtual, so a `Color` object is 16 bytes (vtable pointer + packed value) instead of XNA's
  4-byte value. The other packed vectors share the design. `GraphicsDevice::DrawUserPrimitives`
  with a `VertexDeclaration` passes a caller's non-polymorphic struct through as raw bytes and
  only rejects a struct that is itself polymorphic -- not one that *contains* a polymorphic member.
- **Demo:** `graphics.vertex_declaration.custom_vertex` stores the colour as its packed
  `std::uint32_t` and says why (`restrictions`).
- **Reproduction:**
  ```cpp
  struct MyVertex { Vector3 Position; float Glow; Color Tint; };   // XNA: 20 bytes; CNA: 40
  VertexDeclaration decl{
      VertexElement(0, VertexElementFormat::Vector3, VertexElementUsage::Position, 0),
      VertexElement(12, VertexElementFormat::Single, VertexElementUsage::TextureCoordinate, 1),
      VertexElement(offsetof(MyVertex, Tint), VertexElementFormat::Color, VertexElementUsage::Color, 0)};
  device.DrawUserPrimitives(PrimitiveType::TriangleList, vertices, 0, count, decl);
  ```
- **Observed:** `sizeof(MyVertex)` is 40 and `offsetof(MyVertex, Tint)` 16, while the declaration's
  computed stride is 20; the draw reads every vertex at the wrong stride (and the colour element
  would read the vtable pointer) and renders garbage -- no exception, on SOFTWARE and OPENGLES3.
- **Expected:** XNA's `Color` is a 4-byte struct (its `PackedValue` is the whole object), and
  embedding it in custom vertex structures is the standard XNA pattern (vertex colours alongside
  positions and normals). Porting such code to CNA should work, or fail loudly.
- **Possible directions (for CNA's maintainers):** make `Color` a 4-byte standard-layout type that
  satisfies the packed-vector contract without virtual dispatch; or, at least, reject vertex
  types that are not trivially copyable in the raw `DrawUserPrimitives`/`VertexBuffer::SetData`
  templates (`std::is_trivially_copyable_v` is false for a struct containing a `Color`), turning
  silent garbage into a compile-time error.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux x86_64 (sizes are ABI-dependent but never 4).
- **Status:** open; reproduced as above; not reported upstream yet.

### CNA-DEMOS-ISSUE-006 -- No way to ask whether the renderer applies `MipMapLevelOfDetailBias`

- **Area:** graphics -- `SamplerState::MipMapLevelOfDetailBias` on the OpenGL ES renderers
  (`OPENGLES3`, `WEBGL2`), and the capability API (`CNA::RendererFeature`,
  `GraphicsDevice::SupportsCapability`).
- **Demo:** `graphics.sampler_state.mip_level_control.lod_bias` (skips on GL ES renderers, detected
  through `RendererFeature::ShaderDialectGlslEs` as a proxy).
- **Observed:** on OPENGLES3 a LOD bias of +1 has no effect (copies drawn at 1x, 1/2 and 1/4 size
  read mip levels 0, 1, 2 instead of 1, 2, 3); no exception and no log message. SOFTWARE applies it.
- **Documented by CNA:** `docs/sampler-state-support.md` -- "OpenGL ES has no
  `GL_TEXTURE_LOD_BIAS`", and the parity ledger: "LOD bias is unavailable on GL ES exactly as in
  FNA3D". So the behaviour itself is a known platform limitation, not a defect.
- **The gap:** XNA always honours the bias (Direct3D 9 sampler state), and CNA exposes many
  per-renderer facts through `RendererFeature`/`GraphicsCapability`, but none for this one, so a
  program cannot adapt (for example by biasing in its own shader) without hard-coding renderer
  names.
- **Suggestion:** a `RendererFeature` (or `GraphicsCapability`) entry for sampler LOD bias,
  reported `Unsupported` on GL ES; optionally a one-time warning when a non-zero bias is set there.
- **Same gap, second case:** `PbrEffect` (CNAEXT) is documented in its header as rendered "as an
  untextured/unlit fallback" on SOFTWARE, HEADLESS and the native 2D renderer, and no
  `RendererFeature` reports it either; `graphics.pbr_effect.metallic_roughness` skips by renderer
  name there.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-007 -- `ColorMatrixEffect` is silently ignored by GPU renderers

- **Area:** graphics -- `ColorMatrixEffect` (CNAEXT) passed to `SpriteBatch::Begin` on a GPU
  renderer; EasyGL's `EasyGLSpriteBatchRenderer::SetCustomEffect` stores it and the sprite flush
  draws with the stock sprite shader.
- **Demo:** `graphics.color_matrix_effect.sepia_and_invert` (runs on SOFTWARE; skips elsewhere).
- **Reproduction:** on OPENGLES3,
  ```cpp
  ColorMatrixEffect sepia(device);
  sepia.SetColorMatrix(/* sepia */);
  spriteBatch.Begin(SpriteSortMode::Deferred, &BlendState::Opaque, nullptr, nullptr, nullptr, &sepia);
  spriteBatch.Draw(texture, destination, Color::White);
  spriteBatch.End();
  ```
- **Observed:** no exception; the sprite is drawn with its original colours.
- **Expected:** the header documents ColorMatrixEffect as "unsupported by GPU renderers unless they
  explicitly add the same contract", and its own rationale is that a renderer should not "silently
  ignore" what it cannot do. CNA refuses other unsupported requests explicitly (e.g. compiled
  effects throw `NotSupportedException` where `CompiledEffects` is false). A refusal (or a
  capability flag) would let programs fall back, e.g. to a `ShaderEffect`.
- **Environment:** CNA 0.1.0, Linux, OPENGLES3 (EasyGL) under Xvfb.
- **Status:** open; not reported upstream yet.

### CNA-DEMOS-ISSUE-008 -- Public headers leak unprefixed macros (`YES`, `NO`, `nothing`, ...)

- **Area:** sharp-runtime (CNA's C#-runtime dependency) -- `SharpRuntime/Prop.hpp` defines
  object-like macros with ordinary names at global scope: `YES`, `NO`, `nothing`, `static0`,
  `static1`, `ref0`, `ref1`, `getter0`, `getter1`, `setter0`, `setter1`, `member0`, `member1`,
  `constret0`, `constret1`, `constmet0`, `constmet1`, `IF_0`, `IF_1`, `DDATA`, `DGETTER`, ...
  CNA's public `Microsoft/Xna/Framework/Content/ContentManager.hpp` includes it, so every
  translation unit of a game (through `Game.hpp`) receives them.
- **Demo:** found while writing `input.keyboard.state_values`, whose local variable `nothing`
  (an empty `KeyboardState`) expanded to nothing; the variable was renamed.
- **Reproduction:**
  ```cpp
  #include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
  enum class Answer { YES, NO };        // error: expected identifier before numeric constant
  int main() { int nothing = 0; return nothing; }   // error: expected unqualified-id
  ```
- **Observed:** ordinary identifiers in user code fail to compile with confusing diagnostics
  (GCC 14: "declaration does not declare anything", "expected identifier before numeric
  constant"). An identifier used where an empty token sequence is still valid C++ would change
  meaning silently.
- **Expected:** a library's public headers should not reserve common words; XNA (C#) has no
  macros at all, so ported code freely uses names such as `YES`/`NO` in enums.
- **Suggestion:** prefix the helper macros (e.g. `SHARP_PROP_YES`) or keep them in an internal
  header that public headers do not include, and `#undef` the helpers after use where possible.
- **Environment:** CNA 0.1.0 with sharp-runtime 0.1.0 (`c20373b`), GCC 14.2, Linux.
- **Status:** open (low impact, compile-time only); not reported upstream yet.

### CNA-DEMOS-ISSUE-009 -- `HapticDevice`'s default constructor cannot be used outside CNA

- **Area:** input -- `CNA::Input::HapticDevice` (CNAEXT, `CNA/Input/HapticDevice.hpp`). The class
  holds `std::unique_ptr<CNA::Platform::IPlatformHapticDevice>`, an incomplete type in the public
  header, and declares `HapticDevice() = default;` inline. The destructor and the move operations
  are defined in `HapticDevice.cpp`, where the type is complete; the default constructor is not.
- **Demos:** `input.haptics.rumble`, `input.haptics.effects` hold the device in
  `std::optional<HapticDevice>` (filled from `Haptics::OpenEXT`) and say why in a comment.
- **Reproduction:**
  ```cpp
  #include "CNA/Input/HapticDevice.hpp"
  int main() { CNA::Input::HapticDevice device; return device.IsOpenEXT() ? 1 : 0; }
  ```
- **Observed:** GCC 14: `invalid application of 'sizeof' to incomplete type
  'CNA::Platform::IPlatformHapticDevice'` (`~unique_ptr` is needed by the inline constructor); the
  same for a class with a `HapticDevice` member. The header documents the constructor as
  "Constructs a closed haptic device".
- **Expected:** a closed `HapticDevice` can be created by user code (for example as a member that
  `Haptics::OpenEXT` later fills), like `MouseCursor`, whose default constructor is out of line.
- **Suggested fix (low risk):** declare `HapticDevice();` in the header and define
  `HapticDevice::HapticDevice() = default;` in `HapticDevice.cpp`.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open; not reported upstream yet.

### CNA-DEMOS-ISSUE-010 -- Storage reports errors with C++ standard exceptions, not XNA's types

- **Area:** storage -- `StorageContainer` and `StorageDevice::DeleteContainer`. Argument errors
  (empty names, paths that leave the container) throw `std::invalid_argument`; a failing
  filesystem operation such as `DeleteDirectory` on a non-empty directory lets
  `std::filesystem::filesystem_error` escape. Neither derives from `System::Exception`.
  (Other parts of CNA, for example `StorageContainer::OpenFile` with `FileMode::CreateNew`, do
  throw `System::IO::IOException`; across CNA's modules both conventions occur.)
- **Demos:** `storage.storage_container.path_rules` and `storage.storage_container.directories`
  catch `std::exception`, the common base, and say why.
- **Reproduction:**
  ```cpp
  container->CreateDirectory("replays");
  (void)container->CreateFile("replays/race1.rpl");
  try { container->DeleteDirectory("replays"); }
  catch (const System::IO::IOException&) { /* XNA's type -- not reached */ }
  // terminates with std::filesystem::filesystem_error
  try { (void)container->FileExists(""); }
  catch (const System::ArgumentException&) { /* not reached: std::invalid_argument */ }
  ```
- **Observed:** `filesystem error: cannot remove: Directory not empty [<absolute path>]` and
  `Parameter file must contain a value.` as `std::invalid_argument`.
- **Expected:** XNA reports these as `IOException` and `ArgumentNullException`/`ArgumentException`
  (FNA: `Directory.Delete`, `ArgumentNullException`), and CNA mirrors the .NET hierarchy in
  sharp-runtime (`System::IO::IOException`, `System::ArgumentException`). Code ported from XNA
  that catches those types misses these errors. The containment refusals are a deliberate CNA
  hardening (documented in the source); their exception type could still be `ArgumentException`.
- **Same pattern in `ContentManager`:** `Load<T>` on a disposed manager throws
  `std::runtime_error("ContentManager has been disposed.")`, while XNA (and CNA's own
  `ReadAsset<T>`) throws `ObjectDisposedException`. The focused
  `content.content_manager.several_managers` control now reproduces this as an exact expected
  failure before any asset read or resource creation.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-011 -- `ContentManager::Unload` does not dispose loaded assets

- **Area:** content -- `ContentManager::Unload()` (`ContentManager.cpp`) only clears the cache
  (`loadedAssets_.clear()`). Its header says it "unloads all cached assets and frees the associated
  resources". A texture handle the game still holds stays alive and usable.
- **Demo:** `content.content_manager.unload_disposes` (knownIssue; expected failure).
  `content.content_manager.cache_and_unload` shows the part that works: after `Unload`, the next
  `Load` reads the file again.
- **Reproduction:**
  ```cpp
  ContentManager level(&game.getServicesProperty(), "assets");
  const Texture2D checker = level.Load<Texture2D>("textures/checker");
  level.Unload();
  checker.getIsDisposedProperty();   // false in CNA; true in XNA
  ```
- **Expected (XNA 4.0):** "Disposes all data that was loaded by this ContentManager" -- every
  `IDisposable` asset it loaded is disposed, whether or not the game still references it. XNA
  games rely on it to release a level's textures and sounds in one call; in CNA the memory is only
  released when the game drops every handle (CNA's `Texture2D` copies are handles to one texture).
  A related consequence of the handle model: two `Load` calls return two handles that share the
  texture but not `GraphicsResource::Name`/`Tag`, where XNA returns the same object.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux, SOFTWARE and OPENGLES3.
- **Status:** open; not reported upstream yet.

- **ReadAsset default-recorder extension (438 batch):** `content.content_manager.read_asset_ownership`
  now reads the same original packed XNB with an empty recorder after its successful delegated
  lifetime controls. The nested shared_ptr<Tiles> implements IDisposable and initially contains
  the correct tile bytes; holding it while calling Unload shows that Dispose was not invoked.
  The public ReadAsset header explicitly promises manager recording when the callback is empty.
  ContentReader::RecordDisposable's supplied-callback branch works, but its manager fallback is
  still only a deferred TODO. This is a different route to the same missing disposal behavior,
  not a failure of the caller-owned callback workflow or a new issue identifier.
- **Extension evidence:** `/tmp/cna-438-ownership-{SOFTWARE,OPENGLES3}.{json,log}` in fresh
  Linux GCC 14.2 processes without DISPLAY/WAYLAND_DISPLAY: 16 checks, fifteen passing, one
  initially unlabeled default-recorder failure. All original fourteen checks remain. Explicit
  cleanup follows the assertion and cannot make it pass. No CNA source was edited. Final labeled evidence
  `/tmp/cna-438-ownership-final-{SOFTWARE,OPENGLES3}.{json,log}` retains the same one failure
  with ISSUE-011. Its WILL_FAIL CTest and the other 20 focused tests pass. CNA observed at
  ea41d758d9689cda348887a92a93bbe188864e8b; inspected content modules have no uncommitted
  edits. Full 438 at 973a63a now retains exactly this one new failure on both selections;
  original checkpoint-437 comparisons retain every older failed assertion (29/30 -> 30/31).
  All 451 enabled CTests pass, and the labeled complete evidence/reviews are archived under
  reports/checkpoint-438-*. No graphics/genuine-XNA execution claim.
- **Focused758 extension:** the regenerated ownership fixture now contains two typed fields that
  resolve to one disposable identity. Supplied-callback delivery, caller-side identity
  deduplication, retention after a later parse failure and callback-exception unwinding all pass.
  The empty-callback path still leaves that same nested resource undisposed after Unload, so the
  only failed control remains this issue. Both renderer selections report20/21 and the expected
  SOFTWARE CTest passes. Evidence:`/tmp/checkpoint-758-read-asset-*`.

### CNA-DEMOS-ISSUE-012 -- `ContentManager::Load<T>` does not reliably load through an `OpenStream` override

- **Area:** content -- an ordinary derived `ContentManager::Load<T>` resolves `.xnb`, `.cnb` and
  registered loose-file readers directly on the filesystem (`TryReadAssetBytes`). Only the protected
  `ReadAsset<T>` goes through the virtual `OpenStream`, so an `OpenStream` override is ignored unless
  the derived class can also opt into CNA's protected `RouteLoadsThroughOpenStreamEXT` extension.
  `ResourceContentManager` now opts in, but the routed branch runs before
  `BuiltinContent<T>::Register`; its first `Load<Curve>` in a clean process reaches the resource and
  then rejects the XNB's unregistered built-in `CurveReader`.
- **Demos:** `content.content_manager.open_stream_override` (knownIssue; expected failure): a
  derived manager serving an XNB texture from memory; `ReadAsset` works, `Load` does not.
  `content.content_manager.resource_content_manager` (knownIssue; isolated expected failure): a
  `ResourceContentManager` over a resource family holding a Curve `.xnb`; in a clean process,
  `Load<Curve>("Bounce")` reaches the resource but rejects its unregistered `CurveReader`. The demo
  is process-isolated because an earlier content load otherwise registers the reader globally and
  masks the defect.
- **Reproduction:**
  ```cpp
  class PackContentManager : public ContentManager {
      std::unique_ptr<System::IO::Stream> OpenStream(const std::string& name) override; // memory
  };
  pack.Load<Texture2D>("wheel");   // ContentLoadException: Failed to load image: pack/wheel
                                   // (OpenStream is not called)
  ```
- **Expected (XNA 4.0):** `Load<T>` -> `ReadAsset<T>` -> `OpenStream`, so overriding `OpenStream`
  is the documented way to load from pack files, archives, encrypted or network content, and
  `ResourceContentManager.Load` reads embedded resources.
- **Suggestion:** register `BuiltinContent<T>` before the routed `ReadAsset<T>` call. Also let an
  ordinary derived manager's `Load<T>` fall back to (or prefer) its `OpenStream` override without
  requiring knowledge of a CNA-only opt-in; the tier ladder could remain the base behavior.
- **Environment:** CNA 0.1.0 at `a6e8104d28`, GCC 14.2, Linux.
- **Status:** partially fixed by CNA `c1006b03e` for resource routing; cold built-in reader
  registration and ordinary override routing remain open and are not reported upstream yet.

### CNA-DEMOS-ISSUE-013 -- The content manifest lists the content pipeline's bookkeeping files

- **Area:** content -- `ContentManager::GetContentManifest()` (CNAEXT) reports
  `.cna-content` (`.lock`) and `.cna-content-manifest` (`.json`), the files `cna-content` writes
  into every output directory, as if they were assets.
- **Demo:** `content.content_manager.manifest` prints them (and says so in a comment).
- **Expected:** only loadable assets; tools that walk the manifest (mod loaders, content checks)
  otherwise have to know and filter CNA's internal file names.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-014 -- Before the platform exists, the content root is the working directory

- **Area:** content -- `ContentManager::TitleRootedDirectory()` resolves a relative
  `RootDirectory` against the title path, else against the current platform's base path -- but
  only if a platform already exists (`HasCurrentPlatform()`); otherwise against the working
  directory. Nothing creates the platform until some call needs it (the first `Load` does, as a
  side effect).
- **Found by:** `content.content_manager.manifest`, first written as a console demo, which passed
  when started from `bin/` and failed under CTest (another working directory). It is now a Game
  demo, the context where a platform always exists.
- **Reproduction (console program started from another directory):**
  ```cpp
  ContentManager content(nullptr, "assets");
  content.GetContentManifest().size();      // 0 -- scanned <cwd>/assets
  (void)content.Load<Curve>("curves/bounce"); // works; creates the platform
  content.RefreshContentManifest();
  content.GetContentManifest().size();      // 21 -- now <executable dir>/assets
  ```
- **Expected:** the same root for every call, independent of call order -- XNA resolves content
  under `TitleContainer`/`TitleLocation.Path` (CNA's own comment says so) regardless of what ran
  before. Tools and tests that use a `ContentManager` without a `Game` are affected.
- **Environment:** CNA 0.1.0, GCC 14.2, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-015 -- `Song::FromUri` resolves a relative path against the working directory

- **Area:** media -- `Song::FromUri` (`Song.cpp`) uses a scheme-less path as it is, so a relative
  one is opened relative to the process's working directory. Its comment notes that FNA combines
  a relative URI with `TitleLocation.Path` but says CNA "has no such content-root concept for
  Song" -- yet CNA has `TitleLocation::Path()` (and resolves `ContentManager` and `TitleContainer`
  paths against the title directory).
- **Demos:** every `media.*` demo builds an absolute path from `TitleLocation::Path()` and says why.
- **Reproduction (program started from another directory than its executable's):**
  ```cpp
  Song::FromUri("Rising", "assets/music/rising.wav");
  // throws: Could not find file 'assets/music/rising.wav'.
  ```
- **Expected:** the same resolution as FNA (and as CNA's own content paths): relative to the title
  location, so a game finds its music however it was started (desktop shortcut, launcher, IDE).
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-016 -- Linking `CNA_GamerServices` after another library duplicates CNA in the executable

- **Area:** build system -- `CNA_GamerServices` and `CNA_Net` with a shared CNA
  (`CNA_SHARED_LIBRARY`, `cmake/SharedRuntimeLibrary.cmake`).
- **Demos:** every `gamer_services.*` demo (the first one to exist exposed it); the fix is in
  cna-demos' top-level `CMakeLists.txt`, with a comment pointing here.
- **Environment:** CMake 4.4 (pip), GCC 14, lld, Linux, `CNA_SHARED_LIBRARY=ON` (CNA's default here).
- **Reproduction:** an executable links a static library `framework` that links `CNA`, plus an
  OBJECT library that links `framework` and `CNA::GamerServices` (the usual shape of a project
  with more than one target). Run any program that adds a `GamerServicesComponent` and exits.
- **Observed:** `double free or corruption (!prev)` at exit, in the destructor of
  `SamplerState::PointWrap` (`GraphicsResource::~GraphicsResource`), run once by the executable
  and once by `libcna.so`. `CNA_GamerServices` is not inside `libcna.so` and links CNA's module
  archives (`cna_runtime`, `cna_storage`, ...) directly; CMake places it ahead of `libcna.so` on the
  link line, so the linker takes CNA's graphics, runtime and storage objects from those archives:
  the executable carries a second CNA (3,225 `Graphics` symbols in `cna-demos`) whose static objects
  libcna.so's own references then bind to. Naming the `CNA` umbrella first does not help -- it is
  an INTERFACE library and CMake orders its `libcna.so` after the archives' dependency closure.
  CNA's examples avoid it only because each links exactly `CNA CNA_GamerServices` (their CMake
  comment says the duplicate runtime "aborted every one of these demos at device creation").
- **Workaround in cna-demos:** the executable names the `libcna.so` file itself first on its link
  line (`$<TARGET_FILE:cna_shared>`), so the archives behind it contribute only what libcna.so
  lacks (0 duplicated code symbols afterwards).
- **Expected:** a consumer cannot get two copies of CNA by linking CNA's own targets in any order.
  Possible fixes: link the `CNA` umbrella (not the module archives) from `CNA_GamerServices` and
  `CNA_Net` when CNA is shared, or build them into `libcna.so` too.
- **Additional reproduction (2026-10-11):** `content.custom.xna_pipeline_components` linked
  `CNA::ContentPipelineBuild` and `cna_demos_framework` through the standalone helper on
  Linux/GCC 14/CMake 3.31.6. Its seven pipeline checks passed, then exit aborted in
  TouchPanel::gestures_' deque destructor. GDB shows executable-owned destruction reached
  from libcna.so's global finalizers; nm confirmed TouchPanel::GetState and gestures_ were
  defined in the executable as well. Evidence: `/tmp/cna-xna-pipeline-{gdb,unbuffered}.log`
  and `/tmp/cna-xna-pipeline-duplicate-symbols-{before,after}.txt`.
  The standalone C++ helper now uses the same shared-library-first workaround as the browser.
  The duplicate definitions disappear and both renderer-selected tool runs exit successfully.
  This extends the existing issue to ContentPipelineBuild; no new issue number or expected
  failure hides it. The C API linking path is unchanged.
- **Status:** open (medium: a silent ODR violation that crashes at exit); not reported upstream yet.

### CNA-DEMOS-ISSUE-017 -- `Achievement::EarnedOnline` is always true

- **Area:** gamer services -- `Achievement` (`Achievement.cpp` sets `earnedOnline_(true)` in its
  only constructor; nothing changes it).
- **Demos:** `gamer_services.achievement.award` (expected failure).
- **Reproduction:** sign in a local offline profile (no CNA service), award an achievement from
  the title's offline catalog, read it back with `GetAchievements`:
  ```cpp
  player->AwardAchievement("first_steps");
  // Find(player->GetAchievements(), "first_steps")->getEarnedOnlineProperty() == true
  ```
  Unearned achievements report `true` as well.
- **Expected:** XNA documents `EarnedOnline` as whether the achievement was earned while signed
  in to the online service; an achievement earned by a local offline profile was not (and an
  unearned one was not earned at all). The offline store already records how it was earned
  (`SaveEarnedAchievementEXT`), so the value is available.
- **Environment:** CNA 0.1.0, Linux, offline gamer services.
- **Status:** open (low impact); not reported upstream yet.

### CNA-DEMOS-ISSUE-018 -- System Link socket failures are `std::runtime_error`, not `NetworkNotAvailableException`

- **Area:** net -- `NetworkSession::Create` / `BeginCreate` of a `SystemLink` session
  (`ENetDiscoveryService.cpp`, `ENetHostHandle.cpp` throw `std::runtime_error`; nothing in
  `NetworkSession.cpp` throws `NetworkNotAvailableException`).
- **Demos:** `net.packet.send_and_receive` catches `NetworkNotAvailableException` to report SKIPPED
  (its declared `Need::Network`); with CNA's current exception it reports FAILED instead.
- **Reproduction:** occupy UDP port 61190 (CNA's discovery port) with a socket bound without
  `SO_REUSEADDR`, then create a System Link session:
  ```python
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("0.0.0.0", 61190))
  ```
  ```cpp
  NetworkSession::Create(NetworkSessionType::SystemLink, 2, 4);
  // throws std::runtime_error: "Failed to bind discovery UDP socket."
  ```
- **Expected:** XNA reports a missing or unusable network with
  `NetworkNotAvailableException` (and other transport failures with `NetworkException`), which games
  catch to show "network not available"; a `std::runtime_error` escapes such handlers.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low impact; same family as ISSUE-010); not reported upstream yet.

### CNA-DEMOS-ISSUE-019 -- `Game::ResetElapsedTime` is ignored with a fixed time step

- **Area:** runtime -- `Game::ResetElapsedTime` (`Game.cpp`) sets its flag only when
  `IsFixedTimeStep` is false, and only the variable-step path reads it. The rule is FNA's (its
  `ResetElapsedTime` says the call is "totally ignored" for a fixed time step).
- **Demos:** `game.time_step.reset_elapsed_time` (expected failure).
- **Reproduction:** in a default (fixed 60 Hz) game, stall one `Update` for 200 ms and call
  `ResetElapsedTime()`; count the Updates before the next `Draw`.
- **Observed:** 12 catch-up Updates run before the next Draw (the stall is made up step by step).
- **Expected:** XNA documents the method for exactly this case (recovering from a slow frame
  whose elapsed time is too large to be useful), and Microsoft's XNA GameStateManagement sample
  calls it from its loading screen -- in a game with the default fixed time step -- so that the
  timing does not try to catch up after the long loading frame (paraphrased from the sample's
  comment). The next frame should run one ordinary step.
- **Environment:** CNA 0.1.0, Linux, SOFTWARE and OPENGLES3.
- **Status:** open (medium for games with loading screens: the world jumps ahead after a load);
  not reported upstream yet.

### CNA-DEMOS-ISSUE-020 -- A compiled Song/Video stream is looked up from the manager's root, not beside the asset

- **Area:** content -- `ResolveMediaStreamPathEXT` (`ContentManager.cpp`) resolves a compiled
  `.cnb` Song's or Video's stream reference with `BuildAssetPath(reference)`, i.e. relative to
  the ContentManager's RootDirectory. The reference itself is written relative to the root of
  the content build that compiled it (`music/theme.ogg` for the asset `music/theme`).
- **Demos:** `media.song.compiled_song`, `media.video_player.play_video` (both pass by giving the
  media content build, output to `assets/media`, a ContentManager rooted there).
- **Reproduction:** build a content project with a song `music/theme.ogg` into `assets/media`,
  then `ContentManager(nullptr, "assets").Load<Song>("media/music/theme")`.
- **Observed:** `ContentLoadException: 'media/music/theme' streams 'music/theme.ogg', which was not
  found beside it.` -- the file is beside it (`assets/media/music/theme.ogg`).
- **Expected:** XNA's compiled song/video stores the stream path relative to the `.xnb` and it is
  resolved from the asset's own directory, so a content build can sit anywhere below a manager's
  root (as the message's "beside it" says). Resolving from the asset's directory (or storing the
  reference relative to the asset) would make the load work from any root.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low: one ContentManager per content build avoids it); not reported upstream
  yet.

### CNA-DEMOS-ISSUE-021 -- EasyGL sampled a rendered RenderTargetCube face upside down

- **Area:** graphics, EasyGL renderer (OPENGLES3, OPENGL33). EasyGL kept a render target's GL
  texels bottom-up and corrected 2D sampling per texture unit, but left cube maps out of that
  correction, so a face drawn into with `SetRenderTarget(RenderTargetCube*, CubeMapFace)` was
  sampled flipped vertically by `EnvironmentMapEffect`.
- **Demo:** `graphics.render_target.render_target_cube` (now passing on OPENGLES3 and OPENGL33).
- **Reproduction:** fill the +Z face of a `TextureCube` (SetData) and of a `RenderTargetCube`
  (SpriteBatch into the face) with the same image -- top half red, bottom half black; both read
  back identical with `GetData`. Draw a mirror sphere (EnvironmentMapEffect, FresnelFactor 0) with
  each as its EnvironmentMap, seen from +Z.
- **Observed:** on OPENGLES3 and OPENGL33 the `SetData` cube shows red above the sphere's centre and
  the rendered cube below it. In the demo, cubes orbiting above the sphere are reflected in its
  lower half (0-30 reflected pixels above the centre, ~360 below).
- **Expected:** both cubes sampled the same way, as SOFTWARE, VULKAN and SDL_GPU do (red above the
  centre; ~450 reflected cube pixels above the centre, 0 below): a dynamic environment map must
  reflect the scene upright.
- **Environment:** CNA 0.1.0, Linux, Mesa llvmpipe under Xvfb (GL ES 3.2 and GL 3.3 core).
- **Status:** fixed by CNA `a6e8104d2` (`REMED-GFX-137`, sampling half). Fresh isolated Linux
  Xvfb runs report the green +Z face at the sphere centre and 445/12 reflected cube pixels
  above/below on OPENGLES3, 434/0 on OPENGL33. The runtime known-issue marker was removed.

### CNA-DEMOS-ISSUE-022 -- A compiled glTF with an embedded texture refers to a file the build never writes

- **Area:** content pipeline -- `CNA.GltfImporter` + `CNA.ModelProcessor` (CNB). A glTF whose
  base-colour image is embedded (a PNG in a `bufferView`) compiles into a model that refers to
  `<name>_tex0.png`; the asset's manifest entry lists it under `runtimeReferences`
  (`"textured_crate_tex0.png"`) with no `deploymentFiles`, and no such file is produced.
- **Demos:** `graphics.model.textured_gltf` (expected failure; it draws the run-time import of the
  same file, which works).
- **Reproduction:** compile `content/models/textured_crate.gltf` (tools/gltf_models.py
  `textured_crate`) with `cna_add_content`, then `Content.Load<Model>("models/textured_crate")`.
- **Observed:** `ContentManager: could not load asset 'textured_crate_tex0.png'. ---> Failed to
  load image: <bin>/assets/textured_crate_tex0.png`.
- **Expected:** the model loads with its texture, as the run-time import of the same `.gltf` does
  (`Load<Model>` of a loose `.gltf`): the image is either compiled into the model or written beside
  it where the reference points.
- **Environment:** CNA 0.1.0, Linux, `cna_add_content` (CNB).
- **Status:** open (high for textured glTF models through the pipeline); not reported upstream yet.

### CNA-DEMOS-ISSUE-023 -- A compiled glTF model loses its cameras

- **Area:** content pipeline -- CNB model encoding. `Model::getCamerasEXTProperty()` is filled by
  the run-time glTF import (three cameras for the demo's scene) but is empty for the same scene
  compiled by `cna_add_content`; `CnbModelData` has no camera data.
- **Demos:** `graphics.model.gltf_cameras` (expected failure; it uses the run-time import).
- **Reproduction:** compile `content/models/camera_rig.gltf` (finite perspective, orthographic and
  infinite perspective cameras) and compare `Load<Model>("models/camera_rig")` with the loose copy
  `Load<Model>("models/loose_camera_rig")`.
- **Observed:** 0 cameras from the compiled model, 3 from the loose file. The loose path also
  preserves the camera node's Dolly animation independently through `ModelAnimationsEXT`.
- **Expected:** the same cameras either way (CNA documents `ModelCameraEXT` as imported from the
  glTF scene, `GLTF-317`), or a build warning that the pipeline drops them.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (medium); not reported upstream yet.

### CNA-DEMOS-ISSUE-024 -- `BoundingBox::ToString` and `Ray::ToString` print doubled braces

- **Area:** math -- `BoundingBox.cpp` and `Ray.cpp` write the literal text `{{Min:` ... `}}` and
  `{{Position:` ... `}}`; CNA's own `BoundingBoxTests` pins FNA's string. XNA builds both with
  `string.Format(culture, "{{Min:{0} Max:{1}}}", ...)` / `"{{Position:{0} Direction:{1}}}"`, where
  `{{`/`}}` are format escapes that print one brace each. CNA's `BoundingSphere`, `Plane` and the
  other math types already print single braces.
- **Demos:** `math.formatting.to_string` (expected failure); `design.type_converter.string_round_trip`
  shows the same text through the converters.
- **Reproduction:** `BoundingBox(-Vector3::One, Vector3::One).ToString()`,
  `Ray(Vector3::Zero, Vector3::Forward).ToString()`.
- **Observed:** `{{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}}`,
  `{{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}}`.
- **Expected:** `{Min:{X:-1 Y:-1 Z:-1} Max:{X:1 Y:1 Z:1}}`,
  `{Position:{X:0 Y:0 Z:0} Direction:{X:0 Y:0 Z:-1}}`.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low: text only); not reported upstream yet.

---

### CNA-DEMOS-ISSUE-025 -- Offline, leaderboard columns written after the rating are lost

- **Area:** gamer services -- `LeaderboardWriter::GetLeaderboard` (offline: no CNA service) saves
  an entry to the local store only from the hook `LeaderboardEntry::setRatingProperty` runs
  (`SetOnRatingChangedHookEXT`); writing `LeaderboardEntry::Columns` saves nothing. In XNA the
  row -- rating and columns -- is one write, whatever order the game sets them in.
- **Demos:** `gamer_services.leaderboard.paging` sets its columns before the rating, with a comment
  naming this issue.
- **Reproduction:** in a `LocalWithLeaderboards` game, `entry->setRatingProperty(4200)`, then
  `entry->getColumnsProperty().SetValue("Result", LeaderboardOutcome::Win)`; end the game and
  `LeaderboardReader::Read(board, 0, 10)`.
- **Observed:** the entry has rating 4200 and no columns: `GetValueOutcome("Result")` throws
  `std::out_of_range` (`map::at`).
- **Expected:** the entry reads back `Result = Win`.
- **Environment:** CNA 0.1.0, Linux, offline gamer services.
- **Status:** open; not reported upstream yet.

---

### CNA-DEMOS-ISSUE-026 -- On Linux, a process that searched before a host started does not find it

- **Area:** net -- `ENetDiscoveryService` (System Link discovery). Every process binds one UDP
  socket to port 61190 with `SO_REUSEADDR` (Linux; `SO_REUSEPORT` is set on Apple only) and keeps
  it for the life of the process; a search sends its query by broadcast and to 127.0.0.1, and the
  host answers by unicast to the querying address, port 61190. On Linux a unicast datagram to a
  port shared through `SO_REUSEADDR` reaches only the most recently bound socket, while broadcasts
  reach all of them. When the host's socket is newer than the searcher's, the host's own socket
  receives the answer, and the searcher never does.
- **Demos:** `net.network_session.host_leaves` starts a new player process for each round, with a
  comment naming this issue; `net.network_session.find_and_join` hosts before it starts the
  joiner, which is the order that works.
- **Reproduction:** process A calls `NetworkSession::Find(NetworkSessionType::SystemLink, 1, {})`
  (nothing is hosted yet); then process B on the same machine creates a SystemLink session; A
  calls `Find` again, repeatedly.
- **Observed:** A's searches return no sessions for as long as A runs (B receives its own
  answers). Started after B, a fresh process finds B's session at once. The same delivery rule is
  shown without CNA by two Python UDP sockets bound to one port with `SO_REUSEADDR`.
- **Expected:** A finds B's session -- a lobby screen that searched before a friend on the same PC
  started hosting must find the game (on separate machines the addresses differ and it works).
- **Environment:** CNA 0.1.0, Linux 6.18; affects processes on one machine only.
- **Status:** open; not reported upstream yet. A searcher could take its replies on a socket of its
  own (as the Apple code path's `SearchSocket` does) instead of the shared port.

---

### CNA-DEMOS-ISSUE-027 -- SOFTWARE does not clamp source values above 1 before blending into an 8-bit target

- **Area:** graphics, SOFTWARE renderer -- blending a texel sampled from a floating-point texture
  (`HdrBlendable`) into a `Color` target. Direct3D (XNA) and OpenGL clamp the source colour, the
  destination and the blend factors to [0, 1] before the blend equation when the target is a
  fixed-point format; SOFTWARE uses the raw value, so with `BlendState::AlphaBlend` a source alpha
  of 2 gives the destination a factor `1 - 2 = -1`.
- **Demos:** none fails; found while writing `graphics.render_target.hdr_blendable`, which shows its
  targets with `BlendState::Opaque` (their alpha is a sum of additive draws, not a coverage).
- **Reproduction:** an `HdrBlendable` target filled additively with three `(0.8, 0.5, 0.2, 1)`
  sprites over a cleared black (alpha 1) target, so that alpha reaches 2, 3 and 4 where one, two
  and three sprites overlap; draw it with `SpriteBatch` (default `AlphaBlend`, tint 0.4) over a
  back buffer cleared to `(24, 24, 32)`.
- **Observed (SOFTWARE):** red 58 / 115 / 173 where one / two / three sprites overlap -- 24 below
  OPENGLES3 per unit of alpha above 1 (the background subtracted); blue 0.
- **Expected (OPENGLES3, and XNA):** red 82 / 163 / 245, blue 20 / 41 / 61: the source clamped to 1
  (alpha 1, so the background is replaced).
- **Environment:** CNA 0.1.0, Linux, SOFTWARE renderer.
- **Status:** open (low: only float sources with values above 1 are affected); not reported
  upstream yet.

---

### CNA-DEMOS-ISSUE-028 -- A streaming WaveBank opens a relative path from the working directory

- **Area:** audio -- `WaveBank(AudioEngine*, streamingWaveBankFilename, offset, packetSize)` opens
  the file with a plain file stream (`ParseXwbStreamingHeader`), so a relative name is resolved
  against the working directory; the settings file (`AudioEngine`), the in-memory `WaveBank` and
  the `SoundBank` are read through the title container, relative to the title location. CNA
  follows FNA here (FNA's streaming constructor calls `FAudio_fopen` on the name), so the same
  relative name works for an in-memory bank and fails for a streaming one unless the game runs
  from its own directory.
- **Demos:** `audio.xact.streaming_wave_bank` builds the path from `TitleLocation::Path()`, with a
  comment naming this issue (as ISSUE-015 does for `Song::FromUri`).
- **Reproduction:** run from another directory than the executable's (CTest runs in the build
  tree): `WaveBank(&engine, "assets/xact/music.xwb", 0, 16)` beside
  `AudioEngine("assets/xact/demo.xgs")`.
- **Observed:** `[WaveBank] XWB streaming parse error (assets/xact/music.xwb): XWB: cannot open file
  for streaming`; the bank never becomes prepared and its cues find no wave.
- **Expected:** the streaming bank is found where the other XACT files of the title are.
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open (low; follows FNA); not reported upstream yet.

---

### CNA-DEMOS-ISSUE-029 -- `SoundEffect::FromStream` takes a `std::istream`, not a `System::IO::Stream`

- **Area:** audio API -- XNA's `SoundEffect.FromStream(Stream)` takes the same `Stream` type as
  `Texture2D.FromStream` and `TitleContainer.OpenStream`; CNA's `Texture2D::FromStream` and
  `TitleContainer::OpenStream` use `System::IO::Stream`, but `SoundEffect::FromStream` takes a
  `std::istream&`, and sharp-runtime has no adaptor between the two. A game cannot hand the stream
  `TitleContainer::OpenStream` returns to `SoundEffect::FromStream` (XNA's usual pattern) without
  copying it into a `std::istringstream` first.
- **Demos:** `audio.sound_effect.from_stream` opens the WAV as a `std::ifstream` from
  `TitleLocation::Path()`, with a comment naming this issue.
- **Reproduction:** `SoundEffect::FromStream(*TitleContainer::OpenStream("assets/sounds/tone_440.wav"))`.
- **Observed:** does not compile (no conversion from `System::IO::Stream` to `std::istream&`).
- **Expected:** an overload taking `System::IO::Stream&` (the `std::istream` one can stay as an
  extension).
- **Environment:** CNA 0.1.0.
- **Status:** open (API shape); not reported upstream yet.

---

### CNA-DEMOS-ISSUE-030 -- `SimulatedPacketLoss` drops reliable packets for good

- **Area:** net -- `ENetBackend` applies `NetworkSession::SimulatedPacketLoss` where AppData is
  handed to a local gamer, after ENet's reliable transport has delivered it
  (`ShouldDropForSimulatedLoss`, whatever the packet's `SendDataOptions`). A packet sent with
  `SendDataOptions::Reliable` (or `ReliableInOrder`) is then lost for good. In XNA, reliable delivery
  is guaranteed whatever the connection does -- the simulation makes it late (resent), never lost --
  which is what a game tests with the simulation.
- **Demos:** `net.network_session.simulated_conditions` (expected failure).
- **Reproduction:** two processes in a System Link session; the receiver sets
  `SimulatedPacketLoss = 0.3`; the sender sends one `SendDataOptions::Reliable` packet per frame with a
  sequence number.
- **Observed:** about 70 % of the reliable sequence numbers arrive (68 % in one run), the same share
  as for unreliable packets.
- **Deterministic demo revision (2026-10-11):** separate 24-pair bursts now test baseline,
  200 ms latency, and `SimulatedPacketLoss = 1`. After the final burst settles, the receiver
  restores loss to zero and both peers pump for another second before disconnecting. SOFTWARE
  and OPENGLES3 each received all 24 reliable packets in the first two phases and zero from the
  outage, even after recovery. The source still drops AppData after transport acknowledgement,
  with no reliable retry queued there. Exact authored counts include missing endpoints.
  The earlier short-sample percentage gates for unreliable delivery were demo assumptions,
  not CNA guarantees; their observed fluctuations are not a new CNA issue. This is CNA runtime
  evidence plus source inspection, not a genuine-XNA runtime comparison.
- **Expected:** every reliable packet arrives; only unreliable ones are lost.
- **Environment:** CNA 0.1.0, Linux, System Link (ENet).
- **Status:** open; not reported upstream yet.

- **Demo watchdog correction (2026-10-12):** initial 466 SOFTWARE reached recovery but its
  aggregate 20-second orchestration deadline expired before packet evaluation. The original raw
  failure is archived and was not accepted as an ISSUE-030 reproduction. Unchanged isolated
  runs and the per-phase-watchdog repair both retain all five original child checks: four pass,
  and exactly the reliable-loss assertion fails on both renderers. The repair changes only
  bounded coordination timing, including sign-in waits; packet counts, pacing, latency and
  recovery assertions remain unchanged. This does not add a CNA issue or waive the earlier
  interrupted evaluation. Full repaired466-timing validation passes all479enabled CTests
  and both exact smoke comparisons. Controlled identical8-second phase delays also prove the
  repaired37.41-second exchange retains all five host checks while the original sender times
  out at22.88seconds. Complete466-transfer subsequently preserves the same network checks.

---

### CNA-DEMOS-ISSUE-031 -- glTF STEP and CUBICSPLINE animation samplers play as LINEAR

- **Area:** content, glTF import -- `GltfImportCore`'s `ExtractClips` evaluates each channel
  (STEP, LINEAR or CUBICSPLINE correctly, `EvaluateVec3Channel`/`EvaluateQuatChannel`) but only at
  the bone's own key times, and `KeyframeEXT` has no interpolation mode: `ApplyClipToBonesEXT`
  then interpolates linearly between those keys. A STEP channel slides instead of holding, a
  CUBICSPLINE channel loses its tangents. glTF 2.0 §3.11 defines all three samplers. Both the
  compiled model and the run-time import behave so (the extraction is shared; skinned clips go
  through the same code, not checked separately).
- **Demos:** `graphics.model.clip_interpolation` (expected failure).
- **Reproduction:** `content/models/slide_rig.gltf` and its byte-equivalent generated runtime copy
  `assets/models/loose_slide_rig.gltf` (tools/gltf_models.py: three translation channels keyed
  `x = -2, 0, 2` at `0, 1, 2` s, samplers STEP, LINEAR, CUBICSPLINE with zero tangents). The demo
  loads the first as compiled CNB and the second directly, then calls
  `ApplyClipToBonesEXT(model, clip, 0.25 s)` on both.
- **Observed:** x = -1.5 for all three through both carriers. LINEAR and the common key at one
  second are correct; STEP and CUBICSPLINE each fail their distinct expected values. The loose
  report still says AnimationCount1/ClipCount1 with no warning or loss, and compiled/loose values
  match exactly.
- **Expected:** STEP -2 (held), LINEAR -1.5, CUBICSPLINE -1.6875 (Hermite, flat tangents).
- **Environment:** CNA 0.1.0, Linux.
- **Status:** open; not reported upstream yet. Resampling STEP channels with a hold key just before
  each step, and CUBICSPLINE ones densely (or carrying the mode in the clip), would keep them.
- **Focused runtime confirmation (2026-10-15):** both SOFTWARE and OPENGLES3/Xvfb run twelve
  controls; four fail, exactly the STEP/CUBICSPLINE expectations for compiled and loose carriers.
  The exact known-issue SOFTWARE CTest passes via `WILL_FAIL`. Evidence:
  `/tmp/checkpoint-853-interpolation-carriers-*`. This replaces the previous source-only statement
  that runtime import shared the defect with direct public-path evidence on clean isolated CNA
  `d23e139de9e5c15511b6be745de290896a69ee37`.

---

### CNA-DEMOS-ISSUE-032 -- a System Link join refused for a game in progress reads `SessionNotFound`

- **Area:** net, System Link (`ENetBackend::HandleClientHello`, `NetworkSession::EndJoin`). A host
  whose session is Playing with `AllowJoinInProgress == false` refuses a joiner by disconnecting the
  peer without a reason; the joiner's `EndJoin` turns every failure into
  `NetworkSessionJoinException(SessionNotFound)`. XNA reports `NetworkSessionJoinError::SessionNotJoinable`
  for this case (the error exists for it), and a game shows "game in progress" rather than "not found".
  The online binding (`OnlineSessionBinding`) does distinguish SessionFull and SessionNotJoinable.
- **Demos:** `net.network_session.join_refused` (expected failure).
- **Reproduction:** two processes; the second finds the host's System Link session in its lobby, the
  host calls `StartGame` (AllowJoinInProgress false), the second calls `NetworkSession::Join`.
- **Observed:** `NetworkSessionJoinException` "The requested network session could not be found.",
  `JoinError` SessionNotFound.
- **Expected:** `JoinError` SessionNotJoinable.
- **Environment:** CNA 0.1.0, Linux, System Link (ENet).
- **Status:** open; not reported upstream yet. A refusal message (or an ENet disconnect data value)
  carrying the reason would let the joiner map it.

---

### CNA-DEMOS-ISSUE-033 -- a gamer who joins a System Link game in progress sees the session in Lobby

- **Area:** net, System Link (`ENetBackend`). The host's `ServerWelcomeMessage` carries the roster and
  the session properties but not the session state, and the state is broadcast only when it changes
  (StartGame/EndGame). A joiner admitted while the host is Playing (`AllowJoinInProgress == true`)
  therefore keeps `SessionState == Lobby` and never sees `GameStarted`, while the host is Playing.
- **Demos:** `net.network_session.join_refused` (expected failure; its known issue is ISSUE-032, this
  is the demo's second failing check).
- **Reproduction:** as ISSUE-032, then the host sets `AllowJoinInProgress = true` and the second
  process finds and joins again; it calls `NetworkSession::Update` for 0.5 s.
- **Observed:** the joiner's `SessionState` is Lobby; the host's is Playing.
- **Expected:** Playing (the joiner entered a game in progress).
- **Environment:** CNA 0.1.0, Linux, System Link (ENet).
- **Status:** open; not reported upstream yet. Adding the state to the welcome message would fix it.

---

### CNA-DEMOS-ISSUE-034 -- Stock effect parameters accept a scalar getter on a Vector4

- **Area:** graphics -- `EffectParameter::GetValueSingle` in `EffectParameter.cpp` calls
  `RequireCompiledScalarGetterShape` only for compiled storage. Stock effects use the separate
  float-vector storage path, which returns its first component regardless of the reflected shape.
- **Demo:** `graphics.effect.parameter_values` (expected failure; also exposes ISSUE-035).
- **Reproduction:** take `BasicEffect.Parameters["DiffuseColor"]`, set
  `Vector4(0.2f, 0.4f, 0.6f, 0.8f)`, then call `GetValueSingle()`.
- **Observed:** returns 0.2 on SOFTWARE and OPENGLES3 instead of rejecting the vector-to-scalar
  access. The parameter still reports Vector, Single, one row and four columns.
- **Expected:** `System::InvalidCastException` for this non-array Vector parameter. CNA's own
  `EffectTests.cpp` compiled-effect getter conformance checks require that exception, and
  `docs/software-easygl-parity-ledger.md` SOFTWARE-270 records the recovered Microsoft IL basis.
  The stock path should expose the same public shape contract. Genuine XNA was not run here.
- **Environment:** CNA `2b15506a4`, GCC 14.2, Linux, SOFTWARE and OPENGLES3/private Xvfb.
- **Status:** confirmed compatibility gap, open; not reported upstream. The stock/standalone
  storage leniency is deliberate for C API compatibility, so an upstream fix needs to distinguish
  reflected stock parameters from freely constructed standalone parameters.

---

### CNA-DEMOS-ISSUE-035 -- SkinnedEffect's Bones array has no reflected elements

- **Area:** graphics -- `SkinnedEffect::CacheEffectParameters` constructs `Bones` with
  `AddParam(params, "Bones", MaxBones, 4, Matrix, Single)` and never adds array elements.
- **Demo:** `graphics.effect.parameter_values` (its primary known-issue marker is ISSUE-034).
- **Reproduction:** construct a SkinnedEffect and inspect its `Bones` parameter's Elements,
  RowCount and ColumnCount; write two matrices and read them with GetValueMatrixArray.
- **Observed:** zero elements, 72 rows and four columns. The value-array round trip works, but a
  reflection-driven editor cannot discover the bone entries or their true matrix shape.
- **Expected:** 72 individual matrix elements. CNA's stock shader declares `float4x3 Bones[72]`;
  the stored FNA reflection fixture (`tests/fixtures/compiled-effects/fna-effect-reflection.json`)
  reports 72 elements with RowCount 3 and ColumnCount 4. The old handoff inferred reflected 4x3
  directly from HLSL, which the fixture contradicts. The demo therefore checks the unambiguous
  array cardinality, prints the dimensions, and does not impose an unmeasured genuine-XNA
  row/column convention. Genuine XNA reflection was not executed in this session.
- **Environment:** CNA `2b15506a4`, GCC 14.2, Linux, SOFTWARE and OPENGLES3/private Xvfb.
- **Status:** confirmed stock-effect reflection defect, open (value-array access still works);
  not reported upstream.

---

### CNA-DEMOS-ISSUE-036 -- Linear curve post-loop reads the first key's outgoing tangent

- **Area:** math, `Curve::Evaluate`, `modules/math/src/Curve.cpp` (PostLoop Linear branch).
- **Demo:** `math.curve.loop_types` (marked `knownIssue`).
- **Environment:** Linux, GCC 14.2, CNA `2b15506a48e1a6202c985823131daf5856826f71`.
- **Reproduction:** two keys `(0, 0)` and `(1, 1)`, ComputeTangents(Linear), then set the last
  key's TangentOut to 3 and PostLoop to Linear. Evaluate at position 2.
- **Observed:** 2, from `last.Value + first.TangentOut * (position - last.Position)`.
- **Expected:** 4, from the last key's value 1 plus its outgoing tangent 3 times distance 1.
  The locally available decompilation of genuine XNA 4.0 `Curve.Evaluate` explicitly reads
  `curveKey2.tangentOut`, where curveKey2 is the last key. This is source evidence, not a
  genuine-XNA runtime comparison. CNA's own CurveTest.EvaluatePostLoopLinear currently expects
  the first-key behavior, so passing that upstream test does not establish XNA compatibility.
- **Investigation:** the prior demo expected 2 after computing Linear tangents on a two-key
  ramp. The last outgoing tangent is then zero, so even that original expectation was wrong:
  XNA would hold at 1. The corrected demo sets distinct endpoint slopes explicitly and checks
  the intended contract; first-key incoming extrapolation and negative CycleOffset/Oscillate
  checks remain independent. No CNA source was modified.
- **Related upstream record:** [CNA-BUG-064](https://libcna.com/known-issues/bugs/cna-bug-064.html)
  also describes this post-loop error and the Step error already tracked here as ISSUE-002.
- **Validation:** corrected demo executed on SOFTWARE and OPENGLES3: 16 checks each, exactly
  one failure (actual 2, expected 4). All five curve demos and catalog validation passed their CTest expectations;
  metadata validation reports 404 demos and zero problems. Genuine XNA was not executed.

### CNA-DEMOS-ISSUE-037 -- DynamicSoundEffectInstance header incorrectly says IsLooped is ignored

- **Area:** documentation only, `DynamicSoundEffectInstance::setIsLoopedProperty` in
  `modules/audio/include/Microsoft/Xna/Framework/Audio/DynamicSoundEffectInstance.hpp`.
- **Related demo:** `audio.dynamic_sound_effect_instance.streaming` now checks that true is
  refused and false is accepted. Its failure marker is for the separate disposal issue below.
- **Evidence:** source review at CNA `2b15506a48e1a6202c985823131daf5856826f71`. Both setter
  declarations say the argument is ignored/no effect. The implementation in
  `modules/audio/src/Xna/DynamicSoundEffectInstance.cpp` first checks disposal, then throws
  `System::InvalidOperationException` when the value is true; false is accepted. The rvalue
  overload forwards to this same implementation.
- **Expected:** document that dynamic instances cannot loop: setting true throws, setting false
  is accepted, and using the setter after disposal throws ObjectDisposedException. This agrees
  with the locally available decompilation of genuine XNA 4.0 DynamicSoundEffectInstance.IsLooped.
- **Validation scope:** header, implementation and reference source inspected; the extended
  streaming demo executed both setter values successfully on SOFTWARE and OPENGLES3. Genuine XNA was not run.
  This is a confirmed comment contradiction, not a setter implementation defect. No CNA source
  was changed.
- **Suggested correction:** update both setter comments and their exception documentation.

### CNA-DEMOS-ISSUE-038 -- Dynamic audio getters ignore disposal

- **Area:** `DynamicSoundEffectInstance::getPendingBufferCountProperty` and
  `getIsLoopedProperty`, `modules/audio/src/Xna/DynamicSoundEffectInstance.cpp`.
- **Demo:** `audio.dynamic_sound_effect_instance.streaming` (marked `knownIssue`).
- **Reproduction:** construct the dynamic instance, submit and play PCM, stop it, call Dispose,
  verify IsDisposed, then read PendingBufferCount and IsLooped.
- **Observed:** PendingBufferCount returns 0 and IsLooped returns false; neither getter throws.
  The implementation returns queue size / false without
  checking IsDisposed. The streamed playback, duration conversion and unsupported-request
  checks pass independently; the failing checks concern only the two post-disposal getters.
- **Expected:** both getters throw System::ObjectDisposedException. This is stated in the local
  xna4-spec DynamicSoundEffectInstance.xml and appears explicitly in the genuine-XNA 4.0
  decompilation's two getters. That reference was read, not executed.
- **Investigation:** CNA's PendingBufferCountResetsToZeroAfterDisposeWhilePlaying unit test
  currently expects zero after disposal. Queue cleanup is appropriate, but exposing its count
  through a disposed public object conflicts with XNA; that upstream test does not establish
  compatibility. The same unguarded implementations exist at the CI-pinned CNA revision
  (source checked only). No upstream source was changed.
- **Environment/evidence:** CNA `d23e139de9e5c15511b6be745de290896a69ee37`, Linux GCC 14.2,
  SOFTWARE and OPENGLES3/private Xvfb, SDL dummy audio. Both focused793 runs execute 13 checks:
  eleven pass and exactly these two fail. An independent empty-start control receives exactly
  three canonical BufferNeeded callbacks, safely removes a one-shot listener during dispatch,
  refills synchronously to three pending buffers and remains Playing, so the disposal failures are
  isolated from basic starvation recovery and event mutation. Direct logs:
  `/tmp/checkpoint-793-dynamic-listeners-{SOFTWARE,OPENGLES3}.{log,stderr}`. Targeted CTest,
  catalog and metadata validation pass. No audible validation is claimed.
- **Status:** runtime-confirmed missing disposal guards; not reported upstream.

### CNA-DEMOS-ISSUE-039 -- ContentManager permits changing the root with loaded assets

- **Area:** both ContentManager::setRootDirectoryProperty overloads in
  `modules/content/src/Xna/ContentManager.cpp`.
- **Demo:** `content.content_manager.cache_and_unload` (marked `knownIssue`).
- **Reproduction:** create a manager rooted at assets, Load<Texture2D>("textures/checker"),
  then attempt to set RootDirectory to assets/xnb without first calling Unload.
- **Observed:** the setter accepts the new path. Both overloads directly assign rootDirectory_
  and never check the loaded-asset cache. The old cached asset and the new root can consequently
  describe different content trees under one manager.
- **Expected:** InvalidOperationException and an unchanged root while the manager has loaded
  assets. The local genuine-XNA 4.0 ContentManager decompilation checks loadedAssets.Count > 0
  before assigning the root. This is reference-source evidence; genuine XNA was not executed.
- **Independent behavior:** after Unload, switching to assets/textures and loading checker
  works, creating one new texture. Original first-load/cache/reload checks continue to pass;
  the extended demo has 5 checks with exactly the root-mutation guard failing.
- **Environment/evidence:** CNA `2b15506a48e1a6202c985823131daf5856826f71`, Linux GCC 14.2,
  SOFTWARE and OPENGLES3/private Xvfb. Reports: `/tmp/cna-content-root-{SOFTWARE,OPENGLES3}.json`;
  both execute 5 checks with exactly one failure, and both screenshots were inspected. Build,
  metadata (407/0), targeted CTest and catalog validation pass. The same setter implementation
  exists at the CI pin (source checked only).
- **Status:** runtime-confirmed XNA compatibility gap, not a demo workaround; no CNA source
  changed and no upstream report sent.

### CNA-DEMOS-ISSUE-040 -- Picture and Album thumbnail comments contradict generation paths

- **Area:** documentation only, Picture::GetThumbnail and Album::GetThumbnail in
  `modules/media/include/Microsoft/Xna/Framework/Media/{Picture,Album}.hpp`, plus the
  Album implementation comment described below.
- **Related demos:** `media.media_library.save_picture` (normal Picture thumbnail checks pass;
  its expected-failure marker belongs to the separate short-read ISSUE-041) and
  `media.media_library.album_art` (normal Album thumbnail checks pass).
- **Evidence:** the public comment says no separate thumbnail is generated and that GetThumbnail
  returns the same image as GetImage. The implementation in `modules/media/src/Xna/Picture.cpp`
  instead calls ThumbnailGenerator::CreatePngThumbnail and returns a caller-owned MemoryStream
  containing the generated image when possible; it otherwise falls back to the source FileStream.
- **Reproduction/observed:** save the original 640x320 PNG into a private library, read GetImage
  and verify its bytes equal the source, then save GetThumbnail's stream as a second picture.
  The second Picture reports 128x64, and both files are indexed by a fresh MediaLibrary. The
  demo checks smaller dimensions and preserved aspect ratio, not a fixed XNA thumbnail size.
- **Environment/evidence:** Linux GCC 14.2, current CNA ce74063e6483020835af07c4af1e3614ebc5d962
  (media implementation unchanged from e60fa35f7). The standalone ran under both SOFTWARE and
  OPENGLES3 selections and passed 11 child checks each, including private XDG root verification
  before library access. This is file/stream evidence, not GPU or visual thumbnail validation.
  Reports/logs: `/tmp/cna-save-picture-{SOFTWARE,OPENGLES3}.{json,log}`; parent JSON check count
  is zero for standalone tools, so the checks are in child stdout. No real user media was used.
- **Expected/suggested correction:** document the possible generated thumbnail and original-image
  fallback. The current 128-pixel maximum is an implementation choice, not a universal XNA
  contract. The implementation's useful resizing behavior should not be removed to match the
  stale comment.
- **Additional Album evidence (2026-10-13):** at clean CNA `eff31310179168fb4f47258e6226a5234c2140d0`,
  `Album.hpp::GetThumbnail` repeats the unconditional claim that it returns the same image as
  GetAlbumArt. `Album.cpp::GetThumbnail` instead calls CreatePngThumbnail for file-based art,
  returning generated PNG bytes on success and the original FileStream on failure. Embedded
  art follows a different branch and currently returns GetAlbumArt directly; its implementation
  comment about downscaling decoded bytes also overstates that branch. The album_art demo runs
  the file-based path under both requested renderer selections: GetAlbumArt matches the original
  640x320 PNG bytes, while GetThumbnail returns distinct PNG bytes decoded through public
  SavePicture as 128x64. Both streams remain readable after MediaLibrary destruction. Its eight
  child checks pass identically with parent JSON0; 29 media/CLI CTests pass. This is stream and
  dimension evidence, not visual-quality validation. Embedded-art and fallback branches remain
  source-only. Evidence: `checkpoint-474-art-focused-*`; no runtime knownIssue marker is needed.
- **Status:** confirmed documentation contradictions with native Picture and file-based Album
  evidence. No CNA source changed and no upstream report sent.

### CNA-DEMOS-ISSUE-041 -- SavePicture ignores short reads and saves a zero-padded image

- **Area:** MediaLibrary::SavePicture(std::string, System::IO::Stream*) in
  `modules/media/src/Xna/MediaLibrary.cpp`.
- **Demo:** `media.media_library.save_picture` (marked `knownIssue` for this one failing check).
- **Reproduction:** wrap the valid, original 1,666-byte 640x320 PNG in a public MemoryStream
  subclass whose Read caps each delivery at 13 bytes. Start at position zero. A control reader
  loops over the same fixture and verifies every byte matches the source, so the input is neither
  truncated nor malformed. Pass another such stream to SavePicture and read back its GetImage.
- **Observed:** only one Read occurs, consuming 13/1666 bytes. CNA nevertheless writes the full
  allocated 1,666-byte vector, whose unread tail remains zero, and returns a Picture with 0x0
  decoded dimensions. The saved image differs from the original. Both renderer selections
  reproduce this; ordinary byte-vector and full-memory-stream saves pass independently.
- **Expected:** consume repeated positive short reads until the intended image data is copied;
  detect premature EOF instead of silently writing unread zero-filled storage. The public
  SharpRuntime Stream.hpp contract explicitly permits Read to return fewer than count bytes
  before EOF. This is a stream-consumer defect, not a malformed-image fixture or an assertion
  about a particular genuine-XNA implementation.
- **Investigation:** the overload allocates source->getLengthProperty() bytes, calls Read once,
  ignores its returned count, and forwards the entire vector to the byte-buffer overload. That
  overload writes the bytes and catches the decoder failure, producing width/height zero.
  Partial-read handling is the confirmed defect; nonzero starting positions, unknown-length
  streams and premature EOF have not been exercised by this reproduction.
- **Environment/evidence:** Linux GCC 14.2, CNA ce74063e6483020835af07c4af1e3614ebc5d962
  (media implementation unchanged from e60fa35f7). Initial repro logs/reports:
  `/tmp/cna-save-picture-chunks-{SOFTWARE,OPENGLES3}.{json,log}` (13 checks before adding the
  returned-pointer guard and issue label; exactly one failure in each). Final labeled runs
  `/tmp/cna-save-picture-final-{SOFTWARE,OPENGLES3}.{json,log}` have 14 checks and the same
  single failure; the targeted WILL_FAIL CTest passes. All filesystem work
  used verified private XDG roots; no user pictures were read or written. The normal thumbnail
  path still passed and returned 128x64. No visual/GPU validation is claimed for this file tool.
- **Complete checkpoint:** all 423 demos validated at cna-demos 37ad437. Both original stdout
  logs add exactly this one assertion relative to 422; all older statuses and failures are
  unchanged. SOFTWARE 377/18/25/3, OPENGLES3 398/19/3/3; 436 enabled CTests pass including
  this WILL_FAIL test. Archives/reviews: `build/linux-full/reports/checkpoint-423-*`.
- **Status:** runtime-confirmed CNA bug. The demo keeps the failing byte-preservation check;
  no loop/workaround was added around CNA's SavePicture call, no CNA source changed and no
  upstream report was sent. Follow-up source audit at Focused880 classifies nonzero starting
  position and premature EOF as the same ignored-Read-count mechanism, so they do not get
  duplicate failing controls; SharpRuntime requires a total Length and has no unknown-length
  sentinel. CNA commit `a83673fc53a2c4694e69eb95238a041960dfc560`, four commits after this
  project's pinned `d23e139de9e5c15511b6be745de290896a69ee37`, now loops until the advertised
  length or EOF and resizes to the bytes received. That later fix is source-reviewed here but has
  not been incorporated into or runtime-validated by the Complete871 baseline, so the marker stays.

### CNA-DEMOS-ISSUE-042 -- Root-prefix comments describe an older mesh-cancellation representation

- **Area:** documentation only, SkinningData::SkeletonRootPrefix and
  ApplyBindPoseBoneTransformsEXT comments in `modules/graphics/include/Microsoft/Xna/Framework/Graphics/AnimationPlayer.hpp`.
- **Related demo:** `graphics.model.multiple_skins` (passes; no expected-failure marker).
- **Evidence:** the root-prefix comment describes non-joint ancestry and inverse mesh-node
  cancellation as context retained in SkeletonRootPrefix. The current loose glTF loader instead
  calls BuildSkeleton with scene ancestry alone and parents each skinned ModelMesh to its identity
  model root. The authored mesh node remains in Model.Bones but its placement is not applied to
  the skinned draw. The runtime result is correct; the public storage explanation is stale.
- **Reproduction/observed:** import two one-joint skins sharing a quad and material, with non-joint
  ancestors at X=-0.5/+0.5 and mesh nodes at X=+4/-4. The public prefixes are -0.5/+0.5, not
  -4.5/+4.5. Both mesh parents are the identity model root, while named scene nodes retain +4/-4.
  Animating the left root preserves its ancestor; restoring the right bind pose touches only its
  own effect. Actual red/green positions and the vacated left position pass pixel checks.
- **Environment/evidence:** Linux GCC 14.2, CNA 8b5564f1e81196f3aaee9e9a652b8030e21f4c6c;
  no runtime library change since the preceding checkpoint (external documentation/test cleanup).
  Both SOFTWARE and OPENGLES3 initially passed 17 checks; the final live-bounds extension
  passes 21. Captures were inspected and RGBA-identical; final captures match those inspected
  images: `/tmp/cna-427-multiple-skins-final-{SOFTWARE,OPENGLES3}.{json,log,png}`.
- **Expected/suggested correction:** describe the current loose loader's identity mesh parent and
  ancestor-only prefix, distinguishing the ignored mesh-node transform from a stored inverse
  matrix. Avoid implying that every legacy/custom SkinningData carrier has this same representation.
- **Status:** confirmed public-comment mismatch, not a rendering failure. No CNA source changed,
  no upstream report sent and no failing check suppressed. The final demo independently verifies
  live-bound updates with an intentionally distant palette and restoration; broader skinning
  bounds and multi-influence cases are not implied.

### CNA-DEMOS-ISSUE-043 -- GetTypeReader confuses the target type name with the reader factory key

- **Area:** ContentTypeReaderManager::GetTypeReader/CreateReaderForTargetTypeEXT and the
  constructor association in ContentTypeReader<T>.
- **Demo:** `content.xnb.existing_objects` (two explicit known-failing lookup checks; the other
  eleven checks exercise independent existing-object/raw-payload behavior).
- **Reproduction:** register a capture-free WeaponReader factory under
  `CnaDemos.LoadoutWeaponReader`, as named by the XNB reader table. Its base constructor correctly
  declares target `CnaDemos.LoadoutWeapon`. After the entire two-entry table has been constructed,
  the root reader's Initialize calls GetTypeReader(System::Type::From<WeaponPtr>()).
- **Observed:** GetTypeReader returns null although the weapon reader has already been constructed.
  Explicit CreateReader("CnaDemos.LoadoutWeaponReader") returns the correct reader, whose public
  target name remains `CnaDemos.LoadoutWeapon`. Indexed ReadObject, explicit-reader ReadRawObject,
  existing slot identity/runtime state, null fallback and type-erased tags all work.
- **Expected:** type lookup resolves the reader for that target type without requiring a data
  type's canonical name to equal its reader class/factory name. Those names are also distinct
  throughout CNA's built-in readers (for example System.String versus its StringReader).
- **Cause:** ContentTypeReader<T>'s constructor associates C++ type T with the target name.
  CreateReaderForTargetTypeEXT retrieves that string and passes it directly to CreateReader,
  which indexes factories by reader name. No target-to-reader-name mapping bridges them.
  ContentRuntimeContractTests uses the same string for target and factory key, so its passing
  lookup test does not cover this distinction. The demo now directly executes implicit
  ReadRawObject<T>(existing); it throws before consuming any payload bytes. The explicit-reader
  fallback is guarded by unchanged stream position and the unresolved lookup; partial decode
  failures are rethrown, not hidden.
- **Evidence/environment:** Linux GCC 14.2, both renderer selections WITHOUT DISPLAY or
  WAYLAND_DISPLAY: `/tmp/cna-430-existing-{SOFTWARE,OPENGLES3}.{json,log}` (12 checks, exactly
  one failure before adding the issue label). CNA HEAD 2fda5389d2990b40146d06de0ae0531d05055f24
  with an external C API refactor in progress; the inspected C++ content reader implementation
  is unchanged. No graphics/GPU or genuine-XNA runtime comparison is claimed.
- **Final labeled reproduction:** `/tmp/cna-430-existing-final-{SOFTWARE,OPENGLES3}.{json,log}`
  retains the same single failed check with the issue identifier. Its WILL_FAIL CTest passes;
  full 430 at b113bdc completed with 443 enabled CTests and both complete smoke runs. The only
  added failed assertion versus 428 is this one (25/26 -> 26/27); all older states/check counts
  remain unchanged. Original logs and exact review/hashes: `reports/checkpoint-430-*`.
  Both new external-reference renders pass independently; no failure was hidden by the marker.
- **Extended reproduction (433 batch):** `/tmp/cna-433-existing_objects-{SOFTWARE,OPENGLES3}.{json,log}`
  records 13 checks, exactly two failures, in fresh processes with DISPLAY/WAYLAND_DISPLAY unset.
  The original GetTypeReader assertion is retained verbatim; the new implicit raw-read assertion
  shares ISSUE-043. All eleven other checks still pass. CNA HEAD at build start was 4d774868e9
  with further external C API work; the content lookup source still has the same defect.
  Full 433 at a01ab68 passed all 446 enabled CTests and both complete smoke runs; ORIGINAL
  comparisons to 432 require review exactly for this additional assertion and 12 -> 13 checks.
  Raw totals 26/27 -> 27/28, no older failures removed. Exact evidence reviews/hashes are in
  `reports/checkpoint-433-{SOFTWARE,OPENGLES3}-review.json`.
- **Handling:** retain both failing lookup assertions. A clearly labeled public named-factory
  fallback lets the remaining schema example run; it does not change the target name or add
  a hidden alias registration. No global registry clearing, CNA source edits or upstream report.
  A library fix should preserve the distinction between target identity and reader factory name.

### CNA-DEMOS-ISSUE-045 -- Truncated XNB leaks EndOfStreamException through ContentManager::Load

- **Area:** ContentManager::Load<T>/LoadXnbAsset and the low-level XNB header parser.
- **Demo:** `content.content_manager.asset_precedence`, 14 console checks: thirteen pass and
  one preserves the public exception-type failure. The format-priority behavior itself passes.
- **Reproduction:** a private content directory contains valid same-name CNB, CNJ and native
  rules files. Write a three-byte `rules.xnb` containing `XNB`, then call Load<Rules>("rules")
  on a fresh manager. The valid compiled XNB control first loads the authored value 400.
- **Observed:** Load throws System::IO::EndOfStreamException. Catching only the publicly
  documented ContentLoadException would not recover. No lower-tier loader runs; repairing the
  XNB permits the same manager to load it successfully without Unload. A three-byte CNB is
  correctly refused as ContentLoadException, and its repair/retry also works.
- **Expected:** failure to load an invalid XNB reaches the caller through ContentLoadException,
  consistent with Load<T>'s public @throws contract and its wrapping of ordinary loose-reader
  exceptions. This is a CNA contract comparison, not a genuine-XNA runtime observation.
- **Cause:** Load dispatches directly to LoadXnbAsset, which calls ParseXnbHeader without a
  wrapping boundary. The internal parser intentionally documents EndOfStreamException for a
  truncated header and reads the fourth platform byte after the three magic bytes. That
  low-level behavior escapes through the public content-loading boundary unchanged.
- **Evidence/environment:** `/tmp/cna-434-asset_precedence-{SOFTWARE,OPENGLES3}.{json,log}`,
  fresh Linux GCC 14.2 processes WITHOUT DISPLAY/WAYLAND_DISPLAY, exactly one failure in each
  initial unlabeled 14-check run. CNA observed clean HEAD 9e340c876f6426218e2f82651fcb0b086de25e15
  after build start; external C API refactors are unrelated to the inspected unchanged XNB path.
- **Final labeled reproduction:** `/tmp/cna-434-asset-final-{SOFTWARE,OPENGLES3}.{json,log}`
  keeps all 14 checks and the same one failure with ISSUE-045. Both temporary directories were
  verified removed after exit. Its WILL_FAIL CTest passes along with the other 20 targeted tests.
  Full 434 at 652398e passed all 447 enabled CTests and both complete smoke runs. ORIGINAL
  comparisons to 433 require review only for this new failure and the two passing demo extensions;
  all older failed assertions are retained (27/28 -> 28/29). Original evidence and exact reviews:
  `reports/checkpoint-434-{SOFTWARE,OPENGLES3}*`.
- **Handling:** retain the failing expectation with its issue identifier. The negative probe
  explicitly catches EndOfStreamException solely so independent priority/cache/repair checks
  continue; it does not relabel the exception or fall back to a lower-priority asset. Temporary
  files belong to this demo and are removed by scoped cleanup. No CNA edits or upstream report.


### CNA-DEMOS-ISSUE-046 -- Intermediate XML cannot read its own aliased external TargetType

- **Area:** IntermediateSerializer external-reference writing/reading and namespace resolution.
- **Demo:** `content.custom.intermediate_external_references`; ten child checks, nine passing
  and one native emitted-XML round-trip failure. No graphics or referenced-file loading is needed.
- **Reproduction:** describe Scene and Profile in the same `CnaDemos.External` namespace. Scene
  has an inline Profile preview and two ExternalReference<Profile> wrappers naming one file,
  plus a second file and a null reference. Serialize with an absolute document relocation path,
  then feed that exact XML to Deserialize<Scene> with the same path.
- **Observed:** the writer emits `xmlns:External="CnaDemos.External"` and
  `TargetType="External:Profile"`. Deserialize throws InvalidContentException containing
  `Cannot find type "External:Profile".` The embedded preview loads via the same registered
  Profile serializer. A separately identified control using `CnaDemos.External.Profile` for the
  two TargetType attributes succeeds, including filename relocation and wrapper/null semantics.
- **Expected:** the serializer can deserialize its own valid namespace-qualified external
  references, resolving their target identities in the document's namespace context. This is a
  CNA round-trip contract check, not a genuine-XNA runtime comparison.
- **Cause:** IntermediateWriter::WriteExternalReferences calls SpellDeclaredTypeName, which
  uses an existing namespace alias. IntermediateReader::ReadExternalReferences instead sends
  the raw attribute to IsKnownTypeName and stores it unchanged; CanonicalTypeName does not
  resolve document namespace aliases. ApplyFixups also compares the stored spelling directly
  against the canonical target name. Both lookup and comparison need the resolved identity;
  registering a hidden global alias in the demo would obscure the defect.
- **Evidence/environment:** fresh Linux GCC 14.2 processes WITHOUT DISPLAY/WAYLAND_DISPLAY,
  `/tmp/cna-437-xml-external-{SOFTWARE,OPENGLES3}.{json,log}`. Each initial unlabeled run
  has exactly this failed check and nine passing checks. CNA observed at
  e5014ffaaec2b7f2b018b4dad1f5e299899315f7 after external C API changes; the inspected
  content-pipeline sources have no uncommitted edits. No atomic dependency snapshot is claimed.
- **Handling:** preserve the original XML and failing assertion with ISSUE-046. Catch only the
  diagnosed InvalidContentException so independent controls can run; other failures propagate.
  Canonical-name control XML is explicitly separate and does not replace the original output.
  Referenced filenames are recorded, not imported, compiled or published by this XML tool.
  No CNA code was changed and no upstream report was sent.
- **Final labeled evidence:** `/tmp/cna-437-xml-external-final-{SOFTWARE,OPENGLES3}.{json,log}`
  retains the same one failure with ISSUE-046 and nine passes. Its WILL_FAIL CTest passes with
  the other 20 focused tests. Full 437 at b697786 passed all 450 enabled CTests and both
  renderer smoke runs. Exact original reviews add only this failed assertion (28/29 -> 29/30),
  retaining all older states/check counts/failures. Archives: reports/checkpoint-437-*; original
  streams and final binary hashes are preserved. The first pre-CTest staging-warning attempt
  remains archived separately; it was not an XML demo failure.

### CNA-DEMOS-ISSUE-047 -- AddOutputFile refuses a generated file in the configured output directory

- **Area:** XNA pipeline adapter `ContentProcessorContext::AddOutputFile`, forwarded by
  `XnaBridgeProcessorContext` to canonical `ContentProcessorContext::AddDeploymentFile`.
- **Demo:** `content.custom.xna_pipeline_components` (knownIssue; expected failure).
- **Expected (public CNA contract):** the XNA-shaped header describes related files written by
  the processor beside the compiled asset, to be deployed and cleaned with it. The public build
  environment separately supplies outputDirectory. An existing processor-written file inside
  that configured output directory should be recordable when source/output directories differ.
- **Reproduction:** a real registered route processor reads its label with AddDependency, writes
  `output/patrol.bounds.txt`, closes the file successfully, then calls AddOutputFile with its
  absolute path. The source root is the separate sibling `source/` directory; both directories
  belong to this demo's private temporary tree. The real ContentPipeline::Build refuses the call
  in the Process stage with the correct processor name:
  `deployment source '.../output/patrol.bounds.txt' is outside the primary source root and was
  not explicitly resolved through a named external source capability.`
- **Cause:** the adapter maps the file to a basename deployment destination, but the canonical
  deployment path accepts only source-contained files or exact dependencies previously resolved
  through a named external source capability. It does not recognize the configured output root
  for generated processor files. This entry requests no relaxation for arbitrary outside paths.
- **Controls:** the report's actual bytes contain the expected label and computed bounds before
  registration fails. A separately identified source-contained output layout succeeds and records
  both the processor label dependency and generated deployment file. An explicit following build
  consumes a changed label while retaining identical compiled bounds. This control is not a
  correction of the failed separate-output case and does not prove CLI deployment/cleanup or
  incremental caching. All temporary files are explicitly removed by the demo.
- **Initial evidence:** `/tmp/cna-438-outputs-xna_pipeline_components-{SOFTWARE,OPENGLES3}.{json,log}`,
  fresh displayless Linux GCC 14.2 processes, with the same output-root refusal on both selections.
  These initial logs also retain a separate demo diagnostic mismatch: its own missing-label
  exception omitted the filename expected by the check. ResolveSourceDependency records a
  contained path and does not promise an existence check; this is not a second CNA defect.
  The demo diagnostic now names patrol.label and still verifies Source/LogicalName/Stage/Component.
  Final labeled evidence `/tmp/cna-438-outputs-final-xna_pipeline_components-{SOFTWARE,OPENGLES3}.{json,log}`
  has exactly 12 passing child checks and this one failure per fresh process. All seven original
  checks remain. Both private trees are verified gone. All 22 targeted CTests pass, including
  the expected-failure marker, both audio corrections, all 16 C ABI demos and catalog checks.
  The complete 438-outputs checkpoint at e6327d2 passes all 451 enabled CTests. Both original
  checkpoint-438 comparisons retain every older failed assertion and add exactly this failure
  (30/31 -> 31/32). Full child output has the same twelve passes/one failure as the focused runs;
  evidence and exact reviews are archived under reports/checkpoint-438-outputs-*.
- **Environment:** CNA checkout observed at d751cf46ae41ab49890176530dd564d05482ad32 with external
  C API work ongoing; no atomic source snapshot claimed and no CNA source edited by this task.
  These are CPU pipeline runs under renderer selections, not GPU or genuine-XNA execution.
- **Status:** open; not reported upstream.

### CNA-DEMOS-ISSUE-048 -- Lenient builds report dropped parameters as effective

- **Area:** native `ContentPipeline::Build`, `ContentBuildResult::parameters` and XNA processor
  parameter adapters under `ContentStrictness::XnaCompatible`.
- **Demo:** `content.custom.xna_pipeline_components`; existing ISSUE-047 remains the primary
  knownIssue marker, with this second assertion explicitly labeled and documented.
- **Public contract:** ContentPipeline.hpp documents the returned `parameters` as "Effective
  processor parameters". After warning/default handling removes rejected entries before Process,
  this result should describe what the processor actually received. The caller's original request
  should remain intact; keeping that request is a separate passing check.
- **Reproduction:** register the actual route importer, bounds processor and CNB writer. Request
  `Padding = string("wide")` and `UnknownPadding = double(1.5)` in XnaCompatible mode, then call
  `ContentPipeline::Build`. Compare the returned bytes to a default build, inspect both contextual
  warnings and inspect `result.parameters.Values()`.
- **Observed:** the build succeeds, warns once for each rejected parameter and emits bytes equal
  to the default bounds. Nevertheless, both rejected entries remain in `result.parameters`.
  The current focused demo has 34 passing checks and two failures (this discrepancy plus older
  ISSUE-047) under both displayless SOFTWARE and OPENGLES3 selections. A valid Padding override survives
  removal of the unknown parameter; a missing processor input still refuses the build.
- **Cause verified in source:** RunImportAndProcess computes a local `effective` set and supplies
  it to ContentProcessorContext, but Build later assigns `result.parameters = request.parameters`.
  MakeContentBuildManifestEntry also copies result.parameters. That latter observation does not
  establish a manifest/cache failure: requested configuration may need its own fingerprint identity.
  The fix should distinguish requested versus effective metadata without silently changing cache
  semantics, or explicitly correct the public contract if requested parameters are intended.
- **Evidence:** `/tmp/cna-441-strictness-first-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` retains
  the initial unlabeled failure. Child stdout is authoritative (standalone JSON has zero checks);
  stderr is empty and private directories were independently confirmed absent. Build log:
  `/tmp/cna-441-contracts-build.log` (clean); metadata 441/0. Final labeled evidence is
  `/tmp/cna-441-strictness-final-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`: exactly the same two
  failures, 17 passes, and both returned key names printed. All 13 original checks are retained.
  Original focused streams and inspected source hashes are archived as
  `reports/checkpoint-441-contracts-focused-*`. No incorrect compiled output, GPU behavior or genuine-XNA comparison is claimed.
- **Environment:** Linux, GCC 14.2; observed CNA HEAD
  `69acd67e51b6ec542d8ad429825da3e5bfce89db`. The inspected pipeline header/implementation and
  XnaPipelineBridge.hpp had no uncommitted changes; other external C API work continues. This is
  not an atomic source/dependency snapshot. No CNA source was changed or upstream message sent.
- **Complete checkpoint:** 441-contracts at 2d28a13 passes all 454 enabled CTests and both full
  smoke runs. Exact comparisons with ORIGINAL 441-config add only this assertion (31/32 -> 32/33),
  retain every prior failure and match focused adapter checks. Normalized stderr is unchanged;
  all 30 selected runtime hashes match. Archives: `reports/checkpoint-441-contracts-*`.
- **Status:** open; an observable public metadata/contract discrepancy, not a demo failure.

### CNA-DEMOS-ISSUE-049 -- Build-environment overview incorrectly says built-ins ignore its fields

- **Area:** documentation only, the host-level build-facts overview above ContentBuildEnvironment
  in `modules/content/include/CNA/Content/Pipeline/ContentPipeline.hpp` (around lines 169-176).
- **Related demo:** `content.texture_processor.resize_to_power_of_two` (passes; no expected-failure marker).
- **Mismatch:** the overview says "None of them changes what a built-in CNA component does today."
  Actual TextureProcessor behavior consults targetProfile when processing an XNB-targeted texture.
- **Reproduction/observed:** register the native texture pipeline and ImportAndProcess the original
  100x60 PNG with Xnb output intent, Color format and generateMipmaps=true. Reach refuses it at
  Process/CNA.TextureProcessor with the power-of-two requirement. Changing only targetProfile to
  HiDef produces seven mip payloads at the original size. Explicit resizing instead permits Reach
  with 128x64 and eight levels; disabling mipmaps permits the unscaled Reach case. CNB output
  intentionally has no XNB profile gate and writes/decodes the original seven mip levels.
- **Evidence:** seven checks pass on SOFTWARE and OPENGLES3; the old rendered CNB scene remains
  RGBA-identical before/after and across renderers, and fresh resized base pixels match its loaded
  texture. Original streams/captures/source hashes are `reports/checkpoint-445-profile-focused-*`,
  with baseline captures `checkpoint-445-profile-before-*`; all 33 targeted CTests pass.
  The XNB cases inspect processed values, not written XNB bytes or XNA/Reach hardware execution.
- **Environment:** Linux/GCC 14.2; observed CNA HEAD `30831ba6ca01a8820e84720c12ddd10a04d52051`.
  The inspected pipeline header and texture processor source have no uncommitted changes; other
  C API work continues. This is not an atomic source/dependency snapshot.
- **Expected/suggested correction:** describe the environment as host facts that built-in and
  custom components can consult; remove the obsolete blanket claim. For example, distinguish
  XNB texture-profile policy from CNB processing without implying every field affects every route.
- **Status:** confirmed public-comment mismatch; runtime behavior is correct. No CNA source
  changed, upstream message sent, failing assertion suppressed or new known-failure demo added.

### CNA-DEMOS-ISSUE-050 -- Owner-only C game operations enter the foreign-thread queue

- **Area:** C ABI game-thread affinity; generated `Runtime.inc`, `CnaCApiRuntime.cpp` and
  `CnaCApiDetail.hpp/.cpp` in `modules/c-api/src`.
- **Contract:** public `CNA/C/runtime.h` says driving or destroying the game still refuses a
  foreign caller when foreign-thread dispatch is enabled. Ordinary resource/property calls may
  be queued; this exception protects the owner-driven game lifecycle.
- **Observed:** with dispatch enabled and the owner never pumping, a worker's
  `cna_game_run_frame_ext` or `cna_game_destroy` blocks. Disabling dispatch releases it with
  `CNA_RESULT_INVALID_STATE` and category STATE, rather than its required direct THREAD refusal.
  Both operations reproduce in separate fresh processes under SOFTWARE and OPENGLES3 selections.
  No Game loop, rendering or queued destruction is executed by this probe.
- **Why this proves queuing:** a readiness flag or a delayed thread alone would be inconclusive.
  Here cancellation returns the queue's INVALID_STATE result. Each probe additionally verifies
  that the owner's target step is still 166667 and owner-thread game destruction succeeds.
  A thread that has not entered its call before dispatch is disabled returns a distinct
  inconclusive exit, not this defect result.
- **Reproducer:** `tools/repro_c_api_owner_thread.c` uses only public C APIs and bounded C11
  waits. It never pumps the forbidden call, cancels before joining and terminates the isolated
  process if cancellation cannot release the worker. Exit 0 means direct THREAD refusal;
  exit 1 confirms this defect; exit 2 requires investigation. Build in the existing tree:

  ```bash
  CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv ccache gcc-14 -std=c11 -Wall -Wextra -Werror -pthread \
    -I ../cna/modules/c-api/include tools/repro_c_api_owner_thread.c \
    -L build/linux-full/CNA/modules/c-api -Wl,-rpath,'$ORIGIN/../CNA/modules/c-api' \
    -lcna_c_api -o build/linux-full/bin/cna-repro-c-api-owner-thread
  export CNA_GRAPHICS_RENDERER=SOFTWARE SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11
  env -u WAYLAND_DISPLAY xvfb-run -a build/linux-full/bin/cna-repro-c-api-owner-thread
  env -u WAYLAND_DISPLAY xvfb-run -a build/linux-full/bin/cna-repro-c-api-owner-thread destroy
  ```

- **Evidence:** both initial and final-source probes exit 1 for both operations on both renderer
  selections. Native renderer initialization/selection INFO diagnostics on stderr are preserved. Every final line reports `cancelled=1, result=3, diagnostic=0,
  category=3, disable=0, owner_read=0, ticks=166667, cleanup=0`. Original logs/exits, the initial
  source, selected source/binary hashes and clean-state observations are archived under
  `reports/cna-issue-050-*`. The seven-target incremental rebuild completed without diagnostics.
- **Source analysis:** these generated routes use `CallWithExceptionBarrier`. Their handle
  lookup records a foreign-thread refusal, so that barrier queues the body when dispatch is on.
  `GetCallableGame` adds a callback-reentrancy guard but does not prevent this retry. The same
  generated shape appears on other driving routes; only RunFrameEXT and Destroy were exercised.
  Executing a forbidden queued body through a pump was deliberately not tested.
- **Suggested correction:** encode owner-only routing in the generator/specification and use
  exception translation without foreign-thread retry for these routes. Preserve normal queued
  calls and their error forwarding; do not weaken the declared owner-thread contract.
- **Environment/status:** confirmed on Linux/GCC 14.2. CNA was clean at
  `4ee1e0b553667b6e999248fcefdea729654ee497` before rebuild and after reproduction. This records
  the observed clean source and selected artifacts, not an atomic full dependency snapshot.
  Open; no CNA files changed or upstream message sent.
- **Catalog integration:** `c_api.game.foreign_thread_dispatch` now carries ISSUE-050 and checks
  both owner-only operations without pumping. Focused SOFTWARE/OPENGLES3 runs retain all eighteen
  original passing assertions and produce exactly 28 passes/two labeled failures out of 30.
  Both cancellation results are INVALID_STATE/STATE; the owner remains usable and destroys the
  game. Delayed pre-call scheduling is separately inconclusive. Both stderr streams are empty.
  All 31 focused C/CLI CTests pass, including this expected-failure test (7.31 s). Source/logs:
  reports/checkpoint-446-owner-focused-*. Complete 446-owner at 9e7a27b passes all 459 enabled
  CTests and both full smoke runs. Exact ORIGINAL 446-reentry comparisons add only these two
  failures (32/33 -> 34/35), retain all prior failures and preserve every other demo status/check
  count and C/custom-content child list. Selected hashes and normalized stderr are unchanged.
  Original evidence and strict reviews: reports/checkpoint-446-owner-*.

### CNA-DEMOS-ISSUE-051 -- Host-frame failure contract promises suppressed C exit hooks

- **Area:** public `CNA/C/runtime.h` host-frame contract and the C lifecycle adapter in
  `CnaCApiRuntime.cpp`. This concerns C callback tables, not whether native Game::EndRun executes.
- **Contract:** cna_game_run_frame_ext explicitly says an exit through a failed callback completes
  the run with `exiting`, then `end_run`, returns CALLBACK and reports out_running=false.
- **Observed:** after an ordinary successful variable-time frame, the second Update returns IO
  with a supplied message. RunFrameEXT returns CALLBACK (9) and false, but neither C exit hook
  runs. A later host call returns CALLBACK/false without replay, and destruction returns CALLBACK
  and invokes UnloadContent exactly once. Terminal state and cleanup work; the promised C hook
  sequence is the discrepancy. No rendered-content or error-message preservation claim is made.
- **Reproducer:** tools/repro_c_api_host_failure.c records independent counters and an ordered
  trace. Exit 0 means the promised ILBUDUXEZ sequence; exit 1 confirms ILBUDUZ with both exit
  counters zero; exit 2 is a different/inconclusive result. Build in the existing tree:

  ```bash
  CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv ccache gcc-14 -std=c11 -Wall -Wextra -Werror \
    -I ../cna/modules/c-api/include tools/repro_c_api_host_failure.c \
    -L build/linux-full/CNA/modules/c-api -Wl,-rpath,'$ORIGIN/../CNA/modules/c-api' \
    -lcna_c_api -o build/linux-full/bin/cna-repro-c-api-host-failure
  export CNA_GRAPHICS_RENDERER=SOFTWARE SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11
  env -u WAYLAND_DISPLAY xvfb-run -a build/linux-full/bin/cna-repro-c-api-host-failure
  ```

- **Evidence:** initial and final persistent-source builds reproduce exit 1 on SOFTWARE and
  OPENGLES3. Both produce first `ILBUD`, failed/terminal `ILBUDU`, final `ILBUDUZ`, two updates,
  zero exiting/end_run callbacks and one unload. Renderer INFO stderr is preserved (89/782 bytes)
  and identical between initial/final runs. Original streams, exits, initial source and selected
  source/binary hashes: reports/cna-issue-051-*. GCC 14/C11/-Wall/-Wextra/-Werror builds pass.
- **Cause:** native RunFrameEXT calls OnExiting and EndRun when the run ends. The C adapter routes
  both tables through Invoke, whose existing callbackFailure_ guard suppresses them. UnloadContent
  explicitly opts into runAfterFailure and therefore still runs. This is distinct from ISSUE-050
  and the passing owner-thread reentry guards.
- **Suggested resolution:** reconcile the public promise with the intended failure policy. If
  suppressing ordinary callbacks after failure is intended, document that exception explicitly;
  otherwise provide the promised completion hooks while retaining the original failure and
  exactly-once cleanup. This probe does not establish genuine XNA behavior or recommend changing
  the native Game lifecycle. Other failing callback locations and browser scheduling are untested.
- **Environment/status:** confirmed contract/implementation mismatch on Linux, CNA observed clean
  at 4ee1e0b553667b6e999248fcefdea729654ee497 before/after reproduction. Selected evidence is not an
  atomic dependency snapshot. Open; no CNA edits or upstream message.
- **Catalog integration:** `c_api.game.host_failure` now gives this terminal-failure contract its
  own fresh process, preserving the existing normal host and RunOneFrame scenarios. Focused
  SOFTWARE/OPENGLES3 runs pass nine checks and fail exactly the combined promised C exit-hook
  assertion. The supplied callback diagnostic survives, later host calls remain terminal, unload
  runs once and the released handle is invalid despite destruction returning CALLBACK. Both
  traces are ILBUDUZ and stderr is empty under the demo logger. All32 focused C/CLI CTests pass
  (7.36 s), including the expected-failure marker. Evidence: reports/checkpoint-447-focused-*.
  Complete 447 at 4272ce1 passes all 460 enabled CTests and both full smoke runs. Exact original
  comparisons add only this ID/assertion (34/35 ->35/36), retaining all older statuses/counts/
  failures and C/custom-content child lists. Selected 33 binary/four asset hashes and normalized
  stderr match. Full archives/reviews: reports/checkpoint-447-*. The minimal reproducer remains available.




### CNA-DEMOS-ISSUE-054 -- Cube DDS loading ignores short reads and masks offset truncation

- **Area/API:** `TextureCube::DDSFromStreamEXT`, `modules/graphics/src/Xna/TextureCube.cpp`.
- **Demo:** `graphics.texture_cube.dds_streams`; five controls, three pass and two fail on both
  SOFTWARE and OPENGLES3. The demo is marked with this issue and retains both failing assertions.
- **Fixture and reproduction:** an original 506-byte RGB24 DDS contains six 4x4 faces and authored
  2x2/1x1 levels, with colors varying by face and level and a final nonzero blue byte (255).
  Load it from MemoryStream and verify all eighteen face/level images using public GetData.
  Repeat from position 16 of a stream containing a sixteen-byte prefix plus the same payload.
  Both complete controls pass and return size 4, three levels and opaque Color data.
  Then use a MemoryStream subclass whose Read returns at most seven bytes. Finally remove the
  same last payload byte and try it at positions zero and sixteen.
- **Observed:** the legal short-read stream is read once (seven bytes) and rejected with
  `TextureCube::DDSFromStreamEXT: invalid DDS flags`. Ordinary 505-byte truncation is correctly
  refused with System::FormatException (`truncated DDS stream`). With the sixteen-byte prefix,
  the same truncated payload is accepted and the decoded data no longer matches the full fixture.
  Both renderers produce exactly the same two failed checks and empty stderr after the demo's
  exception catch was corrected.
- **Expected:** honor Stream::Read's documented returned byte count, accumulate positive short
  reads until the intended payload is complete, and reject the same truncated encoded data
  regardless of its starting position. Prefix bytes before Position are not unread DDS bytes.
- **Source investigation:** the method allocates a zero-initialized buffer of total stream Length,
  calls Read once and ignores its return value, then passes the total Length to DecodeDdsCube.
  The prefixed truncated stream has total Length 521 but only 505 unread bytes. The extra zero
  bytes make the decoder's 506-byte bounds check pass and replace the missing nonzero component.
  The decoder itself correctly refuses the unprefixed truncation. This is a stream-consumption
  defect in the cube loader, distinct from SavePicture's analogous ISSUE-041.
- **Demo correction, not a CNA defect:** the first draft caught std::runtime_error for truncation;
  CNA actually throws System::FormatException. Those initial runs stopped after three checks.
  Their source/logs are retained as `checkpoint-459-initial-*`; the catch was corrected explicitly
  before the five-check reproduction, archived as `checkpoint-459-dds-fixed-*` and
  `checkpoint-459-dds-unmarked-source.cpp`. No reader workaround or failure suppression was added.
- **Suggested repair:** read the actual remaining payload to completion (respecting each Read
  result), and give the decoder only initialized bytes actually read; preserve truncation errors.
  No CNA file was modified by this task.
- **Complete checkpoint:** 459 at 5ed8055 passes all 472 enabled CTests and both full smoke runs.
  Original457 comparison adds exactly these two failures (35/36 -> 37/38), retains every older
  failure/status and C child list after six unrelated passing destruction controls. Runtime hashes
  are stable; stderr adds only the separately declared SOFTWARE package skip diagnostic.
- **Environment/status:** Linux, GCC 14.2/CMake 3.31.6 linux-full, explicit X11/Xvfb and dummy audio.
  CNA 2202cb4f1483f7bb54c12c9f9a88c0bf374fe0a6 observed after reproduction, with unrelated
  documentation/content-test changes present. Source observations/hashes are archived separately;
  no atomic build-input snapshot is claimed. Open; no upstream message or genuine-XNA execution.

### CNA-DEMOS-ISSUE-055 -- ShaderEffect clone loses matrices and accepts a disposed original

- **Area/reproduction:** `graphics.shader_effect.clone_state`, OPENGLES3 under isolated X11/Xvfb,
  at least two frames. An explicitly labeled GLSL pair compiles, receives nonidentity World,
  View and Projection, and is cloned through the public owning `Effect*` return. Both programs
  draw one triangle in equal viewports; the original is then disposed while its device stays alive.
- **Observed:** nine checks execute, six pass and exactly three fail. World translation changes
  -0.75 -> 0, View translation 0.25 -> 0 and projection X scale 0.5 -> 1. The clone's triangle is
  larger and differently positioned. Calling Clone on the disposed original returns another
  compiled effect instead of throwing System::ObjectDisposedException. Both GL stderr streams
  are empty. SOFTWARE declares its CustomEffects capability skip; no SOFTWARE clone execution.
- **Expected/contract scope:** Effect::Clone promises an independent clone with current values
  and explicitly throws ObjectDisposedException after disposal. ShaderEffect documents a
  deliberate fresh-program recompilation instead of program sharing; it does not document
  resetting public IEffectMatrices state. Matrix retention is the state-copy interpretation of
  those clone contracts; the disposed-object refusal is explicit. This report makes no claim
  that raw SetUniform values or texture bindings must be copied.
- **Cause observed in source:** ShaderEffect::Clone constructs from the two source strings and
  copies language/labels only. Its three matrix fields retain identity defaults, and it does not
  guard IsDisposed before dereferencing the still-live device and constructing a new effect.
  The demo owns every returned pointer and restores the clone's observed World after its
  independent-mutation check, so it does not silently repair the omitted values before drawing.
- **Positive controls:** original and clone compile in the same explicit language with identical
  sources; changing the clone's World leaves the original unchanged; both draw visible output.
  Public GraphicsResource::Dispose releases the original program while the clone remains valid,
  and the surviving clone's checked pixels stay identical in the next frame.
- **Evidence:** reports/checkpoint-460-initial-* preserves the unmarked runs and both inspected
  800x480 images. Frame one has eight checks; frame two adds the passing survival assertion.
  checkpoint-460-unmarked-clone_state.cpp, -observed-ShaderEffect.cpp/.hpp, -observed-Effect.hpp
  and -focused-observation.json retain source observations. CNA f130abff4 was observed with
  external content-registration changes; these are not atomic build-input snapshots.
- **Suggested repair/status:** guard disposed state and copy World/View/Projection when creating
  the independent program. Open; no CNA source edited, upstream message, physical-GPU or
  desktop-GL runtime claim. Marked focused461 repeats all nine checks and the same three failures;
  its frame-two capture is RGBA-identical to the inspected initial image. Full 461 at a447ad2
  passes 474 enabled CTests and both smokes, retaining every original 459 failure and adding
  exactly these three GL assertions (38 ->41; SOFTWARE37 unchanged). Runtime hashes stay stable;
  stderr adds only separate declared skips and the intentional compiler-diagnostics fixture.

## Resolved

### CNA-DEMOS-ISSUE-053 -- In-flight framework update registration uses an out-of-scope array

- **Area:** external uncommitted `modules/audio/src/Xna/FrameworkDispatcher.cpp` and new
  `CNA/Internal/Audio/FrameworkUpdates.hpp`, during the link-boundary refactor.
- **Reproduction:** documented GCC 14.2/CMake 3.31.6 linux-full incremental build with the shared
  compiler cache and `-j4`, after the first 456 batch build succeeded.
- **Observed:** `FrameworkUpdates::Install` is defined in `CNA::Internal::FrameworkUpdates`,
  but names `frameworkUpdates` without qualification. The array lives in an anonymous namespace
  nested inside `Microsoft::Xna::Framework`; it is not found in the Install function's scope.
  GCC rejects the translation unit, and Ninja exits 1 before the current sentinel-enhanced
  launch-parameter demo can be linked or run. This is a CNA build defect, not a C demo failure.
- **Expected:** both registration and dispatch can access the intended shared update array
  through valid C++ name lookup.
- **Evidence:** `reports/checkpoint-456-failed-build.log`, `-failed-state.json`, archived
  `-failed-FrameworkDispatcher.cpp` and `-failed-FrameworkUpdates.hpp`. HEAD was ced719909 with
  ongoing external audio/media/content/runtime edits; this is not a clean-HEAD reproduction or
  an atomic source snapshot. The successful first build is separately retained as
  `checkpoint-456-initial-build.log`; no new focused runtime or complete 456 checkpoint yet.
- **Capture correction:** direct source review on continuation found that the archived
  failed-FrameworkDispatcher.cpp and failed-state diff already contain the external repair:
  the array is in a translation-unit-global anonymous namespace. The source changed between
  the genuine failed compilation and capture. Comparing current hashes to that repaired capture
  incorrectly prolonged the reported blocker; those prior interpretations are superseded by
  reports/checkpoint-456-evidence-correction.json. The failure log is retained unchanged.
- **Repair observed:** external commit 7d8eba687 contains the global-namespace array.
  HEAD 18d8398af was observed during repair review, with later build-system edits. The existing linux-full retry passed (200 scheduled targets; no compiler diagnostics). Both
  focused launch-parameter runs pass all fifteen child checks with empty stderr. Resolved for
  the exercised build; subsequent full456 validation passed all469enabled CTests and both
  smoke suites with exact prior35/36failed assertions retained. No CNA source edited/reverted, warning suppressed, runtime
  knownIssue marker or upstream message.



### CNA-DEMOS-ISSUE-052 -- In-flight cgltf implementation split breaks the importer build

- **Area:** external uncommitted changes to `modules/content/src/GltfImport/GltfImportCore.cpp`
  and new `CgltfImplementation.cpp`, while refactoring CNA link boundaries/GamerServices.
- **Reproduction:** reuse the documented GCC 14.2/CMake 3.31.6 linux-full tree and run
  `CCACHE_DIR=/rv/cnaccache CCACHE_BASEDIR=/rv cmake --build --preset linux-full -j4`.
- **Observed:** the first 455-batch build exits 1 in GltfImportCore.cpp. Removing
  CGLTF_IMPLEMENTATION from that translation unit makes its existing calls to
  cgltf_component_read_index and cgltf_element_read_float undeclared. Both helpers are static
  inside cgltf.h's implementation section. The same removal loses transitive C memory/string
  declarations used by the local stb implementations (memcpy, memset and related errors).
  The two new material/specular demo objects compile, but the new executable is not linked.
- **Expected:** separating cgltf's public implementation into one translation unit should leave
  the importer self-contained and preserve its sparse-accessor decoding through available APIs
  or deliberately retained local helpers, with explicit standard-header dependencies.
- **Evidence:** `reports/checkpoint-455-initial-build.log`, `-cna-state.json`, archived initial
  GltfImportCore.cpp/CgltfImplementation.cpp/cgltf.h and `-runtime-integrity.json`. HEAD was
  3ee6ae3c83d0eb59551f496d6c85109e377edc81 with ongoing external edits; this is **not** a clean-HEAD
  reproduction or an atomic source snapshot. All 34 selected binaries from complete checkpoint
  453 remained unchanged after the failed build; the new demos have no runtime evidence yet.
- **Resolution:** the external author added explicit string.h and a CgltfInternals.hpp interface;
  CgltfImplementation.cpp supplies wrappers reaching the two implementation-local helpers.
  The same linux-full build then succeeds (111 scheduled targets, known Draco CMP0148 developer
  warning on regeneration, no compiler diagnostics). All 47 relevant C/graphics/CLI CTests pass
  (18.80 s; one capability skip), and both new four-check scenes pass under SOFTWARE/OPENGLES3.
  Original build evidence remains above; repaired-source hashes, retry log and focused results
  are in `reports/checkpoint-455-focused-*`. External uncommitted work is still ongoing, so this
  is not a clean-commit or atomic-input claim. No CNA source edited/reverted or warning suppressed
  by cna-demos. The external cgltf split/fix was subsequently committed as bf9089edc; later clean
  ced719909 was observed during checkpoint 455 runtime. Resolved for the exercised build; no
  runtime knownIssue marker or upstream message.


### CNA-DEMOS-ISSUE-044 -- In-flight C API generation shadows a result-handle parameter

- **Area:** external work in `modules/c-api/src/generated/Cnb.inc`, generated by
  `tools/c-api/generate_routes.py`; found by the cna-demos 432-batch build.
- **Observed:** cna_cnb_cnj_result_copy_asset_type_name receives a handle parameter named result,
  but the generated lookup declared `const CNA_Result result = GetCompiledCnj(result, &object)`.
  The local shadows the parameter even within its initializer, reading an uninitialized local.
  GCC 14.2 with CNA's -Werror rejects both that use and the now-unused handle parameter.
- **Evidence:** `/tmp/cna-432-build.log` records the exact generated source line and diagnostics.
  This was an uncommitted, in-flight extension of CNA's route generation over HEAD 4eed3baa7;
  it is not evidence that clean 4eed3baa7 itself failed this build.
- **Resolution:** the external author changed the generated temporary to routeResult while
  this task was investigating. No CNA source was edited by cna-demos. Resuming the same build
  succeeds without warning/error diagnostics: `/tmp/cna-432-retry-build.log`.
  The new C++ graphics demos subsequently pass on SOFTWARE and OPENGLES3, and all 16 C ABI
  demos pass the focused CTest run (`/tmp/cna-432-targeted-ctest.log`).
- **Scope:** a confirmed transient generator/build defect, not a demo defect or a new expected
  runtime-failure marker. This entry does not claim that every possible generated-name collision
  is covered or that the evolving C API has comprehensive behavioral coverage.
