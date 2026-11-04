# libpaper: paper sizes by name and the system/user default paper (paper(1),
# paperspecs), as used by ghostscript, cups-filters and other print tools.
VERSION=2.3.0
TARBALL=https://github.com/rrthomas/libpaper/releases/download/v2.3.0/libpaper-2.3.0.tar.gz
SHA256=882b1c7636052fc9a318caa20292b35616b588824b70e7053018262b29b1409a
BUILDSYS=autotools
# sysconfdir is $PREFIX/etc (the package tree's own, see hazard 124): the
# system-wide paperspecs and default paper size live there.
CONF_ARGS_EXTRA=( --disable-static --sysconfdir=/usr/local/etc )
