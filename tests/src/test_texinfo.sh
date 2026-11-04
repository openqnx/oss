#!/bin/sh
# texinfo: the target's makeinfo (texi2any 7.2, pure Perl: no XS) turns a
# small manual (menu, chapters, @code/@var, @xref, @example, @enumerate,
# an index, a dir entry) into Info -- node headers with Next/Prev/Up, the
# menu, the cross reference, the index menu, the tag table --, plain text
# and HTML; a reference to a missing node is an error with its line;
# install-info adds the dir entry to a dir file; the info reader prints a
# node non-interactively; texindex sorts a raw index with its initials.
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
N=texinfo
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
has() { awk -v p="$1" 'index($0, p) {f=1} END {exit !f}'; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
D=$(cd "$(dirname "$0")" && pwd)   # helpers next to this test
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
MI=$L/bin/makeinfo
r=$("$MI" --version 2>&1 | head -n 1); [ "$r" = "texi2any (GNU texinfo) 7.2" ] && ok "makeinfo --version: texi2any (GNU texinfo) 7.2" || bad "version: $r"
cat > t.texi <<'EOT'
\input texinfo
@setfilename t.info
@settitle QNX Texinfo Test

@dircategory Testing
@direntry
* QNX test: (t).        A Texinfo document made on QNX.
@end direntry

@node Top
@top QNX Texinfo Test

This manual tests @command{makeinfo} on QNX.

@menu
* Second::      The second chapter.
* Third::       The third chapter.
@end menu

@node Second
@chapter Second Chapter
@cindex second chapter

The function @code{qnx_init} returns @var{status}.
@xref{Third}, for more.

@example
int main(void) @{ return 0; @}
@end example

@node Third
@chapter Third Chapter
@cindex third chapter

@enumerate
@item one
@item two
@end enumerate

@printindex cp
@bye
EOT
r=$(to 120 "$MI" t.texi -o t.info 2>&1); rc=$?
[ $rc -eq 0 ] && [ -s t.info ] && ok "makeinfo t.texi: t.info" || bad "makeinfo (rc $rc): $r"
has "This is t.info, produced by makeinfo version 7.2 from t.texi." < t.info && ok "produced by makeinfo version 7.2" || bad "info header: $(head -n 1 t.info)"
for l in "File: t.info,  Node: Top,  Next: Second,  Up: (dir)" "File: t.info,  Node: Second,  Next: Third,  Prev: Top,  Up: Top" \
         "File: t.info,  Node: Third,  Prev: Second,  Up: Top" "* Second::      The second chapter." \
         "The function ‘qnx_init’ returns STATUS.  *Note Third::, for more." "     int main(void) { return 0; }" \
         "  2. two" "* third chapter:                         Third.                 (line 6)" "End Tag Table"; do
  grep -qF "$l" t.info || { bad "t.info lacks: $l"; m=1; }
done; [ -z "$m" ] && ok "Info: node headers, menu, @code/@var, *Note, example, list, index menu, tag table"
r=$(to 120 "$MI" --plaintext t.texi -o t.txt 2>&1)
[ "$(sed -n 6,7p t.txt)" = "1 Second Chapter
****************" ] && ok "--plaintext: numbered, underlined chapter" || bad "plaintext: $(sed -n 6,7p t.txt) $r"
r=$(to 120 "$MI" --html --no-split t.texi -o t.html 2>&1)
grep -q '<h2 class="chapter" id="Second-Chapter">' t.html && grep -q '<h2 class="chapter" id="Third-Chapter">' t.html && ok "--html: chapter headings with ids" || bad "html: $r"
sed 's/@xref{Third}/@xref{Fourth}/' t.texi > t2.texi
r=$(to 120 "$MI" t2.texi -o t2.info 2>&1); rc=$?
[ $rc -ne 0 ] && echo "$r" | has "t2.texi:25: @xref reference to nonexistent node" && ok "reference to a missing node: t2.texi:25 error (exit $rc)" || bad "t2.texi (rc $rc): $r"
r=$("$L/bin/install-info" --info-dir=. t.info 2>&1); rc=$?
[ $rc -eq 0 ] && grep -q '^Testing$' dir && grep -qF '* QNX test: (t).                A Texinfo document made on QNX.' dir && ok "install-info: dir entry under Testing" || bad "install-info (rc $rc): $r"
r=$("$L/bin/info" -f ./t.info -n Second -o - 2>&1 | head -n 3)
[ "$r" = "File: t.info,  Node: Second,  Next: Third,  Prev: Top,  Up: Top

1 Second Chapter" ] && ok "info -n Second -o -: the node printed" || bad "info: $r"
printf '\\entry{zeta}{2}{zeta}\n\\entry{alpha}{1}{alpha}\n\\entry{Mid}{3}{Mid}\n' > x.cp
r=$("$L/bin/texindex" x.cp 2>&1); rc=$?
[ $rc -eq 0 ] && [ "$(cat x.cps 2>/dev/null | tr '\n' ' ')" = '\initial {A} \entry{alpha}{1} \initial {M} \entry{Mid}{3} \initial {Z} \entry{zeta}{2} ' ] \
  && ok "texindex: sorted, case-folded, with initials" || bad "texindex (rc $rc): $r $(cat x.cps 2>/dev/null)"
stopall
[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
