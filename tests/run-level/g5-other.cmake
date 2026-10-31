# Run-level tests, group g5-other: header-only libraries, Python packages,
# data and font packages. Each test *uses* its component on the target and
# checks known answers. Which component each test checks: g5-other.tsv; what
# the tests need installed beyond what they link: g5-other.extras.
find_package(PkgConfig REQUIRED)
pkg_check_modules(G5_FONTS    REQUIRED IMPORTED_TARGET fontconfig freetype2)
pkg_check_modules(G5_GLIB     REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(G5_GIO      REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(G5_PIXBUF   REQUIRED IMPORTED_TARGET gdk-pixbuf-2.0)
pkg_check_modules(G5_JSONC    REQUIRED IMPORTED_TARGET json-c)
pkg_check_modules(G5_XML2     REQUIRED IMPORTED_TARGET libxml-2.0)
pkg_check_modules(G5_PNG      REQUIRED IMPORTED_TARGET libpng)
pkg_check_modules(G5_EXPAT    REQUIRED IMPORTED_TARGET expat)
pkg_check_modules(G5_XKB      REQUIRED IMPORTED_TARGET xkbcommon)
pkg_check_modules(G5_FONTENC  REQUIRED IMPORTED_TARGET fontenc)
pkg_check_modules(G5_SQLITE   REQUIRED IMPORTED_TARGET sqlite3)
pkg_check_modules(G5_WLCLIENT REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(G5_PTHSTUBS REQUIRED IMPORTED_TARGET pthread-stubs)

# ---- fonts: one source, one program per font package (FONT_PKG) ----------
port_test(test_adwaita_fonts          test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_cantarell         test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_dejavu            test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_hack              test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_inter             test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_liberation        test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_noto_emoji        test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_noto_sans_cjk     test_fonts.c PkgConfig::G5_FONTS)
port_test(test_font_noto_serif_cjk    test_fonts.c PkgConfig::G5_FONTS)
foreach(_f adwaita-fonts font-cantarell font-dejavu font-hack font-inter font-liberation font-noto-emoji font-noto-sans-cjk font-noto-serif-cjk)
  string(REPLACE "-" "_" _t "test_${_f}")
  target_compile_definitions(${_t} PRIVATE FONT_PKG="${_f}")
endforeach()

# ---- header-only and -dev-only libraries ------------------------------------
# Static libraries are named by path: a bare -l<name> is no file dependency,
# and a rebuilt port's .a would not relink the test.
port_test(test_asio               test_asio.cpp            socket)
port_test(test_candb              test_candb.cpp)
port_test(test_cpuinfo            test_cpuinfo.c           "${LIB}/libcpuinfo.a")
port_test(test_fp16               test_fp16.c              m)
port_test(test_fxdiv              test_fxdiv.c)
port_test(test_nanosvg            test_nanosvg.c           "${LIB}/libnanosvgrast.a" "${LIB}/libnanosvg.a" m)
target_include_directories(test_nanosvg PRIVATE "${INC}/nanosvg")
port_test(test_trompeloeil        test_trompeloeil.cpp)
port_test(test_sqlite_orm         test_sqlite_orm.cpp      PkgConfig::G5_SQLITE)
port_test(test_urdfdom_headers    test_urdfdom_headers.cpp m)
target_include_directories(test_urdfdom_headers PRIVATE "${INC}/urdfdom_headers")
port_test(test_kineto             test_kineto.cpp          "${LIB}/libkineto.a")
port_test(test_libmetal           test_libmetal.c          "${LIB}/libmetal.a" cache)
port_test(test_open_amp           test_open_amp.c          "${LIB}/libopen_amp.a" "${LIB}/libmetal.a" cache)
port_test(test_pdfium             test_pdfium.cpp          "${LIB}/libpdfium.a" m)
target_include_directories(test_pdfium PRIVATE "${INC}/pdfium")
# ruy is ~50 small static libraries that depend on each other in a cycle.
file(GLOB _ruy_libs "${LIB}/libruy_*.a")
port_test(test_ruy                test_ruy.cpp             -Wl,--start-group ${_ruy_libs} -Wl,--end-group)
if(EXISTS "${LIB}/libcpuinfo.a")
  target_link_libraries(test_ruy PRIVATE "${LIB}/libcpuinfo.a")
endif()
# rust: the toolchain is a build-host tool; on the target its product is the
# Rust standard library for QNX inside Rust-built code. zenoh-c is one, linked
# statically here so the program carries its own Rust std.
port_test(test_rust               test_rust.c              "${LIB}/libzenohc.a" socket m)

# ---- protocol / header data packages ---------------------------------------
port_test(test_xorgproto          test_xorgproto.c)
port_test(test_xtrans             test_xtrans.c            socket)
port_test(test_libdrm_headers     test_libdrm_headers.c)
target_include_directories(test_libdrm_headers PRIVATE "${INC}/libdrm-uapi")
port_test(test_linux_headers_input_codes test_linux_headers_input_codes.c PkgConfig::G5_XKB)
port_test(test_wayland_protocols  test_wayland_protocols.c PkgConfig::G5_EXPAT)
target_include_directories(test_wayland_protocols PRIVATE "${INC}/wayland-protocols")
# Consumers run wayland-scanner (a build tool) over the XML; do the same with
# the build host's, if it has one, and link the generated code.
find_program(G5_WAYLAND_SCANNER wayland-scanner)
set(_wp "${STAGE}/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml")
if(G5_WAYLAND_SCANNER AND EXISTS "${_wp}")
  set(_g "${CMAKE_CURRENT_BINARY_DIR}/g5-wayland")
  add_custom_command(OUTPUT "${_g}/xdg-shell-client-protocol.h" "${_g}/xdg-shell-protocol.c"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_g}"
    COMMAND ${G5_WAYLAND_SCANNER} client-header "${_wp}" "${_g}/xdg-shell-client-protocol.h"
    COMMAND ${G5_WAYLAND_SCANNER} private-code  "${_wp}" "${_g}/xdg-shell-protocol.c"
    DEPENDS "${_wp}")
  target_sources(test_wayland_protocols PRIVATE "${_g}/xdg-shell-protocol.c" "${_g}/xdg-shell-client-protocol.h")
  target_include_directories(test_wayland_protocols PRIVATE "${_g}")
  target_compile_definitions(test_wayland_protocols PRIVATE HAVE_XDG_SHELL_SCANNED=1)
  target_link_libraries(test_wayland_protocols PRIVATE PkgConfig::G5_WLCLIENT)
endif()
port_test(test_wlr_protocols      test_wlr_protocols.c     PkgConfig::G5_EXPAT)
port_test(test_gsettings_desktop_schemas test_gsettings_desktop_schemas.c PkgConfig::G5_GIO)
target_include_directories(test_gsettings_desktop_schemas PRIVATE "${INC}/gsettings-desktop-schemas")
port_test(test_pthread_stubs      test_pthread_stubs.c     PkgConfig::G5_PTHSTUBS)

# ---- data ------------------------------------------------------------------
port_test(test_ca_certificates    test_ca_certificates.c   crypto)
port_test(test_dnsmasq            test_dnsmasq.c           socket)
port_test(test_encodings          test_encodings.c         PkgConfig::G5_FONTENC)
port_test(test_hicolor_icon_theme test_hicolor_icon_theme.c PkgConfig::G5_GLIB)
port_test(test_lxde_icon_theme    test_lxde_icon_theme.c   PkgConfig::G5_PIXBUF PkgConfig::G5_GLIB)
port_test(test_iso_codes          test_iso_codes.c         PkgConfig::G5_JSONC)
port_test(test_retropie           test_retropie.c          PkgConfig::G5_XML2 PkgConfig::G5_PNG)
port_test(test_webrtc             test_webrtc.c            socket)
port_test(test_xkeyboard_config   test_xkeyboard_config.c  PkgConfig::G5_XKB)

# ---- shell tests: Python packages, Perl ------------------------------------
port_script(test_numpy)
port_script(test_pandas)
port_script(test_python3_dateutil)
port_script(test_python3_packaging)
port_script(test_python3_pip)
port_script(test_python3_pytz)
port_script(test_python3_setuptools)
port_script(test_python3_six)
port_script(test_python3_tzdata)
port_script(test_python3_wheel)
port_script(test_meson)
port_script(test_ntpsec)
port_script(test_xcb_proto)
port_script(test_perl_error)
