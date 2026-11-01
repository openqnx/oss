# Vector SIL Kit: an open-source library for connecting software-in-the-loop
# simulations -- participants exchange CAN, LIN, FlexRay, Ethernet and
# publish/subscribe or RPC data through a registry, with virtual time
# synchronisation. Upstream supports QNX (it ships QNX toolchain presets).
# Built: libSilKit and the utilities (sil-kit-registry, sil-kit-monitor,
# sil-kit-system-controller). Not built: demos, tests, documentation, the
# dashboard client. asio, fmt, spdlog and the YAML parser are SIL Kit's own
# bundled copies (git submodules), linked into libSilKit and not exported.
# patches/0001: std::filesystem::current_path() throws on QNX when the
# working directory is "/" (pathconf EINVAL); getcwd() instead.
VERSION=5.0.8; GIT=https://github.com/vectorgrp/sil-kit.git; REF=v5.0.8
CMAKE_ARGS_EXTRA=( -DSILKIT_BUILD_DEMOS=OFF -DSILKIT_BUILD_TESTS=OFF -DSILKIT_BUILD_UTILITIES=ON
                   -DSILKIT_BUILD_DOCS=OFF -DSILKIT_BUILD_DASHBOARD=OFF -DSILKIT_BUILD_STATIC=OFF
                   -DSILKIT_USE_SYSTEM_LIBRARIES=OFF -DSILKIT_WARNINGS_AS_ERRORS=OFF
                   -DSILKIT_BUILD_REPRODUCIBLE=ON )
