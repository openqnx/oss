# Run-level tests, group n2-sys: system components -- daemon and utmp
# helpers, mDNS/DNS-SD, printing, terminals, archivers and compressors,
# documentation tools, system information. Each test uses the component
# with known answers. Which test checks which component: n2-sys.tsv;
# programs/data a shell test needs installed: n2-sys.extras. The daemon
# tests (avahi, cups) run private instances on loopback and stop them on
# every exit path.
port_test(test_libdaemon      test_libdaemon.c      daemon)
port_test(test_skalibs        test_skalibs.c        skarnet)
port_test(test_libutempter    test_libutempter.c    utempter)
port_test(test_c_blosc        test_c_blosc.c        blosc)
port_script(test_avahi)
port_script(test_cups)
port_script(test_7zip)
port_script(test_xmlto)
port_script(test_asciidoc)
port_script(test_xsltproc)
port_script(test_libxml2_utils)
port_script(test_perl_io_tty)
port_script(test_fastfetch)
