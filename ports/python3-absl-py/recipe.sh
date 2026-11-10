# absl-py: Abseil's Python common libraries (absl.flags, absl.logging,
# absl.app, absl.testing), used by TensorFlow and JAX tooling.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=2.5.1
TARBALL=https://files.pythonhosted.org/packages/b3/7d/01e62f59e4166af1be6238d9f2b7f3453630c51b8044b412d05b5606532a/absl_py-2.5.1-py3-none-any.whl
SHA256=721200f2f0e9960f2ca9dc3a2a706b201f5f75d812c158f059cbbe29eeafbdb8
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
