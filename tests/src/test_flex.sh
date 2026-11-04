#!/bin/sh
# flex: test_flex_scan, a tokenizer generated at build time from
# tests/src/test_flex_scan.l by the build machine's flex 2.6.4 and linked with
# the target's libfl (its yywrap), scans a sample (hex and decimal numbers,
# keywords, identifiers, an escaped string, a comment start condition across
# lines, yylineno, an invalid character) with known tokens and counts; then
# the target's flex (which runs m4 on its skeleton) generates the scanner
# from the same file: its DFA tables are identical to the build-time ones;
# flex -+ writes a C++ yyFlexLexer scanner; an unterminated action is
# reported with its line.
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
N=flex
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
FL=$L/bin/flex
r=$("$FL" --version 2>&1); [ "$r" = "flex 2.6.4" ] && ok "flex --version: flex 2.6.4" || bad "version: $r"
HLP=$D/test_flex_scan
if [ -x "$HLP" ]; then
  r=$("$HLP" 2>&1); case "$r" in *"PASS test_flex_scan"*) ok "generated scanner self-check (libfl yywrap)" ;; *) bad "self-check: $r" ;; esac
  r=$(printf 'x = 0x1F + 42; /* skip 7 \n this */\nif (y >= 10) return "a\\"b";\n@\n' | "$HLP" - 2>&1 | tr '\n' '|')
  exp='1 IDENT x|1 CHAR =|1 HEX 0x1F=31|1 CHAR +|1 NUM 42|1 CHAR ;|3 KEYWORD if|3 CHAR (|3 IDENT y|3 OP >=|3 NUM 10|3 CHAR )|3 KEYWORD return|3 STRING "a\"b" (4)|3 CHAR ;|4 ERROR '"'@'"'|lines=5 idents=2 numbers=3 sum=83 strings=1 comments=1|'
  [ "$r" = "$exp" ] && ok "tokens, line numbers, comment state, escaped string, summary" || { bad "token stream"; echo "$r"; }
else bad "test_flex_scan not found next to the test"; fi
cat > scan.l <<'EOL'
/* A small C-like tokenizer: the scanner test_flex_scan is generated from
   this file at build time (the build machine's flex 2.6.4), and test_flex
   runs the target's flex on the same file. "test_flex_scan -" reads stdin and
   prints one line per token, then a summary; with no argument it scans a
   built-in sample and checks the counts itself. No %option noyywrap:
   yywrap() comes from libfl. */
%{
#include <stdio.h>
#include <stdlib.h>
static long sum, idents, numbers, strings, comments;
%}
%option yylineno nounput noinput
%x COMMENT
DIGIT   [0-9]
ID      [A-Za-z_][A-Za-z0-9_]*
%%
"/*"                    { BEGIN(COMMENT); comments++; }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>.|\n           { }
0[xX][0-9a-fA-F]+       { long v = strtol(yytext, NULL, 16); sum += v; numbers++; printf("%d HEX %s=%ld\n", yylineno, yytext, v); }
{DIGIT}+                { long v = atol(yytext); sum += v; numbers++; printf("%d NUM %ld\n", yylineno, v); }
if|else|while|return    { printf("%d KEYWORD %s\n", yylineno, yytext); }
{ID}                    { idents++; printf("%d IDENT %s\n", yylineno, yytext); }
\"([^"\\\n]|\\.)*\"     { strings++; printf("%d STRING %s (%d)\n", yylineno, yytext, (int)yyleng - 2); }
"=="|"<="|">="|"!="     { printf("%d OP %s\n", yylineno, yytext); }
[-+*/=<>;(){}]          { printf("%d CHAR %c\n", yylineno, yytext[0]); }
[ \t\r\n]+              { }
.                       { printf("%d ERROR '%s'\n", yylineno, yytext); }
%%
int main(int argc, char **argv)
{
    if (argc > 1 && argv[1][0] == '-') {
        yylex();
        printf("lines=%d idents=%ld numbers=%ld sum=%ld strings=%ld comments=%ld\n",
               yylineno, idents, numbers, sum, strings, comments);
        return 0;
    }
    /* Self-check: the sample test_flex also feeds through stdin. */
    YY_BUFFER_STATE b = yy_scan_string("x = 0x1F + 42; /* skip 7 \n this */\n"
                                       "if (y >= 10) return \"a\\\"b\";\n@\n");
    yylex();
    yy_delete_buffer(b);
    printf("lines=%d idents=%ld numbers=%ld sum=%ld strings=%ld comments=%ld\n",
           yylineno, idents, numbers, sum, strings, comments);
    if (yylineno == 5 && idents == 2 && numbers == 3 && sum == 83 && strings == 1 && comments == 1) {
        printf("PASS test_flex_scan\n");
        return 0;
    }
    printf("FAIL test_flex_scan\n");
    return 1;
}
EOL
r=$(to 60 "$FL" -o scan.c scan.l 2>&1); rc=$?
[ $rc -eq 0 ] && [ -s scan.c ] && ok "flex generated scan.c (skeleton through m4)" || bad "flex scan.l (rc $rc): $r"
grep -q '^#define YY_NUM_RULES 13$' scan.c && grep -q '^static const flex_int16_t yy_accept\[47\] =' scan.c \
  && ok "13 rules, 47 DFA states" || bad "YY_NUM_RULES / yy_accept size"
c=$(awk '/^static const flex_int16_t yy_accept\[/ {f=1} /^static const flex_int32_t yy_rule_can_match_eol/ {f=0} f' scan.c | cksum | awk '{print $1, $2}')
[ "$c" = "1919225862 4638" ] && ok "DFA tables identical to the build-time scanner's (cksum 1919225862 4638)" || bad "tables cksum: $c"
r=$(to 60 "$FL" -+ -o cxx.cc scan.l 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q 'int yyFlexLexer::yylex()' cxx.cc && grep -q '#include <FlexLexer.h>' cxx.cc \
  && ok "flex -+: C++ yyFlexLexer scanner" || bad "flex -+ (rc $rc): $r"
[ -f "$L/include/FlexLexer.h" ] && grep -q 'class yyFlexLexer' "$L/include/FlexLexer.h" && ok "FlexLexer.h installed (flex-dev)" || bad "FlexLexer.h missing"
printf '%%%%\n"a" { return 1; \n%%%%\n' > bad.l
r=$(to 60 "$FL" bad.l 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "bad.l:4: EOF encountered inside an action" && ok "unterminated action: bad.l:4 error, exit $rc" || bad "bad.l (rc $rc): $r"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
