# libfyaml: a complete YAML 1.2 (and 1.1) parser and emitter in C, with a
# document tree API, path expressions and the fy-tool program (fy-dump,
# fy-filter, fy-join, fy-testsuite are links to it).
VERSION=0.9.6
TARBALL=https://github.com/pantoniou/libfyaml/releases/download/v0.9.6/libfyaml-0.9.6.tar.gz
SHA256=a59cc3331e2eb903ec36933ad52a45888041cac31e44f553a00511131242c483
BUILDSYS=cmake
# The man pages would be built by Sphinx at install time, with an extension
# (linuxdoc) the build machine does not have: no Sphinx. libclang
# (reflection) is off, the upstream default. Tests are not installed.
CMAKE_ARGS_EXTRA=( -DSPHINX_EXECUTABLE=OFF -DENABLE_LIBCLANG=OFF -DBUILD_TESTING=OFF -DENABLE_NETWORK=OFF )
