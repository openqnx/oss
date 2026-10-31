VERSION=0.15.2; GIT=https://github.com/libcheck/check.git; REF=0.15.2
# checkmk is an awk script whose "#!" line is configured from find_program(awk)
# -- on the build machine, so it named the cross build's linkroot awk
# (/home/.../stage/x86_64/linkroot/x86_64/usr/bin/awk), which no target has.
# Point it at the awk the gawk package installs on the target, and depend on it.
RUNTIME_DEPS="gawk"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DCHECK_ENABLE_TESTS=OFF -DCHECK_ENABLE_TIMEOUT_TESTS=OFF
                   -DAWK_PATH=/usr/local/bin/awk )
