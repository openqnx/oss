#!/bin/sh
# doxygen: document a small C header -- the XML output must hold the
# function, its brief description and its parameter, and HTML must be made.
echo "== doxygen =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/doxygen.XXXXXX); cd "$T" || exit 1
cat > demo.h <<'H'
/** @file demo.h */
/**
 * @brief Adds two numbers on QNX.
 * @param a the first number
 * @param b the second number
 * @return a + b
 */
int qnx_add(int a, int b);
H
printf 'INPUT=demo.h\nGENERATE_XML=YES\nGENERATE_LATEX=NO\nQUIET=YES\nOUTPUT_DIRECTORY=out\n' > Doxyfile
doxygen Doxyfile > log 2>&1 && ok "doxygen ran ($(doxygen --version))" || { bad "doxygen ran"; head -5 log; }
x=$(cat out/xml/demo_8h.xml 2>/dev/null)
echo "$x" | grep -q "<name>qnx_add</name>" && ok "XML: function qnx_add" || bad "XML: function qnx_add"
echo "$x" | grep -q "Adds two numbers on QNX." && ok "XML: brief description" || bad "XML: brief description"
echo "$x" | grep -q "<parametername>a</parametername>" && ok "XML: parameter a" || bad "XML: parameter a"
[ -f out/html/index.html ] && ok "HTML: index.html" || bad "HTML: index.html"
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS doxygen"; exit 0; }
echo "FAIL doxygen ($fail check(s) failed)"; exit 1
