VERSION=0.5
# The gitlab.freedesktop.org repo for this one is not publicly clonable
# (git asks for credentials); the release tarball is.
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz
SHA256=59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc
BUILDSYS=autotools
# Pure .pc file. It exists so that libxcb can say "I need pthreads" portably:
# on platforms where libc already provides them -- QNX among them -- the package
# resolves to no library at all, which is exactly the right answer here.
CONF_ARGS_EXTRA=( )
