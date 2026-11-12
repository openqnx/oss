# python-OBD (module obd): reads OBD-II data (speed, RPM, trouble codes, ...)
# from a car through an ELM327 adapter on a serial port (/dev/ser*,
# /dev/serusb*), decoded into pint quantities. Pure Python: the published
# wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=0.7.3
TARBALL=https://files.pythonhosted.org/packages/3a/47/8ef35b33fc80db40d4a42632b0a22b2b2ed7671832dbc9bda9d912a826b0/obd-0.7.3-py3-none-any.whl
SHA256=f26da700f13683a27855e6a439db72f7025fff6f1fd47ad4c6c33089d9fe7cab
DEPS="python3 python3-pyserial python3-pint"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-pyserial python3-pint"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
