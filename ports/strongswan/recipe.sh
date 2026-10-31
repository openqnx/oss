VERSION=6.0.6; GIT=https://github.com/strongswan/strongswan.git; REF=6.0.6
DEPS="openssl"
BUILDSYS=autotools
# GCC 12 reports a false -Wmaybe-uninitialized in utils/backtrace.c, which the
# build turns into an error.
CFLAGS_APPEND="-Wno-error=maybe-uninitialized"
# --disable-defaults also drops three defaults that matter:
#  * pki, the certificate tool (keys, CSRs, self-signed and issued certs);
#  * constraints, which enforces X.509 name/path-length/policy constraints --
#    without it charon trusts a chain that violates its CA's name constraints,
#    and libstrongswan_tests' certnames suite fails 0/10;
#  * revocation (CRL/OCSP checking of peer certificates).
CONF_ARGS_EXTRA=( --disable-kernel-netlink --enable-kernel-pfkey --enable-kernel-pfroute --disable-gmp --enable-openssl
                  --disable-scripts --disable-stroke --enable-swanctl --disable-defaults --enable-charon
                  --enable-ikev2 --enable-nonce --enable-random --enable-pem --enable-pkcs1 --enable-pkcs8 --enable-x509
                  --enable-pubkey --enable-sha2 --enable-hmac --enable-socket-default --enable-vici --enable-updown
                  --enable-kdf --with-capabilities=no
                  --enable-pki --enable-constraints --enable-revocation )
