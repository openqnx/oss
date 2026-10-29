# The Mozilla CA certificate bundle (as extracted and published by the curl
# project), so TLS clients on the target can verify servers. Installs
# ssl/cert.pem -- the default CA file of this tree's OpenSSL, which is built
# with OPENSSLDIR=/usr/local/ssl -- and the same file as
# etc/ssl/certs/ca-certificates.crt, where other software looks.
VERSION=20261020
TARBALL=https://curl.se/ca/cacert-2026-10-20.pem
SHA256=796f1389bfea13c0ed78f663b765baaff54a508e5483786bd6ece73c5e3e3cbe
BUILDSYS=custom
do_fetch() { mkdir -p "$SRCROOT" "$WS/dist" && local f="$WS/dist/$(basename "$TARBALL")"; [ -f "$f" ] || curl -fsSL -o "$f" "$TARBALL"; \
  echo "$SHA256  $f" | sha256sum -c --status && cp "$f" "$SRCROOT/cacert.pem"; }
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/ssl/certs" "$STAGE$PREFIX/etc/ssl/certs" && \
  install -m644 "$S/cacert.pem" "$STAGE$PREFIX/ssl/cert.pem" && \
  ln -sfn ../cert.pem "$STAGE$PREFIX/ssl/certs/ca-certificates.crt" && \
  ln -sfn ../../../ssl/cert.pem "$STAGE$PREFIX/etc/ssl/certs/ca-certificates.crt"; }
