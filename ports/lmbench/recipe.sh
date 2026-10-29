VERSION=3.0-git-20260929; GIT=https://github.com/intel/lmbench.git; COMMIT=27b43aeec9dc0bb45f420388c78e42dde4c23bf0
BUILDSYS=custom
# lmbench builds through its scripts/build wrapper, which takes the compiler
# and an OS name for the output directory (bin/<OS>).
do_build() { cp -a "$S/." "$B/" && cd "$B/src" && \
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CFLAGS="-O2 -D_QNX_SOURCE -DNO_RPC -DNO_PORTMAPPER -DRUSAGE -DSYS5" LDLIBS="-lm -lsocket" PTHREAD_LIBS="" OS=qnx \
      AR="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" RANLIB="$QNX_HOST/usr/bin/nto${QNX_CPU}-ranlib" ../scripts/build all; }
do_install() { install -d "$STAGE$PREFIX/lib/lmbench/bin" && cp -a "$B/bin/qnx/." "$STAGE$PREFIX/lib/lmbench/bin/" && \
  cp -a "$B/scripts" "$STAGE$PREFIX/lib/lmbench/"; }
