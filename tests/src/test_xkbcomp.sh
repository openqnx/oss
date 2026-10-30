#!/bin/sh
# xkbcomp: compile a US keymap from xkeyboard-config's sources -- to a
# compiled keymap (.xkm) and back to source, which must name the keys of the
# US layout (AC01 is "a").
echo "== xkbcomp =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
R=; for d in /usr/local/share/X11/xkb /data/oss/usr/local/share/X11/xkb; do [ -d "$d/symbols" ] && { R=$d; break; }; done
[ -n "$R" ] && ok "xkeyboard-config data ($R)" || { bad "xkeyboard-config data"; echo "FAIL xkbcomp ($fail check(s) failed)"; exit 1; }
T=$(mktemp -d /tmp/xkbcomp.XXXXXX)
cat > "$T/us.xkb" <<K
xkb_keymap {
  xkb_keycodes { include "evdev+aliases(qwerty)" };
  xkb_types    { include "complete" };
  xkb_compat   { include "complete" };
  xkb_symbols  { include "pc+us+inet(evdev)" };
};
K
xkbcomp -w0 -I"$R" "$T/us.xkb" "$T/us.xkm" 2>"$T/err" && [ -s "$T/us.xkm" ] && ok "compiled to us.xkm ($(wc -c < "$T/us.xkm") bytes)" || { bad "compile"; head -5 "$T/err"; }
xkbcomp -w0 -xkb "$T/us.xkm" "$T/back.xkb" 2>>"$T/err" && ok "us.xkm back to source" || bad "decompile"
# The decompiled entry spans lines: key <AC01> { ... [ a, A ] };
sed -n '/key <AC01> {/,/};/p' "$T/back.xkb" | tr -d ' \n' | grep -q '\[a,A\]' && ok "key <AC01> is a/A" || bad "key <AC01> is a/A"
rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS xkbcomp"; exit 0; }
echo "FAIL xkbcomp ($fail check(s) failed)"; exit 1
