# The Public Suffix List: the cross-vendor database of domain suffixes under
# which anyone can register names (com, co.uk, github.io), used to decide
# cookie and certificate scopes. Installs share/publicsuffix/
# public_suffix_list.dat (with the effective_tld_names.dat name older
# consumers use) and upstream's test_psl.txt checks. Data, the same for
# every architecture; libpsl reads it with psl_load_file() or psl
# --load-psl-file. Upstream has no releases: VERSION is the commit's date.
VERSION=20261031; GIT=https://github.com/publicsuffix/list.git; COMMIT=73f8cc701c084fe07d888ec552b2f0b53281a674
BUILDSYS=custom
do_build() { :; }
do_install() { local d="$STAGE$PREFIX/share/publicsuffix"
  install -Dm644 "$S/public_suffix_list.dat" "$d/public_suffix_list.dat" &&
  install -m644 "$S/tests/test_psl.txt" "$d/test_psl.txt" &&
  ln -sf public_suffix_list.dat "$d/effective_tld_names.dat"; }
