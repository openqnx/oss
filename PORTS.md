# QNX SDP 8.0 Automotive OSS Port Status

Targets **aarch64le** + **x86_64** · SDP `$QNX_SDP_ROOT` (QNX 8.0, gcc 12.2.0)
Everything is built from upstream source; no prebuilt SDP or `repo.oss.qnx.com`
binary is reused. Run `./scripts/qnxbuild -l` for live status.

**NEW** = absent from both github.com/qnx-ports and the repo.oss.qnx.com APK index
(the gaps this workspace adds). **REBUILD** = an upstream recipe exists elsewhere;
rebuilt here from source against this SDP.


## Added in openqnx/oss — 48 packages, both architectures

Built and tested on QEMU `x86_64` and `aarch64le` targets created with `mkqnximage`.

### Industrial & IoT protocols
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [open62541](ports/open62541/) | 1.4.8 | **NEW** | – | OPC UA client/server (IEC 62541) |
| [libmodbus](ports/libmodbus/) | 3.1.11 | **NEW** | – | Modbus RTU/TCP |
| [mosquitto](ports/mosquitto/) | 2.0.20 | REBUILD | – | MQTT broker + client library |
| [libcoap](ports/libcoap/) | 4.3.5 | **NEW** | 1 | CoAP (RFC 7252) with DTLS |
| [libzmq](ports/libzmq/) | 4.3.5 | REBUILD | – | ZeroMQ messaging, CURVE security |
| [cppzmq](ports/cppzmq/) | 4.10.0 | **NEW** | – | C++ binding for ZeroMQ |
| [nng](ports/nng/) | 1.9.0 | **NEW** | 1 | nanomsg-next-gen scalability protocols |

### Web, remote access & version control
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [civetweb](ports/civetweb/) | 1.16 | **NEW** | 1 | embedded HTTP(S)/WebSocket server |
| [libmicrohttpd](ports/libmicrohttpd/) | 1.0.1 | **NEW** | – | embedded HTTP server (GNU) |
| [libssh](ports/libssh/) | 0.11.1 | **NEW** | 1 | SSH client/server library |
| [libgit2](ports/libgit2/) | 1.8.4 | **NEW** | 1 | Git implementation (OTA/config repos) |
| [uriparser](ports/uriparser/) | 0.9.8 | **NEW** | – | RFC 3986 URI parsing |

### Data formats & serialisation
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [flatbuffers](ports/flatbuffers/) | 24.3.25 | REBUILD | – | zero-copy serialisation + flatc |
| [protobuf-c](ports/protobuf-c/) | 1.5.0 | REBUILD | – | Protocol Buffers for C |
| [libcbor](ports/libcbor/) | 0.11.0 | **NEW** | – | CBOR (RFC 8949) |
| [json-c](ports/json-c/) | 0.18 | **NEW** | – | JSON for C |
| [rapidjson](ports/rapidjson/) | 1.1.0+ | REBUILD | – | fast JSON (header-only) |
| [libyaml](ports/libyaml/) | 0.2.5 | REBUILD | – | YAML for C |
| [pugixml](ports/pugixml/) | 1.14 | REBUILD | – | XML DOM + XPath |
| [tomlplusplus](ports/tomlplusplus/) | 3.4.0 | **NEW** | – | TOML (header-only) |
| [cereal](ports/cereal/) | 1.3.2 | **NEW** | – | C++ serialisation (header-only) |

### Compression & storage
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [libdeflate](ports/libdeflate/) | 1.22 | **NEW** | – | fast DEFLATE/zlib/gzip |
| [snappy](ports/snappy/) | 1.2.1 | **NEW** | – | fast compression |
| [lzo](ports/lzo/) | 2.10 | **NEW** | – | LZO real-time compression |
| [libzip](ports/libzip/) | 1.11.1 | **NEW** | – | ZIP archives (zstd/xz/bzip2, AES) |
| [leveldb](ports/leveldb/) | 1.23 | **NEW** | – | embedded key-value store |

### C++ utilities & test frameworks
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [re2](ports/re2/) | 2024-07-02 | REBUILD | – | linear-time regular expressions |
| [double-conversion](ports/double-conversion/) | 3.3.0 | **NEW** | – | fast float<->string |
| [cli11](ports/cli11/) | 2.4.2 | **NEW** | – | command-line parsing (header-only) |
| [magic_enum](ports/magic_enum/) | 0.9.6 | **NEW** | – | enum reflection (header-only) |
| [glm](ports/glm/) | 1.0.1 | **NEW** | – | vector/matrix math (header-only) |
| [benchmark](ports/benchmark/) | 1.9.0 | REBUILD | – | Google Benchmark |
| [catch2](ports/catch2/) | 3.7.1 | REBUILD | – | unit test framework |

### Robotics
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [orocos-kdl](ports/orocos-kdl/) | 1.5.1 | **NEW** | – | kinematics and dynamics |
| [octomap](ports/octomap/) | 1.10.0 | **NEW** | – | 3-D occupancy mapping |
| [fcl](ports/fcl/) | 0.7.0 | **NEW** | – | collision and distance queries |
| [libccd](ports/libccd/) | 2.1 | **NEW** | – | GJK/EPA collision detection |
| [console_bridge](ports/console_bridge/) | 1.0.2 | **NEW** | – | ROS logging bridge |
| [urdfdom_headers](ports/urdfdom_headers/) | 1.1.2 | **NEW** | – | URDF model headers |

### Scripting & audio
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [quickjs-ng](ports/quickjs-ng/) | 0.8.0 | **NEW** | 1 | embeddable JavaScript engine (ES2023) |
| [speexdsp](ports/speexdsp/) | 1.2.1 | **NEW** | – | resampler, echo cancel, preprocessing |
| [mpg123](ports/mpg123/) | 1.32.9 | **NEW** | – | MP3 decoder library |
| [taglib](ports/taglib/) | 2.0.2 | **NEW** | – | audio metadata (ID3, Vorbis, MP4) |

### Shell & command-line tools
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| [bash](ports/bash/) | 5.2.37 | REBUILD | 1 | GNU Bourne-Again shell |
| [tmux](ports/tmux/) | 3.5a | REBUILD | 1 | terminal multiplexer |
| [nano](ports/nano/) | 8.2 | **NEW** | – | text editor |
| [less](ports/less/) | 668 | REBUILD | – | pager |
| [rsync](ports/rsync/) | 3.3.0 | **NEW** | – | file synchronisation |


## Added to match qnx-ports/build-files

