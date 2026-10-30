#!/bin/sh
# Command-line tools, each with a known result: ed edits a file, patch
# applies a diff and takes it back, lzip compresses and restores, wget
# fetches the target's own apk repository index over HTTP(S).
echo "== cli-tools =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
T=$(mktemp -d /tmp/clitools.XXXXXX); cd "$T" || exit 1
if command -v ed >/dev/null; then
  printf 'alpha\nbeta\ngamma\n' > f.txt
  printf '2s/beta/BETA/\n$a\ndelta\n.\nw\nq\n' | ed -s f.txt
  [ "$(cat f.txt | tr '\n' ' ')" = "alpha BETA gamma delta " ] && ok "ed: substitute and append" || bad "ed: got $(cat f.txt | tr '\n' ' ')"
fi
if command -v patch >/dev/null; then
  printf 'one\ntwo\nthree\n' > p.txt
  cat > p.diff <<'D'
--- p.txt
+++ p.txt
@@ -1,3 +1,3 @@
 one
-two
+TWO
 three
D
  patch -s p.txt < p.diff && [ "$(sed -n 2p p.txt)" = TWO ] && ok "patch: hunk applied" || bad "patch: apply"
  patch -s -R p.txt < p.diff && [ "$(sed -n 2p p.txt)" = two ] && ok "patch -R: reversed" || bad "patch: reverse"
fi
if command -v lzip >/dev/null; then
  i=0; : > d.txt; while [ $i -lt 2000 ]; do echo "line $i of the QNX lzip test" >> d.txt; i=$((i+1)); done
  cp d.txt d.orig; lzip -9 d.txt && [ -f d.txt.lz ] && [ "$(head -c 4 d.txt.lz)" = LZIP ] && ok "lzip: compressed ($(wc -c < d.orig) -> $(wc -c < d.txt.lz) bytes, LZIP magic)" || bad "lzip: compress"
  lzip -t d.txt.lz && ok "lzip -t: integrity" || bad "lzip -t"
  lzip -d d.txt.lz && cmp -s d.txt d.orig && ok "lzip -d: identical" || bad "lzip -d"
fi
if command -v wget >/dev/null; then
  R=$(head -n 1 /data/oss/etc/apk/repositories 2>/dev/null)
  case "$(uname -m)" in aarch64*|*arm*|QEMU_virt) A=aarch64 ;; *) A=x86_64 ;; esac
  case "$R" in
    http*) case "$R" in *APKINDEX.tar.gz) U=$R ;; *) U=$R/$A/APKINDEX.tar.gz ;; esac
           CA=/data/oss/usr/local/ssl/cert.pem; [ -f "$CA" ] || CA=/usr/local/ssl/cert.pem
           wget -q --ca-certificate="$CA" -O idx.tgz "$U" && [ "$(od -An -tx1 -N2 idx.tgz | tr -d ' ')" = 1f8b ] \
             && ok "wget: $U ($(wc -c < idx.tgz) bytes, gzip)" || bad "wget: $U" ;;
    *) echo "  info no apk repository configured -- wget not exercised" ;;
  esac
fi
cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS cli-tools"; exit 0; }
echo "FAIL cli-tools ($fail check(s) failed)"; exit 1
