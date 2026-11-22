# BoringSSL: Google's fork of OpenSSL (libcrypto, libssl, the bssl tool),
# at a release tag, with QNX's aarch64 CPU setup (patches/0001, from
# github.com/qnx-ports/boringssl, branch qnx-mediapipe) and the QNX changes
# around it (patches/0002-0004).
VERSION=0.20260929.0; GIT=https://github.com/google/boringssl.git; REF=0.20260929.0
DEPS=""
BUILDSYS=cmake
# It installs libcrypto/libssl and <openssl/*.h> under the same names as the
# openssl component, with a different API and ABI: it gets a prefix of its
# own, /usr/local/opt/boringssl, and a program uses it on purpose
# (-I/-L.../opt/boringssl/{include,lib}). The libraries get the release as
# their soname (libcrypto.so.0.20260929.0, patches/0004), so a BoringSSL
# program never loads OpenSSL's libcrypto.so found first on LD_LIBRARY_PATH
# (QNX's loader takes it before the run path); bssl's run path is that
# prefix's lib/, absolute: QNX's loader does not expand $ORIGIN.
OPT="$PREFIX/opt/boringssl"
# BUILD_TESTING=OFF: the unit tests need Go to run and googletest to build;
# the run-level test checks the library on the target.
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                   -DBUILD_SHARED_LIBS=ON -DBORINGSSL_SOVERSION="$VERSION" -DBUILD_TESTING=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  # The command in bin/, under a name that does not suggest OpenSSL's.
  mkdir -p "$STAGE$PREFIX/bin" && ln -sf ../opt/boringssl/bin/bssl "$STAGE$PREFIX/bin/bssl"
}
