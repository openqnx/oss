# xfce4-genmon-plugin: the generic monitor of the Xfce panel. It runs a
# command or script at a set interval and shows its output in the panel --
# plain text, or text, an icon, a bar, a tooltip and a click command when
# the output uses the <txt>, <img>, <bar>, <tool>, <click> tags. The way to
# put a temperature, a voltage warning or any other reading in the panel
# with a few lines of shell.
VERSION=4.3.0
# Release tarball, as for every Xfce package here: it carries the generated
# sources a git checkout makes with xfce4-dev-tools.
TARBALL=https://archive.xfce.org/src/panel-plugins/xfce4-genmon-plugin/4.3/xfce4-genmon-plugin-4.3.0.tar.xz
SHA256=077197911d84e5ba22e7bb895ce6c038dbbd8e8e0067ed6f4e48502b7167a282
DEPS="glib gtk3 libxfce4util libxfce4ui xfconf xfce4-panel"
# The panel loads the plugin; its settings are in xfconf (channel xfce4-panel).
RUNTIME_DEPS="xfce4-panel xfconf"
BUILDSYS=meson
# No options. The example scripts in share/xfce4/genmon/scripts are upstream's:
# most read Linux's /proc and /sys (monCPU, monBat, monWIFI, sysstat) and are
# examples of the output format rather than monitors for QNX; datetime and
# monTime work as they are.
