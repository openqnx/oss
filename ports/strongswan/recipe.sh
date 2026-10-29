VERSION=6.0.6; GIT=https://github.com/strongswan/strongswan.git; REF=6.0.6
DEPS="openssl"
BUILDSYS=autotools
# GCC 12 reports a false -Wmaybe-uninitialized in utils/backtrace.c, which the
# build turns into an error.
CFLAGS_APPEND="-Wno-error=maybe-uninitialized"
CONF_ARGS_EXTRA=( --disable-kernel-netlink --enable-kernel-pfkey --enable-kernel-pfroute --disable-gmp --enable-openssl
                  --disable-scripts --disable-stroke --enable-swanctl --disable-defaults --enable-charon
                  --enable-ikev2 --enable-nonce --enable-random --enable-pem --enable-pkcs1 --enable-pkcs8 --enable-x509
                  --enable-pubkey --enable-sha2 --enable-hmac --enable-socket-default --enable-vici --enable-updown
                  --enable-kdf --with-capabilities=no )
