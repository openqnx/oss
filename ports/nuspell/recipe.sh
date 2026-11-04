# Nuspell: a spell checking library and program that reads Hunspell
# dictionaries (.aff/.dic), with Unicode handling through ICU
# (libnuspell, the nuspell program). QNX's package list has 5.1.6; this is
# the latest release of the 5.1 series.
VERSION=5.1.9; GIT=https://github.com/nuspell/nuspell.git; REF=v5.1.9
DEPS="icu"
# No tests (they need Catch2) and no man pages (they need pandoc).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF )
