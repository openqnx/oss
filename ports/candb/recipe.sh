VERSION=0.1-git-20210625; GIT=https://github.com/GENIVI/CANdb.git; COMMIT=150c42a8cdfc0748915f09d2cd0c5a68591263a7
# tl-expected: CANdb's API returns tl::expected, so the library is built
# against the installed tl-expected (patch 0003) that its consumers use.
DEPS="tl-expected"
# Header API: consumers compile dbcparser.h, which includes tl/expected.hpp.
RUNTIME_DEPS="tl-expected-dev"
# Upstream installs nothing of its own; patch 0004 installs libCANdb, its
# API headers (include/CANdb/) and dbclint. dbclint is built with CANdb's
# bundled cxxopts 2.2, which is no longer installed (CXXOPTS_ENABLE_INSTALL
# =OFF): it put its header and CMake package where the cxxopts 3 port
# installs its own. dbclint is not built against that port: it uses the 2.x
# API (cxxopts::option_not_exists_exception, option names like "m, messages").
CMAKE_ARGS_EXTRA=( -DCANDB__WITH_TESTS=OFF -DCANDB__WITH_TOOLS=ON -DBUILD_SHARED_LIBS=ON -DCXXOPTS_ENABLE_INSTALL=OFF )
