#!/bin/sh
# pax-utils: scanelf reads installed QNX ELF binaries -- type, NEEDED list
# (libc), interpreter (ldqnx), soname of a shared library, a symbol search,
# a recursive directory scan -- and dumpelf prints the ELF header as C.
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
N=pax-utils
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
SE=$L/bin/scanelf; X=$L/bin/scanelf
T=$(mktemp -d /tmp/qnxport-pax.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
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
info "$("$SE" -V 2>&1 | head -n 1)"
r=$("$SE" -BF '%o' "$X" 2>&1)
case "$r" in ET_DYN*|ET_EXEC*) ok "scanelf type: $(echo $r | cut -d' ' -f1)" ;; *) bad "type: $r" ;; esac
r=$("$SE" -BF '%n' "$X" 2>&1)
case "$r" in *libc.so*) ok "NEEDED of scanelf includes libc ($(echo $r | cut -d' ' -f1))" ;; *) bad "NEEDED: $r" ;; esac
r=$("$SE" -BF '%i' "$X" 2>&1)
case "$r" in *ldqnx*) ok "interpreter: $(echo $r | cut -d' ' -f1)" ;; *) bad "interpreter: $r" ;; esac
# A shared library from a package: zlib (an IFS image such as /proc/boot
# keeps only program headers, so section-based queries find nothing there).
LIB=$(ls "$L"/lib/libz.so.1.* 2>/dev/null | head -n 1)
if [ -n "$LIB" ]; then
  r=$("$SE" -BF '%S' "$LIB" 2>&1); so=$(echo "$r" | awk '{print $1}')
  [ "$so" = libz.so.1 ] && ok "soname of $(basename "$LIB"): $so" || bad "soname of $LIB: $r"
  r=$("$SE" -Bs inflate "$LIB" 2>&1)
  case "$r" in *inflate*) ok "symbol search: inflate found in libz" ;; *) bad "symbol search: $r" ;; esac
else bad "zlib not installed (the test reads its libz.so.1)"; fi
r=$("$SE" -BF '%S' /proc/boot/libc.so.6 2>/dev/null | awk '{print $1}'); [ -f /proc/boot/libc.so.6 ] && info "/proc/boot/libc.so.6 soname via sections: '${r#/*}' (IFS images strip section headers)"
mkdir d; cp "$X" d/one; cp "$L/bin/dumpelf" d/two; echo "not elf" > d/text
n=$("$SE" -BRF '%f' d 2>/dev/null | wc -l | tr -d ' ')
[ "$n" = 2 ] && ok "recursive scan finds 2 ELF files, skips text" || bad "recursive scan: $n"
r=$("$L/bin/dumpelf" "$X" 2>&1 | head -40)
echo "$r" | has "Elf64_Ehdr" && echo "$r" | has "e_ident" && ok "dumpelf: Elf64_Ehdr with e_ident" || { bad "dumpelf"; echo "$r" | head -5; }
r=$("$SE" -qe "$X" 2>&1)
[ -z "$r" ] && ok "scanelf -e: no executable stack in scanelf" || info "scanelf -e: $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
