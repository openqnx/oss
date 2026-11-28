# xfce4-cpugraph-plugin: a graph of the CPU load in the Xfce panel -- one
# history graph (normal, LED, no-history or grid mode) and optional bars per
# processor, colours and update interval configurable, a command on click.
#
# QNX backend (patches/0001): panel-plugin/os.cc reads the processors' time
# counters from /proc/stat (Linux), sysctl (BSD, macOS) or kstat (Solaris)
# and stops with '#error "Your OS is not supported."' anywhere else. On QNX
# each processor's idle time is the CPU time of its idle thread in the kernel
# process (pid 1, thread ids 1..num_cpu, priority 0), read through the
# thread's CPU-time clock (ClockId(1, cpu + 1), ClockTime) -- no /proc access
# needed, so it works for a non-root session too -- against the monotonic
# clock; the processor count is from the system page. One load figure per
# processor (QNX keeps no user/system/iowait split, so "Detailed" colouring
# has nothing to show). patches/0002: a constexpr std::abs that libc++ in
# C++17 does not have.
VERSION=1.3.0
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/panel-plugins/xfce4-cpugraph-plugin/1.3/xfce4-cpugraph-plugin-1.3.0.tar.xz
SHA256=c3305edea13ae785ea8b7ce8efbb40b5d5cac69a6f8bf790e4f2efaa780ca097
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel"
# The panel loads the plugin; its settings are in xfconf (channel xfce4-panel).
RUNTIME_DEPS="xfce4-panel xfconf"
BUILDSYS=meson
# No options (C++17; the plugin has no X11-only part).
