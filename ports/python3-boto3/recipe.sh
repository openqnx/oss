# boto3: the AWS SDK for Python (resources, clients, S3 transfers).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.43.123
TARBALL=https://files.pythonhosted.org/packages/96/b2/a7986c327f80aa6f1544f4cb8f2fa7da4e1fccd472379a21cad0508b8a0e/boto3-1.43.123-py3-none-any.whl
SHA256=d18d66e1e13faa0c25d0b1b8c5ce946e29fb5fc0b6456b655a93a678ff4cc313
DEPS="python3 python3-botocore python3-s3transfer python3-jmespath"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-botocore python3-s3transfer python3-jmespath"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
