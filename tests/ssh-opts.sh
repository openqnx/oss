# Shared SSH options for the target scripts.
#
# tests/ssh/ holds a private key and is deliberately NOT in the repository, so
# a fresh clone has no key at all. Requiring one made "clone, build, run on the
# target" impossible to follow: the deploy scripts failed on a missing file
# with no hint of what to do about it.
#
# Use the workspace key when it is there; otherwise fall back to the caller's
# own SSH setup (agent, ~/.ssh/config, or a password prompt), which is what
# someone following the guide for the first time will have.
#
# accept-new: a freshly built VM has a host key nobody has seen, and BatchMode
# turns ssh's question about it into "Host key verification failed". A key
# that CHANGED for a known address is still refused (rebuilt image: run
# ssh-keygen -f tests/ssh/known_hosts -R <ip>).
qnxport_ssh_opts() {
  local ws=$1
  if [ -f "$ws/tests/ssh/qnx-target" ]; then
    chmod 600 "$ws/tests/ssh/qnx-target" 2>/dev/null
    printf '%s\n' -i "$ws/tests/ssh/qnx-target" \
      -o UserKnownHostsFile="$ws/tests/ssh/known_hosts" \
      -o StrictHostKeyChecking=accept-new \
      -o BatchMode=yes -o ConnectTimeout=10
  else
    # No BatchMode: let ssh ask for a password if that is how the target is set up.
    printf '%s\n' -o ConnectTimeout=10
  fi
}
