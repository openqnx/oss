#!/bin/sh
# csmith: generate random C programs from fixed seeds. The same seed must give
# byte-identical output (the generator uses lrand48, a fixed POSIX LCG), a
# different seed a different program, and every program must be a complete
# translation unit: csmith.h include, func_1, main with the checksum print,
# and balanced braces. Options (--no-pointers, --max-funcs, --output) take
# effect. No compiler is assumed on the target, so the programs are checked
# structurally, not compiled.
echo "== csmith =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
C=/usr/local/bin/csmith; [ -x "$C" ] || C=csmith
T=$(mktemp -d /tmp/qnxport-csmith.XXXXXX) || { echo "FAIL csmith (mktemp)"; exit 1; }
cd "$T" || exit 1
v=$("$C" --version 2>&1 | head -n 1); echo "  info $v"
[ "$v" = "csmith 2.3.0" ] && ok "csmith --version: csmith 2.3.0" || bad "csmith --version: $v"

"$C" --seed 12345 > a.c 2>a.err; rc1=$?
"$C" --seed 12345 > b.c 2>>a.err; rc2=$?
"$C" --seed 54321 > c.c 2>>a.err; rc3=$?
[ $rc1 -eq 0 ] && [ $rc2 -eq 0 ] && [ $rc3 -eq 0 ] && [ -s a.c ] && ok "three programs generated ($(wc -l < a.c), $(wc -l < c.c) lines)" \
  || bad "generation failed: $(head -n 3 a.err)"
echo "  info seed 12345: $(wc -l < a.c | tr -d ' ') lines (csmith 2.3.0 on Linux x86_64 gives 1687)"
cmp -s a.c b.c && ok "same seed -> identical program" || bad "seed 12345 is not reproducible"
cmp -s a.c c.c && bad "different seeds gave the same program" || ok "different seed -> different program"
grep -q '^ \* Seed:      12345$' a.c && ok "header records the seed" || bad "no 'Seed: 12345' header"
grep -q '^ \* Generator: csmith 2.3.0$' a.c && ok "header names the generator" || bad "no generator header"
grep -q '^#include "csmith.h"' a.c && ok "includes csmith.h" || bad "no csmith.h include"
grep -q '^int main (int argc, char\* argv\[\])' a.c && ok "defines main" || bad "no main()"
grep -q 'platform_main_end(crc32_context ^ 0xFFFFFFFFUL, print_hash_value);' a.c && ok "main prints the checksum" || bad "no checksum print"
grep -q 'func_1(void)' a.c && ok "defines func_1" || bad "no func_1"
o=$(tr -cd '{' < a.c | wc -c); c=$(tr -cd '}' < a.c | wc -c)
o=$((o+0)); c=$((c+0))
[ "$o" -eq "$c" ] && [ "$o" -gt 3 ] && ok "braces balanced ($o pairs)" || bad "braces: $o open, $c close"

"$C" --seed 7 --no-pointers --max-funcs 2 --output d.c 2>d.err
if [ -s d.c ]; then
  ok "--output writes the file"
  grep -q 'Options:   --seed 7 --no-pointers --max-funcs 2 --output d.c' d.c && ok "header lists the options" || bad "options header: $(grep Options: d.c)"
  # With --no-pointers no pointer declarations appear ("int *" style).
  n=$(grep -c '^ *[a-z_0-9]* \*\**[ a-z_]*[gl]_[0-9]' d.c)
  [ "$n" -eq 0 ] && ok "--no-pointers: no pointer variables" || bad "--no-pointers: $n pointer declarations"
  f=$(grep -c '^static .* func_[0-9]*(.*);$' d.c)
  [ "$f" -le 2 ] && [ "$f" -ge 1 ] && ok "--max-funcs 2: $f function prototype(s)" || bad "--max-funcs 2: $f prototypes"
else
  bad "--output d.c: $(head -n 3 d.err)"
fi
"$C" --seed 3 --nomain > e.c 2>/dev/null
grep -q '^int main' e.c && bad "--nomain still emitted main" || ok "--nomain: no main()"
"$C" --no-such-option >/dev/null 2>&1 && bad "unknown option accepted" || ok "unknown option rejected"
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS csmith"; exit 0; }
echo "FAIL csmith ($fail check(s) failed)"; exit 1
