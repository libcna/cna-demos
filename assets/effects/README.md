# Compiled effect fixture

`CnaConformanceEffect.fx` is CNA's MS-PL conformance shader source. The adjacent
`CnaConformanceEffect.fxb` was compiled from it with Microsoft's XNA-era `fxc /T fx_2_0` tool and
has SHA-256 `2e1fe1dd74d67f4395ae6db4451c1a19ee4478dfe7906f9d665a499e28d2a074`.

The source deliberately contains scalar, vector, matrix, array, structure, texture, annotation,
technique and pass reflection. `c_api.graphics.effect_core` uses the binary through CNA's public C
constructor. Renderers built with compiled-effect support verify the authored graph; default builds
still verify format recognition and explicit capability refusal.
