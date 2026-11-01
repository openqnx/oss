VERSION=1.15.4; GIT=https://github.com/pocoproject/poco.git; REF=poco-1.15.4-release
DEPS="openssl zlib pcre2 utf8proc expat"
# The POCO C++ Libraries, a core module set: Foundation, Util, Net, XML, JSON
# and Crypto (on the ported OpenSSL). Unbundled: zlib, PCRE2, utf8proc and
# expat are the workspace's, not copies compiled into Foundation and XML.
# Left out: NetSSL, JWT, Data (SQLite/MySQL/PostgreSQL/ODBC), Zip, Encodings,
# MongoDB, Redis, Prometheus, ActiveRecord, PageCompiler, the tools and the
# samples. FastLogger (Quill) disables itself: Quill knows no QNX.
# Upstream's CMake already has the QNX settings (_QNX_SOURCE, poll()).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DPOCO_UNBUNDLED=ON -DENABLE_TESTS=OFF -DENABLE_SAMPLES=OFF
  -DENABLE_FOUNDATION=ON -DENABLE_UTIL=ON -DENABLE_NET=ON -DENABLE_XML=ON -DENABLE_JSON=ON -DENABLE_CRYPTO=ON
  -DENABLE_NETSSL=OFF -DENABLE_JWT=OFF -DENABLE_DATA=OFF -DENABLE_DATA_SQLITE=OFF -DENABLE_DATA_MYSQL=OFF
  -DENABLE_DATA_POSTGRESQL=OFF -DENABLE_DATA_ODBC=OFF -DENABLE_ZIP=OFF -DENABLE_ENCODINGS=OFF
  -DENABLE_MONGODB=OFF -DENABLE_REDIS=OFF -DENABLE_PROMETHEUS=OFF -DENABLE_ACTIVERECORD=OFF
  -DENABLE_ACTIVERECORD_COMPILER=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF
  -DENABLE_APACHECONNECTOR=OFF -DENABLE_FASTLOGGER=OFF -DENABLE_PDF=OFF -DENABLE_SEVENZIP=OFF )
