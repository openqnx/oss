VERSION=1.8.0; GIT=https://github.com/apache/logging-log4cxx.git; REF=rel/v1.8.0
DEPS="apr apr-util expat libiconv"
# Apache log4cxx on APR and APR-util (found through their pkg-config files),
# with XML (DOMConfigurator, via APR-util's expat) and socket appenders. The
# optional fmt layout is left out so the library does not pick up the
# workspace's fmt unasked; no ODBC, SMTP or Qt. Tests and examples off.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DCMAKE_DISABLE_FIND_PACKAGE_fmt=ON
                   -DCMAKE_DISABLE_FIND_PACKAGE_Boost=ON -DLOG4CXX_ENABLE_ODBC=OFF
                   -DLOG4CXX_ENABLE_ESMTP=OFF -DLOG4CXX_QT_SUPPORT=OFF )
