VERSION=0.193
TARBALL=https://sourceware.org/elfutils/ftp/0.193/elfutils-0.193.tar.bz2
SHA256=7857f44b624f4d8d421df851aaae7b1402cfe6bcdd2d8049f15fc07d3dde7635
BUILDSYS=autotools
DEPS="zlib"
# libelf only. The rest of elfutils (libdw, libasm and the eu-* tools) is
# built on glibc's argp, fts and obstack, which QNX does not have; those
# would be three more libraries (argp-standalone, musl-fts, musl-obstack)
# plus the glibc-isms musl distributions patch out. configure insists on
# all three even for libelf, which uses none: its answers say they are
# present, and only lib/ (libelf's helper objects) and libelf/ are built.
CONF_ENV=( ac_cv_search_argp_parse="none required" ac_cv_search_fts_close="none required"
           ac_cv_search__obstack_free="none required"
           CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -D_QNX_SOURCE" )
# The cross-GCC driver, not qcc: elfutils' Makefiles compile with
# "-MD -MP -MF <file> -c -o <obj>", which qcc misparses as several input
# files (hazard 18).
# A prefix of its own, /usr/local/opt/elfutils (include/, lib/, pkgconfig):
# the SDP has a libelf of its own (elftoolchain's, a static libelf.a with
# its own <libelf.h>/<gelf.h>), which glib, emacs and make find when they
# probe -lelf. In the shared prefix this libelf.so would replace it in
# whatever is built after it -- a run-time dependency on elfutils that
# depends on build order. Consumers that want it say so:
# PKG_CONFIG_PATH=.../opt/elfutils/lib/pkgconfig (libelf.pc).
# Compressed sections: zlib only (no bzip2/lzma/zstd).
CONF_ARGS_EXTRA=( --prefix="$PREFIX/opt/elfutils" --disable-debuginfod --disable-libdebuginfod --disable-nls --disable-demangler
                  --without-bzlib --without-lzma --without-zstd --disable-stacktrace )
do_build() { make -C "$B/lib" -j6 && make -C "$B/libelf" -j6 && make -C "$B/config" libelf.pc; }
# <libelf.h> includes <elf.h>, the ELF definitions that glibc provides and
# QNX does not (it has its own <sys/elf.h> and elftoolchain's
# <elfdefinitions.h>). elfutils
# carries its own copy (libelf/elf.h, from glibc) and builds with it: it is
# installed alongside, so consumers of <libelf.h>/<gelf.h> compile.
do_install() { make -C "$B/libelf" DESTDIR="$STAGE" install && \
  install -m644 "$S/libelf/elf.h" "$STAGE$PREFIX/opt/elfutils/include/elf.h" && \
  install -Dm644 "$B/config/libelf.pc" "$STAGE$PREFIX/opt/elfutils/lib/pkgconfig/libelf.pc"; }
