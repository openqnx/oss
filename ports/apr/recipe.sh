VERSION=1.7.6
TARBALL=https://archive.apache.org/dist/apr/apr-1.7.6.tar.bz2
SHA256=49030d92d2575da735791b496dc322f3ce5cff9494779ba8cc28c7f46c5deb32
BUILDSYS=autotools
# configure runs ~30 test programs, none of which can run in a cross build.
# The answers, checked against the SDP 8.0 headers and QNX's behaviour:
#  - /dev/zero exists and can be mapped shared; getaddrinfo/getnameinfo and
#    AI_ADDRCONFIG work; EAI_* codes are positive (BSD numbering);
#  - QNX's socket layer is FreeBSD's: an accepted socket inherits TCP_NODELAY
#    and O_NONBLOCK from the listener (the run-level test checks the latter);
#    there is no TCP_CORK (TCP_NOPUSH instead) and no SCTP;
#  - accept4() and SOCK_CLOEXEC exist (SDP 8.0), dup3() does not;
#  - epoll is only a Linux-compatibility shim (libepoll): APR uses poll();
#  - POSIX semaphores, process-shared and recursive pthread mutexes work;
#    robust mutexes are left out of the default cross-process lock choice,
#    which is then fcntl() locking, APR's classic default;
#  - GCC's 32/64-bit __sync and __atomic builtins; strerror_r returns int;
#    64-bit off_t already (no _LARGEFILE64_SOURCE); struct rlimit.
# QNX's libc has no crypt(), and nothing in apr calls it: configure's search
# would otherwise link (and export to every user) libxcrypt. Likewise libuuid:
# apr_os_uuid_get() is used only when there is one, and APR's own UUIDs
# (random bytes and the clock) need none, so it is not looked for.
CONF_ENV=( ac_cv_file__dev_zero=yes ac_cv_mmap__dev_zero=yes
  ac_cv_working_getaddrinfo=yes apr_cv_gai_addrconfig=yes ac_cv_working_getnameinfo=yes
  ac_cv_negative_eai=no ac_cv_tcp_nodelay_inherited=yes ac_cv_o_nonblock_inherited=yes
  apr_cv_tcp_nodelay_with_cork=no apr_cv_sctp=no ac_cv_ebcdic=no
  apr_cv_accept4=yes apr_cv_sock_cloexec=yes apr_cv_dup3=no
  apr_cv_epoll=no apr_cv_epoll_create1=no apr_cv_epoll_wait_has_reliable_timeout=no apr_cv_aio_msgq=no
  ac_cv_func_sem_open=yes apr_cv_process_shared_works=yes apr_cv_mutex_robust_shared=no
  apr_cv_mutex_recursive=yes
  ap_cv_atomic_builtins=yes ap_cv__atomic_builtins=yes ap_cv_atomic_builtins64=yes ap_cv__atomic_builtins64=yes
  ac_cv_strerror_r_rc_int=yes apr_cv_use_lfs64=no ac_cv_struct_rlimit=yes
  ac_cv_search_crypt=no ac_cv_search_uuid_create=no ac_cv_search_uuid_generate=no apr_cv_osuuid=no )
# The build files apr-util and others take from apr-1-config --installbuilddir
# (apr_rules.mk, libtool) go under share/, not upstream's $PREFIX/build-1.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-installbuilddir="$PREFIX/share/apr-1/build" )
# apr-1-config names the source and build trees for use from an uninstalled
# build; installed (or run from staging, "crosscompile") it never uses them,
# and a package must not name the build machine.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i -e 's|^APR_SOURCE_DIR=.*|APR_SOURCE_DIR=""|' -e 's|^APR_BUILD_DIR=.*|APR_BUILD_DIR=""|' "$STAGE$PREFIX/bin/apr-1-config"
}
