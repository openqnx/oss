# libvncserver: LibVNCServer and LibVNCClient, the VNC (RFB) protocol as C
# libraries, with the Tight/JPEG, ZRLE and other encodings TurboVNC and
# TigerVNC servers use -- and SDLvncviewer, a VNC viewer on SDL, which on QNX
# draws straight onto QNX Screen. With VirtualGL on a Linux machine (vglrun
# in a TurboVNC session) this shows OpenGL programs such as RViz on QNX.
VERSION=0.9.15; GIT=https://github.com/LibVNC/libvncserver.git; REF=LibVNCServer-0.9.15
DEPS="zlib lzo libjpeg-turbo libpng gnutls openssl sdl2"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DWITH_SDL=ON -DWITH_GTK=OFF -DWITH_FFMPEG=OFF -DWITH_SYSTEMD=OFF -DWITH_LIBSSHTUNNEL=OFF
                   -DWITH_GCRYPT=OFF -DWITH_EXAMPLES=ON -DWITH_TESTS=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  install -Dm755 "$B/examples/client/SDLvncviewer" "$STAGE$PREFIX/bin/SDLvncviewer"
}
