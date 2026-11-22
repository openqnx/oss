# yt-dlp: download or stream video and audio from YouTube and many other
# sites (a command line program). Pure Python: the published wheel is
# platform independent, so it is installed as released (scripts/lib/pywheel.sh);
# its console script is written as a launcher (qnx_py_scripts). Without the
# optional extras it needs only Python; ffmpeg (ported) merges separate
# video and audio streams and converts formats when it is installed.
# It plays nothing itself: hand its stream URL to a player (parole, vlc).
# No DRM: sites protected by Widevine (Netflix and the like) are not supported.
VERSION=2026.10.26
TARBALL=https://files.pythonhosted.org/packages/7f/3d/16ea7d5638cf585991f2c5705e8083688dde633d32d0871f2404e0f4d827/yt_dlp-2026.10.26-py3-none-any.whl
SHA256=a05c5b5d8562f7e8a62f5334348b406af4930d56d3a65f8021d14af0e22339ae
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
