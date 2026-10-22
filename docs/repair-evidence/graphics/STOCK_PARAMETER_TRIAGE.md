# ISSUE-034/035 stock parameter semantics: next-batch source audit

## Confirmed current baseline

After054 closure, CNA bda0cf902 (product b6ab302cc) / demos bdd0c05:
session80776 completed0 as an observation, not a green suite. Unchanged
graphics.effect.parameter_values runs all31 checks on SOFTWARE/OPENGLES3/
OPENGL33/SDL_GPU:29PASS/2FAIL each. The only failed labels remain scalar getter
refusal034 and72 Bones elements035. Current getters print the first component;
Bones still reports0 elements/72 rows/4 columns. No source drift, peak140460032
bytes/all memory events0. Reports:build/linux-full/reports/
repair-stock-parameters-before-{SOFTWARE,OPENGLES3,OPENGL33,SDL_GPU}.json/.log
and repair-stock-parameters-before-audit.json. No new product code or tests yet.

C API EFFECTS.md distinguishes standalone parameter creation from aliases into
effect collections; do not change standalone storage/array defaults incidentally.
XNA SkinnedEffect.GetBoneTransforms(count) additionally forces each M44 to1 after
reading the parameter array; inspect this distinction before asserting raw Bones
matrix behavior. An affine bone transform test must protect parent/element aliasing,
partial-array writes and clone independence, not just collection cardinality.

## Original read-only audit (before the baseline above)

2026-10-22, while ISSUE-054 external validation ran. At that point no new034/035
runtime result, fix or closure was claimed.

The permanent reproduction is graphics.effect.parameter_values. Current
EffectParameter.cpp applies RequireCompiledScalarGetterShape only inside the
compiledStorage_ branch of GetValueBoolean/GetValueInt32/GetValueSingle. The
stock path still takes the first vector component (or default integer storage).
RequireCompiledScalarGetterShape itself already expresses the relevant rule.

The actual Microsoft XNA4 decompilation in sibling xna4-decomp, under
reference/xna4/decompiled/windows/Microsoft.Xna.Framework.Graphics/
Microsoft.Xna.Framework.Graphics/EffectParameter.cs:1442/1631/1808, applies the
same guard to all three scalar getters: a non-Scalar with no Elements throws
InvalidCastException before calling D3DX. This is source evidence, not a fresh
Windows GPU execution. Preserve array handling rather than assuming all vectors
are rejected, and distinguish CNA's public standalone constructor contract from
stock effect reflection before changing behavior. Review C API coverage/docs.

The existing private CompiledStorage already shares bounded bytes among element
and structure views. Read its matrix accessors and Effect's reflection factory
before inventing another aliasing mechanism: independent vector-backed children
would break parent/element mutation semantics. Preserve the documented standalone
constructor behavior unless separate contract evidence authorizes a change.

Current SkinnedEffect.cpp:118 still creates Bones with MaxBones rows/four columns
and no element collection. Read the existing fna-effect-reflection.json as JSON
(it is a single very long line): its SkinnedEffect Bones entry is the recorded
reference, not an inference from HLSL spelling. Parent/element writes, clone
independence and rendering must stay coherent; merely fabricating72 independent
metadata entries would not be an adequate root-cause repair.

Parsed the stored FNA JSON directly: SkinnedEffect.fxb Bones is Matrix/Single,
parent3 rows/4 columns,72 elements, first child Matrix/Single3x4 with12 value
components. No genuine XNA dimension measurement is claimed from this FNA file.

Next: run the unchanged demo on available renderers after054 closure, inspect
native Effect/SkinnedEffect tests and genuine XNA reference, minimize the public
stock-versus-standalone distinction, then repair a coherent shared root cause.
Do not change the pending066 blend-toggle contract while doing this batch.
