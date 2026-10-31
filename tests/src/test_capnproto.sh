#!/bin/sh
# Cap'n Proto tools on a small schema: capnp eval prints constants (text and
# JSON), capnp encode turns a text value into the exact 24-byte wire message,
# decode / convert bring it back as text and JSON, the packed encoding round
# trips, capnp id makes a valid file ID, and capnp compile runs the C++ and
# capnp code generator plugins.
echo "== capnproto =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-capnproto.XXXXXX) || { echo "FAIL capnproto (mktemp)"; exit 1; }
cd "$T" || exit 1

V=$(capnp --version 2>&1)
echo "  info $V"
case "$V" in *"1.0.2"*) ok "capnp --version: 1.0.2" ;; *) bad "capnp --version: $V" ;; esac

cat > s.capnp <<'EOF'
@0xbf5147cbbecf40c1;
struct Point {
  x @0 :Int32;
  y @1 :Int32;
}
struct Shape {
  name @0 :Text;
  points @1 :List(Point);
  closed @2 :Bool = true;
}
const origin :Point = (x = 3, y = -4);
const greeting :Text = "hello qnx";
const square :Shape = (name = "sq", points = [(x = 0, y = 0), (x = 1, y = 0), (x = 1, y = 1), (x = 0, y = 1)]);
EOF

out=$(capnp eval s.capnp origin 2>&1)
[ "$out" = "(x = 3, y = -4)" ] && ok "eval origin: (x = 3, y = -4)" || bad "eval origin: $out"
out=$(capnp eval s.capnp greeting 2>&1)
[ "$out" = '"hello qnx"' ] && ok "eval greeting: \"hello qnx\"" || bad "eval greeting: $out"
out=$(capnp eval s.capnp square.points 2>&1)
case "$out" in "[(x = 0, y = 0), (x = 1, y = 0), (x = 1, y = 1), (x = 0, y = 1)]") ok "eval square.points: 4 points" ;;
  *) bad "eval square.points: $out" ;; esac
out=$(capnp eval -o json s.capnp origin 2>&1 | tr -d ' \n')
[ "$out" = '{"x":3,"y":-4}' ] && ok "eval -o json: {\"x\": 3, \"y\": -4}" || bad "eval -o json: $out"

# (x = 1, y = 2): segment table (1 segment of 2 words), root struct pointer
# (offset 0, 1 data word, 0 pointers), then the data word.
echo '(x = 1, y = 2)' | capnp encode s.capnp Point > p.bin 2>err.txt
hex=$(od -An -tx1 p.bin | tr -d ' \n')
want=000000000200000000000000010000000100000002000000
[ "$hex" = "$want" ] && ok "encode: exact 24-byte message" || bad "encode: $hex $(cat err.txt)"
out=$(capnp decode s.capnp Point < p.bin 2>&1)
[ "$out" = "(x = 1, y = 2)" ] && ok "decode: (x = 1, y = 2)" || bad "decode: $out"
out=$(capnp convert binary:json s.capnp Point < p.bin 2>&1 | tr -d ' \n')
[ "$out" = '{"x":1,"y":2}' ] && ok "convert binary:json" || bad "convert binary:json: $out"
echo '{"x": 7, "y": 9}' | capnp convert json:text s.capnp Point > j.txt 2>err.txt
[ "$(cat j.txt)" = "(x = 7, y = 9)" ] && ok "convert json:text" || bad "convert json:text: $(cat j.txt) $(cat err.txt)"

echo '(name = "tri", points = [(x = 0, y = 0), (x = 4, y = 0), (x = 0, y = 3)], closed = false)' \
  | capnp encode --packed s.capnp Shape > sh.pk 2>err.txt
out=$(capnp decode --short --packed s.capnp Shape < sh.pk 2>&1)
[ "$out" = '(name = "tri", points = [(x = 0, y = 0), (x = 4, y = 0), (x = 0, y = 3)], closed = false)' ] \
  && ok "packed encode/decode round trip of a struct with text and a list" || bad "packed: $out $(cat err.txt)"
echo "  info packed Shape message: $(wc -c < sh.pk | tr -d ' ') bytes"
out=$(echo '(name = "x")' | capnp encode s.capnp Shape | capnp decode s.capnp Shape 2>&1)
[ "$out" = '(name = "x", closed = true)' ] && ok "default value: closed = true" || bad "default: $out"

id=$(capnp id 2>&1)
case "$id" in @0x[89abcdef][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f])
  ok "capnp id: $id" ;; *) bad "capnp id: $id" ;; esac

capnp compile -oc++ s.capnp 2>err.txt
if [ -s s.capnp.h ] && [ -s s.capnp.c++ ] && grep -q 'struct Point' s.capnp.h && grep -q 'struct Shape' s.capnp.h; then
  ok "compile -oc++: s.capnp.h / s.capnp.c++ with Point and Shape"
else bad "compile -oc++: $(head -c 300 err.txt)"; fi
out=$(capnp compile -ocapnp s.capnp 2>&1)
echo "$out" | grep -q 'struct Point' && echo "$out" | grep -q 'x @0 :Int32' && ok "compile -ocapnp echoes the schema" || bad "compile -ocapnp: $(echo "$out" | head -3)"
echo 'struct Bad { x @1 :Int32; }' > bad.capnp
capnp compile -o- bad.capnp > /dev/null 2>err.txt && bad "a schema without a file ID / bad ordinal must fail" \
  || ok "invalid schema rejected: $(head -n 1 err.txt | cut -c1-80)"

cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS capnproto"; exit 0; }
echo "FAIL capnproto ($fail check(s) failed)"; exit 1
