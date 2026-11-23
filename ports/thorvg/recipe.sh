VERSION=1.1.4; GIT=https://github.com/thorvg/thorvg.git; REF=v1.1.4
DEPS="libjpeg-turbo libpng zlib"
BUILDSYS=meson
# --default-library=both (the driver's default) makes ThorVG define TVG_STATIC,
# which turns TVG_API into nothing at all -- the shared library then exports no
# tvg_* symbol and every consumer fails to link. Build shared only.
# -Dextra=lottie_exp: 1.x turns OpenMP on by default (extra=openmp), and the
# QNX toolchain has no libgomp; the Lottie expressions stay on.
MESON_ARGS_EXTRA=( --default-library=shared
                   -Dstatic=false
                   -Dloaders=svg,png,jpg,lottie -Dsavers=gif -Dtests=false
                   -Dbindings=capi -Dlog=false -Dthreads=false
                   -Dextra=lottie_exp )