Components from the [qnx-ports/build-files](https://github.com/qnx-ports/build-files) list that were not here. Where qnx-ports keeps its QNX changes in a fork, the fork's QNX commits are imported as patch files against the upstream release (`*-qnx-from-qnx-ports.patch`), so every port is still *upstream + separate patches*. Entries of that list already covered under another name: abseil-cpp → abseil, foonathan_memory_vendor/memory → foonathan_memory, gettext-runtime → gettext, graphite → graphite2, gtk → gtk3/gtk4, SDL → sdl2/sdl3, SDL_ttf → sdl2-ttf, numpy2 → numpy; `epoll` is provided by the QNX 8.0 SDP itself (libepoll).

### Cloud, IoT and networking
| Package | Version | Patches (of which from qnx-ports/aports) |
|---|---|---|
| [aws-c-common](ports/aws-c-common/) | 0.12.6 | 2 (2) |
| [aws-c-cal](ports/aws-c-cal/) | 0.9.13 | – |
| [aws-c-io](ports/aws-c-io/) | 0.26.1 | 2 (2) |
| [aws-checksums](ports/aws-checksums/) | 0.2.10 | – |
| [aws-c-compression](ports/aws-c-compression/) | 0.3.2 | – |
| [aws-c-http](ports/aws-c-http/) | 0.10.14 | – |
| [aws-c-sdkutils](ports/aws-c-sdkutils/) | 0.2.4 | – |
| [aws-c-auth](ports/aws-c-auth/) | 0.10.1 | – |
| [aws-c-mqtt](ports/aws-c-mqtt/) | 0.15.2 | – |
| [aws-c-event-stream](ports/aws-c-event-stream/) | 0.7.0 | – |
| [aws-c-s3](ports/aws-c-s3/) | 0.12.2 | – |
| [s2n-tls](ports/s2n-tls/) | 1.7.0 | 2 (2) |
| [aws-crt-cpp](ports/aws-crt-cpp/) | 0.38.5 | 2 (2) |
| [azure-iot-sdk-c](ports/azure-iot-sdk-c/) | LTS_08_2024 | 4 (4) |
| [amqp-cpp](ports/amqp-cpp/) | 4.3.27 | 2 |
| [asio](ports/asio/) | 1.30.2 | 2 |
| [restbed](ports/restbed/) | 4.8 | 2 |
| [lighttpd](ports/lighttpd/) | 1.4.77 | – |
| [dnsmasq](ports/dnsmasq/) | 2.93test4 | 2 (2) |
| [unbound](ports/unbound/) | 1.24.1 | 2 (2) |
| [strongswan](ports/strongswan/) | 6.0.6 | 4 (2) |
| [chrony](ports/chrony/) | 4.8-git-20260226 | 4 (2) |
| [ntpsec](ports/ntpsec/) | 1.2.4 | 2 (2) |
| [sscep](ports/sscep/) | 0.10.0 | – |
| [tftp-hpa](ports/tftp-hpa/) | 5.2 | 2 |

### Embedded, automotive and SDR
| Package | Version | Patches (of which from qnx-ports/aports) |
|---|---|---|
| [candb](ports/candb/) | 0.1-git-20210625 | 4 (2) |
| [libmetal](ports/libmetal/) | 2024.10.0 | 2 (2) |
| [open-amp](ports/open-amp/) | 2024.10.0 | 2 (2) |
| [rtl-sdr](ports/rtl-sdr/) | 2.0.2 | – |
| [dump1090](ports/dump1090/) | 0-git-20240121 | 2 (2) |
| [minicom](ports/minicom/) | 2.9 | – |
| [picocom](ports/picocom/) | 3.1 | 2 |
| [libcamera](ports/libcamera/) | 0.5.1+rpt20250722 | 2 (2) |

### Math, ML and compute
| Package | Version | Patches (of which from qnx-ports/aports) |
|---|---|---|
| [openblas](ports/openblas/) | 0.3.28 | 4 (2) |
| [clapack](ports/clapack/) | 3.2.1 | 2 (2) |
| [suitesparse](ports/suitesparse/) | 7.11.0 | 2 (2) |
| [metis](ports/metis/) | 5.2.1 | 2 (2) |
| [gklib](ports/gklib/) | 5.1.1-git-20230326 | 2 (2) |
| [sleef](ports/sleef/) | 3.8-git-20250214 | 2 (2) |
| [cpuinfo](ports/cpuinfo/) | 0-git-20240417 | 2 (2) |
| [pthreadpool](ports/pthreadpool/) | 0-git-20250207 | – |
| [fxdiv](ports/fxdiv/) | 0-git-20200417 | – |
| [fp16](ports/fp16/) | 0-git-20260620 | – |
| [ruy](ports/ruy/) | 0-git-20240321 | 2 (2) |
| [xnnpack](ports/xnnpack/) | 0-git-20250209 | 2 (2) |
| [onnx](ports/onnx/) | 1.17.0 | – |
| [gtsam](ports/gtsam/) | 4.2 | – |
| [onetbb](ports/onetbb/) | 2022.0.0 | – |
| [computelibrary](ports/computelibrary/) | 53.0.0 | 2 (2) |
| [farmhash](ports/farmhash/) | 1.1-git-20190513 | 2 (2) |
| [muslflt](ports/muslflt/) | 1.0-git-20240802 | – |
| [kineto](ports/kineto/) | 0-git-20250214 | 2 (2) |
| [tensorflow-lite](ports/tensorflow-lite/) | 2.16.1 | 14 (14) |
| [pytorch](ports/pytorch/) | 2.3.1 | 11 (11) |
| [numpy](ports/numpy/) | 2.3.4 | 2 (2) |
| [pandas](ports/pandas/) | 2.3.3 | – |

### Languages, toolchains and tools
| Package | Version | Patches (of which from qnx-ports/aports) |
|---|---|---|
| [rust](ports/rust/) | 1.98.0 | – |
| [perl](ports/perl/) | 5.40.2 | 2 |
| [capnproto](ports/capnproto/) | 1.0.2 | 2 (2) |
| [check](ports/check/) | 0.15.2 | – |
| [cunit](ports/cunit/) | 2.1-3 | 2 |
| [csmith](ports/csmith/) | 2.3.0 | – |
| [g3log](ports/g3log/) | 2.4 | 2 |
| [trompeloeil](ports/trompeloeil/) | 48 | – |
| [sqlite_orm](ports/sqlite_orm/) | 1.9 | – |
| [libiconv](ports/libiconv/) | 1.18 | – |
| [libjson](ports/libjson/) | 0.8-git-20260512 | – |
| [nanosvg](ports/nanosvg/) | 0-git-20260709 | – |
| [lmbench](ports/lmbench/) | 3.0-git-20260929 | 2 |
| [sysbench](ports/sysbench/) | 1.0.20-git-20250309 | 2 (2) |
| [tree](ports/tree/) | 2.2.1 | – |
| [vim](ports/vim/) | 9.1.0900 | – |

### Graphics, media and games
| Package | Version | Patches (of which from qnx-ports/aports) |
|---|---|---|
| [freeimage](ports/freeimage/) | 3.18.0 | – |
| [openal-soft](ports/openal-soft/) | 1.24.1 | – |
| [vlc](ports/vlc/) | 3.0.21 | 2 (2) |
| [mame](ports/mame/) | 0.281 | 2 (2) |
| [retroarch](ports/retroarch/) | 1.15-git-20230408 | 2 (2) |
| [emulationstation](ports/emulationstation/) | 2.11-git-20241212 | 2 (2) |
| [openttd](ports/openttd/) | 15.0-git-20250501 | 2 (2) |
| [libretro-2048](ports/libretro-2048/) | 0-git-20241227 | – |
| [libretro-samples](ports/libretro-samples/) | 0-git-20221110 | – |
| [libretro-mrboom](ports/libretro-mrboom/) | 0-git-20260625 | 2 (2) |
| [libretro-retro8](ports/libretro-retro8/) | 0-git-20260912 | 2 (2) |
| [libretro-fake08](ports/libretro-fake08/) | 0-git-20260613 | – |
| [retropie](ports/retropie/) | 0.3 | – |
| [qt6](ports/qt6/) | 6.8.1 | – |
| [webrtc](ports/webrtc/) | m132-afaf497 | 60 (60) |
| [pdfium](ports/pdfium/) | 0-git-84a8011 | 30 (30) |

### Robotics
| Package | Version | Patches (of which from qnx-ports/aports) |
|---|---|---|
| [ros2](ports/ros2/) | jazzy | 34 (31) |

## Added from the QNX Everywhere 8.0.4r1 QEMU image

Open source in the QNX Everywhere 8.0.4r1 QEMU image (by APK origin) that was not here. Each is built from its upstream release at the image's version (exceptions are noted in the recipe), with the QNX changes from QNX's own packaging, [qnx-ports/aports](https://github.com/qnx-ports/aports), imported as `*-aports-*.patch` files where it has any. Four more from that image are already here under another name: libsoup3 → [libsoup](ports/libsoup/), webkit2gtk-4.1 → [webkitgtk](ports/webkitgtk/), inih → [libinih](ports/libinih/), tiff → [libtiff](ports/libtiff/).

### Build and development tools
| Package | Target ver | License | Upstream |
|---|---|---|---|
| [abuild](ports/abuild/) | 3.15.0 | GPL-2.0-only | https://gitlab.alpinelinux.org/alpine/abuild/ |
| [apk-tools](ports/apk-tools/) | 3.0.0_rc5 | GPL-2.0-only | https://gitlab.alpinelinux.org/alpine/apk-tools |
| [autoconf](ports/autoconf/) | 2.72 | GPL-3.0-or-later WITH Autoconf-exception-3.0 | https://www.gnu.org/software/autoconf |
| [automake](ports/automake/) | 1.18 | GPL-2.0-or-later | https://www.gnu.org/software/automake |
| [binutils](ports/binutils/) | 2.43 | GPL-2.0-or-later AND LGPL-2.1-or-later AND BSD-3-Clause | https://www.gnu.org/software/binutils/ |
| [bc](ports/bc/) | 1.08.2 | GPL-3.0-or-later | https://www.gnu.org/software/bc/bc.html |
| [busybox](ports/busybox/) | 1.36.1 | GPL-2.0-only | https://www.busybox.net |
| [cmake](ports/cmake/) | 4.1.1 | BSD-3-Clause | https://www.cmake.org/ |
| [fakeroot](ports/fakeroot/) | 1.37.1.1 | GPL-3.0-or-later | https://packages.debian.org/fakeroot |
| [file](ports/file/) | 5.46 | BSD-2-Clause | https://www.darwinsys.com/file/ |
| [gawk](ports/gawk/) | 5.3.2 | GPL-3.0-or-later | https://www.gnu.org/software/gawk/gawk.html |
| [gdb](ports/gdb/) | 14.2 | GPL-3.0-or-later AND LGPL-3.0-or-later | https://www.gnu.org/software/gdb/ |
| [git](ports/git/) | 2.51.0 | GPL-2.0-only | https://www.git-scm.com/ |
| [gperf](ports/gperf/) | 3.1 | GPL-3.0-or-later | https://www.gnu.org/software/gperf/ |
| [grep](ports/grep/) | 3.11 | GPL-3.0-or-later | https://www.gnu.org/software/grep/grep.html |
| [groff](ports/groff/) | 1.23.0 | GPL-3.0-or-later | https://www.gnu.org/software/groff/groff.html |
| [itstool](ports/itstool/) | 2.0.7 | GPL-3.0-or-later | https://itstool.org/ |
| [llvm](ports/llvm/) | 21.1.3 | Apache-2.0 | https://llvm.org/ |
| [m4](ports/m4/) | 1.4.19 | GPL-3.0-or-later | https://www.gnu.org/software/m4 |
| [make](ports/make/) | 4.4.1 | GPL-3.0-or-later | https://www.gnu.org/software/make |
| [meson](ports/meson/) | 1.10.0 | Apache-2.0 | http://mesonbuild.com/ |
| [ninja](ports/ninja/) | 1.13.1 | Apache-2.0 | https://ninja-build.org/ |
| [pax-utils](ports/pax-utils/) | 1.3.8 | GPL-2.0-only | https://wiki.gentoo.org/wiki/Hardened/PaX_Utilities |
| [perl](ports/perl/) | 5.40.2 | GPL-3.0-or-later | https://dev.perl.org/ |
| [perl-error](ports/perl-error/) | 0.17030 | GPL-1.0-or-later OR Artistic-1.0-Perl | https://metacpan.org/release/Error |
| [tar](ports/tar/) | 1.35 | GPL-3.0-or-later | https://www.gnu.org/software/tar/ |
| [man-db](ports/man-db/) | 2.13.1 | GPL-2.0-or-later | https://man-db.gitlab.io/man-db/ |
| [libpipeline](ports/libpipeline/) | 1.5.8 | GPL-3.0-or-later | https://libpipeline.gitlab.io/libpipeline/ |
| [zsh](ports/zsh/) | 5.9 | MIT-Modern-Variant AND GPL-2.0-only | https://www.zsh.org/ |

### Run-time libraries and languages
| Package | Target ver | License | Upstream |
|---|---|---|---|
| [ada](ports/ada/) | 3.4.4 | ( Apache-2.0 OR MIT ) AND MPL-2.0 | https://ada-url.github.io/ada |
| [libxcrypt](ports/libxcrypt/) | 4.4.38 | LGPL-2.1 | https://github.com/besser82/libxcrypt |
| [pcre](ports/pcre/) | 8.45 | BSD-3-Clause | https://www.pcre.org/ |
| [zlib-ng](ports/zlib-ng/) | 2.2.5 | Zlib | https://zlib.net/ |
| [sdl2-compat](ports/sdl2-compat/) | 2.32.60 | Zlib | https://libsdl.org/ |
| [lua5.1](ports/lua5.1/) | 5.1.5 | MIT | https://www.lua.org/ |
| [lua5.3](ports/lua5.3/) | 5.3.6 | MIT | https://www.lua.org/ |
| [lua-lpeg](ports/lua-lpeg/) | 1.1.0 | MIT | http://www.inf.puc-rio.br/~roberto/lpeg |
| [nodejs](ports/nodejs/) | 24.14.1 | MIT | https://nodejs.org/ |
| [npm](ports/npm/) | 11.11.0 | Artistic-2.0 | https://www.npmjs.com/ |
| [python3-packaging](ports/python3-packaging/) | 25.0 | Apache-2.0 AND BSD-2-Clause | https://pypi.org/project/packaging |
| [python3-pip](ports/python3-pip/) | 25.1.1 | MIT | https://pip.pypa.io |
| [python3-setuptools](ports/python3-setuptools/) | 80.8.0 | MIT | https://pypi.org/project/setuptools |
| [python3-wheel](ports/python3-wheel/) | 0.46.1 | MIT | https://github.com/pypa/wheel |
| [openssh](ports/openssh/) | 10.2_p1 | SSH-OpenSSH | https://www.openssh.com/portable.html |
| [sudo](ports/sudo/) | 1.9.17_p2 | LicenseRef-ISC | https://www.sudo.ws |

### Editors
| Package | Target ver | License | Upstream |
|---|---|---|---|
| [neovim](ports/neovim/) | 0.11.4 | Apache-2.0 AND Vim | https://neovim.io/ |
| [emacs](ports/emacs/) | 30.2 | GPL-3.0-or-later | https://www.gnu.org/software/emacs/emacs.html |
| [tree-sitter-c](ports/tree-sitter-c/) | 0.24.1 | MIT | https://github.com/tree-sitter/tree-sitter-c |
| [tree-sitter-lua](ports/tree-sitter-lua/) | 0.4.0 | MIT | https://github.com/tree-sitter-grammars/tree-sitter-lua |
| [tree-sitter-markdown](ports/tree-sitter-markdown/) | 0.5.1 | MIT | https://github.com/tree-sitter-grammars/tree-sitter-markdown |
| [tree-sitter-query](ports/tree-sitter-query/) | 0.7.0 | Apache-2.0 | https://github.com/tree-sitter-grammars/tree-sitter-query/ |
| [tree-sitter-vim](ports/tree-sitter-vim/) | 0.7.0 | MIT | https://github.com/tree-sitter-grammars/tree-sitter-vim/ |
| [tree-sitter-vimdoc](ports/tree-sitter-vimdoc/) | 4.0.0 | Apache-2.0 | https://github.com/neovim/tree-sitter-vimdoc/ |

### Desktop and GUI
| Package | Target ver | License | Upstream |
|---|---|---|---|
| [epiphany](ports/epiphany/) | 43.1 | GPL-3.0-or-later | https://wiki.gnome.org/Apps/Web |
| [glib-networking](ports/glib-networking/) | 2.80.1 | LGPL-2.0-or-later | https://gitlab.gnome.org/GNOME/glib-networking |
| [gobject-introspection](ports/gobject-introspection/) | 1.86.0 | LGPL-2.0-or-later AND GPL-2.0-or-later AND MIT | https://gi.readthedocs.io |
| [gsettings-desktop-schemas](ports/gsettings-desktop-schemas/) | 49.0 | LGPL-2.0-or-later | https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas |
| [libportal](ports/libportal/) | 0.8.1 | LGPL-3.0-or-later | https://github.com/flatpak/libportal |
| [libdisplay-info](ports/libdisplay-info/) | 0.3.0 | MIT | https://gitlab.freedesktop.org/emersion/libdisplay-info |
| [geany](ports/geany/) | 2.1 | GPL-2.0-or-later | https://www.geany.org |
| [geany-plugins](ports/geany-plugins/) | 2.1 | GPL-3.0-or-later | https://plugins.geany.org/ |
| [gthumb](ports/gthumb/) | 3.12.8.2 | GPL-2.0-or-later | https://gitlab.gnome.org/GNOME/gthumb |
| [mate-calc](ports/mate-calc/) | 1.28.0 | GPL-2.0-or-later | https://github.com/mate-desktop/mate-calc |
| [pqiv](ports/pqiv/) | 2.13.2 | GPL-3.0-only | http://www.pberndt.com/Programme/Linux/pqiv/index.html |
| [xfce4-appfinder](ports/xfce4-appfinder/) | 4.21.0 | GPL-2.0-or-later | https://xfce.org/ |
| [xfce4-session](ports/xfce4-session/) | 4.21.0 | GPL-2.0-or-later | https://xfce.org/ |
| [xfce4-settings](ports/xfce4-settings/) | 4.21.0 | GPL-2.0-only | https://xfce.org/ |
| [xfce4-terminal](ports/xfce4-terminal/) | 1.1.5 | GPL-2.0-or-later AND LGPL-2.1-or-later | https://xfce.org/ |
| [xdg-utils](ports/xdg-utils/) | 1.2.1 | MIT | https://www.freedesktop.org/wiki/Software/xdg-utils/ |
| [desktop-file-utils](ports/desktop-file-utils/) | 0.28 | GPL-2.0-or-later | https://www.freedesktop.org/wiki/Software/desktop-file-utils |
| [shared-mime-info](ports/shared-mime-info/) | 2.4 | GPL-2.0-or-later | https://www.freedesktop.org/Software/shared-mime-info |
| [hicolor-icon-theme](ports/hicolor-icon-theme/) | 0.18 | GPL-2.0-or-later | https://www.freedesktop.org/wiki/Software/icon-theme/ |
| [lxde-icon-theme](ports/lxde-icon-theme/) | 0.5.2 | LGPL-3.0-or-later | https://github.com/lxde/lxde-icon-theme/ |
| [wlr-protocols](ports/wlr-protocols/) | 0_git20250702 | MIT | https://gitlab.freedesktop.org/wlroots/wlr-protocols |
| [godot](ports/godot/) | 4.4.1 | MIT | https://godotengine.org |

### Fonts and data
| Package | Target ver | License | Upstream |
|---|---|---|---|
| [adwaita-fonts](ports/adwaita-fonts/) | 49.0 | GPL-3.0-or-later | https://gitlab.gnome.org/GNOME/adwaita-fonts |
| [font-cantarell](ports/font-cantarell/) | 0.303.1 | OFL-1.1 AND Apache-2.0 | https://cantarell.gnome.org |
| [font-dejavu](ports/font-dejavu/) | 2.37 | Bitstream-Vera | https://dejavu-fonts.github.io/ |
| [font-hack](ports/font-hack/) | 3.003 | MIT | https://sourcefoundry.org/hack/ |
| [font-inter](ports/font-inter/) | 4.1 | OFL-1.1 | https://rsms.me/inter/ |
| [font-liberation](ports/font-liberation/) | 2.1.5 | OFL-1.1 | https://github.com/liberationfonts/liberation-fonts |
| [font-noto-emoji](ports/font-noto-emoji/) | 2.048 | OFL-1.1 | https://github.com/googlefonts/noto-emoji |
| [font-noto-sans-cjk](ports/font-noto-sans-cjk/) | 2.004 | OFL-1.1 | https://github.com/notofonts/noto-fonts |
| [font-noto-serif-cjk](ports/font-noto-serif-cjk/) | 2.003 | OFL-1.1 | https://github.com/notofonts/noto-fonts |
| [font-util](ports/font-util/) | 1.4.1 | MIT AND MIT-open-group AND BSD-Source-Code AND Unicode-TOU | https://gitlab.freedesktop.org/xorg/font/util |
| [encodings](ports/encodings/) | 1.1.0 | LicenseRef-scancode-public-domain | https://xorg.freedesktop.org/ |
| [mkfontscale](ports/mkfontscale/) | 1.2.3 | MIT | https://xorg.freedesktop.org/ |
| [ca-certificates](ports/ca-certificates/) | 1 | GPL-2.0-or-later | https://www.mozilla.org/en-US/about/governance/policies/security-group/certs/ |

## Built — 220 packages, both architectures (inherited)

### Automotive middleware & communication
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| vsomeip | 3.5.6 | REBUILD | 1 | SOME/IP transport (COVESA) |
| CommonAPI C++ core runtime | 3.2.4 | **NEW** | 1 | AUTOSAR-style API layer |
| CommonAPI C++ SOME/IP runtime | 3.2.4 | **NEW** | 2 | binding onto vsomeip |
| DLT daemon | 2.18.10 | REBUILD | 1 | AUTOSAR diagnostic log & trace |
| Eclipse Cyclone DDS | 0.10.5 | **NEW** | 3 | ROS 2 default RMW |
| Eclipse iceoryx | 2.0.6 | **NEW** | – | zero-copy IPC + RouDi daemon |
| Fast-DDS | 2.14.4 | REBUILD | 2 | eProsima RTPS/DDS |
| Fast-CDR | 2.2.6 | REBUILD | – | CDR serialization |
| lely-core | 2.3.4 | REBUILD | 1 | CANopen stack |
| Paho MQTT C | 1.3.14 | **NEW** | 2 | telematics uplink |
| Paho MQTT C++ | 1.5.2 | **NEW** | – | C++ wrapper |
| libwebsockets | 4.3.5 | **NEW** | 1 | V2X / remote diagnostics |
| Protocol Buffers | 29.3 | REBUILD | – | serialization |
| **gRPC** | 1.69.0 | REBUILD | 2 | service RPC |

### Security
| Package | Ver | Status | Patches |
|---|---|---|---|
| OpenSSL | 3.5.1 | REBUILD | QNX target config (new) |
| Mbed-TLS | 3.6.2 | **NEW** | – |
| wolfSSL | 5.7.6 | **NEW** | – |
| libsodium | 1.0.20 | **NEW** | – |

### Perception / compute
| Package | Ver | Status | Patches |
|---|---|---|---|
| OpenCV | 4.10.0 | REBUILD | – |
| Eigen | 3.4.0 | REBUILD | – |
| Ceres Solver | 2.2.0 | REBUILD | – |
| **PCL (Point Cloud Library)** | 1.14.1 | **NEW** | – |
| **ONNX Runtime** | 1.20.1 | REBUILD | 1 (6 files) |
| FLANN | 1.9.2 | REBUILD | – |
| glog | 0.7.1 | REBUILD | 1 |
| gflags | 2.2.2 | REBUILD | – |

### Multimedia (GLib / GStreamer / FFmpeg)
| Package | Ver | Status | Patches | Verified on target |
|---|---|---|---|---|
| GLib | 2.82.4 | **NEW** | 1 | – |
| orc | 0.4.40 | **NEW** | – | – |
| GStreamer core | 1.24.10 | **NEW** | – | `gst-inspect-1.0`, pipelines run |
| gst-plugins-base | 1.24.10 | **NEW** | – | 28 plugins |
| gst-plugins-good | 1.24.10 | **NEW** | – | 78 plugins total |
| gst-libav | 1.24.10 | **NEW** | – | 679 libav elements |
| FFmpeg | 7.1.1 | **NEW** | – | encode + probe on target |
| dav1d | 1.5.0 | **NEW** | – | AV1 decoder registered |
| openh264 | 2.5.0 | **NEW** | 1 | H.264 encode on target |
| x264 | stable | **NEW** | 1 | H.264 encode on target |
| x265 | 3.6 | **NEW** | 1 | H.265 encode, 167 fps on target |
| libvpx | 1.15.0 | **NEW** | – | VP8 + VP9 encode on target |
| libtheora | 1.1.1 | **NEW** | – | Theora encode on target |
| LAME | 3.100 | **NEW** | – | MP3 encode on target |

**End-to-end on a QNX 8.0 x86_64 target:** 80 plugins / 1180 features registered;
`videotestsrc ! videoconvert ! avenc_mpeg4 ! mp4mux ! filesink` produced a valid
82 KB MP4, and `filesrc ! qtdemux ! avdec_mpeg4 ! videoconvert ! fakesink` played
it back. FFmpeg's own CLI encodes MPEG-4 video and Opus audio, and ffprobe reads
the results back correctly.

With the external codecs linked in (`--enable-libdav1d --enable-libopenh264
--enable-libx264 --enable-libx265`), all of these encode on target: H.264 via
libx264 (22 KB) and via openh264 (18 KB), H.265 via libx265 (17 KB at 167 fps),
with dav1d registered as the AV1 decoder. VP8 (6 KB), VP9 (5.7 KB) and Theora
(19 KB) encode into their containers, and MP3 audio through LAME (8.6 KB).
ffprobe reads each result back with the right codec and dimensions.

### Text rendering
| Package | Ver | Status | Patches |
|---|---|---|---|
| ICU | 76.1 | **NEW** | 1 |
| HarfBuzz | 10.1.0 | **NEW** | – |
| Pango | 1.54.0 | **NEW** | – |
| cairo | 1.18.2 | **NEW** | – |
| fontconfig | 2.15.0 | **NEW** | – |
| FriBidi | 1.0.16 | **NEW** | 1 |
| graphite2 | 1.3.14 | **NEW** | – |
| pixman | 0.44.2 | **NEW** | – |

Verified together on target by `test_textstack`: ICU reports 76.1, FriBidi
reorders a mixed latin/hebrew run, HarfBuzz buffers and guesses direction, cairo
rasterises and writes a PNG, and Pango lays "Wavefront 123" out through cairo at
144x16 px using fonts found on the target.

### Window system and GUI toolkit
| Package | Ver | Status | Patches |
|---|---|---|---|
| Wayland | 1.23.1 | **NEW** | 1 |
| wayland-protocols | 1.38 | **NEW** | – (data only) |
| libxkbcommon | 1.7.0 | **NEW** | – |
| xkeyboard-config | 2.43 | **NEW** | – (data only) |
| GTK 3 | 3.24.43 | **NEW** | 1 |
| gdk-pixbuf | 2.42.12 | **NEW** | – |
| ATK | 2.38.0 | **NEW** | – |
| libepoxy | 1.5.10 | **NEW** | – |

GTK3 is built with the **Wayland and Broadway backends**; X11 is off, since QNX
has no X server. libepoxy dispatches to the EGL/GLESv2 the SDP provides.

**GTK3's widget layer is verified running on target.** With `broadwayd` started
and `GDK_BACKEND=broadway`, `gtk_init` opens a display and the test drives real
widgets: construction, container hierarchy, label text, the ATK push-button role,
a natural size of 91x54 px, and the widget tree rendered through cairo into a
PNG. Broadway needs no compositor, which is what makes this possible — Weston
could not be ported (see below).

Verified on target by `test_wayland`: a server-side `wl_display` with its event
loop dispatching through QNX's epoll shim, a listening socket (`wayland-0`), a
client connect that fails cleanly with no compositor present, and libxkbcommon
compiling a real keymap from the xkeyboard-config data and resolving a keycode
to a keysym.

`test_gtk3` runs either way: without a display it covers the GObject types GTK
registers, ATK, and gdk-pixbuf's PNG encode/decode round trip; with `broadwayd`
running it additionally drives the full widget layer.

### Compression and serialisation
| Package | Ver | Status | Patches |
|---|---|---|---|
| zstd | 1.5.6 | REBUILD | – |
| xz / liblzma | 5.6.3 | REBUILD | – |
| bzip2 | 1.0.8 | REBUILD | – |
| brotli | 1.1.0 | REBUILD | – |
| xxHash | 0.8.3 | **NEW** | – |
| jansson | 2.14 | **NEW** | – |
| jsoncpp | 1.9.6 | REBUILD | – |
| simdjson | 3.11.6 | **NEW** | – |
| cJSON | 1.7.18 | **NEW** | – |
| msgpack-c | 6.1.0 | **NEW** | – |
| libconfig | 1.8.1 | **NEW** | – |
| libarchive | 3.7.7 | REBUILD | – |
| json-glib | 1.10.6 | **NEW** | – |

### Text, markup and storage
| Package | Ver | Status | Patches |
|---|---|---|---|
| expat | 2.6.4 | REBUILD | – |
| libxml2 | 2.13.5 | REBUILD | – |
| libxslt | 1.1.42 | **NEW** | – |
| PCRE2 | 10.44 | REBUILD | – |
| Oniguruma | 6.9.9 | **NEW** | – |
| Lua | 5.4.7 | REBUILD | – |
| jq / libjq | 1.7.1 | **NEW** | 1 |
| GDBM | 1.24 | **NEW** | – |
| inih | r58 | **NEW** | – |
| tree-sitter | 0.25.9 | **NEW** | – |
| utf8proc | 2.10.0 | **NEW** | – |
| libunistring | 1.3 | **NEW** | – |
| libidn2 | 2.3.7 | **NEW** | – |
| libpsl | 0.21.5 | **NEW** | – |
| ncurses | 6.5 | REBUILD | – |
| readline | 8.2 | REBUILD | 1 |

### Crypto (beyond OpenSSL / Mbed-TLS / wolfSSL)
| Package | Ver | Status | Patches |
|---|---|---|---|
| GMP | 6.3.0 | REBUILD | – |
| nettle | 3.10.1 | **NEW** | – |
| libtasn1 | 4.19.0 | **NEW** | – |
| GnuTLS | 3.8.8 | **NEW** | – |
| libgcrypt | 1.11.0 | **NEW** | 1 |
| libgpg-error | 1.51 | **NEW** | 1 |

### Networking and event loops
| Package | Ver | Status | Patches |
|---|---|---|---|
| curl | 8.11.1 | REBUILD | – |
| nghttp2 | 1.64.0 | REBUILD | – |
| c-ares | 1.34.4 | REBUILD | – |
| libuv | 1.49.2 | REBUILD | 1 |
| libev | 4.33 | **NEW** | – |
| libevent | 2.1.12 | **NEW** | 1 |
| libssh2 | 1.11.1 | **NEW** | – |
| libusb | 1.0.27 | **NEW** | – |
| D-Bus | 1.16.0 | **NEW** | – |
| libsoup | 3.6.1 | **NEW** | – |

### Imaging
| Package | Ver | Status | Patches |
|---|---|---|---|
| libtiff | 4.7.0 | REBUILD | – |
| libexif | 0.6.24 | **NEW** | – |
| Exiv2 | 0.28.5 | **NEW** | – |
| Little-CMS (lcms2) | 2.16 | REBUILD | – |
| libjxl | 0.11.1 | **NEW** | – |
| libyuv | stable | **NEW** | 1 |
| Highway | 1.2.0 | **NEW** | – |

### Audio / video codecs added in the gap sweep
| Package | Ver | Status | Patches |
|---|---|---|---|
| libaom (AV1) | 3.12.1 | **NEW** | – |
| SVT-AV1 | 2.3.0 | **NEW** | 2 |
| fdk-aac | 2.0.3 | **NEW** | – |
| libsndfile | 1.2.2 | **NEW** | – |
| libsamplerate | 0.2.2 | **NEW** | – |
| gst-plugins-ugly | 1.24.10 | **NEW** | – |
| gst-plugins-bad | 1.24.10 | **NEW** | – |
| SDL2 | 2.32.10 | **NEW** | 1 |

### Added closing the rest of the VM gap
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| Boost (all 39 libraries) | 1.86.0 | REBUILD | 4 | was a 13-library subset; Context, Fiber, Coroutine, JSON, URL, Locale, Process and the rest are staged now |
| MPFR | 4.2.1 | REBUILD | – | arbitrary-precision floating point |
| MPC | 1.3.1 | **NEW** | – | arbitrary-precision complex |
| libtool (libltdl) | 2.5.4 | **NEW** | – | portable `dlopen` wrapper |
| p11-kit | 0.25.5 | **NEW** | – | PKCS#11 module loader for GnuTLS |
| libnotify | 0.8.3 | **NEW** | – | desktop notifications |
| libproxy | 0.5.9 | **NEW** | – | proxy configuration |
| libdatrie | 0.2.13 | **NEW** | 1 | double-array trie |
| libthai | 0.1.29 | **NEW** | 1 | Thai word breaking (Pango dependency) |
| libunibreak | 6.1 | **NEW** | – | UAX #14 / #29 line and word breaking |
| libedit | 20240808-3.1 | **NEW** | 1 | BSD editline |
| unibilium | 2.1.2 | **NEW** | – | terminfo parser |
| Duktape | 2.7.0 | **NEW** | – | embeddable ECMAScript engine |
| simdutf | 6.4.2 | **NEW** | – | SIMD UTF validation and transcoding |
| woff2 | 1.0.2 | **NEW** | – | web font compression |
| OpenJPEG | 2.5.3 | **NEW** | – | JPEG 2000 |
| libavif | 1.1.1 | **NEW** | 1 | AVIF, over libaom + dav1d |
| libsoxr | 0.1.3 | **NEW** | – | high-quality resampling |
| zimg | 3.0.5 | **NEW** | 1 | colourspace conversion and scaling |
| libass | 0.17.3 | **NEW** | – | ASS/SSA subtitle rendering |
| libfdt (dtc) | 1.7.2 | **NEW** | – | flattened device tree access |

### X11 client stack and CPython
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| xorgproto | 2024.1 | **NEW** | – | X protocol headers |
| xtrans / libXau / libXdmcp | – | **NEW** | – | transport, authority, display manager |
| xcb-proto / libxcb | 1.17.0 | **NEW** | – | X protocol bindings (27 libraries) |
| pthread-stubs | 0.5 | **NEW** | – | resolves to no library on QNX, which is correct |
| libX11 | 1.8.10 | **NEW** | – | Xlib |
| libXext/Xrender/Xfixes/Xi/Xrandr/Xcursor/Xinerama/Xtst/Xft | – | **NEW** | – | X extensions |
| **CPython** | 3.14.0 | **NEW** | 1 (9 files) | interpreter + 60 extension modules |

The X11 stack needed no source patches at all -- only one QNX-specific build
decision, `--disable-malloc0returnsnull`, settled from QNX's own documentation
("If size is zero, then malloc() returns a valid pointer with a size of zero")
rather than guessed.

### GTK4, Xfce and the last of the VM gap
Everything in this block was built from **release tarballs, not git**, where the
project generates sources at `dist` time -- see the hazard note below.

| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| GTK4 | 4.16.7 | **NEW** | 1 | GTK 4 toolkit (X11 + Wayland + Broadway) |
| graphene | 1.10.8 | **NEW** | – | SIMD vector/matrix types GTK4 requires |
| libhandy | 1.8.3 | **NEW** | – | adaptive GTK3 widgets |
| libdazzle | 3.44.0 | **NEW** | – | GNOME Builder widget/animation kit |
| gtk-layer-shell | 0.9.0 | **NEW** | – | `wlr-layer-shell` panels and docks |
| libxfce4util | 4.20.0 | **NEW** | – | Xfce base utility library |
| xfconf | 4.20.0 | **NEW** | – | Xfce configuration store (D-Bus) |
| libxfce4ui | 4.20.0 | **NEW** | – | Xfce shared widgets |
| libxfce4windowing | 4.20.0 | **NEW** | – | Xfce window-manager abstraction |
| garcon | 4.20.0 | **NEW** | – | freedesktop.org menu implementation |
| exo | 4.20.0 | **NEW** | – | Xfce application support library |
| libsecret | 0.21.4 | **NEW** | 1 | Secret Service client (keyring) |
| gcr | 3.41.2 | **NEW** | 1 | crypto UI and PKCS#11 for libsecret |
| SDL2_image | 2.8.8 | **NEW** | – | image loading for SDL2 |
| SDL2_ttf | 2.24.0 | **NEW** | – | FreeType text rendering for SDL2 |
| SDL2_net | 2.2.0 | **NEW** | – | SDL2 network abstraction |
| ThorVG | 0.15.8 | **NEW** | – | vector graphics / Lottie engine |
| libgsf | 1.14.53 | **NEW** | – | structured-file (OLE2/ZIP) access |
| aspell | 0.60.8.1 | **NEW** | – | spell checker |
| enchant | 2.8.2 | **NEW** | – | spell-checking broker; dlopens its aspell provider |
| hyphen | 2.8.8 | **NEW** | – | Liang hyphenation |
| enca | 1.19 | **NEW** | – | charset analyser |
| libgsm | 1.0.22 | **NEW** | – | GSM 06.10 full-rate speech codec |
| xvidcore | 1.3.7 | **NEW** | – | MPEG-4 ASP codec |
| vid.stab | 1.1.1 | **NEW** | – | video stabilisation (FFmpeg filter) |
| ffmpegthumbnailer | 2.2.3 | **NEW** | – | video thumbnail extraction |
| libfontenc | 1.1.8 | **NEW** | – | X font encoding tables |
| iperf3 | 3.17.1 | **NEW** | 1 | network throughput measurement |

### Desktop integration, accessibility and the last additions
| Package | Ver | Status | Patches | Role |
|---|---|---|---|---|
| at-spi2-core | 2.54.0 | **NEW** | – | accessibility bus + ATK bridge — **and ATK itself**, which upstream merged in at 2.52; the separate `atk` recipe is retired |
| VTE | 0.78.2 | **NEW** | 3 | terminal widget (GTK3 + GTK4) behind xfce4-terminal |
| tumbler | 4.20.0 | **NEW** | – | Xfce thumbnail service — 8 plugins including ffmpeg, GStreamer and OLE2 |
| SDL3 | 3.2.10 | **NEW** | 1 | X11 backend, not Wayland — see the note below |
| libde265 | 1.0.15 | **NEW** | – | HEVC decoder, the other half of the x265 pair |
| libXScrnSaver | 1.2.4 | **NEW** | – | MIT-SCREEN-SAVER client, required by SDL3's X11 backend |
| pkgconf | 2.3.0 | **NEW** | – | a *target* pkg-config, for building on the target itself |
| JavaScriptCore | 2.44.4 | **NEW** | 4 | WebKit's JS engine, JSCOnly port with the CLoop interpreter |
| WebKitGTK | 2.44.4 | **NEW** | 5 | the web engine — X11/Wayland windowing off, see below |
| xfce4-panel | 4.20.0 | **NEW** | – | Xfce panel and `libxfce4panel`, Wayland + gtk-layer-shell |
| Thunar | 4.20.0 | **NEW** | – | Xfce file manager and `thunarx`, its extension library |
| LuaJIT | 2.1 (c6ffc141) | **NEW** | 1 | trace-compiling Lua; JIT verified on target |
| luv | 1.51.0-1 | **NEW** | – | libuv bindings for Lua, against the staged libuv and LuaJIT |
| abseil-cpp | 20240722.0 | **NEW** | 1 | one copy for the tree — protobuf and gRPC used to install two |
| libXcomposite / libXdamage | 0.4.6 / 1.1.6 | **NEW** | – | GTK3 X11 backend dependencies |

Thunar 4.20 hard-requires GTK3's X11 backend (`XDT_CHECK_LIBX11_REQUIRE`,
`gdk/gdkx.h`), so three packages built earlier were **reconfigured**, not just
extended: GTK3 now builds X11 alongside Wayland and Broadway; cairo gained its
Xlib surface (with a patch — see below); and libepoxy gained GLX dispatch,
because GTK3's X11 code includes `epoxy/glx.h` unconditionally. No GL
implementation is involved: epoxy loads libGL lazily and GTK asks through
`epoxy_has_glx()`, which answers false rather than aborting when there is none.


SDL3 takes X11 where the SDL2 recipe here takes Wayland, and that is not a
preference: SDL3's Wayland check lists `wayland-egl` and `egl` as hard
pkg-config requirements, and there is no EGL in this tree — QNX ships no Mesa,
and its own EGL belongs to Screen, which is exactly the kind of SDP prebuilt
this workspace must not link. The X11 backend has no such requirement.

### Foundation / utility
| Package | Ver | Status | Patches |
|---|---|---|---|
| Boost | 1.86.0 | REBUILD | 2 |
| nlohmann/json | 3.12.0 | **NEW** | – |
| spdlog | 1.15.3 | **NEW** | – |
| fmt | 11.2.0 | **NEW** | – |
| yaml-cpp | 0.8.0 | **NEW** | – |
| SQLite | 3.47.2 | REBUILD | – |
| zlib | 1.3.1 | REBUILD | – |
| tinyxml2 | 10.0.0 | REBUILD | – |
| foonathan_memory | 0.7-3 | REBUILD | – |
| googletest | 1.17.0 | REBUILD | – |
| libpcap | 1.10.5 | **NEW** | – |
| lz4 | 1.10.0 | REBUILD | – |

All 72 patch files are verified to apply to pristine upstream checkouts at the
pinned refs.

**Verified on hardware: 165 / 165 port tests pass on both architectures** --
a Raspberry Pi 5 and a QNX 8.0 x86_64 QEMU VM, against a staging tree rebuilt
from scratch: `stage`, `build`, `src` and `dist` moved aside, every source
re-fetched, every patch re-applied.  The x86_64 image is a 147 MB `mkqnximage`
build that cannot hold the 184 MB bundle, so it runs the suite from a
read-only NFS mount of the build host (`tests/run-over-nfs.sh`).
See [README.md](README.md) for the suite and the defects it uncovered.

### WebKitGTK: what works and what cannot
`libwebkit2gtk-4.1.so` (99 MB), `libjavascriptcoregtk-4.1.so`, the Web and
Network processes and the `jsc` shell all build and run; WebKit reports itself
as `Mozilla/5.0 (X11; QNX RaspberryPi5) AppleWebKit/605.1.15`.

What cannot work is a **browser window**. QNX's only EGL belongs to Screen and
types `EGLNativeDisplayType` as an integer handle rather than a display
pointer, so WebKit's X11 and Wayland display backends do not compile against
it. Both windowing targets are off; that costs 2 of 1712 source files and
leaves the engine, DOM, layout, networking and media intact. Rendering into a
window needs a `PlatformDisplay` backend written against QNX Screen, which is
new development rather than porting.

### Defects the test suite found
Every one of these produced a library that built and linked cleanly; only
running the code on a real target exposed them.

| Defect | Effect |
|---|---|
| libffi allocated closures with `malloc` | `FFI_MMAP_EXEC_WRIT` is gated on Linux/Windows only, so a QNX closure trampoline landed in the (non-executable) malloc arena. **Every** libffi callback segfaulted. Fixed by enabling the `mmap(PROT_EXEC)` allocator for QNX. |
| Orc registered no compile target on aarch64 | ARM NEON detection reads `/proc/cpuinfo`, which QNX does not have, so the backend was marked non-executable and every Orc program was interpreted. Advanced SIMD is mandatory on ARMv8-A; fixed the way Windows-ARM64 and macOS-arm64 already are. |
| readline installed no `libreadline.so` | Its `support/shlib-install` has no case for `nto-qnx`, so only `libreadline.so.8` landed. `-lreadline` then silently resolved against the SDP's *prebuilt* copy — the one thing this workspace must never use. Now caught for every package by `scripts/check-devlinks`. |
| libyuv.so had unresolved `jpeg_*` symbols | `-DHAVE_JPEG` compiles the MJPEG decoder into the library, but upstream links libjpeg only into the `yuvconvert` tool. |
| libSvtAv1Enc.so had an unresolved `cpuinfo_isa` | The bundled cpuinfo excludes QNX from its supported-OS list, so the CPUID-based x86 sources were never compiled. |
| libCommonAPI-SomeIP had unresolved `eventfd` | QNX splits `eventfd` out of libc into `libeventfd`. |
| libgrpc had unresolved `MakeDirectoryReader` | QNX is not in gRPC's POSIX platform list. |
| Eleven staged libraries were shadowed by the SDP's prebuilt copies | `find_package()` searched the SDP before staging, so binaries recorded the SDP's sonames (`libz.so.2` against upstream's `libz.so.1`) and loaded QNX's prebuilt libraries on the target. Caught by the new `scripts/check-sdp-leak`. |
| The QNX gcc spec's own `-rpath-link` outranked ours | Transitive `DT_NEEDED` resolution used the SDP's older OpenSSL, turning libcurl's valid `SSL_get0_group_name` call into an undefined reference. Fixed with a link sysroot that redirects the spec's search. |
| An empty `libregex.a` compat stub shadowed QNX's real `libregex.so` | libedit linked with `regcomp`/`regexec`/`regfree` unresolved. |
| The driver never passed `CC`/`CFLAGS`/`CPPFLAGS` to any autotools `configure` | A comment inside a backslash continuation ended the command. 33 packages built anyway, because `--host` finds the cross gcc by itself — which is why it went unnoticed. |
| CommonAPI aborted at exit | `FactoryDeinit` → `unregisterFactory` → `mutex::lock` returning `EINVAL`: a use-after-free across DSO teardown order, root-caused with gdb on the target. |

| ThorVG exported no symbols at all | Meson's `--default-library=both` makes ThorVG define `TVG_STATIC`, which expands `TVG_API` to nothing — the shared object built and installed with zero `tvg_*` symbols. Shared-only build: 121 symbols. Nothing but linking against it could show this. |
| GTK4 and every Xfce library demanded an `exe_wrapper` | They were cloned from git. GTK4 compiles its CSS themes with `sassc` at build time and Xfce regenerates `*-aliasdef.c`; both steps exist only for git checkouts — the release tarball ships the generated files. Switching to tarballs fixed six Xfce packages at once and retired two patches. |

| enchant loaded no spell-checking providers | gnulib's `relocatable.c` finds the running shared library only on Linux and Cygwin, by reading `/proc/self/maps`. On QNX it compiles to nothing, so `relocate()` is the identity and enchant kept composing its provider directory from the configure-time prefix — the broker constructed cleanly and enumerated zero providers anywhere but that exact path. Reimplemented with `dladdr()`, which QNX has. |

| VTE could not open a pty at all | `vte_pty_new_sync()` failed with "Inappropriate I/O control operation". QNX's `<sys/ioctl.h>` *defines* `TIOCPKT` but devc-pty does not implement it. Ignoring the errno would have been worse than the failure: vte.cc's reader is compiled under the same `#if` and strips a leading packet-header byte from every `read()`, so the first byte of real output would have been eaten. Both sites now agree. |
| at-spi2-core silently replaced ATK | ATK moved into at-spi2-core upstream at 2.52, so 2.54 installs its own `libatk-1.0`. With the old standalone `atk` recipe still present, both staged a `libatk-1.0.so.0.*` and build order decided the winner. Only the target caught it: 2.54 aliases `ATK_ROLE_PUSH_BUTTON` to `ATK_ROLE_BUTTON`, so `atk_role_get_name()` changed answer. |
| A staged target binary broke every later CMake build | Porting pkgconf put a QNX `pkgconf` in `stage/<arch>/usr/local/bin`, which is on `CMAKE_PREFIX_PATH`. `find_package(PkgConfig)` matched it, tried to run it, and reported PkgConfig *missing* — naming neither the file nor the reason. Staging's `bin`/`sbin` are now in `CMAKE_IGNORE_PATH`. |

| Two protobufs and two abseils in one gRPC process | gRPC's bundled protobuf 29.0 and abseil 20240722 were installed next to the protobuf recipe's 29.3 and abseil 20240116 — 89 libraries twice, symlinks pointing at whichever was last, headers merged file by file, and c-ares's `libcares.pc` overwritten. `test_grpc` loaded both pairs and passed; protobuf aborts when a descriptor registers twice, so that was luck. One abseil recipe now, providers switched to `package`, and `scripts/check-dupes` guards it. |
| cairo's Xlib surface could not compile | It includes `<sys/shm.h>` unconditionally for MIT-SHM, and QNX 8.0 has no SysV shared memory at all. cairo's existing no-SHM stub path is now taken when the header is absent. |

| LuaJIT crashed on the first line of any script | Its cross build generated the interpreter with a host-side `LUAJIT_OS_OTHER` while the target code used `POSIX`; `LJ_HASPROFILE` differs between them and shifts `GG_State`'s dispatch table by 16 bytes, so every VM access to global state was misaddressed. The recipe comment had called this mismatch harmless. |

| aspell linked QNX's prebuilt ncurses | Its recipe never declared ncurses, so on a from-scratch build nothing had staged ours yet and `-lncursesw` resolved to the SDP's `libncursesw.so.1`. Same shape as the gettext case the previous clean rebuild found. |
| libev quietly changed backends | Once the driver linked QNX's split libc into configure probes, libev found epoll, eventfd, signalfd and inotify where the verified build had found none and used `poll()`. inotify does not compile (no `<sys/statfs.h>`) and is now off; epoll is kept and the test asserts the loop actually selects it. |
| test_python was not testing our Python | See README hazard 85: on the Pi it loaded QNX's stdlib and prebuilt `_ssl`/`_ctypes`/`_sqlite3` from `/system`. |

| 56 libraries linked QNX's prebuilt libintl | gettext was built `--disable-nls` and staged no libintl at all, so glib, GTK, ATK and every Xfce component resolved `-lintl` to the SDP's copy. The audit could not see it — it only flagged sonames we had also staged — and now works from an allowlist instead. |
| JavaScriptCore aborted before running any script | Its structure-heap reservation needs up to 8 GB of address space; a 1 GB QNX VM cannot give that, and the RELEASE_ASSERT fired inside `JSContextGroupCreate()`. Passed on the 8 GB Pi, so it looked architecture-specific. |

## Not built — and why

| Package | Blocker |
|---|---|
| Eclipse zenoh-c / zenoh-pico | Rust. No toolchain on this host, and stock rustup ships no QNX 8.0 std target — needs the `qnx-ports/rust` bootstrap first. |
| Weston | Hard-requires libinput and libdrm at configure time even for the headless backend. libinput needs udev, libevdev and mtdev — the Linux device model; QNX enumerates input through its own `devi-*` resource managers. Making it build means patching weston to treat those as optional for non-DRM backends and porting libdrm headers-only for `drm_fourcc.h`. GTK3's widget layer is covered through Broadway instead. |
| Eclipse Kuksa databroker | Same Rust blocker. |
| can-utils | Not a port. Every tool is written against SocketCAN (`AF_CAN`/`PF_CAN`), which QNX does not have — QNX uses `dev/can` resource managers. This needs a SocketCAN-to-QNX translation layer written first, which is new development rather than porting. |


## QNX porting hazards
97 of them, with the specific failure each produces, are documented in
[README.md](README.md). The ones that cost the most time: `-std=c++17` hiding
POSIX declarations, the absent `librt`/`libdl`/`libpthread`, CMake identifying
qcc as `QCC` rather than `GNU`, and feature detection keyed on `_POSIX_C_SOURCE`
(which QNX does not define) causing shim-vs-libc collisions.
