# Visual regression references

Approved PNGs live under `<renderer>/<demo-id>.png`. Only deterministic Game demos may have a
reference. References are renderer-specific because rasterisation can legitimately differ.

Run the current pilot under a private X11 server:

```bash
SDL_AUDIO_DRIVER=dummy SDL_VIDEO_DRIVER=x11 env -u WAYLAND_DISPLAY \
  xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/visual_regression.py --exe build/linux-full/bin/cna-demos \
  --output build/linux-full/reports/visual-regression
```

The default comparison is exact. `--tolerance` and `--max-differing-pixels` are available for a
reviewed renderer-specific policy, but the committed pilot needs neither. To approve an updated
reference, inspect the actual image and its demo assertions, record why the image changed, then
replace only that renderer/demo PNG. The tool never updates references itself.
