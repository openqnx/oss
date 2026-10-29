VERSION=1.30.2; GIT=https://github.com/chriskohlhoff/asio.git; REF=asio-1-30-2
# Header-only (standalone, without Boost). Upstream's autotools only installs
# headers, so they are copied directly.
BUILDSYS=custom
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/include" && cp -a "$S/asio/include/asio" "$S/asio/include/asio.hpp" "$STAGE$PREFIX/include/"; }
