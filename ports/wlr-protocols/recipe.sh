# wlroots' Wayland protocol extensions (XML; data only). Version as in the image.
VERSION=0-git-20250702; GIT=https://gitlab.freedesktop.org/wlroots/wlr-protocols.git; COMMIT=a5028afbe4a1cf0daf020c4104c1565a09d6e58a
BUILDSYS=custom
do_build() { :; }
do_install() { make -C "$S" DESTDIR="$STAGE" PREFIX="$PREFIX" install; }
