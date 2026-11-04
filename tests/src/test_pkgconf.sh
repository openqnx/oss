#!/bin/sh
# pkgconf: resolve a private set of .pc files with known answers -- cflags and
# libs through a Requires chain, --static with Libs.private/Requires.private,
# version comparisons, variables, --print-requires, a missing module and a
# version conflict. PKG_CONFIG_LIBDIR points only at the test's own files.
echo "== pkgconf =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
# pkgconf ends flag lists with a blank; compare without trailing blanks.
check() { g=$(printf '%s' "$3" | sed 's/[[:space:]]*$//'); if [ "$g" = "$2" ]; then ok "$1"; else bad "$1: expected '$2', got '$g'"; fi; }
P=/usr/local/bin/pkgconf; [ -x "$P" ] || P=pkgconf
T=$(mktemp -d /tmp/qnxport-pkgconf.XXXXXX) || { echo "FAIL pkgconf (mktemp)"; exit 1; }
cd "$T" || exit 1
mkdir pc
cat > pc/qbase.pc <<'EOF'
prefix=/opt/q
libdir=${prefix}/lib
includedir=${prefix}/include
answer=42

Name: qbase
Description: QNX port test base library
Version: 1.4.2
Cflags: -I${includedir}/qbase -DQBASE=1
Libs: -L${libdir} -lqbase
Libs.private: -lm
EOF
cat > pc/qhelper.pc <<'EOF'
prefix=/opt/q
Name: qhelper
Description: private helper
Version: 0.9
Libs: -lqhelper
EOF
cat > pc/qtop.pc <<'EOF'
prefix=/opt/q
includedir=${prefix}/include
Name: qtop
Description: QNX port test top library
Version: 2.0.0
Requires: qbase >= 1.2
Requires.private: qhelper
Cflags: -I${includedir}/qtop
Libs: -L${prefix}/lib -lqtop
EOF
cat > pc/qbad.pc <<'EOF'
Name: qbad
Description: needs a newer qbase
Version: 1.0
Requires: qbase >= 3
Libs: -lqbad
EOF
PKG_CONFIG_LIBDIR="$T/pc"; export PKG_CONFIG_LIBDIR
unset PKG_CONFIG_PATH PKG_CONFIG_SYSROOT_DIR 2>/dev/null
v=$("$P" --version 2>&1); echo "  info pkgconf --version: $v"
check "pkgconf --version" "2.3.0" "$v"
check "--modversion" "2.0.0" "$("$P" --modversion qtop 2>&1)"
check "--cflags through Requires" "-I/opt/q/include/qtop -I/opt/q/include/qbase -DQBASE=1" "$("$P" --cflags qtop 2>&1)"
check "--libs through Requires" "-L/opt/q/lib -lqtop -lqbase" "$("$P" --libs qtop 2>&1)"
check "--libs --static adds private libs" "-L/opt/q/lib -lqtop -lqbase -lm -lqhelper" "$("$P" --libs --static qtop 2>&1)"
check "--libs-only-l" "-lqtop -lqbase" "$("$P" --libs-only-l qtop 2>&1)"
check "--cflags-only-I" "-I/opt/q/include/qtop -I/opt/q/include/qbase" "$("$P" --cflags-only-I qtop 2>&1)"
check "--variable" "42" "$("$P" --variable=answer qbase 2>&1)"
check "--variable expands \${prefix}" "/opt/q/lib" "$("$P" --variable=libdir qbase 2>&1)"
check "--define-variable" "/x/lib" "$("$P" --define-variable=prefix=/x --variable=libdir qbase 2>&1)"
check "--print-requires" "qbase >= 1.2" "$("$P" --print-requires qtop 2>&1)"
check "--print-requires-private" "qhelper" "$("$P" --print-requires-private qtop 2>&1)"
"$P" --exists qtop && ok "--exists qtop" || bad "--exists qtop"
"$P" --exists 'qbase >= 1.4.1' && ok "--exists qbase >= 1.4.1" || bad "--exists qbase >= 1.4.1"
"$P" --exists 'qbase > 1.4.10' && bad "1.4.2 > 1.4.10 must be false (numeric compare)" || ok "1.4.2 is not > 1.4.10"
"$P" --atleast-version=1.4 qbase && ok "--atleast-version=1.4" || bad "--atleast-version=1.4"
"$P" --max-version=1.4 qbase && bad "--max-version=1.4 must fail for 1.4.2" || ok "--max-version=1.4 fails for 1.4.2"
"$P" --exists nosuchmodule 2>/dev/null && bad "missing module reported present" || ok "missing module: --exists fails"
out=$("$P" --libs qbad 2>&1); rc=$?
echo "  info qbad: $(echo "$out" | head -n 1)"
[ $rc -ne 0 ] && echo "$out" | grep -q "qbase" && ok "unsatisfied Requires: error names qbase" || bad "unsatisfied Requires (rc=$rc): $out"
l=$("$P" --list-all 2>&1 | cut -d' ' -f1 | grep -c -E '^q(bad|base|helper|top)$')
check "--list-all finds the 4 modules" "4" "$l"
# PKG_CONFIG_SYSROOT_DIR is applied to -I and -L paths.
check "PKG_CONFIG_SYSROOT_DIR" "-I/sys/opt/q/include/qbase -DQBASE=1" "$(PKG_CONFIG_SYSROOT_DIR=/sys "$P" --cflags qbase 2>&1)"
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS pkgconf"; exit 0; }
echo "FAIL pkgconf ($fail check(s) failed)"; exit 1
