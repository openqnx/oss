# Shared by the vanilla-vm scripts: workspace, architecture, VM directory,
# QNX SDP environment, SSH options. Sourced, not run.
WS=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

# x86_64 | aarch64le (aarch64 accepted)
norm_arch() {
  case "$1" in
    x86_64|x86-64|amd64) echo x86_64 ;;
    aarch64|aarch64le|arm64) echo aarch64le ;;
    *) echo "unknown architecture: $1 (x86_64 or aarch64le)" >&2; return 2 ;;
  esac
}

# One directory per architecture (and one more for the graphics image:
# vm_dir <arch> desktop); mkqnximage keeps local/ and output/ there.
vm_dir() { echo "${VM_ROOT:-$HOME/vm}/oss-${2:-vanilla}-$1"; }

sdp_env() {
  # QNX_SDP_ROOT if set; else ~/qnx800 or ~/qnx805 (never a guess among other
  # SDPs installed side by side: their target libraries differ).
  local sdp=${QNX_SDP_ROOT:-}
  [ -n "$sdp" ] || for d in "$HOME/qnx800" "$HOME/qnx805"; do [ -f "$d/qnxsdp-env.sh" ] && { sdp=$d; break; }; done
  [ -f "$sdp/qnxsdp-env.sh" ] || { echo "QNX SDP 8.0 not found: export QNX_SDP_ROOT=<your SDP 8.0>" >&2; return 1; }
  # shellcheck disable=SC1091
  . "$sdp/qnxsdp-env.sh" >/dev/null
  export QNX_SDP_ROOT=$sdp
  echo ">> QNX SDP: $sdp" >&2
}

# The workspace SSH key (tests/ssh/, not in git): made here on first use, so
# the VM is built with it and every target script finds it.
ssh_key() {
  local k=$WS/tests/ssh/qnx-target
  if [ ! -f "$k" ]; then
    mkdir -p "$WS/tests/ssh"
    ssh-keygen -q -t ed25519 -N '' -C qnx-target -f "$k"
  fi
  echo "$k"
}

. "$WS/tests/ssh-opts.sh"
mapfile -t SSH_O < <(qnxport_ssh_opts "$WS")
on_vm() { local ip=$1; shift; ssh "${SSH_O[@]}" "root@$ip" "$@"; }

vm_ip() { (cd "$(vm_dir "$1" "$2")" 2>/dev/null && mkqnximage --getip 2>/dev/null | grep -Eo '([0-9]+\.){3}[0-9]+' | head -n 1); }
