# boto3: the AWS SDK for Python (resources, clients, S3 transfers).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.44.9
TARBALL=https://files.pythonhosted.org/packages/9e/66/73c8c57119a8acf3688dfbe95c4648e8b3e4d6a7a3783b0619b65d201f02/boto3-1.44.9-py3-none-any.whl
SHA256=aff889e8ca6f6006f0b5658b712941471d21e4675d6d48421974a20a5a17a918
DEPS="python3 python3-botocore python3-s3transfer python3-jmespath"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-botocore python3-s3transfer python3-jmespath"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
