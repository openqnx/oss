# virtualgl: the client side of VirtualGL -- vglclient, which receives the
# OpenGL frames a VirtualGL server (vglrun on a Linux machine with a GPU, e.g.
# RViz in an Ubuntu guest) renders and draws them on this machine's X display
# (Xwayland on the QNX desktop), and vglconnect, which sets up that link over
# SSH. The server side (the GL faker, vglrun) belongs on the Linux machine and
# is not built: it needs desktop OpenGL 4.5, which QNX does not have.
VERSION=3.1.5; GIT=https://github.com/VirtualGL/virtualgl.git; REF=3.1.5
DEPS="libx11 libxext libjpeg-turbo"
BUILDSYS=cmake
CMAKE_ARGS_EXTRA=( -DVGL_BUILDSERVER=OFF -DVGL_USEGL=OFF -DVGL_USEXV=OFF -DVGL_FAKEXCB=OFF -DTJPEG_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DTJPEG_LIBRARY="$STAGE$PREFIX/lib/libturbojpeg.so" -DCMAKE_INSTALL_DOCDIR=share/doc/virtualgl
                   -DCMAKE_INSTALL_BINDIR=bin )
