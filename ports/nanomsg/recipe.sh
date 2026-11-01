# nanomsg: the original socket library for scalability protocols (pair,
# pub/sub, req/rep, pipeline, survey, bus) over inproc, ipc, tcp and ws.
VERSION=1.3.0; GIT=https://github.com/nanomsg/nanomsg.git; REF=1.3.0
# The poller and the wake-up fd are picked from what CMake finds. On QNX,
# epoll_create and eventfd are the Linux-compatibility shims (libepoll,
# libeventfd); poll() and pipe() are native, and nanomsg has backends for both
# (the ones it uses on the BSDs without kqueue), so the epoll/eventfd probes are
# pre-answered "absent" -- the same choice the nng recipe makes.
CMAKE_ARGS_EXTRA=( -DNN_STATIC_LIB=OFF -DNN_TESTS=OFF -DNN_TOOLS=ON -DNN_ENABLE_DOC=OFF
                   -DNN_HAVE_EPOLL=OFF -DNN_HAVE_EVENTFD=OFF )
# The package version comes from `git describe`, which in a shallow clone is a
# commit hash (nanomsg.pc said "Version: 73e89d2", and the CMake package
# directory was named after it). The release's version is written to
# .version, which CMakeLists.txt reads when there is no .git directory.
do_patch() { rm -rf "$SRCROOT/.git" && echo "$VERSION" > "$SRCROOT/.version"; }
