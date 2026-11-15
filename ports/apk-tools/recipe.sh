# apk-tools, the Alpine package manager the QNX Everywhere image (and
# repo.oss.qnx.com) uses -- with it a target can install this repository's APK
# packages. Version as in the image.
VERSION=3.0.8; GIT=https://github.com/alpinelinux/apk-tools.git; REF=v3.0.8
DEPS="openssl zlib zstd"
# Not linked, but needed to fetch from an https:// repository (the GitHub
# releases): the CA bundle at /usr/local/ssl/cert.pem, OpenSSL's default.
RUNTIME_DEPS="ca-certificates"
BUILDSYS=meson
# help: "apk --help" and "apk <command> --help" print the text that a Lua
# script builds in from doc/*.scd at build time (native lua: hosttools,
# scripts/build-host-tools.sh). Not compressed: that would need lua-zlib there.
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dhelp=enabled -Dlua_bin=lua -Dcompressed-help=false -Dlua=disabled -Dpython=disabled -Dtests=disabled -Dcrypto_backend=openssl -Dzstd=true -Dc_args=-D_QNX_SOURCE -Dc_link_args=-lsocket )
