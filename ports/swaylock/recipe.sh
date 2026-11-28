# swaylock: the screen locker of wlroots compositors such as labwc. It locks
# the session through ext-session-lock-v1 (the compositor shows nothing but
# the locker's surfaces and keeps the session locked even if the locker dies),
# draws a colour or an image with the typing indicator, and checks the
# password through PAM. Typically started by swayidle: swaylock -f -c 000000.
# SIGUSR1 unlocks.
VERSION=1.8.6; GIT=https://github.com/swaywm/swaylock.git; REF=v1.8.6
# openpam: libpam.so.2 -- the same library and soname the SDP ships; the
# PAM modules that check the password are QNX's own (/usr/lib/pam_qnx.so).
# gdk-pixbuf: background images other than PNG.
DEPS="wayland wayland-protocols libxkbcommon cairo gdk-pixbuf openpam"
# The keymap the compositor hands over is compiled by libxkbcommon (it needs
# the XKB data only for the compose table); a font for the indicator text.
RUNTIME_DEPS="xkeyboard-config font-dejavu"
BUILDSYS=meson
# - pam=enabled: the policy is etc/pam.d/swaylock ("auth include login"),
#   which OpenPAM finds in /usr/local/etc/pam.d after /etc/pam.d; "login" is
#   QNX's own policy (pam_qnx.so). The alternative, the shadow backend, needs
#   a setuid-root binary and <crypt.h>;
# - no man page (scdoc is not a build tool here); the shell completions are data;
# - werror=false: upstream builds with -Werror.
MESON_ARGS_EXTRA=( -Dpam=enabled -Dgdk-pixbuf=enabled -Dman-pages=disabled -Dwerror=false
                   -Dzsh-completions=true -Dbash-completions=true -Dfish-completions=true )
