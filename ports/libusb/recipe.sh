VERSION=1.0.30; GIT=https://github.com/libusb/libusb.git; REF=v1.0.30
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-udev --disable-tests
                  --disable-examples )
