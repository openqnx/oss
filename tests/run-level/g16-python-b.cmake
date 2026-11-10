# Run-level tests, group g16-python-b: Python packages -- flake8 with its
# checkers and plugins, pure-Python libraries (absl-py, gast, beniget,
# bracex, cppy, fs, opt_einsum, py, distro), the ROS tool modules (vcstool,
# rosdistro, rospkg), the build tools scons, gyp and pythran, and the C
# extensions pycairo, PyGObject (with glib-typelibs), dbus-python and pydbus,
# protobuf (upb), sounddevice (PortAudio) and SciPy. Known answers
# throughout. Which test checks which component: g16-python-b.tsv; what a
# test needs installed: g16-python-b.extras.
port_script(test_python3_flake8)
port_script(test_python3_pylibs_b)
port_script(test_python3_ros_tools)
port_script(test_python3_buildtools)
port_script(test_python3_cairo)
port_script(test_python3_gobject3)
port_script(test_python3_dbus)
port_script(test_python3_protobuf)
port_script(test_python3_sounddevice)
port_script(test_python3_scipy)
