"""
QNX: the CAN_RAW socket of python-can's socketcan interface.

QNX 8.0's network stack has no PF_CAN and Python's socket module no AF_CAN.
CAN sockets come from qnx-socketcan: its library libqsocketcan defines
socket(PF_CAN, SOCK_RAW, CAN_RAW), bind(), setsockopt(), send(), recvmsg()
for them (a CAN socket is a connection to the CAN bus daemon qcand).
:class:`CanSocket` calls that library through ctypes and has the methods of
:class:`socket.socket` that socketcan.py uses, with Linux's arguments and
results: the option numbers of ``constants.py``, ``bind((channel,))``, and
``recvmsg()`` returning the frame, one ``SO_TIMESTAMPNS`` control message
(seconds and nanoseconds as two C longs), the message flags and
``(channel,)``.
"""

import ctypes
import errno
import os
import socket
import struct
import subprocess

from can.interfaces.socketcan import constants

# <linux/can.h>, <linux/socket.h> of qnx-socketcan and <sys/socket.h> of QNX
AF_CAN = 240
_SO_TIMESTAMP = 0x0400  # struct timeval, in a SCM_TIMESTAMP control message
_SCM_TIMESTAMP = 0x02
_MSG_DONTWAIT = 0x80
_CMSG_DATA = 16  # CMSG_LEN(0): struct cmsghdr { socklen_t, int, int } aligned to 8


class _SockaddrCan(ctypes.Structure):  # struct sockaddr_can, 24 bytes
    _fields_ = [
        ("can_family", ctypes.c_uint16),
        ("can_ifindex", ctypes.c_int),
        ("can_addr", ctypes.c_uint8 * 16),
    ]


class _Iovec(ctypes.Structure):
    _fields_ = [("iov_base", ctypes.c_void_p), ("iov_len", ctypes.c_size_t)]


class _Msghdr(ctypes.Structure):  # QNX <sys/socket.h>, 48 bytes
    _fields_ = [
        ("msg_name", ctypes.c_void_p),
        ("msg_namelen", ctypes.c_uint32),
        ("msg_iov", ctypes.POINTER(_Iovec)),
        ("msg_iovlen", ctypes.c_int),
        ("msg_control", ctypes.c_void_p),
        ("msg_controllen", ctypes.c_uint32),
        ("msg_flags", ctypes.c_int),
    ]


_lib = None


def _library():
    global _lib  # pylint: disable=global-statement
    if _lib is None:
        lib = ctypes.CDLL("libqsocketcan.so.1", use_errno=True)
        lib.socket.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
        lib.bind.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint32]
        lib.setsockopt.argtypes = [
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_void_p,
            ctypes.c_uint32,
        ]
        lib.send.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int]
        lib.send.restype = ctypes.c_ssize_t
        lib.sendto.argtypes = [
            ctypes.c_int,
            ctypes.c_void_p,
            ctypes.c_size_t,
            ctypes.c_int,
            ctypes.c_void_p,
            ctypes.c_uint32,
        ]
        lib.sendto.restype = ctypes.c_ssize_t
        lib.recvmsg.argtypes = [ctypes.c_int, ctypes.POINTER(_Msghdr), ctypes.c_int]
        lib.recvmsg.restype = ctypes.c_ssize_t
        lib.close.argtypes = [ctypes.c_int]
        lib.if_nametoindex.argtypes = [ctypes.c_char_p]
        lib.if_nametoindex.restype = ctypes.c_uint
        lib.if_indextoname.argtypes = [ctypes.c_uint, ctypes.c_char_p]
        lib.if_indextoname.restype = ctypes.c_char_p
        _lib = lib
    return _lib


def _oserror() -> OSError:
    code = ctypes.get_errno()
    return OSError(code, os.strerror(code))


