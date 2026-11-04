# Run-level tests, group g13-rviz: RViz 2 natively on QNX (ros2-rviz, an
# overlay on the ros2 component: OGRE 1.12 on OpenGL ES/EGL/Screen, Qt 6) and
# assimp, which it loads meshes with. Each test uses its component with known
# answers; drawing on Screen is checked on a graphics target (screenshots),
# not here. Which test checks which component: g13-rviz.tsv; what a test
# needs installed: g13-rviz.extras.
port_test(test_assimp     test_assimp.c     assimp)
port_script(test_ros2_rviz)
