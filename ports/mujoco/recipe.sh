# mujoco: DeepMind's MuJoCo physics engine (multi-body dynamics with contact),
# the library and its model compiler; no simulate viewer (needs GLFW/OpenGL).
# Its dependencies (tinyxml2, qhull, ccd, lodepng, ...) are fetched at configure
# time at the versions MuJoCo pins, and linked in statically.
VERSION=3.10.0; GIT=https://github.com/google-deepmind/mujoco.git; REF=3.10.0
DEPS=""
BUILDSYS=cmake
# Link-time optimisation is on by default for release builds and CMake has no
# IPO support for QNX's compiler ("CMake doesn't support IPO"): off.
# libccd (fetched) compiles as C90, and QNX's C headers use "static inline":
# for C, inline is spelled __inline__, which GCC takes in every mode and
# gives the mode's own inline semantics.
CMAKE_ARGS_EXTRA=( -DCMAKE_C_FLAGS=-Dinline=__inline__ -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF -DMUJOCO_BUILD_EXAMPLES=OFF -DMUJOCO_BUILD_SIMULATE=OFF -DMUJOCO_BUILD_TESTS=OFF -DMUJOCO_TEST_PYTHON_UTIL=OFF -DMUJOCO_ENABLE_AVX=OFF -DMUJOCO_ENABLE_AVX_INTRINSICS=OFF )
