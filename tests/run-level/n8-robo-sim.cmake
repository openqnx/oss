# Run-level tests, group n8-robo-sim: robotics and simulation components
# (Bullet physics, urdfdom, Cartographer SLAM, Vector SIL Kit, ioquake3, lxml,
# netifaces). Each test computes or exchanges something with a known answer;
# servers it starts listen on loopback only and are stopped on every exit
# path. Which test checks which component: n8-robo-sim.tsv; what a test needs
# installed beyond what it links: n8-robo-sim.extras.

# ---- Bullet: falling sphere to rest, ray and contact queries ----
port_test(test_bullet3          test_bullet3.cpp         BulletDynamics BulletCollision LinearMath m)
target_include_directories(test_bullet3 PRIVATE "${INC}/bullet")

# ---- urdfdom: parse a URDF and check the model ----
port_test(test_urdfdom          test_urdfdom.cpp         urdfdom_model)
target_include_directories(test_urdfdom PRIVATE "${INC}/urdfdom" "${INC}/urdfdom_headers")

# ---- Cartographer: a static library; its CMake package carries the
# link line (Ceres, glog, gflags, protobuf, abseil, Lua, Boost, cairo) ----
find_package(cartographer CONFIG REQUIRED)
port_test(test_cartographer     test_cartographer.cpp    cartographer)

# ---- SIL Kit: two participants, pub/sub through a registry process ----
find_package(SilKit CONFIG REQUIRED)
port_test(test_silkit           test_silkit.cpp          SilKit::SilKit)
# ---- SIL Kit Adapter TAP: a host tapN bridged to a SIL Kit Ethernet network ----
port_test(test_silkit_adapters_tap test_silkit_adapters_tap.cpp SilKit::SilKit socket)

# ---- ioquake3: the dedicated server, headless, on loopback ----
port_test(test_quake3           test_quake3.c            socket)

# ---- Python packages ----
port_script(test_python3_lxml)
port_script(test_python3_netifaces)
