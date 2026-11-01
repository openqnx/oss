# qpdf: structural, content-preserving PDF transformation -- the library
# (libqpdf, C++ and C API) and the qpdf, fix-qdf and zlib-flate programs:
# linearise, encrypt/decrypt, split/merge pages, inspect PDF structure.
# 11.10 (the last 11.x), not 12: cups-filters 1.28 uses the PointerHolder and
# replaceOrRemoveKey() API that qpdf 12 removed.
VERSION=11.10.1
TARBALL=https://github.com/qpdf/qpdf/releases/download/v11.10.1/qpdf-11.10.1.tar.gz
SHA256=defca435cf57d26f8a0619864841aa21f5469fddc6eb5662f62d8443021c069d
BUILDSYS=cmake
DEPS="zlib libjpeg-turbo openssl"
# Crypto (RC4/AES/SHA for PDF encryption) from OpenSSL; the native
# implementation stays built in as well. No static library, no manual, no
# example sources.
CMAKE_ARGS_EXTRA=( -DBUILD_STATIC_LIBS=OFF -DBUILD_DOC=OFF -DUSE_IMPLICIT_CRYPTO=OFF -DREQUIRE_CRYPTO_NATIVE=ON -DREQUIRE_CRYPTO_OPENSSL=ON
                   -DDEFAULT_CRYPTO=openssl -DINSTALL_EXAMPLES=OFF -DINSTALL_MANUAL=OFF )
