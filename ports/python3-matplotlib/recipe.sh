# Matplotlib: 2D plotting for Python. Its C++ extensions (Agg rasterizer,
# FreeType/libraqm text, path and image operations, Qhull triangulation;
# pybind11) are cross compiled like numpy's (meson-python with a cross file,
# scripts/lib/pywheel.sh), against the ported FreeType, libraqm and Qhull
# (system-*=true): Matplotlib otherwise downloads and builds its own copies
# of FreeType, HarfBuzz, SheenBidi, libraqm and Qhull during the build
# (--wrap-mode=nodownload makes a missing one an error instead).
# b_lto=false: pyproject.toml turns LTO on, and the cross binutils' ar has no
# LTO plugin, so the bundled Agg static library got no symbol index and
# _path/_backend_agg linked with its symbols unresolved ("Unresolved
# symbols" at import on the target).
# Backends that work on QNX: Agg (PNG), PDF, PS, SVG -- the default is Agg
# (no GUI toolkit binding is ported).
VERSION=3.11.2
TARBALL=https://files.pythonhosted.org/packages/e7/c8/9aa712a0afb882649424dd8de8ad9aa6235e796e84c6052e8f6dc1598d0d/matplotlib-3.11.2.tar.gz
SHA256=cec596316640f2b394b8f0daa0ea61a8eae82d017b620b9f202befb972a59ea4
DEPS="numpy freetype libraqm qhull python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil"
RUNTIME_DEPS="numpy python3-contourpy python3-cycler python3-fonttools python3-kiwisolver python3-packaging python3-pillow python3-pyparsing python3-dateutil"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
# pybind11 (headers only) comes from the build venv; Meson finds it through
# its config tool, which a cross build looks up in the cross file.
do_configure() {
  qnx_py_venv "meson-python>=0.18" "meson>=1.2" "pybind11>=2.13.2,!=2.13.3" "setuptools_scm>=7,<10" "patchelf>=0.11.0" || return 1
  qnx_py_crossfile && echo "pybind11-config = '$B/venv/bin/pybind11-config'" >> "$B/qnx-cross.ini" || return 1
  # FreeType's .pc requires zlib's, which zlib installs in share/pkgconfig:
  # that one file is added (all of share/pkgconfig would also bring a
  # pybind11.pc, which Meson would prefer to the venv's pybind11).
  mkdir -p "$B/pc" && ln -sfn "$STAGE$PREFIX/share/pkgconfig/zlib.pc" "$B/pc/zlib.pc" || return 1
  sed -i "s#^pkg_config_libdir = \['\([^']*\)'\]#pkg_config_libdir = ['\1', '$B/pc']#" "$B/qnx-cross.ini"
}
do_build() {
  qnx_py_wheel -Cbuild-dir="$B/build" -Csetup-args="--cross-file=$B/qnx-cross.ini" \
    -Csetup-args="--native-file=$B/qnx-native.ini" \
    -Csetup-args="-Dsystem-freetype=true" -Csetup-args="-Dsystem-libraqm=true" \
    -Csetup-args="-Dsystem-qhull=true" -Csetup-args="-Dmacosx=false" \
    -Csetup-args="--wrap-mode=nodownload" -Csetup-args="-Db_lto=false"
}
do_install() { qnx_py_install; }
