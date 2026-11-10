# Run-level tests, group g15-robotics: robotics libraries (AprilTag, OSQP,
# Ruckig, OMPL, CGAL, CasADi, ACADO, gloo), the Zenoh router, Lanelet2 and
# the ROS 2 Jazzy overlays (ros2_control, Nav2, MoveIt 2) -- and test_ros2
# (in tests/CMakeLists.txt), which also checks rclpy and tf2 from Python.
# Each test computes something with a known answer. Which test checks which
# component: g15-robotics.tsv; what a test needs installed: g15-robotics.extras.
port_test(test_apriltag   test_apriltag.c   apriltag m)
port_test(test_osqp       test_osqp.c       osqp m)
port_test(test_ruckig     test_ruckig.cpp   ruckig)
port_test(test_ompl       test_ompl.cpp     ompl)
target_include_directories(test_ompl PRIVATE "${INC}/ompl-1.7" "${INC}/eigen3")
port_test(test_cgal       test_cgal.cpp     gmp mpfr)
port_test(test_gloo       test_gloo.cpp     gloo)
port_test(test_casadi     test_casadi.cpp   casadi)
port_test(test_acado      test_acado.cpp    acado_toolkit_s)
target_include_directories(test_acado PRIVATE "${INC}/acado" "${STAGE}/share/acado/external_packages")
# ACADO's headers carry its bundled Eigen 3.2, which uses std::binder2nd
# (removed in C++17): compile as C++14 (ACADO itself is built as C++11).
set_target_properties(test_acado PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
port_script(test_ros2_jazzy_control)
port_test(test_zenoh_router test_zenoh_router.c zenohc)
# ---- Lanelet2 (ROS packages in the ROS prefix) ----
set(ROSP "${STAGE}/opt/ros/jazzy")
port_test(test_lanelet2   test_lanelet2.cpp
          "${ROSP}/lib/liblanelet2_routing.so" "${ROSP}/lib/liblanelet2_traffic_rules.so" "${ROSP}/lib/liblanelet2_io.so"
          "${ROSP}/lib/liblanelet2_projection.so" "${ROSP}/lib/liblanelet2_core.so" boost_serialization boost_filesystem)
target_include_directories(test_lanelet2 PRIVATE "${ROSP}/include" "${INC}/eigen3")
port_script(test_ros2_jazzy_nav2)
port_script(test_ros2_jazzy_moveit2)
