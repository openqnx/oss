# Run-level tests, group g9-media-gfx: image formats, graph drawing, the
# Khronos shader toolchain, audio, text-mode graphics and the foot terminal
# (gd, graphviz, imath, openexr, libheif, portaudio, frei0r-plugins, libgme,
# libcaca, libdecor, gumbo-parser, litehtml, spirv-headers, spirv-tools,
# glslang, shaderc, tllist, fcft, foot). Each test uses the component (known
# answers), not just its loading. Which test checks which component:
# g9-media-gfx.tsv; programs/data a test needs installed: g9-media-gfx.extras.

# ---- headers and small libraries ----
port_test(test_tllist           test_tllist.c)
port_test(test_spirv_headers    test_spirv_headers.c)
port_test(test_imath            test_imath.cpp           Imath-3_2)

# ---- images ----
port_test(test_openexr          test_openexr.cpp         OpenEXR-3_4 OpenEXRCore-3_4 IlmThread-3_4 Iex-3_4 Imath-3_2)
target_include_directories(test_openexr PRIVATE "${INC}/OpenEXR" "${INC}/Imath")
port_test(test_libheif          test_libheif.c           heif)
port_test(test_gd               test_gd.c                gd)
port_script(test_graphviz)

# ---- text, HTML ----
port_test(test_libcaca          test_libcaca.c           caca)
port_test(test_gumbo_parser     test_gumbo_parser.c      gumbo)
port_test(test_litehtml         test_litehtml.cpp        litehtml gumbo)
target_include_directories(test_litehtml PRIVATE "${INC}/litehtml")
port_test(test_fcft             test_fcft.c              fcft pixman-1)
target_include_directories(test_fcft PRIVATE "${INC}/pixman-1")

# ---- audio, video effects ----
port_test(test_portaudio        test_portaudio.c         portaudio)
port_test(test_libgme           test_libgme.c            gme m)
port_test(test_frei0r_plugins   test_frei0r_plugins.c)

# ---- Wayland: against a headless labwc ----
port_test(test_libdecor         test_libdecor.c          decor-0 wayland-client)
target_include_directories(test_libdecor PRIVATE "${INC}/libdecor-0")
port_script(test_foot)

# ---- the Khronos shader toolchain ----
port_script(test_spirv_tools)
port_script(test_glslang)
port_script(test_shaderc)
