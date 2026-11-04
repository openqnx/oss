#!/bin/sh
# swig: the target's swig 4.3.1 (built with PCRE2) generates the Python and
# the C wrappers for tests/src/test_swig_example.i (embedded below): every
# generated file is byte-identical to what the build machine's swig 4.3.1
# makes from the same input (known checksums); the regex %rename worked
# (gcd, not qs_gcd); then the target's python3 imports the Python module
# the target just generated, over the extension built at build time
# (test_swig_py writes _qnxswig.so), and calls it; a syntax error in an
# interface is reported with its line. test_swig_c and test_swig_py run the
# build-time wrappers (C, Python) on their own.
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
N=swig
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
SW=$L/bin/swig
r=$("$SW" -version 2>&1); echo "$r" | has "SWIG Version 4.3.1" && ok "swig -version: 4.3.1" || bad "version: $r"
echo "$r" | has "Configured options: +pcre" && ok "built with PCRE2 (+pcre)" || bad "no +pcre: $r"
r=$("$SW" -swiglib 2>&1); [ "$r" = "$L/share/swig/4.3.1" ] || [ "$r" = "/usr/local/share/swig/4.3.1" ] && ok "swig library: $r" || bad "swiglib: $r"
cat > test_swig_example.h <<'EOH'
/* The C library the swig test wraps (tests/src/test_swig_example.i): plain
   functions, a global variable, a struct, a string result. */
#ifndef TEST_SWIG_EXAMPLE_H
#define TEST_SWIG_EXAMPLE_H
extern int qs_calls;                       /* counts the calls below */
int qs_gcd(int a, int b);
double qs_hypot3(double x, double y, double z);
unsigned qs_crc8(const char *s);           /* CRC-8 (poly 0x07) of a string */
char *qs_reverse(const char *s);           /* new string, caller frees */
typedef struct { int n; long total; } qs_acc;
void qs_acc_add(qs_acc *a, int v);
#endif
EOH
cat > test_swig_example.i <<'EOH'
/* SWIG interface for test_swig_example.h. The regex %rename (PCRE2) drops
   the qs_ prefix in every target language: qs_gcd becomes gcd. */
%module qnxswig
%{
#include "test_swig_example.h"
%}
%rename("%(regex:/^qs_(.*)$/\\1/)s") "";
%newobject qs_reverse;
%include "test_swig_example.h"
EOH
r=$(to 60 "$SW" -python -o py_wrap.c test_swig_example.i 2>&1); rc=$?
[ $rc -eq 0 ] && [ -s py_wrap.c ] && [ -s qnxswig.py ] && ok "swig -python: py_wrap.c and qnxswig.py" || bad "swig -python (rc $rc): $r"
r=$(to 60 "$SW" -c -o c_wrap.c test_swig_example.i 2>&1); rc=$?
[ $rc -eq 0 ] && [ -s c_wrap.c ] && [ -s c_wrap.h ] && ok "swig -c (experimental C target): c_wrap.c and c_wrap.h" || bad "swig -c (rc $rc): $r"
for x in "py_wrap.c 3352411739 130999" "qnxswig.py 2676095911 2706" "c_wrap.c 3809040953 7257" "c_wrap.h 914484883 1238"; do
  f=${x%% *}; c=$(cksum < "$f" 2>/dev/null | awk '{print $1, $2}')
  [ "$c" = "${x#* }" ] && ok "$f identical to the build machine's swig output (cksum $c)" || bad "$f: cksum $c, expected ${x#* }"
done
grep -q 'SWIGIMPORT int qnxswig_gcd(int carg1, int carg2);' c_wrap.h && grep -q '^def gcd(a, b):' qnxswig.py \
  && ok "PCRE2 %rename: qs_gcd is qnxswig_gcd (C) and gcd (Python)" || bad "renamed functions missing"
PY=$L/bin/python3
if [ -x "$PY" ] && [ -x "$D/test_swig_py" ]; then
  mkdir m && "$D/test_swig_py" m && cp qnxswig.py m/qnxswig.py || bad "test_swig_py could not write the extension"
  r=$(cd m && PYTHONPATH=. to 60 "$PY" -c 'import qnxswig as q; a = q.acc(); q.acc_add(a, 40); q.acc_add(a, 2); print(q.gcd(1071, 462), q.crc8("QNX"), q.reverse("abc"), a.n, a.total, q.cvar.calls)' 2>&1)
  [ "$r" = "21 77 cba 2 42 5" ] && ok "python3: the target-generated qnxswig.py over _qnxswig.so: gcd 21, crc8, reverse, struct, cvar" || bad "python3 import: $r"
else bad "python3 or test_swig_py missing"; fi
printf '%%module bad\nint f(int;\n' > bad.i
r=$(to 60 "$SW" -python bad.i 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "bad.i:2: Error: Syntax error in input(1)." && ok "syntax error reported at bad.i:2 (exit $rc)" || bad "bad.i (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
