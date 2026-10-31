#!/bin/sh
# npm (run by the installed node), offline: --version, npm pack of a tiny
# package (the tarball must hold package/package.json and index.js), npm run
# of a package script, npm pkg get, npm install of the packed tarball from
# the local file into another project (no network), and the installed
# module required from node. npx runs a local bin.
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
N=npm
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
NO=$L/bin/node; CLI=$L/lib/node_modules/npm/bin/npm-cli.js; NPX=$L/lib/node_modules/npm/bin/npx-cli.js
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
HOME=$T; npm_config_cache=$T/cache; npm_config_offline=true; npm_config_audit=false; npm_config_fund=false; npm_config_update_notifier=false; npm_config_loglevel=error
PATH=$L/bin:$PATH; export HOME npm_config_cache npm_config_offline npm_config_audit npm_config_fund npm_config_update_notifier npm_config_loglevel PATH
npm() { to 100 "$NO" "$CLI" "$@"; }
v=$(npm --version 2>&1); case "$v" in [0-9]*.[0-9]*) ok "npm --version: $v (node $("$NO" --version 2>&1))" ;; *) bad "npm --version: $v" ;; esac
mkdir qnx-demo && cd qnx-demo
cat > package.json <<'EOF'
{
  "name": "qnx-demo",
  "version": "1.2.3",
  "description": "tiny package for the QNX npm test",
  "main": "index.js",
  "bin": { "qnx-demo": "cli.js" },
  "scripts": { "answer": "node -e \"console.log('answer=' + 6*7)\"" },
  "license": "MIT"
}
EOF
echo 'module.exports = (a, b) => a + b;' > index.js
printf '#!/usr/bin/env node\nconsole.log("qnx-demo cli " + require("./index.js")(40, 2));\n' > cli.js
r=$(npm run answer --silent 2>&1); [ "$r" = answer=42 ] && ok "npm run answer: $r" || bad "npm run: $r"
r=$(npm pkg get version 2>&1); [ "$r" = '"1.2.3"' ] && ok "npm pkg get version: $r" || bad "npm pkg get: $r"
r=$(npm pack 2>&1 | tail -n 1)
[ "$r" = qnx-demo-1.2.3.tgz ] && [ -f qnx-demo-1.2.3.tgz ] && ok "npm pack: $r" || bad "npm pack: $r"
l=$(gzip -dc qnx-demo-1.2.3.tgz 2>/dev/null | tar -tf - 2>/dev/null | sort | tr '\n' ' ')
[ "$l" = "package/cli.js package/index.js package/package.json " ] && ok "tarball holds package/{cli.js,index.js,package.json}" || bad "tarball contents: $l"
cd "$T"; mkdir app && cd app && echo '{"name":"app","version":"0.0.1","private":true}' > package.json
npm install "$T/qnx-demo/qnx-demo-1.2.3.tgz" > i.log 2>&1 && [ -f node_modules/qnx-demo/index.js ] && ok "npm install of the local tarball (offline)" || { bad "npm install"; tail -5 i.log; }
r=$("$NO" -e 'console.log(require("qnx-demo")(20, 22))' 2>&1); [ "$r" = 42 ] && ok "require('qnx-demo')(20, 22) = 42" || bad "require: $r"
awk '/"qnx-demo"/ {f=1} END {exit !f}' package.json && ok "dependency recorded in package.json" || bad "package.json dependency"
r=$(to 60 "$NO" "$NPX" --no-install qnx-demo 2>&1); [ "$r" = "qnx-demo cli 42" ] && ok "npx qnx-demo (local bin): $r" || info "npx: $r"
r=$(npm ls --depth=0 2>&1); case "$r" in *qnx-demo@1.2.3*) ok "npm ls: qnx-demo@1.2.3" ;; *) bad "npm ls: $r" ;; esac
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
