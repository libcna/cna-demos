# Third-party notices

cna-demos is licensed under the Microsoft Public License (Ms-PL); see [LICENSE](LICENSE).

## Code

cna-demos contains no third-party source code. It builds against CNA and CNA's dependencies,
which are separate checkouts with their own licences and are not part of this repository:

| Dependency | Licence | Where its notices are |
|---|---|---|
| CNA | Ms-PL (portions derived from FNA, Ms-PL) | `../cna/LICENSE`, `../cna/NOTICE.md`, `../cna/THIRD_PARTY_NOTICES.md` |
| sharp-runtime | MIT | `../sharp-runtime/LICENSE` |
| easy-gl, meta-gl (GL renderers only) | MIT | `../easy-gl/LICENSE`, `../meta-gl/LICENSE` |
| SDL3, SDL3_image, SDL3_mixer, Draco (CNA submodules) | zlib / Apache-2.0 (see each) | `../cna/third_party/*/` |

A binary distribution of cna-demos also distributes CNA and its dependencies and must carry
their notices.

## Assets

Every file under `assets/` and `content/` is original cna-demos content, generated
deterministically by [`tools/generate_assets.py`](tools/generate_assets.py) from code in this
repository. No external images, sounds, fonts or models are included.

| Path | Content | Origin |
|---|---|---|
| `assets/textures/checker.png` | 64x64 checkerboard | generated, original |
| `assets/textures/arrow.png` | 64x64 arrow with an orientation marker | generated, original |
| `assets/textures/gradient.png` | 128x128 colour gradient | generated, original |
| `assets/textures/soft_circle.png` | 64x64 white disc, soft straight-alpha edge | generated, original |
| `assets/textures/sprite_strip.png` | 128x32 strip of four animation frames | generated, original |
| `assets/sounds/tone_440.wav` | 0.5 s 440 Hz sine, 22050 Hz mono PCM16 | generated, original |
| `assets/sounds/tone_660.wav` | 0.25 s 660 Hz sine, 22050 Hz mono PCM16 | generated, original |
| `content/fonts/pixel_font.png` | pixel font (printable ASCII, 5x9 grid at 2x) as an XNA font-texture sheet; glyph shapes in `tools/pixel_font.py` | drawn for cna-demos, original |

When a non-original asset is ever added, list it here with its source, author, licence and the
exact licence text location, and keep the licence file next to the asset.
