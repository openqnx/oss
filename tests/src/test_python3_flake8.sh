#!/bin/sh
# python3-flake8 and its checkers and plugins: flake8 lints a sample file
# with one known error for every checker -- pyflakes (F401/F821/F841),
# pycodestyle (E722/E741), mccabe (C901), and the plugins blind-except
# (B901/B902), builtins (A001), class-newline (CNL100), comprehensions
# (C403/C416), deprecated (D001), docstrings/pydocstyle (D101/D103),
# import-order (I100) and quotes (Q000) -- and must report exactly that set
# at those lines and columns; a clean file gives nothing and exit 0. The
# pyflakes, pycodestyle and pydocstyle commands run on their own too, and
# snowballstemmer (pydocstyle's imperative-mood check) stems known words.
echo "== python3-flake8 =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
BIN=$(dirname "$PY")
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-flake8.XXXXXX") || { echo "FAIL python3-flake8 (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
# No __pycache__ in the installation's site-packages (docs/porting-hazards.md
# 183), but bytecode in a private cache under $T: this test starts Python
# many times, and recompiling flake8/SCons/pythran from source on every
# start took it past the 15-minute limit on the emulated aarch64 VM.
unset PYTHONDONTWRITEBYTECODE; export PYTHONPYCACHEPREFIX="$T/pycache"
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails+1)); }
cat > sample.py <<'P'
"""Sample module."""
import sys
import os
from collections import OrderedDict


def list(a, b):
    """Shadow a builtin."""
    try:
        return a + b
    except:
        return None


def comp(xs):
    """Comprehensions."""
    s = set([x for x in xs])
    return s


def undocumented(x):
    y = 1
    return x + 'quoted' + "double"


class Thing:
    def method(self):
        """Do."""
        self.assertEquals(1, 1)
        return undefined_name


def blind():
    """Blind except."""
    try:
        pass
    except Exception:
        pass
    l = 1
    return l
P
cat > clean.py <<'P'
"""A clean module."""
import os


def main():
    """Print the current directory."""
    print(os.getcwd())
P
cat > want <<'P'
2:1 F401
3:1 F401
3:1 I100
4:1 F401
4:1 I100
7:1 A001
7:1 C901
11:1 B901
11:5 E722
17:9 C403
17:13 C416
21:1 D103
22:5 F841
23:27 Q000
26:1 D101
27:1 CNL100
29:9 D001
30:16 F821
33:1 C901
37:1 B902
39:5 E741
P
FLAKE8="$BIN/flake8"; [ -x "$FLAKE8" ] || FLAKE8=flake8
"$FLAKE8" --version > ver 2>&1 || bad "flake8 --version runs"
for p in flake8-blind-except flake8-builtins flake8-class-newline flake8-comprehensions flake8-deprecated \
         flake8-docstrings flake8-quotes flake8_import_order mccabe pycodestyle pyflakes; do
  tr -d '\n' < ver | grep -q "$p:" || bad "flake8 --version lists the plugin $p"
done
echo "  info $(head -1 ver)"
"$FLAKE8" --isolated --max-complexity=1 --import-order-style=google --inline-quotes=single sample.py > out; rc=$?
[ $rc -eq 1 ] && ok "flake8 exits 1 on a file with errors" || bad "flake8 exits 1 on a file with errors (rc=$rc)"
sed -n 's/^sample\.py:\([0-9]*:[0-9]*\): \([A-Z]*[0-9]*\) .*/\1 \2/p' out | sort > got
sort want > want.s
if cmp -s got want.s; then ok "flake8 reports exactly the 21 known errors (all checkers and plugins)"
else bad "flake8 reports exactly the 21 known errors"; diff want.s got | sed 's/^/  info /'; fi
grep -q "^sample.py:30:16: F821 undefined name 'undefined_name'$" out && ok "F821 message text" || bad "F821 message text"
"$FLAKE8" --isolated --import-order-style=google clean.py > out2; rc=$?
[ $rc -eq 0 ] && [ ! -s out2 ] && ok "a clean file: no output, exit 0" || { bad "a clean file: no output, exit 0 (rc=$rc)"; sed 's/^/  info /' out2; }
"$BIN/pyflakes" sample.py > pf 2>&1; [ "$(wc -l < pf | tr -d ' ')" = 5 ] && grep -q "sample.py:30:16: undefined name 'undefined_name'" pf \
  && ok "pyflakes command: 5 messages" || { bad "pyflakes command: 5 messages"; sed 's/^/  info /' pf; }
"$BIN/pycodestyle" sample.py > pc 2>&1; [ "$(cut -d' ' -f2 pc | sort | tr '\n' ' ')" = "E722 E741 " ] \
  && ok "pycodestyle command: E722 E741" || { bad "pycodestyle command: E722 E741"; sed 's/^/  info /' pc; }
"$BIN/pydocstyle" --select=D101,D103 sample.py > pd 2>&1; [ "$(grep -c '^ *D10[13]:' pd)" = 2 ] \
  && ok "pydocstyle command: D101 and D103" || { bad "pydocstyle command: D101 and D103"; sed 's/^/  info /' pd; }
"$PY" -c '
import snowballstemmer
s = snowballstemmer.stemmer("english")
assert s.stemWords(["running", "connection", "generously"]) == ["run", "connect", "generous"], s.stemWords(["running", "connection", "generously"])
assert snowballstemmer.stemmer("german").stemWord("katzen") == "katz"
' && ok "snowballstemmer: english and german stems" || bad "snowballstemmer: english and german stems"
[ $fails -eq 0 ] && { echo "PASS python3-flake8"; exit 0; }
echo "FAIL python3-flake8 ($fails)"; exit 1
