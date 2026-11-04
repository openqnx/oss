#!/bin/sh
# LLD (ld.lld, installed by the llvm component) on the target: llc compiles
# a small IR module to x86_64 and aarch64 ELF objects, ld.lld links each into
# a static executable (entry _start, a call from _start to add() that the
# linker must resolve) and a shared library (-soname, exported symbols), and
# llvm-readelf / llvm-nm / llvm-objdump check the results: ELF type, machine,
# entry point = address of _start, the resolved call target, SONAME and
# dynamic symbols. The executables are Linux-ABI and are not run.
echo "== lld =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-lld.XXXXXX) || { echo "FAIL lld (mktemp)"; exit 1; }
cd "$T" || exit 1

V=$(ld.lld --version 2>&1)
case "$V" in *"LLD 22.1"*) ok "ld.lld --version: LLD 22.1" ;; *) bad "ld.lld --version: $V" ;; esac

cat > t.ll <<'EOF'
@counter = global i32 7

define i32 @add(i32 %a, i32 %b) {
  %s = add i32 %a, %b
  ret i32 %s
}

define void @_start() {
  %r = call i32 @add(i32 40, i32 2)
  store i32 %r, ptr @counter
  br label %spin
spin:
  br label %spin
}
EOF

for tr in x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu; do
  case $tr in x86_64*) mach='X86-64'; emu=elf_x86_64 ;; *) mach='AArch64'; emu=aarch64linux ;; esac
  llc -O1 -mtriple=$tr -relocation-model=pic -filetype=obj t.ll -o t-$tr.o 2>err.txt || bad "llc $tr: $(cat err.txt)"

  ld.lld -m $emu -static -e _start t-$tr.o -o exe-$tr 2>err.txt \
    && ok "ld.lld $tr: static executable linked" || bad "ld.lld $tr: $(cat err.txt)"
  h=$(llvm-readelf -h exe-$tr 2>&1)
  echo "$h" | grep -q 'Type:.*EXEC' && echo "$h" | grep -q "Machine:.*$mach" \
    && ok "llvm-readelf $tr: EXEC, Machine $mach" || bad "llvm-readelf -h $tr: $(echo "$h" | grep -E 'Type|Machine')"
  entry=$(echo "$h" | sed -n 's/.*Entry point address:[[:space:]]*0x0*\([0-9a-f]*\).*/\1/p')
  start=$(llvm-nm exe-$tr 2>/dev/null | sed -n 's/^0*\([0-9a-f]*\) T _start$/\1/p')
  addr=$(llvm-nm exe-$tr 2>/dev/null | sed -n 's/^0*\([0-9a-f]*\) T add$/\1/p')
  [ -n "$entry" ] && [ "$entry" = "$start" ] && ok "entry point 0x$entry = _start" \
    || bad "entry point 0x$entry, _start 0x$start"
  # The call in _start must go to add()'s final address.
  d=$(llvm-objdump -d --no-show-raw-insn exe-$tr 2>&1 | sed -n '/<_start>:/,/^$/p')
  echo "$d" | grep -E -q "(call|bl)[[:space:]]+(0x)?0*$addr[[:space:]]+<add>" \
    && ok "call _start -> add resolved to 0x$addr" || bad "call to add not resolved: $(echo "$d" | head -6)"
  llvm-readelf -l exe-$tr 2>/dev/null | grep -q 'LOAD' && ok "program headers: LOAD segments" || bad "no LOAD segment"

  ld.lld -m $emu -shared -soname libqnxlld.so.1 t-$tr.o -o libt-$tr.so 2>err.txt \
    && ok "ld.lld -shared $tr" || bad "ld.lld -shared $tr: $(cat err.txt)"
  llvm-readelf -d libt-$tr.so 2>/dev/null | grep -q 'SONAME.*libqnxlld.so.1' \
    && ok "SONAME libqnxlld.so.1" || bad "SONAME: $(llvm-readelf -d libt-$tr.so 2>&1 | head -5)"
  ds=$(llvm-nm -D --defined-only libt-$tr.so 2>&1)
  echo "$ds" | grep -q ' T add$' && echo "$ds" | grep -q ' D counter$' \
    && ok "dynamic symbols: add (T), counter (D)" || bad "dynamic symbols: $ds"
done

# An undefined symbol is a link error, reported by name.
printf 'declare i32 @missing()\ndefine i32 @f() {\n  %%r = call i32 @missing()\n  ret i32 %%r\n}\n' > u.ll
llc -mtriple=x86_64-unknown-linux-gnu -filetype=obj u.ll -o u.o 2>/dev/null
if ld.lld -m elf_x86_64 -e f u.o -o u 2>err.txt; then bad "undefined symbol was not an error"
else grep -q 'undefined symbol: missing' err.txt && ok "undefined symbol: missing -> link error" || bad "error text: $(cat err.txt)"; fi

cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS lld"; exit 0; }
echo "FAIL lld ($fail check(s) failed)"; exit 1
