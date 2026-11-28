# qnxport-timeout.sh -- qto: a time limit for one command of a port test.
#
#   . "$(dirname "$0")/qnxport-timeout.sh" || { echo "FAIL <test> (qnxport-timeout.sh missing)"; exit 1; }
#   qto 30 some command ...      # its exit status; 124 when it was stopped
#
# Why not timeout(1): the target's timeout (toybox) sends SIGTERM at the
# limit and then waits for the command for as long as it takes; a command
# that ignores SIGTERM, or that cannot end, keeps the test waiting for ever.
# qto sends SIGTERM, gives the command 2 s, then SIGKILL -- to the command
# and to the processes it started -- and always comes back. A command that
# ran out of time is a failure of that command: there is no "it had already
# printed its answer" exception.
#
# The command runs in the background of this shell with this shell's standard
# input, so "printf ... | qto 20 cmd" and "qto 20 cmd > file" work as with
# timeout(1). Plain sh (QNX ksh): no arrays, no "local".
qto_tree() {   # $1 = pid: that process and everything below it, deepest first
  pidin -F "%a %e" 2>/dev/null | awk -v root="$1" '
    $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ { par[$1] = $2 }
    END { n = 0; list[n++] = root
          for (i = 0; i < n; i++) for (c in par) if (par[c] == list[i]) { dup = 0
            for (j = 0; j < n; j++) if (list[j] == c) dup = 1
            if (!dup) list[n++] = c }
          for (i = n - 1; i >= 0; i--) print list[i] }'
}
qto_kill_tree() {  # $1 = signal, $2 = pid
  for _qc in $(qto_tree "$2"); do kill "-$1" "$_qc" 2>/dev/null; done
}
qto() {
  _qs=$1; shift
  exec 9<&0             # a background job would get /dev/null as its input
  "$@" <&9 &
  _qp=$!
  exec 9<&-
  _qi=0; _qn=$((_qs * 10))
  while kill -0 "$_qp" 2>/dev/null && [ "$_qi" -lt "$_qn" ]; do sleep 0.1; _qi=$((_qi + 1)); done
  if kill -0 "$_qp" 2>/dev/null; then
    qto_kill_tree TERM "$_qp"
    _qi=0
    while kill -0 "$_qp" 2>/dev/null && [ "$_qi" -lt 20 ]; do sleep 0.1; _qi=$((_qi + 1)); done
    qto_kill_tree KILL "$_qp"
    wait "$_qp" 2>/dev/null
    return 124
  fi
  wait "$_qp"
}
# The name the tests used for their own wrapper around timeout(1).
to() { qto "$@"; }
