# Run-level tests, group n1-net: network and security components (gnupg and
# its libraries, nspr/nss, boringssl, nginx, dhcpcd, lldpd, mosh, ZeroTierOne,
# iperf2, ntttcp). Each test uses the component (known answers), not just its
# loading. Which test checks which component: n1-net.tsv; programs/data a test
# needs installed: n1-net.extras.

# ---- gnupg and its libraries ----
port_test(test_npth             test_npth.c              npth)
port_test(test_libassuan        test_libassuan.c         assuan gpg-error)
port_test(test_libksba          test_libksba.c           ksba gpg-error)
port_script(test_gnupg)

# ---- mosh, ZeroTier One ----
port_test(test_mosh             test_mosh.c)
port_script(test_zerotier_one)

# ---- boringssl, nginx ----
# BoringSSL in its own prefix (its libcrypto/libssl/<openssl/*.h> would clash
# with OpenSSL's): headers first, libraries by path, so OpenSSL cannot stand in.
port_test(test_boringssl        test_boringssl.c         "${STAGE}/opt/boringssl/lib/libssl.so" "${STAGE}/opt/boringssl/lib/libcrypto.so")
target_include_directories(test_boringssl BEFORE PRIVATE "${STAGE}/opt/boringssl/include")
target_link_options(test_boringssl PRIVATE "-Wl,-rpath-link,${STAGE}/opt/boringssl/lib")
# nginx: the server on loopback; the test makes its TLS certificate with OpenSSL.
port_test(test_nginx            test_nginx.c             ssl crypto)

# ---- dhcpcd, lldpd ----
port_script(test_dhcpcd)
port_script(test_lldpd)

# ---- nspr, nss ----
port_test(test_nspr             test_nspr.c              plds4 plc4 nspr4)
target_include_directories(test_nspr PRIVATE "${INC}/nspr")
# test_nss also runs certutil; NSS dlopens libsoftokn3/libfreebl3 (extras).
port_test(test_nss              test_nss.c               ssl3 smime3 nss3 nssutil3 plds4 plc4 nspr4)
target_include_directories(test_nss PRIVATE "${INC}/nss" "${INC}/nspr")

# ---- iperf2, ntttcp-for-linux ----
port_script(test_iperf2)
port_script(test_ntttcp_for_linux)
