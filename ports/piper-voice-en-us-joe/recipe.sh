# piper-voice-en-us-joe: a voice for Piper (ports/piper): U.S. English, one
# male speaker, "medium" quality, 22050 Hz -- en_US-joe-medium.onnx (63 MB,
# the VITS model) and en_US-joe-medium.onnx.json (its configuration: the
# phoneme table, the espeak-ng voice "en-us", inference defaults), from the
# Piper project's voice collection (huggingface.co/rhasspy/piper-voices),
# installed in share/piper-voices/ with the model card.
#   echo 'Hello.' | piper_exe -m /usr/local/share/piper-voices/en_US-joe-medium.onnx -f hello.wav
#
# License: the collection is published under the MIT license; this voice's
# recordings are CC0 (github.com/OHF-Voice/voice-datasets) and it was
# fine-tuned from a base model that is CC0 as well (MODEL_CARD, installed
# next to the model). It was chosen for that: most of the collection's small
# English voices were fine-tuned from a voice whose recordings are
# CC BY-NC-SA or have no stated license.
#
# Data, the same for both architectures; nothing is compiled. The files are
# not in a tarball or a git repository that could be cloned (the collection
# is 100+ GB of git-lfs): do_fetch downloads the three files at a pinned
# revision of the collection and checks each SHA-256. The model needs Piper
# 1.5 or newer (model card).
# VERSION: the collection has no per-voice versions; 1.0.0 is this component's
# own, raised when the pinned revision changes the files.
VERSION=1.0.0
DEPS=""
BUILDSYS=custom
VOICE=en_US-joe-medium
VOICE_REV=ae9d8352b73c1b1750748843e753a504b00e05b0
VOICE_URL=https://huggingface.co/rhasspy/piper-voices/resolve/$VOICE_REV/en/en_US/joe/medium
VOICE_FILES="$VOICE.onnx:abf4b998e34045cbd00eaf050de2de620f81ded6eb8b85a24b022a1c2b3f08d2
$VOICE.onnx.json:7b5f0293c0bf5bda8e377be1696da84af798befeef31038e43beefc01fe858da
MODEL_CARD:4b319248135916e8f5433ce7a9fe5271026b05cb598bab05807b22cb17fd129b"
do_fetch() {
  local e f sum c="$DIST_DIR/piper-voice-$VOICE-$VOICE_REV"
  mkdir -p "$c" "$SRCROOT" || return 1
  for e in $VOICE_FILES; do
    f=${e%%:*}; sum=${e##*:}
    [ -f "$c/$f" ] || curl -fsSL --retry 3 -o "$c/$f" "$VOICE_URL/$f" || { rm -f "$c/$f"; rm -rf "$SRCROOT"; return 1; }
    echo "$sum  $c/$f" | sha256sum -c - || { echo "!! checksum mismatch: $f"; rm -f "$c/$f"; rm -rf "$SRCROOT"; return 1; }
    cp "$c/$f" "$SRCROOT/$f" || return 1
  done
}
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/share/piper-voices"
  install -Dm644 "$S/$VOICE.onnx" "$d/$VOICE.onnx" &&
  install -m644 "$S/$VOICE.onnx.json" "$d/$VOICE.onnx.json" &&
  install -m644 "$S/MODEL_CARD" "$d/$VOICE.MODEL_CARD"
}
