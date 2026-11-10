# AprilTag 3: visual fiducial (tag) detection for robotics -- finds tags in
# an 8-bit grey image and returns their IDs, corners, homography and pose.
# The C library and its demo program only: the examples (one is an OpenCV
# camera demo) and the Python wrapper are not built.
VERSION=3.4.5; GIT=https://github.com/AprilRobotics/apriltag.git; REF=v3.4.5
DEPS=""
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF -DBUILD_PYTHON_WRAPPER=OFF )
