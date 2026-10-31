# Run-level tests, group g2-libs-b (component -> test: g2-libs-b.tsv).
# paho.mqtt.cpp is checked by test_paho_mqtt_cpp, defined in tests/CMakeLists.txt.

# ---- compression, crypto, dl, audio --------------------------------------
port_test(test_zlib_ng            test_zlib_ng.c            z-ng)
port_test(test_libtool            test_libtool.c            ltdl m)
port_test(test_libvorbis          test_libvorbis.c          vorbisfile vorbisenc vorbis ogg m)
port_test(test_libxcrypt          test_libxcrypt.c          crypt)

# ---- X11 client libraries (no server) -------------------------------------
port_test(test_libx11             test_libx11.c             X11)
port_test(test_libxau             test_libxau.c             Xau)
port_test(test_libxdmcp           test_libxdmcp.c           Xdmcp)
port_test(test_libxcb             test_libxcb.c             xcb)
port_test(test_libxcursor         test_libxcursor.c         Xcursor X11)
port_test(test_libxkbcommon       test_libxkbcommon.c       xkbcommon xkbregistry)

# ---- X11 extensions, against a headless labwc + Xwayland started by the test
port_test(test_libxrender         test_libxrender.c         Xrender X11)
pkg_check_modules(G2B_XFT REQUIRED IMPORTED_TARGET xft fontconfig freetype2)
port_test(test_libxft             test_libxft.c             PkgConfig::G2B_XFT Xrender X11)
port_test(test_libxext            test_libxext.c            Xext X11)
port_test(test_libxrandr          test_libxrandr.c          Xrandr X11)
port_test(test_libxi              test_libxi.c              Xi X11)
port_test(test_libxcomposite      test_libxcomposite.c      Xcomposite X11)
port_test(test_libxdamage         test_libxdamage.c         Xdamage Xfixes X11)
port_test(test_libxfixes          test_libxfixes.c          Xfixes X11)
port_test(test_libxinerama        test_libxinerama.c        Xinerama X11)
port_test(test_libxscrnsaver      test_libxscrnsaver.c      Xss X11)
port_test(test_libxtst            test_libxtst.c            Xtst X11)

# ---- Xfce ----------------------------------------------------------------
pkg_check_modules(G2B_XFCE4UTIL REQUIRED IMPORTED_TARGET libxfce4util-1.0)
port_test(test_libxfce4util       test_libxfce4util.c       PkgConfig::G2B_XFCE4UTIL)
pkg_check_modules(G2B_XFW REQUIRED IMPORTED_TARGET libxfce4windowing-0 gtk+-3.0)
port_test(test_libxfce4windowing  test_libxfce4windowing.c  PkgConfig::G2B_XFW)
pkg_check_modules(G2B_TUMBLER REQUIRED IMPORTED_TARGET tumbler-1 gdk-pixbuf-2.0)
port_test(test_tumbler            test_tumbler.c            PkgConfig::G2B_TUMBLER)

# ---- servers and clients on loopback --------------------------------------
port_test(test_lighttpd           test_lighttpd.c)
port_test(test_unbound            test_unbound.c            unbound)
port_test(test_restbed            test_restbed.cpp          restbed)
port_test(test_s2n_tls            test_s2n_tls.c            s2n)
port_test(test_sdl2_net           test_sdl2_net.c           SDL2_net SDL2)

# ---- SDL (the sdl2/sdl3 .pc files require alsa: link by name) --------------
port_test(test_sdl2_image         test_sdl2_image.c         SDL2_image SDL2)
port_test(test_sdl2_ttf           test_sdl2_ttf.c           SDL2_ttf SDL2)
port_test(test_sdl3               test_sdl3.c               SDL3)

# ---- Lua modules (run under LuaJIT) and tree-sitter grammars ---------------
port_script(test_lua_lpeg)
port_script(test_luv)
port_test(test_tree_sitter_c        test_tree_sitter_c.c        tree-sitter)
port_test(test_tree_sitter_lua      test_tree_sitter_lua.c      tree-sitter)
port_test(test_tree_sitter_markdown test_tree_sitter_markdown.c tree-sitter)
port_test(test_tree_sitter_query    test_tree_sitter_query.c    tree-sitter)
port_test(test_tree_sitter_vim      test_tree_sitter_vim.c      tree-sitter)
port_test(test_tree_sitter_vimdoc   test_tree_sitter_vimdoc.c   tree-sitter)

# ---- compute ---------------------------------------------------------------
port_test(test_onetbb             test_onetbb.cpp           tbb tbbmalloc)
port_test(test_onnx               test_onnx.cpp             onnx onnx_proto protobuf::libprotobuf)
target_compile_definitions(test_onnx PRIVATE ONNX_NAMESPACE=onnx ONNX_ML=1)
port_test(test_openblas           test_openblas.c           openblas m)
target_include_directories(test_openblas PRIVATE "${INC}/openblas")
port_test(test_pthreadpool        test_pthreadpool.c        pthreadpool)
port_test(test_sleef              test_sleef.c              sleef m)
port_test(test_suitesparse        test_suitesparse.c        umfpack klu cholmod amd cxsparse suitesparseconfig m)
port_test(test_xnnpack            test_xnnpack.c            XNNPACK pthreadpool m)
port_test(test_muslflt            test_muslflt.c            muslflt m)