class CanSocket:
    """A CAN_RAW socket of libqsocketcan with :class:`socket.socket`'s methods."""

    def __init__(self) -> None:
        self._lib = _library()
        self._fd = self._lib.socket(AF_CAN, socket.SOCK_RAW, constants.CAN_RAW)
        if self._fd < 0:
            raise _oserror()
        self._names: dict[int, str] = {}

    def fileno(self) -> int:
        return self._fd

    def close(self) -> None:
        if self._fd >= 0:
            self._lib.close(self._fd)
            self._fd = -1

    def __del__(self) -> None:
        self.close()

    def _address(self, address) -> _SockaddrCan:
        channel = address[0] if isinstance(address, tuple) else address
        index = 0
        if channel:
            index = self._lib.if_nametoindex(str(channel).encode())
            if not index:
                raise OSError(errno.ENODEV, f"no CAN interface '{channel}' (is qcand running?)")
        return _SockaddrCan(AF_CAN, index)

    def bind(self, address) -> None:
        addr = self._address(address)
        if self._lib.bind(self._fd, ctypes.byref(addr), ctypes.sizeof(addr)) < 0:
            raise _oserror()
        # The library learns a frame's interface, time and "own" flag from
        # qcand for the frames that arrive after the socket's first recvmsg():
        # ask now, before any frame is read.
        self._recvmsg(_MSG_DONTWAIT)

    def setsockopt(self, level: int, optname: int, value) -> None:
        if level == socket.SOL_SOCKET and optname == constants.SO_TIMESTAMPNS:
            optname = _SO_TIMESTAMP
        data = struct.pack("@i", value) if isinstance(value, int) else bytes(value)
        if self._lib.setsockopt(self._fd, level, optname, data, len(data)) < 0:
            raise _oserror()

    def send(self, data: bytes) -> int:
        sent = self._lib.send(self._fd, bytes(data), len(data), 0)
        if sent < 0:
            raise _oserror()
        return sent

    def sendto(self, data: bytes, address) -> int:
        addr = self._address(address)
        sent = self._lib.sendto(
            self._fd, bytes(data), len(data), 0, ctypes.byref(addr), ctypes.sizeof(addr)
        )
        if sent < 0:
            raise _oserror()
        return sent

    def _recvmsg(self, flags: int):
        frame = ctypes.create_string_buffer(constants.CANFD_MTU)
        control = ctypes.create_string_buffer(128)
        addr = _SockaddrCan()
        iov = _Iovec(ctypes.cast(frame, ctypes.c_void_p), len(frame))
        msg = _Msghdr(
            ctypes.cast(ctypes.pointer(addr), ctypes.c_void_p),
            ctypes.sizeof(addr),
            ctypes.pointer(iov),
            1,
            ctypes.cast(control, ctypes.c_void_p),
            len(control),
            0,
        )
        received = self._lib.recvmsg(self._fd, ctypes.byref(msg), flags)
        if received < 0:
            return None
        seconds = nanoseconds = 0
        offset = 0
        while offset + _CMSG_DATA <= msg.msg_controllen:
            length, c_level, c_type = struct.unpack_from("@Iii", control.raw, offset)
            if length < _CMSG_DATA:
                break
            if c_level == socket.SOL_SOCKET and c_type == _SCM_TIMESTAMP:
                seconds, micro = struct.unpack_from("@qi", control.raw, offset + _CMSG_DATA)
                nanoseconds = micro * 1000
                break
            offset += (length + 7) & ~7
        return frame.raw[:received], seconds, nanoseconds, msg.msg_flags, addr.can_ifindex

    def recvmsg(self, bufsize: int, ancbufsize: int = 0, flags: int = 0):
        result = self._recvmsg(flags)
        if result is None:
            raise _oserror()
        frame, seconds, nanoseconds, msg_flags, index = result
        name = self._names.get(index)
        if name is None:
            buffer = ctypes.create_string_buffer(64)
            name = (self._lib.if_indextoname(index, buffer) or b"").decode()
            self._names[index] = name
        ancillary = [
            (
                socket.SOL_SOCKET,
                constants.SO_TIMESTAMPNS,
                struct.pack("@ll", seconds, nanoseconds),
            )
        ]
        return frame[:bufsize], ancillary, msg_flags, (name,)


def find_interfaces() -> list[str]:
    """The names of qcand's CAN interfaces (``qcanctl list``)."""
    try:
        output = subprocess.check_output(
            ["qcanctl", "list"], text=True, stderr=subprocess.DEVNULL
        )
    except Exception:  # pylint: disable=broad-except
        return []
    return [line.split()[0] for line in output.splitlines() if line.split()]
