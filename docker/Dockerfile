# Build environment for openqnx/oss -- everything except the QNX SDP itself.
#
# The SDP is licensed and is never copied into the image; docker/run.sh mounts
# the host's installation (and the QNX license in ~/.qnx) read-only at run time.
# The image pins the host tools whose versions matter to the cross builds:
#
#   Meson >= 1.3   glib needs >= 1.2, pixman >= 1.3 (Ubuntu ships older)
#   CMake 4.x      what every recipe was verified with; 3.x fails x265
#
#   docker build -t openqnx/oss-build docker/
#   docker/run.sh                         # shell in the container
#   docker/run.sh ./scripts/qnxbuild zlib # or run one command
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential git curl ca-certificates pkg-config file patch xz-utils \
      bzip2 unzip zstd rsync autoconf automake autopoint libtool gettext \
      bison flex gperf texinfo help2man nasm perl python3 python3-pip \
      python3-venv python3-jinja2 python3-yaml python3-mako python3-packaging \
      libffi-dev libssl-dev zlib1g-dev libexpat1-dev libxml2-utils xsltproc \
      openssh-client sudo locales \
    && rm -rf /var/lib/apt/lists/* \
    && locale-gen en_US.UTF-8

# Pinned Meson / CMake / Ninja, in their own venv so they cannot fight the
# distribution's python packages.
RUN python3 -m venv /opt/hosttools-venv \
    && /opt/hosttools-venv/bin/pip install --no-cache-dir \
         "meson==1.6.1" "cmake==4.0.3" "ninja==1.11.1.4"
ENV PATH=/opt/hosttools-venv/bin:$PATH \
    LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
    QNX_SDP_ROOT=/opt/qnx800

# run.sh passes the caller's uid/gid so files written to the bind-mounted
# workspace stay owned by the user who ran it.
ARG UID=1000
ARG GID=1000
RUN (getent group $GID || groupadd -g $GID builder) \
    && (id -u $UID >/dev/null 2>&1 || useradd -m -u $UID -g $GID -s /bin/bash builder) \
    && echo "ALL ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/builder
WORKDIR /work
CMD ["/bin/bash"]
