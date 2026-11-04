# Run-level tests, group g8-mathlibs: fftw, armadillo, ensmallen, mlpack,
# xsimd, xtensor, nanoflann, cxxopts, doctest, tl-expected, log4cpp, http-parser,
# libb2, libfyaml, md4c. Each test uses the component with known answers
# (an FFT of a known signal, matrix decompositions, optimiser minima, BLAKE2
# vectors, a YAML round trip, Markdown to HTML, ...). Header-only
# components (-dev packages) are tested by compiling a real program against
# them. Which test checks which component: g8-mathlibs.tsv; programs a test
# needs installed: g8-mathlibs.extras.

# ---- C libraries ----
port_test(test_fftw           test_fftw.c          fftw3 fftw3f fftw3_threads fftw3f_threads m)
port_test(test_libb2          test_libb2.c         b2)
port_test(test_http_parser    test_http_parser.c   http_parser)
port_test(test_md4c           test_md4c.c          md4c-html md4c)
port_test(test_libfyaml       test_libfyaml.c      fyaml)

# ---- C++ libraries ----
port_test(test_armadillo      test_armadillo.cpp   armadillo)
port_test(test_log4cpp        test_log4cpp.cpp     log4cpp)
port_test(test_ensmallen      test_ensmallen.cpp   armadillo)
port_test(test_mlpack         test_mlpack.cpp      armadillo)

# ---- header-only (-dev packages): a program compiled against them ----
port_test(test_cxxopts        test_cxxopts.cpp)
port_test(test_doctest        test_doctest.cpp)
port_test(test_nanoflann      test_nanoflann.cpp)
port_test(test_tl_expected    test_tl_expected.cpp)
port_test(test_xsimd          test_xsimd.cpp)
port_test(test_xtensor        test_xtensor.cpp)
# xtensor 0.27 is C++20 (concepts); xtensorConfig.cmake would define
# XTENSOR_USE_XSIMD for a CMake consumer, as the port builds it.
set_target_properties(test_xtensor PROPERTIES CXX_STANDARD 20)
target_compile_definitions(test_xtensor PRIVATE XTENSOR_USE_XSIMD)

# ---- programs ----
port_script(test_mlpack_cli)
