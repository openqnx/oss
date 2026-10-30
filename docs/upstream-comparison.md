# Comparison with QNX's own lists

What QNX publishes, compared with `ports/` here (2026-10-31). Names were
matched case- and separator-insensitively plus the aliases below, then checked
by hand.

| QNX's list | Entries | Here | Not here |
|---|---|---|---|
| [repo.oss.qnx.com](https://repo.oss.qnx.com/) 8.0.5 apk repository (package origins, core + extra) | 162 | 142 | 20 |
| [qnx-ports/build-files](https://github.com/qnx-ports/build-files) `ports/` | 151 | 147 | 4 (none real) |
| [github.com/qnx-ports](https://github.com/qnx-ports) repositories | 216 | 130 | 86 |

Here under another name: clang, clang22, llvm22 -> `llvm`; libxml2.13 ->
`libxml2`; ros2-jazzy -> `ros2`; vte3 -> `vte`; webkit2gtk-4.1 -> `webkitgtk`;
python3-tz -> `python3-pytz`; python3-pandas -> `pandas`; wlroots0.20 ->
`wlroots`; libexpat -> `expat`; memory, foonathan_memory_vendor ->
`foonathan_memory`; mcap -> `mcap-cpp`; llvm-project -> `llvm`; ggml (inside
`whisper.cpp`, `stable-diffusion.cpp`). epoll is QNX's own library in the SDP.

## repo.oss.qnx.com origins not here

- LLVM subprojects: lld22, mlir22, flang22, llvm-runtimes (compiler-rt), openmp
  -- `llvm` builds LLVM and clang; these extend that build.
- glfw, raylib -- windowing on QNX; possible now that the Wayland stack
  (`wlroots`, `labwc`) runs.
- gnupg (needs libassuan, libksba, npth), nss (and nspr), nginx, dhcpcd,
  asciidoc, perl-io-tty (a Perl XS module).
- python3-matplotlib, python3-ml-dtypes (C/C++ extensions), python3-colcon-common-extensions.
- QNX's own pieces, not open source projects to port: filesystem (the base
  layout of QNX's image), openrc (QNX's init integration),
  qnx-posix-publish-subscribe, libsysv-ipc-shim.

## github.com/qnx-ports repositories not here

Infrastructure and forks of QNX's own tooling (build-files, aports, qsc-apk,
perl-cross, check-tools, up-conan-recipes, es-theme-qnx, mason.nvim,
qnx-ros2-workspace, cairo-test-result, gyp-next, .github); Rust crates (libc-rs,
cc-rs, filetime, gitoxide, nix, socket2-rs, tokio, target-lexicon,
platform-info, getrandom-rs, rustix, rust_libloading, rust-ctrlc, rust-errno);
large frameworks and toolchains (bazel, jdk25u, node, mediapipe, openvino,
tflite-micro, executorch, MNN, llama.cpp, xla, gloo, MPItrampoline,
sentencepiece, perfetto, godot-QNX, chipStar, HIPCC and the ROCm repositories,
oneDNN, mlas); and ports still to do here (boringssl, zenoh-pico, zenoh-cpp,
up-cpp, up-transport-zenoh-cpp, lldpd, samba, nss, nspr, xmlto, psutil,
libdaemon, avahi, xtl, cups, skalibs, mosh, hailort, libutempter, ZeroTierOne,
pygame, qdirstat, c-blosc, riegeli, fastfetch, 7zip, iperf2, libpnet,
ntttcp-for-linux, qualcomm-alsa, openrc, libsysv-ipc-shim).

Ported here from these lists in this round, with QNX's own changes where QNX
publishes them: wlroots (QNX Screen backend), labwc, htop and btop (QNX
platforms), mimalloc, pistache, whisper.cpp, stable-diffusion.cpp.
