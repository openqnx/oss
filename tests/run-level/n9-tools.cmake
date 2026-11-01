# Run-level tests, group n9-tools: bison, elvis, elfutils (libelf), libxo,
# ntp (the reference implementation, in opt/ntp), openpam, ptpd, tcpdump,
# inetutils, libtomcrypt and libtommath, toybox. Each test uses the component
# (known answers), not just its loading. Which test checks which component:
# n9-tools.tsv; programs/data a test needs installed: n9-tools.extras.
# Helper programs (test_bison_calc, test_tcpdump_mkpcap,
# test_inetutils_inetd) are installed as tests too: run alone, each checks
# itself; the shell tests call them with arguments.

# ---- libraries ----
port_test(test_libtommath       test_libtommath.c        tommath)
port_test(test_libtomcrypt      test_libtomcrypt.c       tomcrypt tommath)
# tomcrypt.h declares the libtommath descriptor (ltm_desc) only with these.
target_compile_definitions(test_libtomcrypt PRIVATE USE_LTM LTM_DESC)
port_test(test_libxo            test_libxo.c             xo)
port_test(test_openpam          test_openpam.c           pam)
# elfutils' libelf is in a prefix of its own (the SDP has another libelf):
# its headers first, the library by path.
port_test(test_elfutils         test_elfutils.c          "${STAGE}/opt/elfutils/lib/libelf.so")
target_include_directories(test_elfutils BEFORE PRIVATE "${STAGE}/opt/elfutils/include")

# ---- bison: a parser generated at build time by the build machine's bison
# (the same 3.8.2), and the target's bison on the same grammar ----
find_program(N9_HOST_BISON bison NO_CMAKE_FIND_ROOT_PATH REQUIRED)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test_bison_calc.c"
  COMMAND "${N9_HOST_BISON}" -o "${CMAKE_CURRENT_BINARY_DIR}/test_bison_calc.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/test_bison_calc.y"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/test_bison_calc.y")
# port_test() takes its source under src/: name the generated file from there.
file(RELATIVE_PATH N9_BISON_CALC "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/test_bison_calc.c")
port_test(test_bison_calc       ${N9_BISON_CALC})
port_script(test_bison)

# ---- programs ----
port_script(test_elvis)
port_test(test_tcpdump_mkpcap   test_tcpdump_mkpcap.c)
port_script(test_tcpdump)
port_script(test_ntp)
port_script(test_ptpd)
port_test(test_inetutils_inetd  test_inetutils_inetd.c)
port_script(test_inetutils)
port_script(test_toybox)
