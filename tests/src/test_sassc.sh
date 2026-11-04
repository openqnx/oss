#!/bin/sh
# sassc: the command-line compiler. Known answers: -v names sassc 3.6.2
# over libsass 3.6.6; a stylesheet with an @import from -I, a mixin, a
# @function, @each and colour functions compiled to the default (nested)
# and compressed styles, to stdout and to an output file; stdin (-s) with
# --style expanded; the indented syntax (--sass); an undefined variable
# reported with exit status 65 (EX_DATAERR) and its position ("stdin" is
# shown relative to the working directory: "/stdin" when that is /).
echo "== sassc =="
P=${QNX_PORT_PREFIX:-/usr/local}
S=$P/bin/sassc
fails=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fails=$((fails + 1)); }
chk() { if [ "$2" = "$3" ]; then ok "$1"; else bad "$1"; echo "  info want: $3"; echo "  info got:  $2"; fi; }
[ -x "$S" ] || { echo "  FAIL sassc installed ($S)"; echo "FAIL sassc"; exit 1; }
chk "sassc -v" "$("$S" -v 2>&1 | head -n 2 | tr '\n' ' ')" "sassc: 3.6.2 libsass: 3.6.6 "
T=${TMPDIR:-/tmp}/sassc-test.$$; mkdir -p "$T/inc"
cat > "$T/inc/_colors.scss" <<'X'
$brand: #3366cc;
@function double($n) { @return $n * 2; }
X
cat > "$T/main.scss" <<'X'
@import "colors";
@mixin box($pad: 4px) { padding: $pad; border: 1px solid darken($brand, 10%); }
.nav {
  color: $brand;
  ul { margin: double(3px); @include box(2px + 3px); }
  &:hover { color: lighten($brand, 20%); }
}
@each $n in 1, 2 { .m-#{$n} { width: percentage($n / 4); } }
X
want='.nav {
  color: #3366cc; }
  .nav ul {
    margin: 6px;
    padding: 5px;
    border: 1px solid #2952a3; }
  .nav:hover {
    color: #85a3e0; }

.m-1 {
  width: 25%; }

.m-2 {
  width: 50%; }'
chk "nested style to stdout (-I for the @import)" "$("$S" -I "$T/inc" "$T/main.scss" 2>&1)" "$want"
"$S" -t compressed -I "$T/inc" "$T/main.scss" "$T/out.css" 2>&1
chk "compressed style to an output file" "$(cat "$T/out.css" 2>&1)" ".nav{color:#36c}.nav ul{margin:6px;padding:5px;border:1px solid #2952a3}.nav:hover{color:#85a3e0}.m-1{width:25%}.m-2{width:50%}"
chk "stdin (-s), --style expanded" "$(echo '.a { b { c: 1 + 2 } }' | "$S" -s --style expanded 2>&1)" ".a b {
  c: 3;
}"
chk "indented syntax (--sass)" "$(printf '.x\n  y: 1px + 1px\n' | "$S" -s --sass 2>&1)" ".x {
  y: 2px; }"
out=$(echo '.a { color: $nope; }' | "$S" -s 2>&1); rc=$?
if [ $rc -eq 65 ] && echo "$out" | grep -q 'Undefined variable: "$nope"' && echo "$out" | grep -q "on line 1:13 of .*stdin"; then
  ok "undefined variable: message with line:column, exit 65"
else bad "undefined variable (rc $rc)"; echo "$out"; fi
rm -rf "$T"
[ $fails -eq 0 ] && { echo "PASS sassc"; exit 0; }
echo "FAIL sassc ($fails)"; exit 1
