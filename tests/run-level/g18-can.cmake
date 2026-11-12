# Run-level tests, group g18-can: the CAN ecosystem on qnx-socketcan.
# python-can with wrapt; the CAN libraries of Python (cantools, canmatrix,
# can-isotp, udsoncan, can-j1939, pyserial and their helpers); the
# applications (python-OBD with pint, Scapy); isotp-c, libcanard
# (OpenCyphal/CAN), cannelloni and CANopenNode (canopend, cocomm). Each test
# uses the component (known answers, frames on a private qcand bus), not just
# its loading. test_python3_canapps drives Scapy's CANSocket through python-can
# (an optional module of Scapy: named in g18-can.extras). Which test checks which component: g18-can.tsv; programs/data
# a test needs installed: g18-can.extras.

# ---- libraries ----
port_test(test_isotp_c          test_isotp_c.c           isotp)
# The build options of libisotp.a (isotp-c.pc): the structures depend on them.
target_include_directories(test_isotp_c PRIVATE "${INC}/isotp_c")
target_compile_definitions(test_isotp_c PRIVATE ISO_TP_MAX_CAN_FRAME_SIZE=64 ISO_TP_DEFAULT_TX_DL=8
  ISO_TP_FRAME_PADDING ISO_TP_FRAME_PADDING_VALUE=0xAA ISO_TP_USER_SEND_CAN_ARG ISO_TP_USER_SEND_CAN_FLAGS)
port_test(test_libcanard        test_libcanard.c         canard)

# ---- Python ----
port_script(test_python3_can)
port_script(test_python3_canlibs)
port_script(test_python3_canapps)

# ---- programs ----
port_script(test_cannelloni)
port_script(test_canopennode)
