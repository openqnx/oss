# oss-greeter: the log-in screen of the QNX desktop -- the place of Ubuntu's
# GDM or Raspberry Pi OS' greeter. Written for this tree (files/), C and
# GTK 3 on gtk-layer-shell.
#
# qnx-desktop-start (qnx-desktop-core) shows it when qnx-desktop.conf has
# DESKTOP_GREETER=yes; by default the session still logs in by itself. Each
# round: libexec/oss-greeter/oss-greeter-session starts, as root, a labwc of
# its own on QNX Screen that runs only the greeter (no menu, no key or mouse
# binding: share/oss-greeter/labwc) and ends with it. The greeter lists root
# and the ordinary users of /etc/passwd, checks the password through PAM and
# writes its answer -- "login <user>", "reboot" or "poweroff" -- to
# /data/oss/var/run/oss-greeter/result (a directory only root can read);
# qnx-desktop-start then starts that user's session and shows the greeter
# again after Log Out.
# PAM: OpenPAM's policy etc/pam.d/oss-greeter (found in /usr/local/etc/pam.d)
# names QNX's own module by its path, /system/lib/dll/pam/pam_qnx.so, for the
# four facilities -- as swaylock's. pam_qnx.so links OpenPAM's libpam.so.2
# from /usr/local/lib: the session script sets LD_LIBRARY_PATH (oss-env.sh).
VERSION=1.0
DEPS="gtk3 gtk-layer-shell openpam"
# labwc runs it; the background is an SVG (librsvg's pixbuf loader).
RUNTIME_DEPS="labwc openpam librsvg xfdesktop"
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" && cp -a "$RECIPE_DIR/oss-greeter/files/." "$SRCROOT/"; }
do_build() {
  mkdir -p "$B" && cd "$B" && \
  qcc -V"$QCC_TARGET" -D_QNX_SOURCE -O2 -Wall -DPREFIX="\"$PREFIX\"" -o oss-greeter "$S/oss-greeter.c" \
      $(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0) -lpam
}
do_install() {
  install -Dm755 "$B/oss-greeter" "$STAGE$PREFIX/libexec/oss-greeter/oss-greeter" && \
  install -Dm755 "$S/oss-greeter-session" "$STAGE$PREFIX/libexec/oss-greeter/oss-greeter-session" && \
  install -Dm644 "$S/labwc/rc.xml" "$STAGE$PREFIX/share/oss-greeter/labwc/rc.xml" && \
  install -Dm644 "$S/labwc/menu.xml" "$STAGE$PREFIX/share/oss-greeter/labwc/menu.xml" && \
  install -Dm644 "$S/pam.d/oss-greeter" "$STAGE$PREFIX/etc/pam.d/oss-greeter"
}
