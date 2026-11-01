# uvgRTP: RTP library (RFC 3550) in C++ with RTCP, and payload formats for
# H.264, H.265, H.266 (VVC) and Opus, from Ultra Video Group.
VERSION=4.0.1; GIT=https://github.com/ultravideo/uvgRTP.git; REF=v4.0.1
BUILDSYS=cmake
# Shared library. SRTP/ZRTP need Crypto++, which is not ported: off. No unit
# tests (they download googletest) and no examples.
CMAKE_ARGS_EXTRA=( -DUVGRTP_BUILD_SHARED=ON -DUVGRTP_DISABLE_CRYPTO=ON -DUVGRTP_DISABLE_TESTS=ON
                   -DUVGRTP_DISABLE_EXAMPLES=ON -DUVGRTP_RELEASE_COMMIT=ON )
