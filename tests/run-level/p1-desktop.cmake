# Run-level tests, group p1-desktop: the desktop components added for a
# complete Xfce/labwc session -- dconf, the notification daemon, the desktop
# manager, the document viewer, the archive manager, the user directories,
# the idle daemon and screen locker, four panel plugins, the log-in screen,
# and the applications added with them (editor, task manager, screenshots,
# office, the software centre) with their libraries. The target has no
# display: the tests start a private D-Bus session bus and a headless
# labwc (WLR_BACKENDS=headless, pixman renderer) where a program needs them.
# Which component each test checks: p1-desktop.tsv; what the tests need
# installed beyond what they link: p1-desktop.extras.
find_package(PkgConfig REQUIRED)

# ---- settings storage, session services ----
port_script(test_dconf)
port_script(test_xfce4_notifyd)
port_script(test_xdg_user_dirs)

# ---- desktop, applications ----
port_script(test_xfdesktop)
port_script(test_evince)
port_script(test_xarchiver)

# ---- idle and lock (wlroots clients) ----
port_script(test_swayidle)
port_script(test_swaylock)

# ---- panel plugins ----
port_script(test_xfce4_genmon_plugin)
pkg_check_modules(P1_GMODULE REQUIRED IMPORTED_TARGET gmodule-2.0)
port_test(test_xfce4_cpugraph_plugin  test_xfce4_cpugraph_plugin.c  PkgConfig::P1_GMODULE)
port_script(test_xfce4_cpugraph_panel)
port_script(test_xfce4_alsa_plugin)
port_script(test_oss_network_plugin)

# ---- log-in screen ----
port_script(test_oss_greeter)

# ---- applications and their libraries (GTK windows on a broadwayd display) ----
port_script(test_gtksourceview4)
port_script(test_mousepad)
port_script(test_xfce4_taskmanager)
port_script(test_xfce4_screenshooter)
port_script(test_librsvg)
port_script(test_wv)
port_script(test_abiword)
port_script(test_goffice)
port_script(test_gnumeric)
port_script(test_oss_software)

# ---- Flutter (flutter-engine, qnx-flutter-wayland, flutter-counter-example) ----
# The engine runs the counter app headless with the software renderer; the
# embedder and the app run on a headless labwc.
port_test(test_flutter_engine   test_flutter_engine.c   flutter_engine)
port_script(test_qnx_flutter_wayland)
port_script(test_flutter_counter_example)
port_script(test_flutter_hmi_demo)
