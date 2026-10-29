VERSION=3.14.0
TARBALL=https://www.python.org/ftp/python/3.14.0/Python-3.14.0.tar.xz
SHA256=2299dae542d395ce3883aca00d3c910307cd68e0b2f7336098c8e7b7eee9f3e9
# Also linked at run time (found by scripts/gen-dependency-index): zstd
DEPS="zlib openssl libffi expat bzip2 xz sqlite ncurses readline gdbm zstd"
BUILDSYS=autotools
# Cross-building CPython needs a *native* interpreter of the same version: the
# build freezes its own stdlib modules and runs setup steps with it.
# scripts/build-host-tools.sh puts one in hosttools/.
CONF_ENV=(
  # These probe for device files, which configure cannot do across a cross
  # build; QNX has /dev/ptmx but no /dev/ptc.
  ac_cv_file__dev_ptmx=yes
  ac_cv_file__dev_ptc=no
  # configure otherwise tries to *run* a target binary to decide these.
  ac_cv_buggy_getaddrinfo=no
  ac_cv_have_long_long_format=yes
  ac_cv_little_endian_double=yes
  # QNX ships <sys/timerfd.h> with timerfd_create() and timerfd_settime() but
  # no timerfd_gettime(), so os.timerfd_* cannot be implemented completely.
  # QNX's own Python 3.13 port reaches the same conclusion -- its pyconfig.h
  # leaves HAVE_SYS_TIMERFD_H and HAVE_TIMERFD_CREATE undefined.
  ac_cv_header_sys_timerfd_h=no
  ac_cv_func_timerfd_create=no
  # QNX's <sys/termio.h> and <termios.h> are mutually exclusive by design: the
  # header ends in "#error termio/termios incompatibility" when both are pulled
  # in. The termios module includes both if configure finds sys/termio.h.
  ac_cv_header_sys_termio_h=no
)
CONF_ARGS_EXTRA=(
  --enable-shared --without-ensurepip --disable-test-modules
  --with-system-ffi --with-system-expat --with-system-libmpdec
  --with-build-python="$WS/hosttools/bin/python3.14"
  --build=x86_64-pc-linux-gnu
  --without-static-libpython
)
