VERSION=4.10.0; GIT=https://github.com/opencv/opencv.git; REF=4.10.0
DEPS="zlib eigen"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF
                   -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_JAVA=OFF
                   -DBUILD_opencv_apps=OFF -DBUILD_opencv_python2=OFF
                   -DBUILD_opencv_python3=OFF
                   # find_host_package(PythonInterp OFF) trips OpenCV's own macro under
                   # CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY; no Python is wanted anyway
                   -DOPENCV_PYTHON_SKIP_DETECTION=ON
                   -DBUILD_opencv_python_bindings_generator=OFF
                   # QNX has no V4L/GTK/X11 desktop capture stack
                   -DWITH_V4L=OFF -DWITH_GTK=OFF -DWITH_QT=OFF -DWITH_WIN32UI=OFF
                   -DWITH_FFMPEG=OFF -DWITH_GSTREAMER=OFF -DWITH_1394=OFF
                   -DWITH_OPENCL=OFF -DWITH_CUDA=OFF -DWITH_IPP=OFF
                   -DWITH_LAPACK=OFF -DWITH_PROTOBUF=OFF -DWITH_ITT=OFF
                   -DWITH_TBB=OFF -DWITH_OPENEXR=OFF -DWITH_JASPER=OFF
                   # VA-API is auto-enabled on x86_64; QNX has no libva
                   -DWITH_VA=OFF -DWITH_VA_INTEL=OFF
                   -DWITH_WEBP=OFF -DWITH_TIFF=OFF -DWITH_OPENJPEG=OFF
                   -DBUILD_ZLIB=OFF -DBUILD_PNG=ON -DBUILD_JPEG=ON
                   -DWITH_EIGEN=ON -DOPENCV_GENERATE_PKGCONFIG=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
