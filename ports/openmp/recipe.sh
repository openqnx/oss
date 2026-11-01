VERSION=22.1.8; GIT=https://github.com/llvm/llvm-project.git; REF=llvmorg-22.1.8
# LLVM's OpenMP runtime, libomp, from the same llvm-project tree and QNX
# patches as the llvm component (ports/llvm: qnx-ports' fork, whose
# "qnx/openmp: add qnx target support" commit is the QNX port of libomp), as
# a component of its own so that llvm is not rebuilt. Built standalone from
# openmp/ with the SDP's GCC.
# libomp also implements GCC's GOMP_* entry points, and the SDP's GCC has no
# libgomp: code compiled with "qcc -fopenmp" links against libomp
# (-fopenmp to compile, -lomp to link), as does clang -fopenmp on the target.
SRCROOT="$SRC_DIR/llvm"
# The llvm component's source tree is shared (it has these patches applied).
# Fetched here only when it is not there yet: llvm-project at the same tag
# with ports/llvm/patches -- the tree the llvm component builds from.
do_fetch() {
  git clone -q --depth 1 --branch "$REF" "$GIT" "$SRCROOT" || return 1
  local p; for p in "$WS"/ports/llvm/patches/*.patch; do
    echo "-- patch (llvm) $(basename "$p")"; patch -p1 -d "$SRCROOT" < "$p" || { rm -rf "$SRCROOT"; return 1; }
  done
}
SUBDIR=openmp
# No offloading (libomptarget: GPUs), no OMPD debugging library (needs
# gdb-plugin pieces and Python), no Fortran module. The GNU/Intel name
# aliases (libgomp.so, libiomp5.so symlinks) are not installed: a libgomp.so
# that is libomp would shadow GCC's should it ever be added. Python is the
# build machine's: libomp generates its export lists with a script; the
# staging tree's python3 is a QNX program. -lcatalog: libomp reads its
# messages with catopen()/catgets(), which QNX has in libcatalog, not libc
# (unlinked, every program using libomp failed to link on them).
CMAKE_ARGS_EXTRA=( -DOPENMP_STANDALONE_BUILD=ON -DOPENMP_ENABLE_LIBOMPTARGET=OFF -DLIBOMP_OMPD_SUPPORT=OFF
                   -DLIBOMP_FORTRAN_MODULES=OFF -DLIBOMP_INSTALL_ALIASES=OFF -DOPENMP_ENABLE_TESTING=OFF
                   -DLIBOMP_ENABLE_SHARED=ON -DLIBOMP_USE_HWLOC=OFF -DOPENMP_ENABLE_OMPT_TOOLS=OFF
                   -DLIBOMP_LIBFLAGS=-lcatalog -DPython3_EXECUTABLE=/usr/bin/python3 )
