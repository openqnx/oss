# bash-completion: programmable completion for bash -- the completion
# framework (share/bash-completion/bash_completion) and the completions of
# several hundred commands, loaded on demand. Shell scripts, the same for
# every architecture; RUNTIME_DEPS bash (ports/bash, 5.2).
VERSION=2.18.0
TARBALL=https://github.com/scop/bash-completion/releases/download/2.18.0/bash-completion-2.18.0.tar.xz
SHA256=88bcf85124f77f74f2f2f8bcd16ac4382d807a827ede742a64940c7116aea33f
RUNTIME_DEPS="bash"
BUILDSYS=autotools
# sysconfdir is the package tree's own etc (hazard 124): the profile.d
# snippet that interactive shells source goes to $PREFIX/etc/profile.d.
CONF_ARGS_EXTRA=( --sysconfdir=/usr/local/etc )
