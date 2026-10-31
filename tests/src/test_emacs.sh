#!/bin/sh
# GNU Emacs in batch mode: edits a file (search-and-replace, sort-lines,
# save-buffer), evaluates Emacs Lisp (recursion, mapcar, format, bignums,
# regexps, a JSON round trip), and etags indexes a C file. This Emacs is
# built without a dump image, so every start loads its Lisp: slow, but real.
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
N=emacs
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
E=$L/bin/emacs
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
HOME=$T; TERM=dumb; export HOME TERM
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
v=$(to 150 "$E" --version 2>&1 | head -n 1); case "$v" in "GNU Emacs"*) ok "--version: $v" ;; *) bad "--version: $v" ;; esac
printf 'zeta foo\nalpha foo bar\nmid foo\n' > f.txt
to 150 "$E" --batch -Q f.txt --eval '(progn (goto-char (point-min)) (while (search-forward "foo" nil t) (replace-match "FOO" t t)) (sort-lines nil (point-min) (point-max)) (save-buffer))' > log 2>&1
[ "$(cat f.txt | tr '\n' '|')" = "alpha FOO bar|mid FOO|zeta FOO|" ] && ok "batch edit: replace-match, sort-lines, save-buffer" || { bad "batch edit"; cat f.txt; tail -3 log; }
r=$(to 150 "$E" --batch -Q --eval '(progn
  (defun qfact (n) (if (<= n 1) 1 (* n (qfact (1- n)))))
  (princ (format "fact30=%d\n" (qfact 30)))
  (princ (format "squares=%S\n" (mapcar (lambda (x) (* x x)) (number-sequence 1 5))))
  (princ (format "upper=%s\n" (upcase "qnx emacs")))
  (princ (format "match=%s\n" (and (string-match "\\([0-9]+\\)\\.\\([0-9]+\\)" "SDP 8.0") (match-string 2 "SDP 8.0"))))
  (princ (format "json=%s\n" (gethash "os" (json-parse-string (json-serialize (let ((h (make-hash-table :test (quote equal)))) (puthash "os" "QNX" h) h))))))
  (princ (format "float=%.3f\n" (sqrt 2.0))))' 2> lisp.err)
exp='fact30=265252859812191058636308480000000
squares=(1 4 9 16 25)
upper=QNX EMACS
match=0
json=QNX
float=1.414'
[ "$r" = "$exp" ] && ok "Emacs Lisp: bignum recursion, mapcar, regexp, native JSON, floats" || { bad "Lisp evaluation"; echo "$r" | tail -6; tail -3 lisp.err; }
cat > demo.c <<'EOF'
int qnx_add(int a, int b) { return a + b; }
static void helper(void) { }
struct point { int x, y; };
EOF
"$L/bin/etags" -o TAGS demo.c 2>&1 && awk '/qnx_add/ {a=1} /helper/ {b=1} END {exit !(a && b)}' TAGS && ok "etags indexed qnx_add and helper" || bad "etags"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
