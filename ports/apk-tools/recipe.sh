# apk-tools, the Alpine package manager the QNX Everywhere image (and
# repo.oss.qnx.com) uses -- with it a target can install this repository's APK
# packages. Version as in the image.
VERSION=3.0.0_rc5; GIT=https://github.com/alpinelinux/apk-tools.git; REF=v3.0.0_rc5
DEPS="openssl zlib zstd"
# Not linked, but needed to fetch from an https:// repository (the GitHub
# releases): the CA bundle at /usr/local/ssl/cert.pem, OpenSSL's default.
RUNTIME_DEPS="ca-certificates"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dhelp=disabled -Dlua=disabled -Dpython=disabled -Dtests=disabled -Dcrypto_backend=openssl -Dzstd=true -Dc_args=-D_QNX_SOURCE -Dc_link_args=-lsocket )
