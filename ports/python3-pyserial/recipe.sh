# pyserial: serial port access for Python (serial.Serial on /dev/ser*, the
# loop:// and socket:// URL handlers, serial.tools.miniterm). Pure Python,
# built from the source distribution because it needs QNX patches.
# python-can's serial and slcan interfaces and python-OBD use it.
VERSION=3.5
TARBALL=https://files.pythonhosted.org/packages/1e/7d/ae3f0a63f41e4d2f6cb66a5b57197850f919f59e558159a4dd3a818f5082/pyserial-3.5.tar.gz
SHA256=3c77e014170dfffbd816e6ffc205e9842efb10be9f58ec16d3e8675b4925cddb
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
