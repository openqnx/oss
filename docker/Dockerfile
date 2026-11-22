# Build environment for openqnx/oss -- everything except the QNX SDP itself.
#
# The SDP is licensed and is never copied into the image; docker/run.sh mounts
# the host's installation (and the QNX license in ~/.qnx) read-only at run time.
# The image holds every host package the recipes use (the complete rebuild of
# all components for SDP 8.0.6 was made in it) and pins the host tools whose
# versions matter to the cross builds:
#
#   Meson 1.3.2    glib needs >= 1.2, pixman >= 1.3 (Ubuntu ships older)
#   CMake 3.28.3   CMake 4 rejects CMAKE_POLICY(... OLD), which older
#                  projects still use (acado)
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

# What single recipes run or link natively on the build machine: compilers
# and libraries for their host-side helpers (clang/LLVM 18, 32-bit libc,
# GLib, Cairo, Pango, SDL2, GnuTLS, ...), documentation and data tools
# (itstool, xmlto, docbook-xsl, sassc, appstreamcli, gpg, mkfontscale,
# gdk-pixbuf-pixdata, pnp.ids from hwdata), Python and Perl modules.
RUN apt-get update && apt-get install -y --no-install-recommends \
      clang-18 libclang-18-dev llvm-18-dev lld-18 gawk chrpath cpio diffstat jq wget zip lz4 \
      lib32z1-dev libc6-dev-i386 libglib2.0-dev libglib2.0-bin libcairo2-dev libpango1.0-dev \
      libepoxy-dev libpixman-1-dev libsdl2-dev libmagic-dev libncurses-dev libxml2-dev \
      python3-full python3-sphinx python3-git python3-pexpect luajit dbus itstool \
      gobject-introspection libgirepository1.0-dev glslang-tools default-jdk-headless \
      ruby sassc desktop-file-utils shared-mime-info gtk-update-icon-cache xmlto docbook-xsl \
      subversion ccache time bc procps less vim-tiny \
      libgdk-pixbuf2.0-bin xfonts-utils libltdl-dev libgtk-3-bin x11-xkb-utils python3-dev python3-gi \
      python3-pil python3-jsonschema python3-attr python3-six python3-dateutil python3-typing-extensions \
      ninja-build gnupg appstream libxml-parser-perl hwdata libzstd-dev libgnutls28-dev \
    && rm -rf /var/lib/apt/lists/*

# Pinned Meson / CMake / Ninja, in their own venv so they cannot fight the
# distribution's python packages.
RUN python3 -m venv /opt/hosttools-venv \
    && /opt/hosttools-venv/bin/pip install --no-cache-dir \
         "meson==1.3.2" "cmake==3.28.3" "ninja==1.11.1.4"
# Newer Meson / CMake / Ninja beside them, for the components whose release
# needs one (recipe: HOSTTOOLS_NEW=1, scripts/qnxbuild). Not on PATH: the
# pinned ones above stay the default for everything else.
RUN python3 -m venv /opt/hosttools-new \
    && /opt/hosttools-new/bin/pip install --no-cache-dir \
         "meson==1.12.1" "cmake==3.31.10" "ninja==1.13.2"
ENV PATH=/opt/hosttools-venv/bin:$PATH \
    LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
    QNX_SDP_ROOT=/opt/qnx800

# dbus-binding-tool (dbus-glib's code generator) runs on the build machine
# while building parole: the build host's, as glib-compile-resources.
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      libdbus-glib-1-dev-bin \
    && rm -rf /var/lib/apt/lists/*

# run.sh passes the caller's uid/gid so files written to the bind-mounted
# workspace stay owned by the user who ran it.
ARG UID=1000
ARG GID=1000
RUN (getent group $GID || groupadd -g $GID builder) \
    && (id -u $UID >/dev/null 2>&1 || useradd -m -u $UID -g $GID -s /bin/bash builder) \
    && echo "ALL ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/builder
WORKDIR /work
CMD ["/bin/bash"]
