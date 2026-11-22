# lldpd from upstream 1.0.22 with QNX's changes (qnx-ports/lldpd, branch
# qnx-1.0.22): QNX is recognised as an OS and gets the BSD network backend
# (route socket, BPF on the single /dev/bpf clone device, getifaddrs()).
# From git rather than the release tarball: patch 0001 changes configure.ac,
# m4/os.m4 and Makefile.am, so configure has to be regenerated (autogen.sh).
VERSION=1.0.22; GIT=https://github.com/lldpd/lldpd.git; REF=1.0.22
BUILDSYS=autotools
DEPS="libevent"
# --disable-privsep: privilege separation needs a dedicated "_lldpd" user and
#   an empty chroot directory, and a QNX image has neither (a package cannot
#   create users). Without it lldpd runs in one process and emulates the
#   monitor in-process; QNX's patches (sockets opened early, getifaddrs()
#   through the monitor) work the same in that mode.
# --runstatedir=/var/run: the control socket and pid file go where QNX keeps
#   run-time state, not under /usr/local.
# --without-snmp/xml/readline/seccomp: optional features with no QNX
#   dependency staged (net-snmp, libxml2 output) or Linux-only (seccomp);
#   lldpcli still has its own command line and JSON/keyvalue output.
# --with-libbsd=no: libbsd is not ported; lldpd carries its own copies of the
#   few BSD functions it needs (QNX's libc has strlcpy/strlcat already).
# --enable-min-macro (added by QNX's patch 0001): lldpd uses MIN() from
#   <sys/param.h> as the BSDs and glibc define it; QNX's headers do not, and
#   the calls ended up as undefined references to a function MIN.
CONF_ARGS_EXTRA=( --disable-privsep --runstatedir=/var/run --localstatedir=/var
                  --without-snmp --without-xml --without-readline --without-seccomp
                  --with-libbsd=no --with-embedded-libevent=no
                  --disable-static --enable-shared --enable-min-macro )
# The version comes from `git describe`, which reports "1.0.22-dirty" for a
# patched checkout; a release tarball carries .dist-version instead.
do_patch() { printf '%s' "$VERSION" > "$SRCROOT/.dist-version"; }
