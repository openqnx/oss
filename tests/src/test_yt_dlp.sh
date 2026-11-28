#!/bin/sh
# yt-dlp: --version names the release; the extractor list holds the
# common sites (youtube, vimeo, generic); a direct media link served here
# (a small file on a loopback HTTP server, python3 -m http.server) is
# fetched through the generic extractor: the
# download completes and the file is byte for byte the same. No outside
# network is used.
# LC_ALL is set: with no locale in the environment, the python3 port's
# locale.getlocale() fails on QNX's "C;ctype:POSIX" (a python3 issue being
# looked into, not yt-dlp's); users set LANG/LC_ALL as on any system.
LC_ALL=C.UTF-8; export LC_ALL
. "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL yt-dlp (qnxport-timeout.sh missing)"; exit 1; }
echo "== yt-dlp =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=${QNX_PORT_PREFIX:-/usr/local}
Y=$P/bin/yt-dlp
T=$(mktemp -d /tmp/qnxport-yt-dlp.XXXXXX) || { echo "FAIL yt-dlp (mktemp)"; exit 1; }
S=""
trap '[ -n "$S" ] && kill $S 2>/dev/null; cd /; rm -rf "$T"' EXIT
trap 'exit 1' INT TERM HUP
cd "$T" || exit 1

v=$("$Y" --version 2>&1 | head -n 1)
[ "$v" = "2026.10.26" ] && ok "yt-dlp --version: $v" || bad "yt-dlp --version: $v"
e=$("$Y" --list-extractors 2>/dev/null)
c=$(echo "$e" | grep -c .)
echo "  info $c extractors"
for x in youtube vimeo generic; do echo "$e" | grep -qi "^$x\b" || { bad "extractor $x missing"; continue; }; done
[ "$c" -gt 1000 ] && ok "extractors: $c, youtube/vimeo/generic among them" || bad "extractor count: $c"

mkdir srv && i=0 && while [ $i -lt 2000 ]; do echo "media block $i"; i=$((i+1)); done > srv/clip.mp4
PORT=$((24000 + $$ % 2000))
( cd srv && exec python3 -m http.server $PORT --bind 127.0.0.1 ) > http.log 2>&1 &
S=$!
i=0; while [ $i -lt 20 ] && ! grep -q "Serving" http.log 2>/dev/null; do sleep 0.5; i=$((i+1)); done
qto 120 "$Y" --no-progress -o out.%\(ext\)s "http://127.0.0.1:$PORT/clip.mp4" > dl.out 2>&1; rc=$?
cmp -s srv/clip.mp4 out.mp4 && ok "generic extractor downloaded http://127.0.0.1/clip.mp4 byte for byte" || bad "download rc=$rc: $(tail -n 3 dl.out)"

[ $fail -eq 0 ] && { echo "PASS yt-dlp"; exit 0; }
echo "FAIL yt-dlp ($fail check(s) failed)"; exit 1
