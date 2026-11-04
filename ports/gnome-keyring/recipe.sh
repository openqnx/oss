# GNOME Keyring: gnome-keyring-daemon, the session's secret store (the
# org.freedesktop.secrets D-Bus service that libsecret clients use) with
# its PKCS#11 modules, and the gnome-keyring control tool. No systemd
# socket activation, no PAM helper (pam_gnome_keyring needs a desktop
# login), no SSH agent (gcr's ssh-agent replaces it upstream).
VERSION=51.1
TARBALL=https://download.gnome.org/sources/gnome-keyring/51/gnome-keyring-51.1.tar.xz
SHA256=dfe7adc5876ef0670e7eba2907799bdbb8875ce58bb4208e1a52169bd16a47ca
DEPS="glib gcr libgcrypt libgpg-error p11-kit"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dsystemd=disabled -Dpam=false -Dssh-agent=false -Dselinux=disabled -Dmanpage=false -Ddebug-mode=false )
