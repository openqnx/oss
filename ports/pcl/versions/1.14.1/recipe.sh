VERSION=1.14.1; GIT=https://github.com/PointCloudLibrary/pcl.git; REF=pcl-1.14.1
DEPS="boost eigen flann"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON
                   # no VTK/OpenGL/X11 on a QNX target -- visualization is out
                   -DWITH_VTK=OFF -DBUILD_visualization=OFF -DWITH_OPENGL=OFF
                   -DWITH_QT=OFF -DWITH_PCAP=OFF -DWITH_PNG=OFF
                   -DWITH_QHULL=OFF -DBUILD_surface_on_nurbs=OFF
                   -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_ENSENSO=OFF
                   -DWITH_DAVIDSDK=OFF -DWITH_DSSDK=OFF -DWITH_RSSDK=OFF
                   -DWITH_LIBUSB=OFF -DWITH_OPENMP=OFF -DWITH_CUDA=OFF
                   -DBUILD_apps=OFF -DBUILD_examples=OFF -DBUILD_tools=OFF
                   -DBUILD_global_tests=OFF
                   -DBOOST_ROOT="$STAGE$PREFIX"
                   -DEigen3_DIR="$STAGE$PREFIX/share/eigen3/cmake"
                   -DFLANN_ROOT="$STAGE$PREFIX"
                   # PCL probes aligned allocation with check_cxx_source_runs,
                   # which can never succeed when cross-compiling -- CMake marks
                   # it FAILED_TO_RUN and pcl_macros.h then #errors with
                   # "aligned_malloc not supported on your platform". QNX does
                   # provide posix_memalign (verified against the SDP), so seed
                   # the run result instead of letting the probe decide.
                   -DHAVE_POSIX_MEMALIGN=1
                   -DHAVE_POSIX_MEMALIGN_EXITCODE=0
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
