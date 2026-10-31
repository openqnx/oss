#!/bin/sh
# LLVM/Clang/LLD on the target: assemble a tiny IR module (llvm-as/llvm-dis
# round trip), interpret it with lli (main returns a known exit code),
# optimise it with opt (constant folding leaves "ret i32 42"), compile it
# with llc for x86_64, aarch64 and wasm32 and inspect the objects with
# llvm-nm / llvm-objdump / llvm-readelf, compile C with clang -c, archive with
# llvm-ar, and check llvm-config's version.
echo "== llvm =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-llvm.XXXXXX) || { echo "FAIL llvm (mktemp)"; exit 1; }
cd "$T" || exit 1

V=$(llvm-config --version 2>&1)
echo "  info llvm-config --version: $V; host target $(llvm-config --host-target 2>&1)"
case "$V" in 22.1.*) ok "llvm-config --version is 22.1.x" ;; *) bad "llvm-config --version: $V" ;; esac
case " $(llvm-config --targets-built 2>&1) " in
  *AArch64*X86*|*X86*AArch64*) ok "targets built include AArch64 and X86" ;;
  *) bad "llvm-config --targets-built: $(llvm-config --targets-built 2>&1)" ;;
esac

cat > t.ll <<'EOF'
define i32 @add(i32 %a, i32 %b) {
entry:
  %s = add i32 %a, %b
  ret i32 %s
}

define i32 @answer() {
entry:
  %x = mul i32 6, 7
  ret i32 %x
}

define i32 @main() {
entry:
  %r = call i32 @add(i32 40, i32 2)
  ret i32 %r
}
EOF

llvm-as t.ll -o t.bc 2>err.txt && [ "$(od -An -c -N2 t.bc | tr -d ' ')" = BC ] \
  && ok "llvm-as: bitcode written (BC magic)" || bad "llvm-as: $(cat err.txt)"
llvm-dis t.bc -o t2.ll 2>err.txt && grep -q 'define i32 @add(i32 %a, i32 %b)' t2.ll \
  && ok "llvm-dis: round trip keeps @add" || bad "llvm-dis: $(cat err.txt)"

lli -force-interpreter t.bc > /dev/null 2>err.txt; rc=$?
[ $rc -eq 42 ] && ok "lli -force-interpreter: main() returns 42" || bad "lli interpreter: rc=$rc $(cat err.txt)"
lli t.bc > /dev/null 2>err.txt; rc=$?
[ $rc -eq 42 ] && ok "lli (JIT): main() returns 42" \
  || { echo "  info lli JIT: rc=$rc $(head -c 300 err.txt)"; bad "lli JIT: main() should return 42"; }

opt -passes=instcombine -S t.ll -o t.opt.ll 2>err.txt && \
  sed -n '/@answer/,/^}/p' t.opt.ll | grep -q 'ret i32 42' && ! sed -n '/@answer/,/^}/p' t.opt.ll | grep -q mul \
  && ok "opt instcombine folds 6*7 to ret i32 42" || bad "opt: $(cat err.txt)"

for tr in x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu; do
  llc -O2 -mtriple=$tr -filetype=obj t.ll -o t-$tr.o 2>err.txt
  if [ -s t-$tr.o ] && [ "$(od -An -c -j1 -N3 t-$tr.o | tr -d ' ')" = ELF ]; then ok "llc $tr: ELF object"
  else bad "llc $tr: $(cat err.txt)"; fi
  nm=$(llvm-nm t-$tr.o 2>&1)
  echo "$nm" | grep -q ' T add$' && echo "$nm" | grep -q ' T answer$' && echo "$nm" | grep -q ' T main$' \
    && ok "llvm-nm $tr: T add / answer / main" || bad "llvm-nm $tr: $nm"
  case $tr in x86_64*) m='elf64-x86-64'; i='lea|add' ;; *) m='elf64-littleaarch64'; i='add' ;; esac
  d=$(llvm-objdump -d t-$tr.o 2>&1)
  echo "$d" | grep -q "file format $m" && echo "$d" | grep -q '<add>:' && echo "$d" | grep -E -q "$i" \
    && ok "llvm-objdump -d $tr: $m, <add> disassembled" || bad "llvm-objdump $tr: $(echo "$d" | head -5)"
done
case "$(llvm-readelf -h t-aarch64-unknown-linux-gnu.o 2>&1)" in
  *AArch64*) ok "llvm-readelf -h: Machine AArch64" ;; *) bad "llvm-readelf -h" ;;
esac
llc -mtriple=wasm32-unknown-unknown -filetype=obj t.ll -o t.wasm 2>err.txt \
  && [ "$(od -An -c -j1 -N3 t.wasm | tr -d ' ')" = asm ] && ok "llc wasm32: \\0asm object" || bad "llc wasm32: $(cat err.txt)"
llc -mtriple=x86_64-unknown-linux-gnu t.ll -o t.s 2>err.txt && grep -q 'add:' t.s \
  && ok "llc: x86_64 assembly text" || bad "llc -S: $(cat err.txt)"
llvm-mc -triple=x86_64-unknown-linux-gnu -filetype=obj t.s -o t-mc.o 2>err.txt \
  && llvm-nm t-mc.o | grep -q ' T add$' && ok "llvm-mc assembles the llc output" || bad "llvm-mc: $(cat err.txt)"

cat > c.c <<'EOF'
static int square(int x) { return x * x; }
int sum_squares(int n) { int s = 0; for (int i = 1; i <= n; i++) s += square(i); return s; }
const char qnx_banner[] = "hello from clang on QNX";
EOF
clang --target=x86_64-unknown-linux-gnu -O2 -c c.c -o c.o 2>err.txt \
  && llvm-nm c.o | grep -q ' T sum_squares$' && llvm-nm c.o | grep -q ' R qnx_banner$' \
  && ok "clang -c: sum_squares (T) and qnx_banner (R)" || bad "clang -c: $(head -c 400 err.txt)"
llvm-strings c.o 2>/dev/null | grep -q 'hello from clang on QNX' && ok "llvm-strings finds the banner" || bad "llvm-strings"
clang --target=x86_64-unknown-linux-gnu -O2 -S -emit-llvm c.c -o c.ll 2>err.txt && grep -q 'define.*@sum_squares' c.ll \
  && ok "clang -emit-llvm" || bad "clang -emit-llvm: $(head -c 400 err.txt)"
llvm-ar rcs libt.a c.o t-x86_64-unknown-linux-gnu.o 2>err.txt && [ "$(llvm-ar t libt.a | tr '\n' ' ')" = "c.o t-x86_64-unknown-linux-gnu.o " ] \
  && ok "llvm-ar: archive with 2 members" || bad "llvm-ar: $(cat err.txt)"

cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS llvm"; exit 0; }
echo "FAIL llvm ($fail check(s) failed)"; exit 1
