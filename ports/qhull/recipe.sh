# Qhull: convex hulls, Delaunay triangulations, Voronoi diagrams and halfspace
# intersections (libqhull_r, the reentrant library matplotlib's triangulation
# uses; also the qhull, qconvex, qdelaunay ... programs and libqhullcpp).
VERSION=8.0.2; GIT=https://github.com/qhull/qhull.git; REF=v8.0.2
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON -DLINK_APPS_SHARED=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
