VERSION=1.8.2
# The release tarball, not the git tag: jq's lexer.c and parser.c are generated
# by flex and bison, and the generator only runs under maintainer mode, which
# this build disables. A git checkout therefore has neither file and the build
# dies on "src/lexer.c: No such file or directory" -- while printing the reason
# a few lines earlier, "NOT building lexer.c!". The tarball ships both.
#
# This never surfaced on the machine the port was developed on, because an
# earlier build there had left the generated files behind in src/jq.
TARBALL=https://github.com/jqlang/jq/releases/download/jq-1.8.2/jq-1.8.2.tar.gz
SHA256=71b8d6e8f5fe81f6c6d0d110e3892251f6ce76ed095abd315e26e6e1193af3af
DEPS="oniguruma"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-docs --disable-maintainer-mode
                  --with-oniguruma="$STAGE$PREFIX" )
