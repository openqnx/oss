# Run-level tests, group n10-libs-svc: service and data libraries -- boto3
# and its Python dependencies, folly, Avro C++, nanomsg, neon, Redis,
# RocksDB, the zserio C++ runtime, SOEM, fast_float -- and GTK 4's libpanel
# with libadwaita, AppStream and libxmlb under it. Servers (redis-server, the
# test's own HTTP servers) listen on 127.0.0.1 only and are stopped on every
# exit path; the GTK tests run on a headless labwc (src/n10_labwc.h). Which
# component each test checks: n10-libs-svc.tsv; what the tests need installed
# beyond what they link: n10-libs-svc.extras.
find_package(PkgConfig REQUIRED)

port_script(test_python3_boto3)
port_test(test_nanomsg            test_nanomsg.c            nanomsg)
port_test(test_neon               test_neon.c               neon z)
port_test(test_redis              test_redis.c)
# avro's headers use fmt; avro-cpp itself uses it header-only (no libfmt in
# its NEEDED, nor in its package's dependencies), and so does the test
port_test(test_avro_cpp           test_avro_cpp.cpp         avrocpp)
target_compile_definitions(test_avro_cpp PRIVATE FMT_HEADER_ONLY)
# RocksDB 11's headers are C++20
port_test(test_rocksdb            test_rocksdb.cpp          rocksdb)
target_compile_features(test_rocksdb PRIVATE cxx_std_20)
# the zserio runtime is a static library: named by path (hazard 113)
port_test(test_zserio             test_zserio.cpp           "${LIB}/libZserioCppRuntime.a")
# libsoem.a and the static libpcap.a it needs, by path
port_test(test_soem               test_soem.c               "${LIB}/libsoem.a" "${LIB}/libpcap.a")
target_include_directories(test_soem PRIVATE "${INC}/soem")
port_test(test_fast_float         test_fast_float.cpp)
# folly's headers need C++20 and call glog and fmt inline
port_test(test_folly              test_folly.cpp            folly glog gflags fmt)
target_compile_features(test_folly PRIVATE cxx_std_20)
target_compile_definitions(test_folly PRIVATE GLOG_USE_GLOG_EXPORT GFLAGS_IS_A_DLL=0)

pkg_check_modules(N10_XMLB REQUIRED IMPORTED_TARGET xmlb)
port_test(test_libxmlb            test_libxmlb.c            PkgConfig::N10_XMLB z lzma)
pkg_check_modules(N10_AS REQUIRED IMPORTED_TARGET appstream)
port_test(test_appstream          test_appstream.c          PkgConfig::N10_AS)
pkg_check_modules(N10_ADW REQUIRED IMPORTED_TARGET libadwaita-1)
port_test(test_libadwaita         test_libadwaita.c         PkgConfig::N10_ADW)
# libpanel-1.pc lists libadwaita only privately; the test calls it directly
pkg_check_modules(N10_PANEL REQUIRED IMPORTED_TARGET libpanel-1 libadwaita-1)
port_test(test_libpanel           test_libpanel.c           PkgConfig::N10_PANEL)
