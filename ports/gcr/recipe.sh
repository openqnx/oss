VERSION=3.41.2; GIT=https://gitlab.gnome.org/GNOME/gcr.git; REF=3.41.2
DEPS="glib libgcrypt p11-kit libsecret gtk3"
BUILDSYS=meson
# GTK enabled: gcr-3 (the UI part, prompts and certificate viewer) is what
# GNOME Web (epiphany) needs; gcr-base-3 alone is not enough.
MESON_ARGS_EXTRA=( -Dgtk_doc=false -Dintrospection=false
                   -Dgtk=true -Dssh_agent=false -Dsystemd=disabled )
