# Pythran: an ahead-of-time compiler from a subset of Python with numpy to
# C++ extension modules (scipy uses it). The compiler and its C++ headers
# run on the target; compiling there needs a C++ compiler on the target.
# Pure Python, built from the source distribution (scripts/lib/pywheel.sh)
# so that patch 0001 applies: its bundled "omp" module looked for an OpenMP
# library with ctypes.util.find_library() on every import attempt, which on
# QNX spawns ldconfig and ld (neither exists) -- 144 failed spawns, 50 s of
# every "import pythran" on an x86_64 VM, far longer on an emulated aarch64.
# Its console scripts are written as launchers in bin/ (qnx_py_scripts).
VERSION=0.19.0
TARBALL=https://files.pythonhosted.org/packages/bb/a3/03098a8a5d2c9f801c227cf941d85976934f5f47307ef3d819734370acdc/pythran-0.19.0.tar.gz
SHA256=18a096daa8b4bbe3ace06ce5d7325be5ff57aec4cf67b91834cf1b757b3e5392
DEPS="python3 python3-ply python3-setuptools python3-gast python3-beniget numpy"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-ply python3-setuptools python3-gast python3-beniget numpy"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
