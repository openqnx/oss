# Run-level tests, group g14-python-a: Python packages -- Cython, cffi and
# pycparser, Jinja2 and MarkupSafe, msgpack and zstandard, pexpect and
# ptyprocess, parsing/typing/utility modules (lark, ply, tomli, attrs, cattrs,
# typeguard, ...), the packaging tool chain (build, installer, gpep517 and
# the build backends flit-core, hatchling, meson-python, scikit-build-core),
# virtualenv, and pytest plugins (xdist, mock, flaky, time-machine). Known
# answers throughout. Which test checks which component: g14-python-a.tsv;
# what a test needs installed: g14-python-a.extras.

# ---- Cython: an extension module, carried inside the test program --------
# The ported Cython (the staged package, its pure-Python modules: the QNX
# extension modules next to them do not load on the build machine) turns
# test_cython_qnxcy.pyx into C, run in src/ with a relative path so no build
# machine path ends up in the module; the QNX compiler builds it for the
# target's python3 (headers only, no libpython), and it is embedded as a byte
# array; test_cython writes it out and imports it.
set(G14_CY "${CMAKE_BINARY_DIR}/g14-cython")
add_custom_command(OUTPUT "${G14_CY}/qnxcy.c"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${G14_CY}"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../hosttools/bin/python3.14" -B -s -c
          "import sys; sys.path.insert(0, sys.argv.pop(1)); sys.argv[0] = 'cython'; from Cython.Compiler.Main import setuptools_main; sys.exit(setuptools_main())"
          "${STAGE}/lib/python3.14/site-packages" -3 --module-name qnxcy -o "${G14_CY}/qnxcy.c" test_cython_qnxcy.pyx
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/test_cython_qnxcy.pyx" VERBATIM)
add_library(g14_qnxcy MODULE "${G14_CY}/qnxcy.c")
set_target_properties(g14_qnxcy PROPERTIES PREFIX "" OUTPUT_NAME qnxcy SUFFIX ".so"
                      C_VISIBILITY_PRESET hidden LIBRARY_OUTPUT_DIRECTORY "${G14_CY}")
target_include_directories(g14_qnxcy PRIVATE "${INC}/python3.14")
target_compile_options(g14_qnxcy PRIVATE -Wno-unused-function)
target_link_options(g14_qnxcy PRIVATE -s)
add_custom_command(OUTPUT "${G14_CY}/qnxcy_so.h"
  COMMAND "${CMAKE_COMMAND}" -DIN=$<TARGET_FILE:g14_qnxcy> -DOUT=${G14_CY}/qnxcy_so.h
          -DNAME=qnxcy_so -P "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake"
  DEPENDS g14_qnxcy "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake" VERBATIM)
port_test(test_cython           test_cython.c)
target_sources(test_cython PRIVATE "${G14_CY}/qnxcy_so.h")
target_include_directories(test_cython PRIVATE "${G14_CY}")

port_script(test_python3_cffi)
port_script(test_python3_jinja2)
port_script(test_python3_msgpack_zstandard)
port_script(test_python3_pexpect)
port_script(test_python3_pylibs)
port_script(test_python3_pybuild)
port_script(test_python3_virtualenv)
port_script(test_python3_pytest_plugins)
