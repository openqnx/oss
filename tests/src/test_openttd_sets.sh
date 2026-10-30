#!/bin/sh
# OpenTTD's free base sets: OpenTTD itself must find and accept each one
# (openttd -h lists the graphics, sound and music sets it can load).
echo "== openttd-sets =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
O=/usr/local/games/openttd; [ -x "$O" ] || O=$(command -v openttd)
out=$("$O" -h 2>&1)
for s in "OpenGFX 8.0" "OpenSFX 1.0.3" "OpenMSX 0.4.2"; do
  echo "$out" | grep -qF "[$s]" && ok "openttd lists $s" || bad "openttd lists $s"
done
[ $fail -eq 0 ] && { echo "PASS openttd-sets"; exit 0; }
echo "FAIL openttd-sets ($fail check(s) failed)"; exit 1
