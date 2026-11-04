# frei0r-plugins: the frei0r video effect plugins (filters, generators, mixers;
# about 150 .so files in lib/frei0r-1) that ffmpeg, MLT, Kdenlive and
# GStreamer load through the one-header frei0r API.
VERSION=2.4.0; GIT=https://github.com/dyne/frei0r.git; REF=v2.4.0
DEPS="cairo opencv freetype"
BUILDSYS=cmake
# With the cairo plugins (cairogradient, cairoimagegrid, cairoaffineblend ...)
# and the OpenCV ones (facedetect, facebl0r); without the gavl ones (scale0tilt,
# vectorscope: gavl is not ported).
CMAKE_ARGS_EXTRA=( -DWITHOUT_GAVL=ON -DBUILD_TESTING=OFF )
