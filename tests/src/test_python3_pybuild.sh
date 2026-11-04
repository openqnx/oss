#!/bin/sh
# The Python packaging tool chain, run offline on the target with known
# answers. build (pyproject-build, through pyproject-hooks) builds wheels
# with each backend: flit-core, hatchling (version from git through hatch-vcs
# and setuptools-scm, README composed by hatch-fancy-pypi-readme), setuptools
# with calver (version from SOURCE_DATE_EPOCH), meson-python (meson and
# ninja) and scikit-build-core (cmake and ninja); gpep517 builds one too;
# installer installs a wheel into a destination directory, which then
# imports; pyproject-metadata turns a [project] table into core metadata;
# editables makes a project importable from its source tree.
echo "== python3-pybuild =="
PY=${QNX_PORT_PYTHON:-/usr/local/bin/python3}; [ -x "$PY" ] || PY=python3
# No __pycache__ left in the installation's site-packages (docs/porting-hazards.md 183)
export PYTHONDONTWRITEBYTECODE=1
T=$(mktemp -d "${TMPDIR:-/tmp}/qnxport-pybuild.XXXXXX") || { echo "FAIL python3-pybuild (no temporary directory)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
cd "$T" || exit 1
export HOME="$T" GIT_CONFIG_NOSYSTEM=1 PIP_NO_INDEX=1
fail=0
ok()  { echo "  ok   $1"; }
bad() { echo "  FAIL $1"; fail=1; }
# build <dir> <name-version>: python -m build --wheel --no-isolation makes
# one pure wheel <name-version>-*-none-any.whl; its path is left in $W
build() {
  ( cd "$1" && "$PY" -m build --wheel --no-isolation --outdir dist . ) > "$1.log" 2>&1
  W=$(ls "$1"/dist/"$2"-*-none-any.whl 2>/dev/null | head -n 1)
  if [ -n "$W" ]; then ok "$1: build makes $(basename "$W")"; else bad "$1: build makes $2-*-none-any.whl (got: $(ls "$1/dist" 2>/dev/null | tr '\n' ' '))"; tail -n 15 "$1.log" | sed 's/^/  | /'; W=/nonexistent; fi
}
# wheel_has <wheel> <member> [text]: the wheel contains member (with text)
wheel_has() {
  "$PY" - "$@" <<'P'
import sys, zipfile
z = zipfile.ZipFile(sys.argv[1]); m = sys.argv[2]
ok = m in z.namelist() and (len(sys.argv) < 4 or sys.argv[3] in z.read(m).decode())
sys.exit(0 if ok else 1)
P
}

# ---- flit-core
mkdir -p flitp/qnxflit
printf '"""QNX flit test."""\n__version__ = "1.2.0"\ndef answer():\n    return 42\n' > flitp/qnxflit/__init__.py
cat > flitp/pyproject.toml <<'P'
[build-system]
requires = ["flit_core>=3.4"]
build-backend = "flit_core.buildapi"
[project]
name = "qnxflit"
dynamic = ["version", "description"]
P
build flitp qnxflit-1.2.0
FW=$W
wheel_has "$FW" qnxflit-1.2.0.dist-info/METADATA "Summary: QNX flit test." && ok "flit-core: version and summary from the module" || bad "flit-core: version and summary from the module"

# ---- installer
"$PY" -m installer --destdir "$T/root" "$FW" > inst.log 2>&1 || { bad "installer exits 0"; cat inst.log; }
SP=$(find "$T/root" -type d -name site-packages | head -n 1)
[ -f "$SP/qnxflit/__init__.py" ] && [ -f "$SP/qnxflit-1.2.0.dist-info/RECORD" ] && ok "installer: package and RECORD under the destination's site-packages" || bad "installer: package and RECORD installed ($SP)"
r=$(PYTHONPATH="$SP" "$PY" -c "import qnxflit, importlib.metadata as m; print(qnxflit.answer(), m.version('qnxflit'))" 2>&1)
[ "$r" = "42 1.2.0" ] && ok "installer: installed package imports, its metadata reads 1.2.0" || bad "installed package imports ($r)"

# ---- hatchling + hatch-vcs + setuptools-scm + hatch-fancy-pypi-readme, built by gpep517
mkdir -p hatchp/src/qnxhatch
echo 'def hello(): return "hatch"' > hatchp/src/qnxhatch/__init__.py
cat > hatchp/pyproject.toml <<'P'
[build-system]
requires = ["hatchling", "hatch-vcs", "hatch-fancy-pypi-readme"]
build-backend = "hatchling.build"
[project]
name = "qnxhatch"
dynamic = ["version", "readme"]
[tool.hatch.version]
source = "vcs"
[tool.hatch.build.targets.wheel]
packages = ["src/qnxhatch"]
[tool.hatch.metadata.hooks.fancy-pypi-readme]
content-type = "text/markdown"
[[tool.hatch.metadata.hooks.fancy-pypi-readme.fragments]]
text = "# qnxhatch\n\nFragment one from QNX.\n"
[[tool.hatch.metadata.hooks.fancy-pypi-readme.fragments]]
path = "CHANGES.md"
[[tool.hatch.metadata.hooks.fancy-pypi-readme.substitutions]]
pattern = "#(\\d+)"
replacement = "[#\\1](https://example.invalid/\\1)"
P
printf 'Changes: fixed #17.\n' > hatchp/CHANGES.md
( cd hatchp && git init -q && git add -A && git -c user.name=t -c user.email=t@example.invalid commit -q -m one && git tag v2.5.1 ) > git.log 2>&1 || { bad "git repository for hatch-vcs"; cat git.log; }
w=$(cd hatchp && "$PY" -m gpep517 build-wheel --wheel-dir dist --output-fd 1 2> ../gpep517.log | tail -n 1)
case "$w" in qnxhatch-2.5.1-*-none-any.whl) okw=1 ;; *) okw=0 ;; esac
HW=hatchp/dist/$w
if [ $okw = 1 ] && [ -f "$HW" ]; then ok "gpep517 + hatchling: version 2.5.1 from the git tag (hatch-vcs, setuptools-scm)"; else bad "gpep517 + hatchling + hatch-vcs: wheel qnxhatch-2.5.1 (got '$w')"; tail -n 15 gpep517.log | sed 's/^/  | /'; fi
wheel_has "$HW" qnxhatch-2.5.1.dist-info/METADATA "Fragment one from QNX." && \
wheel_has "$HW" qnxhatch-2.5.1.dist-info/METADATA "fixed [#17](https://example.invalid/17)" && \
  ok "hatch-fancy-pypi-readme: README from a text and a file fragment, with substitution" || bad "hatch-fancy-pypi-readme: README fragments in METADATA"
