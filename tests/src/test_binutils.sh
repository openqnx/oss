#!/bin/sh
# GNU binutils, natively on the target: as assembles a small function for
# this CPU (with debug line info), and nm, objdump -d, readelf, size,
# strings, objcopy -O binary (exact machine-code bytes), addr2line, ar,
# ranlib, ld -shared, strip and c++filt each give a known answer about it;
# readelf also reads one of the installed QNX binaries.
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
N=binutils
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
B=$L/bin
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
info "$("$B/as" --version 2>&1 | head -n 1)"
mach=$("$B/readelf" -h "$B/readelf" 2>/dev/null | awk -F: '/Machine:/ {sub(/^ +/, "", $2); print $2}')
info "host binary machine: $mach"
case "$mach" in
  *AArch64*) A=aarch64; INSN="w0, w0, w1"; BYTES=0000010bc0035fd6; TSIZE=8
    printf '\t.text\n\t.globl qnx_add\n\t.type qnx_add, %%function\nqnx_add:\n\tadd w0, w0, w1\n\tret\n\t.size qnx_add, .-qnx_add\n' > t.s ;;
  *) A=x86_64; INSN="lea"; BYTES=8d0437c3; TSIZE=4
    printf '\t.text\n\t.globl qnx_add\n\t.type qnx_add, @function\nqnx_add:\n\tleal (%%rdi,%%rsi), %%eax\n\tret\n\t.size qnx_add, .-qnx_add\n' > t.s ;;
esac
printf '\t.data\n\t.globl qnx_msg\nqnx_msg:\n\t.asciz "QNX-binutils-marker"\n' >> t.s
"$B/as" -g -o t.o t.s 2> as.err && ok "as ($A): t.o assembled" || { bad "as"; head -3 as.err; }
"$B/nm" t.o 2>&1 | has "T qnx_add" && ok "nm: T qnx_add" || bad "nm qnx_add"
"$B/nm" t.o 2>&1 | has "D qnx_msg" && ok "nm: D qnx_msg" || bad "nm qnx_msg"
d=$("$B/objdump" -d t.o 2>&1)
echo "$d" | has "<qnx_add>:" && echo "$d" | has "$INSN" && ok "objdump -d: <qnx_add> with '$INSN'" || { bad "objdump -d"; echo "$d" | tail -4; }
"$B/readelf" -h t.o 2>&1 | has "REL (Relocatable file)" && ok "readelf -h: relocatable" || bad "readelf -h"
"$B/readelf" -s t.o 2>&1 | awk '/qnx_add/ && /FUNC/ && /GLOBAL/ {f=1} END {exit !f}' && ok "readelf -s: qnx_add FUNC GLOBAL" || bad "readelf -s"
s=$("$B/size" t.o 2>&1 | awk 'NR==2 {print $1}')
[ "$s" = "$TSIZE" ] && ok "size: text = $TSIZE bytes" || bad "size: text = $s, want $TSIZE"
"$B/strings" t.o 2>&1 | has "QNX-binutils-marker" && ok "strings finds the marker" || bad "strings"
"$B/objcopy" -O binary -j .text t.o t.bin && [ "$(od -An -tx1 t.bin | tr -d ' \n')" = "$BYTES" ] && ok "objcopy -O binary: machine code $BYTES" || bad "objcopy bytes: $(od -An -tx1 t.bin 2>/dev/null | tr -d ' \n')"
ln=$(awk '/qnx_add:/ {print NR + 1; exit}' t.s)
r=$("$B/addr2line" -e t.o 0 2>&1)
case "$r" in *t.s:$ln) ok "addr2line 0 -> t.s:$ln" ;; *) bad "addr2line: $r (want t.s:$ln)" ;; esac
"$B/ar" rc libq.a t.o && "$B/ranlib" libq.a && [ "$("$B/ar" t libq.a)" = t.o ] && ok "ar rc / ranlib / ar t" || bad "ar"
"$B/nm" -s libq.a 2>&1 | awk '/Archive index/ {i=1} i && /qnx_add in t.o/ {f=1} END {exit !f}' && ok "nm -s: archive index has qnx_add" || bad "archive index"
"$B/ld" -shared -o libq.so t.o 2> ld.err && "$B/readelf" -h libq.so | has "DYN (Shared object file)" && ok "ld -shared: libq.so" || { bad "ld -shared"; head -3 ld.err; }
"$B/readelf" --dyn-syms libq.so 2>&1 | has qnx_add && ok "libq.so exports qnx_add" || bad "dynamic symbol"
cp libq.so s.so && "$B/strip" s.so && [ "$(wc -c < s.so)" -lt "$(wc -c < libq.so)" ] && "$B/nm" -D s.so | has qnx_add && ok "strip: smaller, dynamic symbols kept" || bad "strip"
r=$(echo _ZN3qnx6kernel5startEv | "$B/c++filt")
[ "$r" = "qnx::kernel::start()" ] && ok "c++filt: $r" || bad "c++filt: $r"
"$B/readelf" -l "$B/objdump" 2>&1 | has ldqnx && ok "readelf -l on objdump: QNX interpreter (ldqnx)" || bad "readelf -l interpreter"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
