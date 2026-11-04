#!/bin/sh
# CMake, CTest and CPack: a script (-P) doing string, list, math, regex and
# JSON work; cmake -E tools (sha256sum, tar); then a LANGUAGES NONE project
# configured with the Ninja generator whose custom command generates a file,
# built, tested with ctest (one passing, one expected-to-fail test) and
# packaged with cpack -G TGZ. (No compiler is needed for any of it.)
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=cmake
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
CM=$L/bin/cmake; CT=$L/bin/ctest; CP=$L/bin/cpack
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path (track PID;
# qstop_hook for anything else); the outer shell above kills them on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
qstop_hook() { :; }
stopall() {
  qstop_hook 2>/dev/null
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
info "$("$CM" --version 2>&1 | head -n 1)"
cat > s.cmake <<'EOF'
set(l c a b)
list(SORT l)
list(JOIN l "," j)
string(TOUPPER "qnx neutrino" u)
math(EXPR m "(6 * 7) << 1" OUTPUT_FORMAT HEXADECIMAL)
string(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\2.\\1" r "8.0")
string(JSON jv GET [=[{"os":{"name":"QNX","ver":[8,0]}}]=] os ver 0)
string(SHA256 h "abc")
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/w.txt" "line1\nline2\n")
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/w.txt" lines)
list(LENGTH lines nl)
message("j=${j} u=${u} m=${m} r=${r} json=${jv} lines=${nl}")
message("sha=${h}")
EOF
r=$("$CM" -P s.cmake 2>&1)
exp='j=a,b,c u=QNX NEUTRINO m=0x54 r=0.8 json=8 lines=2
sha=ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'
[ "$r" = "$exp" ] && ok "cmake -P: list/string/math/regex/JSON/SHA256/file" || { bad "cmake -P output"; echo "$r" | head -5; }
printf abc > abc.txt
r=$("$CM" -E sha256sum abc.txt 2>&1 | cut -d' ' -f1)
[ "$r" = ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad ] && ok "cmake -E sha256sum" || bad "cmake -E sha256sum: $r"
mkdir td; echo data > td/f.txt
"$CM" -E tar czf td.tgz td && rm -rf td && "$CM" -E tar xzf td.tgz && [ "$(cat td/f.txt)" = data ] && ok "cmake -E tar czf/xzf round trip" || bad "cmake -E tar"
mkdir proj
cat > proj/CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.20)
project(qnxdemo VERSION 1.2.3 LANGUAGES NONE)
add_custom_command(OUTPUT gen.txt
  COMMAND ${CMAKE_COMMAND} -E echo "generated ${PROJECT_VERSION}" > gen.txt
  COMMENT "generating gen.txt")
add_custom_target(gen ALL DEPENDS gen.txt)
enable_testing()
add_test(NAME passes COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_BINARY_DIR}/gen.txt ${CMAKE_SOURCE_DIR}/expected.txt)
add_test(NAME fails COMMAND ${CMAKE_COMMAND} -E false)
set_tests_properties(fails PROPERTIES WILL_FAIL TRUE)
install(FILES ${CMAKE_BINARY_DIR}/gen.txt DESTINATION share/qnxdemo)
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_FILE_NAME qnxdemo-pkg)
include(CPack)
EOF
echo "generated 1.2.3" > proj/expected.txt
if [ -x "$L/bin/ninja" ] || command -v ninja >/dev/null 2>&1; then G=Ninja; else G="Unix Makefiles"; fi
"$CM" -S proj -B build -G "$G" > cfg.log 2>&1 && ok "configure ($G generator, LANGUAGES NONE)" || { bad "configure"; tail -15 cfg.log; }
"$CM" --build build > bld.log 2>&1 && [ "$(cat build/gen.txt 2>/dev/null)" = "generated 1.2.3" ] && ok "build: custom command made gen.txt" || { bad "build"; tail -10 bld.log; }
"$CT" --test-dir build > ct.log 2>&1 && awk '/100% tests passed/ {f=1} END {exit !f}' ct.log && ok "ctest: 2 tests, 100% passed (one WILL_FAIL)" || { bad "ctest"; tail -10 ct.log; }
"$CM" --install build --prefix "$T/inst" > inst.log 2>&1 && [ -f inst/share/qnxdemo/gen.txt ] && ok "cmake --install" || { bad "install"; tail -5 inst.log; }
( cd build && "$CP" > ../cp.log 2>&1 ) && [ -f build/qnxdemo-pkg.tar.gz ] && gzip -dc build/qnxdemo-pkg.tar.gz | tar -tf - 2>/dev/null | awk '/share\/qnxdemo\/gen.txt$/ {f=1} END {exit !f}' \
  && ok "cpack -G TGZ: package holds share/qnxdemo/gen.txt" || { bad "cpack"; tail -5 cp.log; }
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
