VERSION=1.7.1
# The release tarball, not the git tag: jq's lexer.c and parser.c are generated
# by flex and bison, and the generator only runs under maintainer mode, which
# this build disables. A git checkout therefore has neither file and the build
# dies on "src/lexer.c: No such file or directory" -- while printing the reason
# a few lines earlier, "NOT building lexer.c!". The tarball ships both.
#
# This never surfaced on the machine the port was developed on, because an
# earlier build there had left the generated files behind in src/jq.
TARBALL=https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-1.7.1.tar.gz
SHA256=478c9ca129fd2e3443fe27314b455e211e0d8c60bc8ff7df703873deeee580c2
DEPS="oniguruma"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-docs --disable-maintainer-mode
                  --with-oniguruma="$STAGE$PREFIX" )
