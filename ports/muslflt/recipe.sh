# muslflt: musl's libm (math.h, fenv.h, complex.h) and its string<->float
# conversions, packaged by qnx-ports so QNX programs can get the float
# behaviour of other UNIX systems by linking -lmuslflt. A qnx-ports project
# (not an upstream release); pinned commit.
VERSION=1.0-git-20240802; GIT=https://github.com/qnx-ports/muslflt.git; COMMIT=37ac677512d09b83e26533807d2fb330d4fcabd6
BUILDSYS=custom
# qnx-ports builds it with QNX recursive make from a list of source
# directories. The same here, explicitly: every .c/.s/.S in string, complex,
# fenv, math and qnx, where a file in an architecture directory (math/<cpu>,
# fenv/<cpu>) replaces the generic file of the same name -- musl's rule.
do_build() {
  local cpu=$QNX_ARCH dirs="string complex fenv math qnx" d f base
  declare -A pick
  for d in $dirs; do for f in "$S/$d"/*.c; do [ -e "$f" ] && pick[$(basename "${f%.*}")]="$f"; done; done
  for d in fenv/$cpu math/$cpu; do for f in "$S/$d"/*.[csS]; do [ -e "$f" ] && pick[$(basename "${f%.*}")]="$f"; done; done
  mkdir -p "$B/obj"; local inc="-I$S/fenv/$cpu -I$S/math/$cpu -I$S/math -I$S/fenv -I$S/string -I$S/complex -I$S/qnx"
  local objs=() n=0
  for base in "${!pick[@]}"; do
    f=${pick[$base]}
    qcc -V$QCC_TARGET -c -O2 -fPIC -D_QNX_SOURCE -Wno-parentheses -Werror=implicit-function-declaration \
        -Wno-unused-but-set-variable $inc "$f" -o "$B/obj/$base.o" || return 1
    objs+=("$B/obj/$base.o"); n=$((n+1))
  done
  echo "muslflt: $n objects"
  # -Bsymbolic: musl's functions call each other (snprintf -> vsnprintf,
  # cexp -> exp, printf's %a -> frexpl). Through the PLT those calls bind to the
  # first definition in the process, and QNX's libc/libm are always loaded
  # before this library (libc.so is the first DT_NEEDED of every executable),
  # so muslflt's snprintf ran QNX libc's vsnprintf. Bind them inside the library.
  qcc -V$QCC_TARGET -shared -Wl,-soname,libmuslflt.so.1 -Wl,-Bsymbolic -o "$B/libmuslflt.so.1" "${objs[@]}" -lm || return 1
  "$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" rcs "$B/libmuslflt.a" "${objs[@]}"
}
do_install() {
  install -d "$STAGE$PREFIX/lib" && install -m755 "$B/libmuslflt.so.1" "$STAGE$PREFIX/lib/" && \
  ln -sfn libmuslflt.so.1 "$STAGE$PREFIX/lib/libmuslflt.so" && install -m644 "$B/libmuslflt.a" "$STAGE$PREFIX/lib/"
}
