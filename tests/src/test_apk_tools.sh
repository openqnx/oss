#!/bin/sh
# apk-tools without touching the system database: version comparison with
# known answers, two packages built with apk mkpkg (one depending on the
# other), an index built with apk mkndx, both dumped with apk adbdump, then a
# private root (--root) where apk add resolves the dependency from that local
# repository and installs the files, apk info lists them, and apk del
# removes them again.
echo "== apk-tools =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-apk.XXXXXX) || { echo "FAIL apk-tools (mktemp)"; exit 1; }
cd "$T" || exit 1

V=$(apk --version 2>&1)
echo "  info $V"
case "$V" in *"3.0.0_rc5"*) ok "apk --version: 3.0.0_rc5" ;; *) bad "apk --version: $V" ;; esac

vt() {
  r=$(apk version -t "$1" "$2" 2>&1)
  [ "$r" = "$3" ] && ok "apk version -t $1 $2: $3" || bad "apk version -t $1 $2: got '$r', want '$3'"
}
vt 1.2 1.10 "<"
vt 2.0-r1 2.0 ">"
vt 1.0_rc5 1.0 "<"
vt 3.0.0 3.0.0 "="
vt 1.0a 1.0 ">"

mkdir -p files/usr/share/qnxtest repo
echo "hello from apk" > files/usr/share/qnxtest/hello.txt
apk mkpkg -I name:qnxtest-hello -I version:1.0-r0 -I arch:noarch -I "description:QNX apk test" \
  -F files -o repo/qnxtest-hello-1.0-r0.apk 2>err.txt \
  && [ -s repo/qnxtest-hello-1.0-r0.apk ] && ok "mkpkg qnxtest-hello" || bad "mkpkg qnxtest-hello: $(cat err.txt)"
apk mkpkg -I name:qnxtest-meta -I version:2.0-r1 -I arch:noarch -I depends:qnxtest-hello \
  -o repo/qnxtest-meta-2.0-r1.apk > /dev/null 2>err.txt \
  && [ -s repo/qnxtest-meta-2.0-r1.apk ] && ok "mkpkg qnxtest-meta (depends on qnxtest-hello)" || bad "mkpkg qnxtest-meta: $(cat err.txt)"
d=$(apk adbdump repo/qnxtest-hello-1.0-r0.apk 2>&1)
echo "$d" | grep -q 'name: qnxtest-hello' && echo "$d" | grep -q 'version: 1.0-r0' && echo "$d" | grep -q 'hello.txt' \
  && ok "adbdump package: name, version and hello.txt" || bad "adbdump package: $(echo "$d" | head -5)"

apk --allow-untrusted mkndx -o repo/index.adb repo/qnxtest-hello-1.0-r0.apk repo/qnxtest-meta-2.0-r1.apk > /dev/null 2>err.txt \
  && [ -s repo/index.adb ] && ok "mkndx: index written" || bad "mkndx: $(cat err.txt)"
d=$(apk adbdump repo/index.adb 2>&1)
echo "$d" | grep -q 'name: qnxtest-hello' && echo "$d" | grep -q 'name: qnxtest-meta' \
  && ok "adbdump index: lists both packages" || bad "adbdump index: $(echo "$d" | head -5)"

# A private root, as apk's own test suite sets one up.
R=$T/root
mkdir -p "$R/etc/apk/cache" "$R/lib/apk/db" "$R/tmp" "$R/var/log"
: > "$R/etc/apk/world"; : > "$R/lib/apk/db/installed"
ln -s /dev/null "$R/var/log/apk.log"
UM=""; [ "$(id -u 2>/dev/null || echo 0)" = 0 ] || UM=--usermode
A="apk --root $R --allow-untrusted --no-interactive --no-cache"
$A --repository "$T/repo/index.adb" add --initdb $UM qnxtest-meta > add.txt 2>&1; rc=$?
echo "  info apk add: $(tr '\n' ' ' < add.txt | cut -c1-300)"
[ $rc -eq 0 ] && ok "apk add qnxtest-meta from the local index" || bad "apk add: rc=$rc"
[ "$(cat "$R/usr/share/qnxtest/hello.txt" 2>/dev/null)" = "hello from apk" ] \
  && ok "dependency qnxtest-hello installed its file" || bad "hello.txt not installed"
grep -q '^qnxtest-meta' "$R/etc/apk/world" && ok "world file records qnxtest-meta" || bad "world: $(cat "$R/etc/apk/world")"
inf=$($A info 2>&1)
echo "$inf" | grep -q '^qnxtest-hello' && echo "$inf" | grep -q '^qnxtest-meta' \
  && ok "apk info lists both packages" || bad "apk info: $inf"
$A info -L qnxtest-hello 2>&1 | grep -q 'usr/share/qnxtest/hello.txt' \
  && ok "apk info -L qnxtest-hello lists hello.txt" || bad "apk info -L: $($A info -L qnxtest-hello 2>&1 | head -3)"
$A info -e qnxtest-meta > /dev/null 2>&1 && ok "apk info -e: qnxtest-meta installed" || bad "apk info -e qnxtest-meta"
$A info -e qnxtest-absent > /dev/null 2>&1 && bad "apk info -e of an absent package succeeded" || ok "apk info -e: absent package is absent"
$A del qnxtest-meta > del.txt 2>&1; rc=$?
[ $rc -eq 0 ] && [ ! -e "$R/usr/share/qnxtest/hello.txt" ] \
  && ok "apk del qnxtest-meta removes the now unneeded dependency too" || bad "apk del: rc=$rc $(head -c 300 del.txt)"

cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS apk-tools"; exit 0; }
echo "FAIL apk-tools ($fail check(s) failed)"; exit 1
