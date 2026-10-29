VERSION=0.15.8; GIT=https://github.com/thorvg/thorvg.git; REF=v0.15.8
DEPS="libjpeg-turbo libpng zlib"
BUILDSYS=meson
# --default-library=both (the driver's default) makes ThorVG define TVG_STATIC,
# which turns TVG_API into nothing at all -- the shared library then exports no
# tvg_* symbol and every consumer fails to link. Build shared only.
MESON_ARGS_EXTRA=( --default-library=shared
                   -Dstatic=false
                   -Dloaders=svg,png,jpg,lottie -Dsavers=gif -Dtests=false
                   -Dexamples=false -Dbindings=capi -Dlog=false -Dthreads=false )
