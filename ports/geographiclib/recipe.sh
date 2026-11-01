VERSION=2.7; GIT=https://github.com/geographiclib/geographiclib.git; REF=v2.7
# The C++ library and its command-line tools (GeoConvert, GeodSolve, ...).
# No documentation (needs doxygen) and no tests (they run the tools on the
# build machine). The data directory for geoid/gravity/magnetic models is
# compiled in as $PREFIX/share/GeographicLib; the models themselves are
# separate downloads.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DBUILD_DOCUMENTATION=OFF
                   -DBUILD_MANPAGES=OFF -DEXAMPLEDIR= )
