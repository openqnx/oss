# OpenPAM, the BSD PAM implementation (FreeBSD, NetBSD, macOS, and QNX: the
# SDP ships OpenPAM 20190224 "Tabebuia" as libpam.so.2). This is the current
# release, 20250531, built from source like every other library here.
VERSION=20250531; GIT=https://git.des.dev/OpenPAM/OpenPAM.git; REF=openpam-20250531
BUILDSYS=autotools
# The git tree has no configure: qnxbuild runs autogen.sh (libtoolize,
# aclocal, automake, autoconf).
# libpam.so.2, the soname OpenPAM has kept since 2002 (and QNX's own copy
# carries): the same ABI, so a program built against either loads the other.
# Modules are looked up in /usr/lib (where QNX keeps its own: pam_qnx,
# pam_secpol, ...) and then /usr/local/lib (LOCALBASE), where this package
# installs pam_deny, pam_permit and pam_return; policies come from /etc/pam.d,
# /etc/pam.conf, then /usr/local/etc/pam.d. pamtest, OpenPAM's test client,
# is installed too. No documentation (it needs mandoc tools).
CONF_ARGS_EXTRA=( --without-doc --with-pamtest --disable-static )
