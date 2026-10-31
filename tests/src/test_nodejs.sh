#!/bin/sh
# Node.js: evaluates JavaScript with known answers -- BigInt, JSON, regex,
# classes and async/await; crypto SHA-256 and an AES round trip (OpenSSL);
# zlib gzip round trip; Intl number formatting (ICU); fs in the temp dir;
# a child process; and an HTTP server and client on loopback (libuv).
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=nodejs
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
NO=$L/bin/node
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
# Whatever the test starts stops with it, on every exit path (track PID;
# qstop_hook for anything else); the outer shell above kills them on a hang.
QP=$QNXPORT_INNER.pids; : > "$QP"
track() { echo "$1" >> "$QP"; }
qstop_hook() { :; }
stopall() {
  qstop_hook 2>/dev/null
  for p in $(cat "$QP" 2>/dev/null); do kill "$p" 2>/dev/null; done
  : > "$QP"; [ -n "$T" ] && [ -d "$T" ] && { cd /; rm -rf "$T"; }
}
trap stopall EXIT
trap 'exit 143' TERM
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
HOME=$T; export HOME
v=$("$NO" --version 2>&1); case "$v" in v[0-9]*) ok "node --version: $v" ;; *) bad "node --version: $v" ;; esac
e() { to 120 "$NO" -e "$1" 2>&1; }
eq() { [ "$2" = "$3" ] && ok "$1" || bad "$1: got '$2', want '$3'"; }
eq "BigInt 2n**100n" "$(e 'console.log((2n ** 100n).toString())')" 1267650600228229401496703205376
eq "JSON round trip" "$(e 'const o = JSON.parse(JSON.stringify({os: "QNX", v: [8, 0]})); console.log(o.os + o.v.join("."))')" QNX8.0
eq "regex named groups" "$(e 'const m = "qnx-8.0".match(/(?<n>[a-z]+)-(?<maj>\d+)/); console.log(m.groups.n, m.groups.maj)')" "qnx 8"
eq "class + async/await" "$(e 'class A { constructor(x) { this.x = x } async twice() { await null; return this.x * 2 } }; new A(21).twice().then(v => console.log(v))')" 42
eq "crypto sha256('abc')" "$(e 'console.log(require("crypto").createHash("sha256").update("abc").digest("hex"))')" ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
eq "crypto AES-256-GCM round trip" "$(e 'const c = require("crypto"); const k = c.randomBytes(32), iv = c.randomBytes(12);
  const en = c.createCipheriv("aes-256-gcm", k, iv); const ct = Buffer.concat([en.update("secret QNX"), en.final()]); const tag = en.getAuthTag();
  const de = c.createDecipheriv("aes-256-gcm", k, iv); de.setAuthTag(tag); console.log(Buffer.concat([de.update(ct), de.final()]).toString())')" "secret QNX"
eq "zlib gzip round trip" "$(e 'const z = require("zlib"); const s = "QNX ".repeat(1000); const g = z.gzipSync(s); console.log(g[0], g[1], g.length < 100, z.gunzipSync(g).toString() === s)')" "31 139 true true"
eq "Intl.NumberFormat en-US (ICU)" "$(e 'console.log(new Intl.NumberFormat("en-US").format(1234567.891))')" "1,234,567.891"
eq "Intl de-DE" "$(e 'console.log(new Intl.NumberFormat("de-DE").format(1234567.891))')" "1.234.567,891"
eq "fs write/read in /tmp" "$(e 'const fs = require("fs"); fs.writeFileSync("n.txt", "from node"); console.log(fs.readFileSync("n.txt", "utf8"), fs.statSync("n.txt").size)')" "from node 9"
eq "child_process.execFileSync(sh)" "$(e 'console.log(require("child_process").execFileSync("/bin/sh", ["-c", "echo $((6*7))"]).toString().trim())')" 42
cat > srv.js <<'EOF'
const http = require("http");
const srv = http.createServer((req, res) => { res.setHeader("X-QNX", "yes"); res.end("hello " + req.url); });
srv.listen(0, "127.0.0.1", () => {
  const { port } = srv.address();
  http.get({ host: "127.0.0.1", port, path: "/qnx" }, (res) => {
    let b = ""; res.on("data", (d) => (b += d));
    res.on("end", () => { console.log(res.statusCode, res.headers["x-qnx"], b); srv.close(); });
  }).on("error", (err) => { console.log("error " + err.message); srv.close(); });
});
EOF
eq "HTTP server + client on 127.0.0.1" "$(to 120 "$NO" srv.js 2>&1)" "200 yes hello /qnx"
info "$(e 'console.log("openssl " + process.versions.openssl + ", icu " + process.versions.icu + ", uv " + process.versions.uv)')"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
