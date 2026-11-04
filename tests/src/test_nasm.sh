#!/bin/sh
# nasm: the target's nasm assembles a small x86-64 source (a %define, a
# %macro, RIP-relative lea, a string, times, equ) to a flat binary whose
# 39 bytes are known exactly, and to an ELF64 object (x86-64 machine type,
# the same object the build machine's nasm 2.16.03 makes); ndisasm
# disassembles the binary back to the instructions; a syntax error is
# reported with its line. The code is only assembled, never run (it is
# x86 code also on aarch64).
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
N=nasm
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
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
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
NA=$L/bin/nasm; ND=$L/bin/ndisasm
r=$("$NA" -v 2>&1); echo "$r" | has "NASM version 2.16.03" && ok "nasm -v: 2.16.03" || bad "version: $r"
cat > t.asm <<'EOA'
; NASM test: x86-64 code with a macro, a times loop and a data table.
bits 64
%define ANSWER 42
%macro ADDI 2
        add %1, %2
%endmacro
start:  mov eax, ANSWER
        ADDI eax, 0x100
        xor rdi, rdi
        lea rsi, [rel msg]
        syscall
        ret
msg:    db "QNX", 0
        times 4 dw 0xBEEF
len     equ $ - msg
        dd len
EOA
r=$(to 60 "$NA" -f bin -o t.bin -l t.lst t.asm 2>&1); rc=$?
b=$(od -A n -t x1 -v t.bin | tr -s ' \n' '  ' | sed 's/^ //; s/ $//')
[ $rc -eq 0 ] && [ "$b" = "b8 2a 00 00 00 05 00 01 00 00 48 31 ff 48 8d 35 03 00 00 00 0f 05 c3 51 4e 58 00 ef be ef be ef be ef be 0c 00 00 00" ] \
  && ok "-f bin: the 39 bytes are exactly the expected encoding" || bad "-f bin (rc $rc): $b $r"
grep -q 'ADDI eax, 0x100' t.lst && ok "listing file written (-l)" || bad "listing"
r=$("$ND" -b 64 t.bin 2>&1 | head -n 6 | awk '{$1=""; $2=""; sub(/^  /, ""); print}' | tr '\n' '|')
[ "$r" = "mov eax,0x2a|add eax,0x100|xor rdi,rdi|lea rsi,[rel 0x17]|syscall|ret|" ] && ok "ndisasm -b 64: the six instructions back" || bad "ndisasm: $r"
r=$(to 60 "$NA" -f elf64 -o t.o t.asm 2>&1); rc=$?
h=$(od -A n -t x1 -N 20 t.o | tr -s ' \n' '  ' | sed 's/^ //; s/ $//'); c=$(cksum < t.o | awk '{print $1, $2}')
[ $rc -eq 0 ] && [ "$h" = "7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 01 00 3e 00" ] && ok "-f elf64: ELF64 relocatable, machine x86-64 (62)" || bad "elf64 header (rc $rc): $h"
[ "$c" = "576074934 656" ] && ok "ELF object identical to the build machine's nasm output (cksum $c)" || bad "t.o cksum: $c"
printf 'bits 64\n mov eax, [rbx+\n' > bad.asm
r=$(to 60 "$NA" -f bin bad.asm 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "bad.asm:2: error: expression syntax error" && ok "syntax error at bad.asm:2 (exit $rc)" || bad "bad.asm (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
