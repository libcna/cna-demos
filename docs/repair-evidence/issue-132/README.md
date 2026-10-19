# ISSUE-132 preparation (not yet repaired)

Source audit of genuine XNA Color.Lerp/Multiply and PackUtils finds that the
nonfinite differences are part of a broader fixed-point factor contract.
Lerp uses PackUNorm(65536, amount), including ties-to-even factor rounding and
NaN-to-zero. Multiply first scales by65536, clamps the factor to[0,16777215],
then multiplies unsigned channel values and shifts16 bits before saturation.
Merely special-casing NaN Lerp and positive-infinite Multiply would miss finite
factor quantization. Zero channels remain zero even for infinite scale.

`ColorFixedPointOracle.cs` passes all1,782 checks against genuine XNA, comparing
the fixed-point model with genuine methods/operators across nine colors and
eighteen finite/nonfinite factors. Logs: `repair-color-fixed-point-xna{,-build}.log`.
`ColorFixedPointProbe.cpp` reproduces162 exact native failures among the same1782
checks against the unchanged Color implementation; strict warning-as-error build
passes. Reports: `repair-color-fixed-point-native-before{,-build}.log`.
No production or demo change for132 has been made. Next add CNA-native failing
regressions before implementing the measured repair. Existing rounding/packing
controls must remain; do not replace the fixed-point contract with isolated
nonfinite branches.
