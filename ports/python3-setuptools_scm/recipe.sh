# setuptools-scm: package versions from git/hg metadata.
# Version 9.2.2 (the version QNX ships): 10 splits the core into vcs-versioning.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=9.2.2
TARBALL=https://files.pythonhosted.org/packages/3d/ea/ac2bf868899d0d2e82ef72d350d97a846110c709bacf2d968431576ca915/setuptools_scm-9.2.2-py3-none-any.whl
SHA256=30e8f84d2ab1ba7cb0e653429b179395d0c33775d54807fc5f1dd6671801aef7
DEPS="python3 python3-packaging python3-setuptools"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-packaging python3-setuptools"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
