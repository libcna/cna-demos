# =============================================================================
# Demo categories. One line per top-level category (the first segment of every
# demo ID in it). Source files below each directory are found automatically --
# see cmake/CnaDemosCategories.cmake. Categories mirror CNA's own modules.
# =============================================================================

cna_demos_add_category(core)                 # CNA core: version, logging, renderer identity
cna_demos_add_category(math)                 # Vector*, Matrix, Quaternion, bounding volumes, Color, Curve
cna_demos_add_category(game)                 # Game lifecycle, GameTime, components, services, window
cna_demos_add_category(graphics)             # GraphicsDevice, states, buffers, effects, textures, SpriteBatch
cna_demos_add_category(content)              # ContentManager, XNB, CNB, CNJ, glTF
# The custom CNJ loader owns its JSON parsing. Reuse CNA's existing header-only dependency
# helper so cross builds consume portable JSON headers without a host libc include directory.
include("${CNA_ROOT_DIR}/cmake/HeaderOnlyJson.cmake")
cna_link_json_headers(cna_demos_cat_content)
cna_demos_add_category(audio)                # SoundEffect, instances, 3D audio, dynamic sound, XACT
cna_demos_add_category(media)                # Song, MediaPlayer, MediaLibrary, Video
cna_demos_add_category(input)                # Keyboard, Mouse, GamePad, TouchPanel, text input
cna_demos_add_category(storage)              # StorageDevice/StorageContainer, TitleContainer
cna_demos_add_category(platform)             # CNA platform layer and renderer selection
cna_demos_add_category(diagnostics)          # CNA diagnostics and instrumentation
cna_demos_add_category(devices)              # Microsoft.Devices sensors and CNA device extensions
cna_demos_add_category(phone   LIBRARIES CNA::Phone)    # Microsoft.Phone shell and notifications
cna_demos_add_category(design  LIBRARIES CNA::Design)   # Microsoft.Xna.Framework.Design converters
cna_demos_add_category(cnaext  REQUIRES CNA_CNAEXT)      # CNA graphics extensions (modules/graphics-ext)
cna_demos_add_category(net            REQUIRES CNA_ENABLE_NET LIBRARIES CNA::Net CNA::GamerServices)
cna_demos_add_category(gamer_services REQUIRES CNA_ENABLE_NET LIBRARIES CNA::GamerServices CNA::Net)  # Net: leaderboards are written in sessions
