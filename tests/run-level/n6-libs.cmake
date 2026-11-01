# Run-level tests, group n6-libs: APR and APR-util, log4cxx, POCO, pybind11,
# nsync, gemmlowp, fft2d, stringencoders, TinyXML, libuuid, GeographicLib.
# Known answers throughout; sockets on 127.0.0.1 only. Which component each
# test checks: n6-libs.tsv; what a test needs installed beyond what it
# links: n6-libs.extras.

port_test(test_apr              test_apr.c              apr-1)
target_include_directories(test_apr PRIVATE "${INC}/apr-1")
port_test(test_apr_util         test_apr_util.c         aprutil-1 apr-1)
target_include_directories(test_apr_util PRIVATE "${INC}/apr-1")
port_test(test_log4cxx          test_log4cxx.cpp        log4cxx)
port_test(test_poco             test_poco.cpp           PocoNet PocoUtil PocoXML PocoJSON PocoCrypto PocoFoundation)
port_test(test_nsync            test_nsync.c            nsync)
port_test(test_gemmlowp         test_gemmlowp.cpp)
port_test(test_fft2d            test_fft2d.c            fft2d m)
port_test(test_stringencoders   test_stringencoders.c   modpbase64)
port_test(test_tinyxml          test_tinyxml.cpp        tinyxml)
port_test(test_libuuid          test_libuuid.c          uuid)
port_test(test_geographiclib    test_geographiclib.cpp  GeographicLib)

# ---- pybind11: an extension module, carried inside the test program ------
# Built like any pybind11 module for the target's python3 (its headers, no
# libpython: the interpreter that imports it provides the C API), then
# embedded as a byte array; test_pybind11 writes it out and imports it.
add_library(n6_qnxpb11 MODULE src/test_pybind11_mod.cpp)
set_target_properties(n6_qnxpb11 PROPERTIES PREFIX "" OUTPUT_NAME qnxpb11 SUFFIX ".so"
                      CXX_VISIBILITY_PRESET hidden LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/n6-pybind11")
target_include_directories(n6_qnxpb11 PRIVATE "${INC}/python3.14")
target_link_options(n6_qnxpb11 PRIVATE -s)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/n6-pybind11/qnxpb11_so.h"
  COMMAND "${CMAKE_COMMAND}" -DIN=$<TARGET_FILE:n6_qnxpb11> -DOUT=${CMAKE_BINARY_DIR}/n6-pybind11/qnxpb11_so.h
          -DNAME=qnxpb11_so -P "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake"
  DEPENDS n6_qnxpb11 "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake" VERBATIM)
port_test(test_pybind11         test_pybind11.c)
target_sources(test_pybind11 PRIVATE "${CMAKE_BINARY_DIR}/n6-pybind11/qnxpb11_so.h")
target_include_directories(test_pybind11 PRIVATE "${CMAKE_BINARY_DIR}/n6-pybind11")
