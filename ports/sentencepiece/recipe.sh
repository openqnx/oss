# sentencepiece: Google's unsupervised text tokenizer and detokenizer (BPE and
# unigram language-model subword units) -- libsentencepiece, its trainer
# library and the spm_train/spm_encode/spm_decode/spm_export_vocab commands.
# QNX's branch (github.com/qnx-ports/sentencepiece, qnx-mediapipe-v0.10.26)
# only adapts v0.1.96 for MediaPipe's build; this port needs no QNX changes.
#
# 0.2.1, not 0.2.2: 0.2.2 needs abseil's StatusBuilder (abseil 2026), and
# this tree has one abseil, 20240722 (ports/abseil). It is linked as that
# package, and protobuf as the staged one, instead of the copies sentencepiece
# can embed (a protobuf-lite 3.x and abseil): libsentencepiece.so exports
# what it embeds, and a program that also uses protobuf or abseil would get
# two versions of the same symbols.
VERSION=0.2.1; GIT=https://github.com/google/sentencepiece.git; REF=v0.2.1
DEPS="protobuf abseil"
BUILDSYS=cmake
# With the abseil package, sentencepiece's CMake replaces its own minimal
# third_party/absl (its sources include "third_party/absl/...") with a
# symbolic link into the staging tree -- in the source tree both
# architectures share. Moved aside first (the CMake then skips the link);
# the link is made in each architecture's build tree instead and put on the
# include path.
do_patch() { mv "$SRCROOT/third_party/absl" "$SRCROOT/third_party/absl.org"; }
# protobuf_generate_cpp() runs protoc on the build machine: the host protoc the
# protobuf recipe built (29.3, the version its generated code must match).
_host_protoc() { find "$BUILD_DIR/host/protobuf" -maxdepth 1 -name 'protoc*' -type f -perm -u+x | head -1; }
CMAKE_ARGS_EXTRA=( -DSPM_ENABLE_SHARED=ON -DSPM_ENABLE_TCMALLOC=OFF -DSPM_BUILD_TEST=OFF
                   -DSPM_ABSL_PROVIDER=package -DSPM_PROTOBUF_PROVIDER=package
                   -Dabsl_DIR="$STAGE$PREFIX/lib/cmake/absl" -DProtobuf_PROTOC_EXECUTABLE="$(_host_protoc)" )
do_configure() {
  mkdir -p "$B/qnx-inc/third_party" && ln -sfn "$STAGE_SYSROOT$PREFIX/include/absl" "$B/qnx-inc/third_party/absl" || return 1
  CXXFLAGS="-I$B/qnx-inc" cmake -S "$S" -B "$B" -G Ninja -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/qnx-$QNX_ARCH.cmake" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 "${CMAKE_ARGS_EXTRA[@]}"
}
