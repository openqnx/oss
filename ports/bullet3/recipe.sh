# Bullet Physics SDK: the core libraries only -- LinearMath, collision
# detection, rigid/soft body dynamics, Bullet3 (the GPU pipeline's common,
# geometry, collision and dynamics parts, and clew, which only looks for an
# OpenCL library at run time) and the Extras (world importers, inverse dynamics, convex decomposition,
# GImpact utilities). Not BulletRobotics, BulletRoboticsGUI and obj2sdf:
# they are built from the examples tree (the pybullet shared-memory and
# GUI client code). No examples, demos, GUI, OpenGL or pybullet: the
# target has no display.
VERSION=3.25; GIT=https://github.com/bulletphysics/bullet3.git; REF=3.25
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DINSTALL_LIBS=ON
                   -DBUILD_BULLET2_DEMOS=OFF -DBUILD_CPU_DEMOS=OFF -DBUILD_OPENGL3_DEMOS=OFF
                   -DBUILD_EGL=OFF -DBUILD_ENET=OFF -DBUILD_CLSOCKET=OFF
                   -DBUILD_UNIT_TESTS=OFF -DBUILD_PYBULLET=OFF
                   -DBUILD_EXTRAS=ON -DINSTALL_EXTRA_LIBS=ON -DBUILD_BULLET3=ON
                   -DBUILD_BULLET_ROBOTICS_EXTRA=OFF -DBUILD_BULLET_ROBOTICS_GUI_EXTRA=OFF
                   -DBUILD_OBJ2SDF_EXTRA=OFF -DENABLE_VHACD=OFF -DUSE_GLUT=OFF -DUSE_GRAPHICAL_BENCHMARK=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
