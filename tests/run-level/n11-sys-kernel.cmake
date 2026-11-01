# Run-level tests, group n11-sys-kernel: kernel-facing system components --
# netmap (io-sock's built-in netmap: VALE switches and netmap pipes, no NIC
# needed) and the CAN tools that work without SocketCAN (QNX has none).
# Each test uses the component with known answers. Which test checks which
# component: n11-sys-kernel.tsv; programs a shell test needs installed:
# n11-sys-kernel.extras. Every process a test starts is stopped on every
# exit path.
port_script(test_netmap)
port_script(test_can_utils)
