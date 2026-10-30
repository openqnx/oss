#!/bin/sh
# perl-mozilla-ca: Perl finds Mozilla::CA, its bundle file exists and holds
# Mozilla's root certificates (well over a hundred PEM blocks).
echo "== perl-mozilla-ca =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=/usr/local/bin/perl; [ -x "$P" ] || P=perl
f=$("$P" -MMozilla::CA -e 'print Mozilla::CA::SSL_ca_file()' 2>&1)
[ -f "$f" ] && ok "Mozilla::CA::SSL_ca_file = $f" || bad "Mozilla::CA loads ($f)"
n=$(grep -c "BEGIN CERTIFICATE" "$f" 2>/dev/null); [ "${n:-0}" -gt 100 ] && ok "$n certificates" || bad "certificates: ${n:-0}"
v=$("$P" -MMozilla::CA -e 'print $Mozilla::CA::VERSION'); [ "$v" = 20261020 ] && ok "version 20261020" || bad "version $v"
[ $fail -eq 0 ] && { echo "PASS perl-mozilla-ca"; exit 0; }
echo "FAIL perl-mozilla-ca ($fail check(s) failed)"; exit 1
