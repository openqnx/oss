# npm, the Node.js package manager (JavaScript; runs on the target's Node.js).
# Installed as released into lib/node_modules/npm. Version as in the QNX
# Everywhere 8.0.4r1 image.
VERSION=11.11.0
TARBALL=https://registry.npmjs.org/npm/-/npm-11.11.0.tgz
SHA256=cbcf4cc03148ccdb586a8bf2093c952f093fb43d5cbc97593c98b67ef8c003b0
DEPS="nodejs"
BUILDSYS=custom
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/lib/node_modules/npm" "$STAGE$PREFIX/bin" && cp -a "$S/." "$STAGE$PREFIX/lib/node_modules/npm/" && \
  ln -sfn ../lib/node_modules/npm/bin/npm-cli.js "$STAGE$PREFIX/bin/npm" && ln -sfn ../lib/node_modules/npm/bin/npx-cli.js "$STAGE$PREFIX/bin/npx"; }
