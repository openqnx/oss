# Install the .data directory of an unpacked wheel the way pip does.
# Sourced by recipe.sh files after scripts/lib/pywheel.sh:
#
#   source "$WS/scripts/lib/pywheel-data.sh"
#   do_install() { qnx_py_install_pure && qnx_py_install_data; }   # or qnx_py_install
#
# qnx_py_install / qnx_py_install_pure unzip a wheel into site-packages. A
# wheel can also carry <dist>-<version>.data/{data,scripts,headers,purelib,
# platlib}/, which pip moves out of site-packages at install time; unzipped,
# they stay there unused (colcon-cd's shell function, colcon-argcomplete's
# completion hook, empy's em.py script, pygame's C headers). This does what
# pip does with them:
#   data/     -> $PREFIX/
#   scripts/  -> $PREFIX/bin/ (a "#!python" line becomes the target's python3)
#   headers/  -> $PREFIX/include/python$QNX_PYV/<dist>/
#   purelib/, platlib/ -> site-packages/
qnx_py_install_data() {
  local sp="$STAGE$PREFIX/lib/python${QNX_PYV:-3.14}/site-packages" d dist f
  for d in "$sp"/*.data; do
    [ -d "$d" ] || continue
    dist=$(basename "$d" .data); dist=${dist%-*}
    if [ -d "$d/data" ]; then mkdir -p "$STAGE$PREFIX" && cp -a "$d/data/." "$STAGE$PREFIX/" || return 1; fi
    if [ -d "$d/scripts" ]; then
      mkdir -p "$STAGE$PREFIX/bin" || return 1
      for f in "$d"/scripts/*; do
        [ -f "$f" ] || continue
        sed '1s|^#!python.*|#!'"$PREFIX"'/bin/python3|' "$f" > "$STAGE$PREFIX/bin/$(basename "$f")" && \
          chmod 755 "$STAGE$PREFIX/bin/$(basename "$f")" || return 1
      done
    fi
    if [ -d "$d/headers" ]; then
      mkdir -p "$STAGE$PREFIX/include/python${QNX_PYV:-3.14}/$dist" && \
        cp -a "$d/headers/." "$STAGE$PREFIX/include/python${QNX_PYV:-3.14}/$dist/" || return 1
    fi
    for f in purelib platlib; do
      if [ -d "$d/$f" ]; then cp -a "$d/$f/." "$sp/" || return 1; fi
    done
    rm -rf "$d"
  done
}
