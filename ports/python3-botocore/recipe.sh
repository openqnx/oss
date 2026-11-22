# botocore: the low-level, data-driven core of the AWS SDK for Python. Without the optional awscrt extra (the AWS Common Runtime bindings).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.44.9
TARBALL=https://files.pythonhosted.org/packages/20/ad/f8143d7a78503c3b67d135589cd1da82d3867ed1f37149e64d6f9bef6cfb/botocore-1.44.9-py3-none-any.whl
SHA256=6966752d38514a8e457ea062ea171b2b320c55bad23c8ae4f74f88296e00890d
DEPS="python3 python3-jmespath python3-dateutil python3-urllib3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-jmespath python3-dateutil python3-urllib3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
