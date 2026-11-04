# Run-level tests, group g7-buildtools: build tools and macro packages
# (fio, flex, swig, ccache, nasm, texinfo, help2man, chrpath, unifdef, scdoc,
# zip, rhash, datefudge, intltool, util-macros, mate-common, xfce4-dev-tools,
# icon-naming-utils). Each test uses the component (known answers), not just
# its loading. Which test checks which component: g7-buildtools.tsv;
# programs/data a test needs installed: g7-buildtools.extras.

port_script(test_fio)

# ---- flex: a scanner generated at build time by the build machine's flex
# (the same 2.6.4) and linked with the target's libfl, and the target's flex
# on the same .l file ----
find_program(G7_HOST_FLEX flex NO_CMAKE_FIND_ROOT_PATH REQUIRED)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_flex_scan.c"
  COMMAND "${G7_HOST_FLEX}" -o "${CMAKE_CURRENT_BINARY_DIR}/test_flex_scan.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/test_flex_scan.l"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/test_flex_scan.l")
# port_test() takes its source under src/: name the generated file from there.
file(RELATIVE_PATH G7_FLEX_SCAN "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/test_flex_scan.c")
port_test(test_flex_scan        ${G7_FLEX_SCAN}          fl)
port_script(test_flex)

# ---- swig: wrappers generated at build time by the build machine's swig
# (the same 4.3.1, hosttools/: scripts/build-host-tools.sh) from
# src/test_swig_example.i: the C target language (swig -c), linked into
# test_swig_c, and Python (swig -python), built into the extension
# _qnxswig.so for the target's python3 (its headers, no libpython), which
# test_swig_py carries with its shadow module qnxswig.py. test_swig runs the
# target's swig on the same interface and compares. ----
find_program(G7_HOST_SWIG swig PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../hosttools/bin" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH REQUIRED)
set(G7_SW "${CMAKE_BINARY_DIR}/g7-swig")
file(MAKE_DIRECTORY "${G7_SW}")
add_custom_command(OUTPUT "${G7_SW}/c_wrap.c" "${G7_SW}/c_wrap.h" "${G7_SW}/py_wrap.c" "${G7_SW}/qnxswig.py"
  COMMAND "${G7_HOST_SWIG}" -c -I${CMAKE_CURRENT_SOURCE_DIR}/src -o "${G7_SW}/c_wrap.c" -outdir "${G7_SW}" "${CMAKE_CURRENT_SOURCE_DIR}/src/test_swig_example.i"
  COMMAND "${G7_HOST_SWIG}" -python -I${CMAKE_CURRENT_SOURCE_DIR}/src -o "${G7_SW}/py_wrap.c" -outdir "${G7_SW}" "${CMAKE_CURRENT_SOURCE_DIR}/src/test_swig_example.i"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/test_swig_example.i" "${CMAKE_CURRENT_SOURCE_DIR}/src/test_swig_example.h" VERBATIM)
port_test(test_swig_c           test_swig_c.c            m)
target_sources(test_swig_c PRIVATE "${G7_SW}/c_wrap.c" src/test_swig_example.c)
target_include_directories(test_swig_c PRIVATE "${G7_SW}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
add_library(g7_qnxswig MODULE "${G7_SW}/py_wrap.c" src/test_swig_example.c)
set_target_properties(g7_qnxswig PROPERTIES PREFIX "" OUTPUT_NAME _qnxswig SUFFIX ".so"
                      LIBRARY_OUTPUT_DIRECTORY "${G7_SW}/mod")
target_include_directories(g7_qnxswig PRIVATE "${INC}/python3.14" "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(g7_qnxswig PRIVATE m)
target_link_options(g7_qnxswig PRIVATE -s)
add_custom_command(OUTPUT "${G7_SW}/qnxswig_so.h" "${G7_SW}/qnxswig_py.h"
  COMMAND "${CMAKE_COMMAND}" -DIN=$<TARGET_FILE:g7_qnxswig> -DOUT=${G7_SW}/qnxswig_so.h -DNAME=qnxswig_so -P "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake"
  COMMAND "${CMAKE_COMMAND}" -DIN=${G7_SW}/qnxswig.py -DOUT=${G7_SW}/qnxswig_py.h -DNAME=qnxswig_py -P "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake"
  DEPENDS g7_qnxswig "${G7_SW}/qnxswig.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/bin2h.cmake" VERBATIM)
port_test(test_swig_py          test_swig_py.c)
target_sources(test_swig_py PRIVATE "${G7_SW}/qnxswig_so.h" "${G7_SW}/qnxswig_py.h")
target_include_directories(test_swig_py PRIVATE "${G7_SW}")
port_script(test_swig)
port_script(test_nasm)
port_script(test_unifdef)

# ---- chrpath: test_chrpath lists, replaces and deletes the DT_RPATH of
# test_chrpath_prog (old-style DT_RPATH: --disable-new-dtags; CMake's own
# build rpath left out, so the program carries exactly this path) ----
port_test(test_chrpath_prog     test_chrpath_prog.c      m)
set_target_properties(test_chrpath_prog PROPERTIES SKIP_BUILD_RPATH ON INSTALL_RPATH "")
target_link_options(test_chrpath_prog PRIVATE "-Wl,--disable-new-dtags,-rpath,/opt/qnx-chrpath-test/lib:/usr/local/lib")
port_script(test_chrpath)
port_script(test_scdoc)
port_script(test_ccache)
port_script(test_texinfo)
port_script(test_help2man)

# ---- rhash ----
port_test(test_rhash_lib        test_rhash_lib.c         rhash)
port_script(test_rhash)
port_script(test_zip)

# ---- datefudge: test_datefudge runs test_datefudge_clock under datefudge ----
port_test(test_datefudge_clock  test_datefudge_clock.c)
port_script(test_datefudge)
port_script(test_util_macros)
port_script(test_mate_common)
port_script(test_icon_naming_utils)
port_script(test_intltool)
port_script(test_xfce4_dev_tools)
