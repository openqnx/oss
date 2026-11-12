# python-can: CAN bus access for Python -- can.Bus, Message, Notifier,
# log readers and writers (ASC, BLF, candump log, CSV, MF4, TRC, SQLite),
# can.logger / can.player / can.viewer.
#
# The "socketcan" interface works on QNX: QNX 8.0 has no PF_CAN and Python's
# socket module no AF_CAN, so on QNX the interface's socket is
# can/interfaces/socketcan/qnx.py (files/qnx.py), a socket object on
# qnx-socketcan's libqsocketcan through ctypes (the buses are qcand's:
# `qcand`, then can.Bus("vcan0", interface="socketcan")). Filters, CAN FD,
# loopback, receive_own_messages and receive timestamps work as on Linux;
# cyclic sending (send_periodic) uses python-can's thread-based task, as
# there is no CAN_BCM. Other interfaces that are pure Python work as they
# are: virtual, udp_multicast, socketcand, serial and slcan (python3-pyserial).
# Built from the source distribution into a platform-independent wheel.
VERSION=4.6.1
TARBALL=https://files.pythonhosted.org/packages/74/f9/a9d99d36dd33be5badb747801c9255c3c526171a5542092eaacc73350fb8/python_can-4.6.1.tar.gz
SHA256=290fea135d04b8504ebff33889cc6d301e2181a54099116609f940825ffe5005
DEPS="python3 python3-wrapt python3-packaging python3-typing-extensions qnx-socketcan"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-wrapt python3-packaging python3-typing-extensions qnx-socketcan"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77" "setuptools_scm>=8" wheel; }
do_build() {
  cp "$RECIPE_DIR/python3-can/files/qnx.py" "$S/can/interfaces/socketcan/qnx.py" || return 1
  rm -rf "$B/wheel" && mkdir -p "$B/wheel" && \
  env -u CC -u CXX -u CFLAGS -u LDFLAGS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
    "$B/venv/bin/python" -m pip wheel --no-deps --no-build-isolation -q -w "$B/wheel" "$S" || return 1
  ls "$B"/wheel/*-none-any.whl >/dev/null 2>&1 || { echo "not a pure wheel: $(ls "$B/wheel")"; return 1; }
}
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
