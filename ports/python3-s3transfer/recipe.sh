# s3transfer: Amazon S3 transfer manager (multipart uploads/downloads) for boto3.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.19.3
TARBALL=https://files.pythonhosted.org/packages/e2/a4/88f9e10498759121060b8050a1554753dfc04df39bbbcc7aad9ecc8d9440/s3transfer-0.19.3-py3-none-any.whl
SHA256=24eb7535804306355e31c952a90022bf5b2f9d760f9a11062c7493a217c08553
DEPS="python3 python3-botocore"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-botocore"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
