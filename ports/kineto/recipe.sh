# Kineto: PyTorch's CPU/GPU profiling library (libkineto), at the upstream
# commit qnx-ports' QNX port is based on. Built standalone for CPU tracing;
# patches/: QNX build support.
VERSION=0-git-20250214; GIT=https://github.com/pytorch/kineto.git; COMMIT=a054a4be0db117c579a21747debf19c863631f26
SUBDIR=libkineto
CMAKE_ARGS_EXTRA=( -DKINETO_BUILD_TESTS=OFF -DLIBKINETO_NOCUPTI=ON -DLIBKINETO_NOROCTRACER=ON -DLIBKINETO_NOXPUPTI=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPython3_EXECUTABLE=/usr/bin/python3
                   -DFMT_INSTALL=OFF )
# kineto compiles its bundled fmt (third_party/fmt, header-only) and, left to
# itself, installs it: an older fmt's headers, CMake package, fmt.pc and
# libfmt.a over the fmt port's, after which every fmt user failed to link
# ("undefined reference to fmt::v10::vformat"). Keep the bundled copy private.
