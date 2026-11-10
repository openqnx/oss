# python-sounddevice: play and record sound with Python (NumPy arrays or
# buffers) through PortAudio -- the ported portaudio, on QNX's ALSA-compatible
# libasound. Pure Python: cffi in ABI mode, the _sounddevice module generated
# at build time from sounddevice_build.py, built from the source
# distribution (scripts/lib/pywheel.sh). Patch 0001: the library is opened by
# its soname when ctypes.util.find_library() finds nothing, which it never
# does on QNX (no ldconfig, no compiler on the target).
VERSION=0.5.6
TARBALL=https://files.pythonhosted.org/packages/ec/db/0c890e2d9aab9ba284021efc02e1d3aebfecab1b611762d7434602209bcf/sounddevice-0.5.6.tar.gz
SHA256=8ec9fbfde2e32f020b167e348f3ab3bac6625a5f15af524d790108ac7147a410
DEPS="python3 portaudio python3-cffi"
# Python modules are imported, not linked; libportaudio is dlopen()ed.
RUNTIME_DEPS="python3 portaudio python3-cffi"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv cffi "setuptools-scm"; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
