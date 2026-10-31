# Run-level tests, group g4-programs-libs: components that install both
# programs and libraries. Each test uses the component (known answers), not
# just its loading. Which test checks which component: g4-programs-libs.tsv;
# programs/data a test needs installed: g4-programs-libs.extras.
# test_paho_mqtt_c is defined in tests/CMakeLists.txt (src rewritten for this group).

# ---- fork A: CLI / text tools ----
port_test(test_file             test_file.c              magic)
port_script(test_gawk)
port_script(test_gettext)
port_test(test_libiconv         test_libiconv.c          iconv charset)
port_test(test_pcre             test_pcre.cpp            pcrecpp pcreposix pcre16 pcre)
port_script(test_pkgconf)
port_test(test_aspell           test_aspell.c            aspell)
port_script(test_man_db)
port_test(test_fakeroot         test_fakeroot.c)
port_script(test_csmith)
port_test(test_libgpg_error     test_libgpg_error.c      gpg-error)
port_test(test_check            test_check.c             check)

# --- fork B: llvm lua5.1 lua5.3 capnproto metis libde265 libdisplay-info apk-tools strongswan iperf3 paho.mqtt.c dlt-daemon
port_script(test_llvm)
port_test(test_lua5_1            test_lua5_1.c            lua5.1)
port_test(test_lua5_3            test_lua5_3.c            lua5.3)
port_script(test_capnproto)
port_test(test_metis             test_metis.c             metis)  # libmetis.so NEEDs libGKlib.so (ports/metis 0002), which NEEDs libregex (ports/gklib 0002)
port_test(test_libde265          test_libde265.c          de265)
port_test(test_libdisplay_info   test_libdisplay_info.c   display-info)
port_script(test_apk_tools)
port_script(test_strongswan)
port_script(test_iperf3)
port_test(test_dlt_daemon        test_dlt_daemon.c        dlt)
# test_paho_mqtt_c: already a target in tests/CMakeLists.txt (paho-mqtt3cs paho-mqtt3as)

# ---- g4 fork C: media / graphics components ----
port_test(test_libjpeg_turbo      test_libjpeg_turbo.c      turbojpeg jpeg)
pkg_check_modules(G4C_GSTBASE REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0 gstreamer-pbutils-1.0)
port_test(test_gst_plugins_base   test_gst_plugins_base.c   PkgConfig::G4C_GSTBASE)
pkg_check_modules(G4C_VLC REQUIRED IMPORTED_TARGET libvlc)
port_test(test_vlc                test_vlc.c                PkgConfig::G4C_VLC)
pkg_check_modules(G4C_FFTHUMB REQUIRED IMPORTED_TARGET libffmpegthumbnailer libavformat libavcodec libavutil libturbojpeg)
port_test(test_ffmpegthumbnailer  test_ffmpegthumbnailer.c  PkgConfig::G4C_FFTHUMB)
pkg_check_modules(G4C_OPENAL REQUIRED IMPORTED_TARGET openal)
port_test(test_openal_soft        test_openal_soft.c        PkgConfig::G4C_OPENAL)
port_test(test_sdl2_compat        test_sdl2_compat.c)
pkg_check_modules(G4C_RTLSDR REQUIRED IMPORTED_TARGET librtlsdr)
port_test(test_rtl_sdr            test_rtl_sdr.c            PkgConfig::G4C_RTLSDR)
pkg_check_modules(G4C_PANGO REQUIRED IMPORTED_TARGET pangocairo)
port_test(test_pango              test_pango.c              PkgConfig::G4C_PANGO)
pkg_check_modules(G4C_GI REQUIRED IMPORTED_TARGET gobject-introspection-1.0)
port_test(test_gobject_introspection test_gobject_introspection.c PkgConfig::G4C_GI)
pkg_check_modules(G4C_GSF REQUIRED IMPORTED_TARGET libgsf-1)
port_test(test_libgsf             test_libgsf.c             PkgConfig::G4C_GSF)
pkg_check_modules(G4C_PROXY REQUIRED IMPORTED_TARGET libproxy-1.0)
port_test(test_libproxy           test_libproxy.c           PkgConfig::G4C_PROXY)
# libproxy.so NEEDs libpxbackend-1.0.so from lib/libproxy (found at run time
# through its RPATH /usr/local/lib/libproxy); the linker needs to be told too.
target_link_options(test_libproxy PRIVATE "-Wl,-rpath-link,${LIB}/libproxy")

pkg_check_modules(G4D_GCR REQUIRED IMPORTED_TARGET gcr-base-3)
port_test(test_gcr                test_gcr.c                PkgConfig::G4D_GCR)
pkg_check_modules(G4D_WEBKIT REQUIRED IMPORTED_TARGET webkit2gtk-4.1 javascriptcoregtk-4.1)
port_test(test_webkitgtk          test_webkitgtk.c          PkgConfig::G4D_WEBKIT)
pkg_check_modules(G4D_GLIB REQUIRED IMPORTED_TARGET glib-2.0 gobject-2.0 gio-2.0 gmodule-2.0)
port_test(test_epiphany           test_epiphany.c           PkgConfig::G4D_GLIB)
port_test(test_gthumb             test_gthumb.c             PkgConfig::G4D_GLIB)
# Not geany.pc's Cflags: its -I include/geany puts geany's own spawn.h in
# front of the system <spawn.h> the test needs.  <geany/utils.h> is enough.
pkg_check_modules(G4D_GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
port_test(test_geany              test_geany.c              PkgConfig::G4D_GTK3 geany)
target_include_directories(test_geany PRIVATE "${INC}/geany/scintilla" "${INC}/geany/tagmanager")
target_compile_definitions(test_geany PRIVATE GTK)
pkg_check_modules(G4D_THUNARX REQUIRED IMPORTED_TARGET thunarx-3)
port_test(test_thunar             test_thunar.c             PkgConfig::G4D_THUNARX)
pkg_check_modules(G4D_PANEL REQUIRED IMPORTED_TARGET libxfce4panel-2.0 gmodule-2.0)
port_test(test_xfce4_panel        test_xfce4_panel.c        PkgConfig::G4D_PANEL)
pkg_check_modules(G4D_EXO REQUIRED IMPORTED_TARGET exo-2)
port_test(test_exo                test_exo.c                PkgConfig::G4D_EXO)
pkg_check_modules(G4D_XFCE4UI REQUIRED IMPORTED_TARGET libxfce4ui-2)
port_test(test_libxfce4ui         test_libxfce4ui.c         PkgConfig::G4D_XFCE4UI)
port_test(test_xfce4_settings     test_xfce4_settings.c     PkgConfig::G4D_GLIB)
pkg_check_modules(G4D_XFCONF REQUIRED IMPORTED_TARGET libxfconf-0)
port_test(test_xfconf             test_xfconf.c             PkgConfig::G4D_XFCONF)

