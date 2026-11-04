#!/bin/sh
# git: init, add, commit, log, branch, merge, tag, diff, a known blob hash,
# a local clone (runs git-upload-pack) with a pushed commit coming back, and
# archive -- all in a temp dir with no system or global configuration.
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
N=git
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
GT=$L/bin/git
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
HOME=$T; GIT_CONFIG_NOSYSTEM=1; GIT_PAGER=cat; PAGER=cat; GIT_TERMINAL_PROMPT=0; export HOME GIT_CONFIG_NOSYSTEM GIT_PAGER PAGER GIT_TERMINAL_PROMPT
GIT_AUTHOR_DATE="2026-01-02T03:04:05Z"; GIT_COMMITTER_DATE=$GIT_AUTHOR_DATE; export GIT_AUTHOR_DATE GIT_COMMITTER_DATE
g() { "$GT" -c user.name=QNX -c user.email=qnx@example.invalid -c init.defaultBranch=main -c advice.detachedHead=false "$@"; }
info "$("$GT" --version 2>&1) (exec-path $("$GT" --exec-path 2>&1))"
r=$(echo hello | g hash-object --stdin 2>&1)
[ "$r" = ce013625030ba8dba906f756967f9e9ca394464a ] && ok "hash-object of 'hello' = ce01362" || bad "hash-object: $r"
g init -q repo && cd repo && ok "init" || { bad "init"; cd "$T"; }
printf 'line one\nline two\n' > a.txt; mkdir d; echo nested > d/b.txt
g add . && g commit -q -m "first commit" && ok "commit 1" || bad "commit 1"
echo "line three" >> a.txt
r=$(g diff --stat 2>&1 | tail -n 1)
case "$r" in *"1 file changed, 1 insertion(+)"*) ok "diff --stat: 1 insertion" ;; *) bad "diff --stat: $r" ;; esac
g commit -q -am "second commit" && ok "commit -a" || bad "commit -a"
g checkout -q -b feature && echo feat > f.txt && g add f.txt && g commit -q -m "feature work" && ok "branch feature + commit" || bad "branch"
g checkout -q main && g merge -q --no-ff -m "merge feature" feature && [ -f f.txt ] && ok "merge --no-ff" || bad "merge"
g tag -a v1.0 -m "release 1.0" && [ "$(g describe 2>&1)" = v1.0 ] && ok "annotated tag, describe = v1.0" || bad "tag/describe: $(g describe 2>&1)"
r=$(g log --first-parent --format=%s 2>&1 | tr '\n' '|')
[ "$r" = "merge feature|second commit|first commit|" ] && ok "log --first-parent: merge, second, first" || bad "log: $r"
[ "$(g rev-list --count HEAD 2>&1)" = 4 ] && ok "rev-list --count = 4" || bad "rev-list --count"
[ "$(g show HEAD~1:a.txt 2>/dev/null | wc -l | tr -d ' ')" = 3 ] && ok "show HEAD~1:a.txt (3 lines)" || bad "show HEAD~1:a.txt"
g fsck --strict >/dev/null 2>&1 && ok "fsck --strict" || bad "fsck"
cd "$T"
g clone -q repo clone 2> clone.err && [ -f clone/d/b.txt ] && ok "clone (local, via upload-pack)" || { bad "clone"; head -3 clone.err; }
if [ -d clone ]; then
  ( cd clone && echo pushed > p.txt && g add p.txt && g commit -q -m "from clone" && g push -q origin HEAD:refs/heads/fromclone ) 2> push.err \
    && [ "$(cd repo && g log -1 --format=%s fromclone 2>&1)" = "from clone" ] && ok "push back to the origin (receive-pack)" || { bad "push"; head -3 push.err; }
fi
( cd repo && g archive --format=tar --prefix=arch/ v1.0 ) > a.tar 2>/dev/null && tar -tf a.tar 2>/dev/null | awk '$0=="arch/d/b.txt"{f=1} END{exit !f}' && ok "archive of v1.0" || bad "archive"
( cd repo && g gc -q ) 2>/dev/null && [ "$(cd repo && g count-objects -v | awk '/^in-pack/ {print $2}')" -gt 0 ] && ok "gc packed the objects" || bad "gc"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
