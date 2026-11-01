# Run-level tests, group n5-zenoh-llvm: Eclipse Zenoh's C (zenoh-pico) and C++
# (zenoh-cpp) libraries, Eclipse uProtocol (up-core-api, up-cpp,
# up-transport-zenoh-cpp), and the LLVM runtimes built apart from the llvm
# component (openmp, compiler-rt) plus a test of llvm's own ld.lld. Each test
# uses its component with known answers. Which component each test checks:
# n5-zenoh-llvm.tsv; what the tests need installed beyond what they link:
# n5-zenoh-llvm.extras.
find_package(zenohpico REQUIRED)
find_package(zenohc REQUIRED)
find_package(zenohcxx REQUIRED)
find_package(up-transport-zenoh-cpp REQUIRED)   # finds up-cpp, up-core-api, protobuf

port_test(test_zenoh_pico              test_zenoh_pico.c              zenohpico::lib)
port_test(test_zenoh_cpp               test_zenoh_cpp.cpp             zenohcxx::zenohc)
port_test(test_up_core_api             test_up_core_api.cpp           up-core-api::up-core-api)
port_test(test_up_cpp                  test_up_cpp.cpp                up-cpp::up-cpp)
port_test(test_up_transport_zenoh_cpp  test_up_transport_zenoh_cpp.cpp
          up-transport-zenoh-cpp::up-transport-zenoh-cpp)

# openmp: compiled by the SDP's GCC with -fopenmp (GOMP_* calls), linked
# against libomp -- not with -fopenmp at link time, which would ask for the
# libgomp the SDP does not have.
port_test(test_openmp                  test_openmp.c                  omp)
target_compile_options(test_openmp PRIVATE -fopenmp)

# compiler-rt: the builtins archive by path (as for any static library here:
# a rebuilt archive relinks the test), ahead of libgcc.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  set(_crt_triple aarch64-unknown-qnx)
else()
  set(_crt_triple x86_64-pc-qnx)
endif()
port_test(test_compiler_rt             test_compiler_rt.c
          "${LIB}/clang/22/lib/${_crt_triple}/libclang_rt.builtins.a")

# lld is part of the llvm component (ld.lld is installed with it); not a
# component of its own.
port_script(test_lld)
