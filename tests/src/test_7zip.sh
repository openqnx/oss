#!/bin/sh
# 7-Zip (7zz): the RAM size it detects (the system page's, as pidin
# reports it), known hashes (CRC32 and SHA-256 of "123456789"), then a tree
# of files (text, 800 KB of compressible text, an empty file, a
# subdirectory, a symlink stored as a link with -snl) archived as 7z (LZMA2,
# 4 threads), zip, tar and xz, listed, tested and extracted back byte for
# byte; an AES-256 encrypted 7z with encrypted headers opens with the right
# password only.
echo "== 7zip =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/qnxport-7zip.XXXXXX) || { echo "FAIL 7zip (mktemp)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

V=$(7zz 2>&1 | grep -m1 '7-Zip')
echo "  info $V"
case "$V" in *"25.01"*) ok "7zz version 25.01" ;; *) bad "7zz version: $V" ;; esac

# The RAM size 7-Zip sizes its buffers by (the system page's, as pidin):
# the benchmark header prints it; the benchmark itself is cut short.
ram=$(timeout 60 7zz b -md16 -mmt1 2>&1 | grep -m1 'RAM size' | sed -n 's/.*RAM size: *\([0-9]*\) MB.*/\1/p')
pram=$(pidin info 2>/dev/null | sed -n 's/.*FreeMem:[0-9]*MB\/\([0-9]*\)MB.*/\1/p')
echo "  info RAM size: 7zz ${ram} MB, pidin ${pram} MB"
[ -n "$ram" ] && [ "$ram" = "$pram" ] && ok "RAM size $ram MB, as pidin" || bad "RAM size: '$ram' (pidin: '$pram')"

printf 123456789 > check.txt
h=$(7zz h -scrcCRC32 check.txt 2>&1 | grep -i -m1 'check.txt' | awk '{print $1}')
[ "$h" = CBF43926 ] && ok "CRC32(123456789) = CBF43926" || bad "CRC32: '$h'"
h=$(7zz h -scrcSHA256 check.txt 2>&1 | grep -i -m1 'check.txt' | awk '{print toupper($1)}')
[ "$h" = 15E2B0D3C33891EBB0F1EF609EC419420C20E320CE94C65FBC8C3312448EB225 ] && ok "SHA-256(123456789)" || bad "SHA-256: '$h'"

mkdir -p src/sub
printf 'QNX SDP 8.0\n' > src/hello.txt
: > src/empty
i=0; while [ $i -lt 2000 ]; do echo "line $i of a compressible file for 7-Zip on QNX"; i=$((i+1)); done > src/sub/big.txt
cat src/sub/big.txt src/sub/big.txt src/sub/big.txt src/sub/big.txt src/sub/big.txt src/sub/big.txt src/sub/big.txt src/sub/big.txt > src/sub/bigger.txt
ln -s hello.txt src/link
size=$(wc -c < src/sub/bigger.txt | tr -d ' ')
echo "  info input: $size bytes in sub/bigger.txt"

same() {  # same <dir>: extracted tree equals src
  cmp -s src/hello.txt "$1/hello.txt" && cmp -s src/sub/big.txt "$1/sub/big.txt" && \
  cmp -s src/sub/bigger.txt "$1/sub/bigger.txt" && [ -f "$1/empty" ] && [ ! -s "$1/empty" ]
}

(cd src && 7zz a -snl -t7z -m0=lzma2 -mx=5 -mmt=4 ../a.7z . > ../a7.log 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q 'Everything is Ok' a7.log && ok "7z archive created (LZMA2, 4 threads)" || bad "7z create rc=$rc: $(tail -3 a7.log)"
asz=$(wc -c < a.7z | tr -d ' ')
echo "  info a.7z: $asz bytes"
[ "$asz" -gt 0 ] && [ "$asz" -lt $((size / 20)) ] && ok "7z compresses the repetitive text more than 20x" || bad "7z size $asz for $size"
7zz l -slt a.7z > l.log 2>&1
grep -q "Path = sub/bigger.txt" l.log && grep -q "Size = $size" l.log && ok "7z listing: sub/bigger.txt, $size bytes" || bad "7z listing: $(grep -c Path l.log) paths"
grep -q 'Method = LZMA2' l.log && ok "7z listing: method LZMA2" || bad "7z method: $(grep Method l.log | head -1)"
7zz t a.7z > t.log 2>&1 && grep -q 'Everything is Ok' t.log && ok "7z integrity test" || bad "7z test: $(tail -3 t.log)"
7zz x -oout7z a.7z > x.log 2>&1 && same out7z && ok "7z extracted byte for byte" || bad "7z extract: $(tail -3 x.log)"
[ -L out7z/link ] && [ "$(readlink out7z/link)" = hello.txt ] && ok "7z keeps the symlink (-snl)" || bad "7z symlink: $(ls -l out7z/link 2>&1)"

for t in zip tar; do
  (cd src && 7zz a -t$t ../a.$t . > ../a$t.log 2>&1)
  7zz x -oout$t a.$t > x$t.log 2>&1 && same out$t && ok "$t: created and extracted byte for byte" || bad "$t: $(tail -3 x$t.log)"
done
7zz a -txz big.xz src/sub/bigger.txt > axz.log 2>&1
7zz e -so big.xz 2>/dev/null | cmp -s - src/sub/bigger.txt && ok "xz: compress and decompress to stdout" || bad "xz: $(tail -3 axz.log)"

(cd src && 7zz a -t7z -p'qnx-secret' -mhe=on ../enc.7z . > ../enc.log 2>&1)
7zz l -slt -p'qnx-secret' enc.7z 2>&1 | grep -q 'Encrypted = +' && ok "encrypted 7z: entries marked encrypted" || bad "encrypted listing"
7zz x -p'wrong' -oencbad enc.7z > encbad.log 2>&1 && bad "wrong password accepted" || ok "wrong password rejected"
7zz x -p'qnx-secret' -oencok enc.7z > encok.log 2>&1 && same encok && ok "right password: extracted byte for byte" || bad "encrypted extract: $(tail -3 encok.log)"

cd /; rm -rf "$T"; trap - EXIT
[ $fail -eq 0 ] && { echo "PASS 7zip"; exit 0; }
echo "FAIL 7zip ($fail check(s) failed)"; exit 1
