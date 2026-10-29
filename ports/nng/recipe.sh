VERSION=1.9.0; GIT=https://github.com/nanomsg/nng.git; REF=v1.9.0
# QNX 8 epoll (libepoll) has no EPOLLONESHOT, which nng's epoll poller is
# built on. nng picks its poller from what it detects, so pre-answer the epoll
# probe as "absent" and it falls back to the portable poll() poller.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DNNG_TESTS=OFF -DNNG_TOOLS=ON -DNNG_ENABLE_NNGCAT=ON
                   -DNNG_HAVE_EPOLL=OFF )
