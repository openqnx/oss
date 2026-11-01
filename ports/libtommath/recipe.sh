VERSION=1.3.0
TARBALL=https://github.com/libtom/libtommath/releases/download/v1.3.0/ltm-1.3.0.tar.xz
SHA256=296272d93435991308eb73607600c034b558807a07e829e751142e65ccfa9d08
BUILDSYS=cmake
# The shared library (libtommath.so.1, the soname distributions use);
# libtomcrypt's math descriptor links against it.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON )
