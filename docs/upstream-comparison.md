# Upstream comparison

How this set relates to [qnx-ports/build-files](https://github.com/qnx-ports/build-files), compared against its `ports/` directory as of 2026-09-27.

| | Count |
|---|---|
| Components here | 222 |
| Components upstream | 150 |
| Upstream components also here | 68 |
| Upstream components **not** here | 82 |
| Here but not upstream | 154 |

Names were matched case- and separator-insensitively, then checked by hand; matching is approximate by nature, so treat the counts as close rather than exact.

## Upstream components not in this set

The gap is mostly frameworks with their own large toolchains (PyTorch, TensorFlow, Qt, ROS 2, Rust), host utilities (vim, bash, minicom) and the AWS/Azure IoT SDKs — none blocked, simply not in scope for an automotive middleware and HMI stack.

- `amqp-cpp`
- `asio`
- `aws-c-cal`
- `aws-c-common`
- `aws-c-io`
- `aws-crt-cpp`
- `azure-iot-sdk-c`
- `bash`
- `benchmark`
- `CANdb`
- `capnproto`
- `Catch2`
- `check`
- `chrony`
- `clapack`
- `ComputeLibrary`
- `cpuinfo`
- `csmith`
- `cunit`
- `dnsmasq`
- `dump1090`
- `epoll`
- `farmhash`
- `flatbuffers`
- `FreeImage`
- `g3log`
- `gtsam`
- `kineto`
- `libcamera`
- `libiconv`
- `libjson` — different library from jsoncpp
- `libmetal`
- `libyaml` — built as a host tool only, not a target port
- `libzmq`
- `lighttpd1.4`
- `lmbench`
- `mame`
- `memory`
- `METIS`
- `minicom`
- `mosquitto`
- `muslflt`
- `nanosvg`
- `NTPsec`
- `numpy`
- `numpy2`
- `oneTBB`
- `open-amp`
- `openal-soft`
- `OpenBLAS`
- `pandas`
- `pdfium`
- `perl5`
- `picocom`
- `pugixml`
- `pytorch`
- `qt`
- `rapidjson`
- `re2` — different library from pcre2
- `restbed`
- `RetroPie`
- `ros2`
- `rtl-sdr`
- `rust`
- `ruy`
- `s2n-tls`
- `SDL_ttf`
- `sleef`
- `sqlite_orm` — C++ ORM wrapper, not SQLite itself
- `sscep`
- `strongswan`
- `SuiteSparse`
- `sysbench`
- `tensorflow`
- `tftp-hpa`
- `tree`
- `trompeloeil`
- `unbound`
- `vim`
- `vlc`
- `webrtc`
- `XNNPACK`

## Named differently

| Upstream | Here |
|---|---|
| `abseil-cpp` | [`abseil`](../ports/abseil/README.md) |
| `cJSON` | [`cjson`](../ports/cjson/README.md) |
| `Fast-CDR` | [`fastcdr`](../ports/fastcdr/README.md) |
| `Fast-DDS` | [`fastdds`](../ports/fastdds/README.md) |
| `foonathan_memory_vendor` | [`foonathan_memory`](../ports/foonathan_memory/README.md) |
| `gettext-runtime` | [`gettext`](../ports/gettext/README.md) |
| `graphite` | [`graphite2`](../ports/graphite2/README.md) |
| `gtk` | [`gtk3`](../ports/gtk3/README.md) |
| `libexpat` | [`expat`](../ports/expat/README.md) |
| `onnx` | [`onnxruntime`](../ports/onnxruntime/README.md) |
| `SDL` | [`sdl2`](../ports/sdl2/README.md) |

## In this set, not upstream

222 components against 150 upstream: the surplus is the GTK/Xfce desktop stack, GStreamer with its plugin sets, FFmpeg and its codecs, WebKitGTK and JavaScriptCore, CPython, the X11 client libraries, and the dependency tail each of those pulls in.

