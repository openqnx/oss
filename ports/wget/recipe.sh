# wget: non-interactive HTTP/HTTPS/FTP downloader (TLS through GnuTLS, IDN, recursive mirroring).
VERSION=1.25.0
TARBALL=https://ftp.gnu.org/gnu/wget/wget-1.25.0.tar.gz
SHA256=766e48423e79359ea31e41db9e5c289675947a7fcf2efdcedb726ac9d0da3784
DEPS="gnutls libidn2 zlib pcre2 libpsl"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --with-ssl=gnutls --disable-nls --without-libuuid --disable-iri --without-metalink )
