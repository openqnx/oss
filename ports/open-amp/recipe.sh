# qnx-ports/open-amp's qnx-v2024.10.0 branch is identical to upstream v2024.10.0:
# it builds for QNX unmodified.
VERSION=2024.10.0; GIT=https://github.com/OpenAMP/open-amp.git; REF=v2024.10.0
DEPS="libmetal"
CMAKE_ARGS_EXTRA=( -DWITH_APPS=OFF -DWITH_PROXY=OFF )