wheel_has "$HW" qnxhatch/__init__.py && ok "hatchling: package from src/ in the wheel" || bad "hatchling: package in the wheel"
( cd hatchp && echo x >> CHANGES.md && git -c user.name=t -c user.email=t@example.invalid commit -q -am two )
v=$(cd hatchp && "$PY" -c "from setuptools_scm import get_version; print(get_version())" 2>&1)
case "$v" in 2.5.2.dev1+g*) ok "setuptools-scm: one commit after v2.5.1 is $v" ;; *) bad "setuptools-scm: 2.5.2.dev1+g... after one commit (got $v)" ;; esac

# ---- setuptools + calver
mkdir -p calp/qnxcal
echo 'X = 1' > calp/qnxcal/__init__.py
printf '[build-system]\nrequires = ["setuptools", "calver"]\nbuild-backend = "setuptools.build_meta"\n' > calp/pyproject.toml
printf 'from setuptools import setup\nsetup(name="qnxcal", use_calver=True, packages=["qnxcal"])\n' > calp/setup.py
SOURCE_DATE_EPOCH=946728000 build calp qnxcal-2000.1.1

# ---- meson-python (meson + ninja)
mkdir -p mesp/qnxmes
echo 'def where(): return "meson"' > mesp/qnxmes/__init__.py
printf "project('qnxmes', version: '0.3.0')\npy = import('python').find_installation(pure: true)\npy.install_sources('qnxmes/__init__.py', subdir: 'qnxmes')\n" > mesp/meson.build
printf '[build-system]\nrequires = ["meson-python"]\nbuild-backend = "mesonpy"\n[project]\nname = "qnxmes"\ndynamic = ["version"]\n' > mesp/pyproject.toml
build mesp qnxmes-0.3.0
wheel_has "$W" qnxmes/__init__.py && ok "meson-python: source installed by meson is in the wheel" || bad "meson-python: qnxmes/__init__.py in the wheel"

