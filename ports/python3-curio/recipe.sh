# curio: coroutine-based concurrency (tasks, sockets, queues) without asyncio.
# Pure Python. The last release, 1.6 (2022), does not import on Python 3.12+
# (ssl.wrap_socket is gone, multiprocessing's handshake constants renamed);
# the project's final commit (2025-12-21, "Final farewell") supports them:
# built from it into a platform-independent wheel (scripts/lib/pywheel.sh).
VERSION=1.6_git20251221; GIT=https://github.com/dabeaz/curio.git; COMMIT=c29701fbb8d0a7b8f28ea60639b36b75f5550366
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_build() { qnx_py_wheel_pure; }
do_install() { qnx_py_install; }
