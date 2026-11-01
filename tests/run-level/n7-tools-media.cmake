# Run-level tests, group n7-tools-media: file system tools, relays, printing
# filters, GnuPG's client library, VPN control and media libraries
# (e2fsprogs, socat, sshpass, qpdf, cups-filters, ippusbxd, gpgme, davici,
# libhevc, uvgRTP). Each test uses the component (known answers), not just
# its loading. Which test checks which component: n7-tools-media.tsv;
# programs/data a test needs installed: n7-tools-media.extras.

# ---- tools ----
port_test(test_e2fsprogs        test_e2fsprogs.c         ext2fs ss com_err)
port_script(test_socat)
port_script(test_sshpass)

# ---- printing ----
port_test(test_qpdf             test_qpdf.cpp            qpdf)
# the qpdf 11 API without the old PointerHolder (a header warning otherwise)
target_compile_definitions(test_qpdf PRIVATE POINTERHOLDER_TRANSITION=4)
port_script(test_cups_filters)
port_script(test_ippusbxd)

# ---- GnuPG, strongSwan ----
port_test(test_gpgme            test_gpgme.c             gpgme assuan gpg-error)
port_test(test_davici           test_davici.c            davici)

# ---- media ----
# libhevc is a static library (as on Android): link the decoder archive.
port_test(test_libhevc          test_libhevc.c           "${LIB}/libhevcdec.a")
port_test(test_uvgrtp           test_uvgrtp.cpp          uvgrtp)
