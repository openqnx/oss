#!/bin/sh
# GNU Bison: test_bison_calc, a calculator parser generated at build time
# from tests/src/test_bison_calc.y, evaluates expressions (precedence,
# associativity, unary minus, a semantic error, a syntax error); then the
# target's bison (which runs m4 for its skeletons) generates the parser
# from the same grammar: the tables match the build-time ones, the report
# lists its 22 states, and an ambiguous grammar gets its one
# shift/reduce conflict reported.
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
N=bison
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
BI=$L/bin/bison
info "$("$BI" --version 2>&1 | head -n 1)"
HLP=$D/test_bison_calc
if [ -x "$HLP" ]; then
  r=$("$HLP" "2+3*4" "(2+3)*4" "2^3^2" "-3^2" "7%3" "100/7-1" 2>&1)
  exp="2+3*4 = 14
(2+3)*4 = 20
2^3^2 = 512
-3^2 = -9
7%3 = 1
100/7-1 = 13"
  [ "$r" = "$exp" ] && ok "generated parser: precedence, right-assoc ^, unary minus" || { bad "generated parser"; echo "$r"; }
  r=$("$HLP" "1/0" 2>&1); case "$r" in *"division by zero"*) ok "semantic action error (YYERROR)" ;; *) bad "1/0: $r" ;; esac
  r=$("$HLP" "2+" 2>&1); case "$r" in *"syntax error"*) ok "syntax error reported" ;; *) bad "2+: $r" ;; esac
else bad "test_bison_calc not found next to the test"; fi
cat > calc.y <<'EOG'
/* A desk calculator: the parser test_bison_calc is generated from this
   grammar at build time (bison -o test_bison_calc.c), and test_bison runs
   the target's bison on the same grammar. */
%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
static const char *in;
static long result;
int yylex(void);
void yyerror(const char *s);
%}
%define api.value.type {long}
%token NUM
%left '+' '-'
%left '*' '/' '%'
%precedence NEG
%right '^'
%%
input: expr              { result = $1; }
     ;
expr: NUM
    | expr '+' expr      { $$ = $1 + $3; }
    | expr '-' expr      { $$ = $1 - $3; }
    | expr '*' expr      { $$ = $1 * $3; }
    | expr '/' expr      { if ($3 == 0) { yyerror("division by zero"); YYERROR; } $$ = $1 / $3; }
    | expr '%' expr      { if ($3 == 0) { yyerror("division by zero"); YYERROR; } $$ = $1 % $3; }
    | '-' expr %prec NEG { $$ = -$2; }
    | expr '^' expr      { long r = 1, i; for (i = 0; i < $3; i++) r *= $1; $$ = r; }
    | '(' expr ')'       { $$ = $2; }
    ;
%%
EOG
r=$(to 60 "$BI" --report=state -o calc.c calc.y 2>&1); rc=$?
[ $rc -eq 0 ] && [ -s calc.c ] && ok "bison generated calc.c (its skeletons expanded by m4)" || { bad "bison calc.y (rc $rc): $r"; }
has "A Bison parser, made by GNU Bison 3.8.2." < calc.c && ok "made by GNU Bison 3.8.2" || bad "calc.c header"
for d in "YYFINAL  8" "YYLAST   33" "YYNTOKENS  13" "YYNRULES  11" "YYNSTATES  22"; do
  has "#define $d" < calc.c || { bad "calc.c: #define $d missing"; }
done && ok "table sizes: YYFINAL 8, YYLAST 33, 13 tokens, 11 rules, 22 states"
t=$(awk '/yytable\[\] =/ {f=1} f {printf "%s", $0} f && /};/ {exit}' calc.c | tr -d ' ')
[ "$t" = "staticconstyytype_int8yytable[]={6,7,11,12,13,8,14,16,17,18,19,20,21,9,10,11,12,13,1,14,2,15,14,0,0,0,3,9,10,11,12,13,0,14};" ] \
  && ok "yytable identical to the build-time parser's" || { bad "yytable: $t"; }
n=$(grep -c '^State ' calc.output 2>/dev/null)
[ "$n" = 22 ] && ok "--report=state: 22 states in calc.output" || bad "calc.output states: $n"
printf '%%%%\ne: e %s e | %s ;\n' "'+'" "'n'" > amb.y
r=$(to 60 "$BI" -o amb.c amb.y 2>&1)
echo "$r" | has "1 shift/reduce conflict" && ok "ambiguous grammar: 1 shift/reduce conflict reported" || bad "amb.y: $r"
printf '%%%%\ne: e e e ;\n%%%%%%\n' > bad.y
r=$(to 60 "$BI" -o bad.c bad.y 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "bad.y:" && ok "a malformed grammar fails with a located error" || bad "bad.y (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