# ---- scikit-build-core (cmake + ninja)
mkdir -p skbp/src/qnxskb
echo 'def where(): return "cmake"' > skbp/src/qnxskb/__init__.py
printf 'VERSION 0.4.0\n' > skbp/data.txt
printf 'cmake_minimum_required(VERSION 3.15)\nproject(qnxskb LANGUAGES NONE)\ninstall(FILES data.txt DESTINATION qnxskb)\n' > skbp/CMakeLists.txt
printf '[build-system]\nrequires = ["scikit-build-core"]\nbuild-backend = "scikit_build_core.build"\n[project]\nname = "qnxskb"\nversion = "0.4.0"\n[tool.scikit-build]\nwheel.py-api = "py3"\nwheel.platlib = false\nwheel.packages = ["src/qnxskb"]\n' > skbp/pyproject.toml
build skbp qnxskb-0.4.0
wheel_has "$W" qnxskb/data.txt "VERSION 0.4.0" && wheel_has "$W" qnxskb/__init__.py && \
  ok "scikit-build-core: file installed by CMake and the Python package in the wheel" || bad "scikit-build-core: CMake-installed file and package in the wheel"

# ---- pyproject-metadata, editables
cat > meta.py <<'P'
import sys, os, site, subprocess
import pyproject_metadata as pm, editables
fails = 0
def check(ok, what):
    global fails
    print(("  ok   " if ok else "  FAIL ") + what); fails += (not ok)
md = pm.StandardMetadata.from_pyproject({"project": {"name": "QNX_Thing", "version": "1.0.post1", "description": "d",
      "dependencies": ["attrs>=20", "tomli; python_version<'3.11'"], "requires-python": ">=3.10",
      "optional-dependencies": {"cli": ["click"]}, "license": "MIT", "keywords": ["rtos", "qnx"]}})
msg = str(md.as_rfc822())
check(md.canonical_name == "qnx-thing" and str(md.version) == "1.0.post1", "pyproject-metadata: canonical name and version")
check("Requires-Dist: attrs>=20" in msg and 'Requires-Dist: click; extra == "cli"' in msg and "Requires-Python: >=3.10" in msg, "pyproject-metadata: Requires-Dist, extras, Requires-Python in core metadata")
check("License-Expression: MIT" in msg and "Keywords: rtos,qnx" in msg, "pyproject-metadata: License-Expression and Keywords")
try:
    pm.StandardMetadata.from_pyproject({"project": {"name": "x"}}); check(False, "pyproject-metadata: missing version raises")
except Exception as e:
    check("version" in str(e), "pyproject-metadata: missing version is an error")
os.makedirs("edproj/qnxed", exist_ok=True); os.makedirs("edsite", exist_ok=True)
open("edproj/qnxed/__init__.py", "w").write("WHERE = __file__\n")
p = editables.EditableProject("qnxed", os.path.abspath("edproj"))
p.map("qnxed", "qnxed")
files = dict(p.files())
check(set(files) == {"_editable_impl_qnxed.pth", "_editable_impl_qnxed.py"} and p.dependencies() == ["editables"], "editables: .pth and import hook files")
for n, c in files.items(): open(os.path.join("edsite", n), "w").write(c)
r = subprocess.run([sys.executable, "-c", "import site, sys; site.addsitedir(sys.argv[1]); import qnxed; print(qnxed.WHERE)", os.path.abspath("edsite")], capture_output=True, text=True)
check(r.stdout.strip() == os.path.abspath("edproj/qnxed/__init__.py"), "editables: the package imports from its source tree")
sys.exit(1 if fails else 0)
P
"$PY" meta.py || fail=1
[ $fail -eq 0 ] && { echo "PASS python3-pybuild"; exit 0; }
echo "FAIL python3-pybuild"; exit 1
