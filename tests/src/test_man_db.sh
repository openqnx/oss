#!/bin/sh
# man-db: a private manual hierarchy with one page. lexgrog parses its NAME
# section, mandb builds the index database, whatis/apropos answer from it,
# man -w finds the page and manpath reports MANPATH; man -l formats the page
# through groff's nroff when groff is installed (MAN_KEEP_FORMATTING=1, so
# man does not need col(1), which QNX does not have).
echo "== man-db =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
B=/usr/local/bin; [ -x "$B/man" ] || B=$(dirname "$(command -v man 2>/dev/null || echo /usr/local/bin/man)")
T=$(mktemp -d /tmp/qnxport-man-db.XXXXXX) || { echo "FAIL man-db (mktemp)"; exit 1; }
cd "$T" || exit 1
HOME=$T; export HOME
v=$("$B/man" --version 2>&1 | head -n 1); echo "  info $v"
case "$v" in *2.13.1*) ok "man --version: 2.13.1" ;; *) bad "man --version: $v" ;; esac

mkdir -p man/man1
cat > man/man1/qnxtest.1 <<'EOF'
.TH QNXTEST 1 "2026-01-01" "qnx-port-tests" "QNX Port Tests"
.SH NAME
qnxtest \- a QNX port test page
.SH SYNOPSIS
.B qnxtest
.RI [ options ]
.SH DESCRIPTION
The qnxtest program prints the word marmalade and exits.
EOF
out=$("$B/lexgrog" man/man1/qnxtest.1 2>&1); echo "  info lexgrog: $out"
[ "$out" = 'man/man1/qnxtest.1: "qnxtest - a QNX port test page"' ] && ok "lexgrog parses the NAME section" || bad "lexgrog: $out"
"$B/lexgrog" -w man/man1/qnxtest.1 >/dev/null 2>&1 && ok "lexgrog -w: whatis info found" || bad "lexgrog -w"

out=$("$B/mandb" -c -q "$T/man" 2>&1); rc=$?
[ -n "$out" ] && echo "  info mandb: $out"
ls man | grep -q '^index' && ok "mandb -c: index database created ($(ls man | grep '^index'))" || bad "mandb (rc=$rc): no index in man/"
out=$("$B/whatis" -M "$T/man" qnxtest 2>&1); echo "  info whatis: $out"
case "$out" in "qnxtest (1)"*"- a QNX port test page") ok "whatis from the index" ;; *) bad "whatis: $out" ;; esac
out=$("$B/apropos" -M "$T/man" 'port test' 2>&1)
case "$out" in "qnxtest (1)"*) ok "apropos 'port test'" ;; *) bad "apropos: $out" ;; esac
"$B/whatis" -M "$T/man" nosuchpage >/dev/null 2>&1 && bad "whatis nosuchpage succeeded" || ok "whatis of a missing page fails"
out=$("$B/man" -M "$T/man" -w qnxtest 2>&1)
[ "$out" = "$T/man/man1/qnxtest.1" ] && ok "man -w locates the page" || bad "man -w: $out"
out=$("$B/man" -M "$T/man" -w 1 qnxtest 2>&1)
[ "$out" = "$T/man/man1/qnxtest.1" ] && ok "man -w 1 qnxtest (by section)" || bad "man -w 1: $out"
"$B/man" -M "$T/man" -w 8 qnxtest >/dev/null 2>&1 && bad "man -w 8 found a section-1 page" || ok "man -w 8 qnxtest: not found"
out=$(MANPATH="$T/man" "$B/manpath" 2>/dev/null)
[ "$out" = "$T/man" ] && ok "manpath honours MANPATH" || bad "manpath: $out"

if command -v nroff >/dev/null 2>&1 || [ -x /usr/local/bin/nroff ]; then
  PATH=$PATH:/usr/local/bin; export PATH
  out=$(MAN_KEEP_FORMATTING=1 "$B/man" -P cat -l man/man1/qnxtest.1 2>&1)
  if echo "$out" | grep -q "marmalade" && echo "$out" | grep -q "QNX port test page"; then
    ok "man -l formats the page through nroff ($(echo "$out" | wc -l) lines)"
  else bad "man -l: $(echo "$out" | head -n 5)"; fi
else
  echo "  info nroff (groff) not installed -- man -l formatting not exercised"
fi
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS man-db"; exit 0; }
echo "FAIL man-db ($fail check(s) failed)"; exit 1
