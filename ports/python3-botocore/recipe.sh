# botocore: the low-level, data-driven core of the AWS SDK for Python. Without the optional awscrt extra (the AWS Common Runtime bindings).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.43.123
TARBALL=https://files.pythonhosted.org/packages/db/96/4f2dffd304425c07160ab2c12cb13322d852fd956fe9d4386228c93f02ea/botocore-1.43.123-py3-none-any.whl
SHA256=e30580f9c1282c321529238ff8ff75066096a2adbd0a293c8ae592c21d1733a3
DEPS="python3 python3-jmespath python3-dateutil python3-urllib3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-jmespath python3-dateutil python3-urllib3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
