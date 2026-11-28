# swayidle: the idle management daemon of wlroots compositors such as labwc.
# It asks the compositor (ext-idle-notify-v1) to tell it when there has been no
# input for a given time and runs commands then and when activity resumes:
#   swayidle -w timeout 300 'swaylock -f' timeout 600 'wlopm --off "*"' resume 'wlopm --on "*"'
# SIGUSR1 runs the idle commands at once.
VERSION=1.9.1; GIT=https://github.com/swaywm/swayidle.git; REF=v1.9.1
# wayland: libwayland-client, and libwayland-server for its event loop.
DEPS="wayland wayland-protocols"
BUILDSYS=meson
# - logind (systemd/elogind): the before-sleep, after-resume, lock and unlock
#   events come from logind over sd-bus; QNX has neither, so only the timeout
#   and resume commands are built;
# - no man page (scdoc is not a build tool here); the shell completions are data;
# - werror=false: upstream builds with -Werror, and the QNX headers under
#   -D_POSIX_C_SOURCE with GCC 12 are not warning-free on both architectures.
MESON_ARGS_EXTRA=( -Dlogind=disabled -Dman-pages=disabled -Dwerror=false
                   -Dzsh-completions=true -Dbash-completions=true -Dfish-completions=true )
