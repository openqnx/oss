# CGAL, the Computational Geometry Algorithms Library: convex hulls,
# triangulations (Delaunay, constrained, regular), arrangements, polygon and
# mesh processing, with exact predicates and constructions. Header-only since
# CGAL 5; installed are its headers and CMake package (CGALConfig.cmake),
# which brings in GMP and MPFR for the exact number types (Epeck, Gmpq).
VERSION=6.2.1; GIT=https://github.com/CGAL/cgal.git; REF=v6.2.1
DEPS="gmp mpfr boost"
RUNTIME_DEPS="gmp mpfr"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DBUILD_TESTING=OFF -DWITH_CGAL_Qt6=OFF -DWITH_demos=OFF -DWITH_examples=OFF )
