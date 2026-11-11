#!/bin/sh
# native-toolchain: compile, link and run programs on the target itself.
# cc/gcc/c++/g++ are clang with QNX's headers and libraries from
# <prefix>/qnx-sysroot (copied from the user's SDP by scripts/target-sysroot;
# without it this test fails and says so). Known answers:
#  - C: a thread computes sqrt(16)*10 = 40 (pthread, libm), QNX's own API
#    (<sys/neutrino.h>, ClockTime) compiles and runs; -O2 gives the same;
#  - C++20: std::vector/std::string/iostream, an exception thrown and caught,
#    a std::thread; it is libc++ (_LIBCPP_VERSION) and needs libc++.so.2 --
#    the ABI of the C++ components here;
#  - a shared library (-fPIC -shared) and a static archive (ar) link and run;
#  - a ported C++ library through pkg-config: fmt::format("{}-{}", 6*7,
#    std::string("qnx")) is "42-qnx" (a std::string crosses the boundary);
#  - make with a Makefile, and cmake + ninja with find_package(fmt).
echo "== native-toolchain =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }
P=/usr/local
for t in cc gcc c++ g++ clang ld pkg-config make; do
  [ -x "$P/bin/$t" ] && ok "$t is installed" || bad "no $P/bin/$t"
done
if [ ! -f "$P/qnx-sysroot/usr/include/stdio.h" ] || [ ! -f "$P/qnx-sysroot/usr/lib/crt1.o" ]; then
  bad "no QNX sysroot in $P/qnx-sysroot: run scripts/target-sysroot <target> on the host (QNX's headers and libraries come from your SDP)"
  echo "FAIL native-toolchain ($fail check(s) failed)"; exit 1
fi
T=$(mktemp -d /tmp/qnxport-ntc.XXXXXX) || { echo "FAIL native-toolchain (mktemp)"; exit 1; }
cd "$T" || exit 1
say() { head -c 600 "$1" | tr '\n' ' '; }

clang --version > v.txt 2>&1
grep -q "Configuration file: .*clang.cfg" v.txt && ok "clang reads clang.cfg ($(sed -n 's/^Target: //p' v.txt))" || bad "clang --version: $(say v.txt)"

cat > hello.c <<'CEOF'
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/neutrino.h>
static void *run(void *a) { return (void *)(long)(sqrt((double)(long)a) * 10); }
int main(void) {
    pthread_t t; void *r; uint64_t now = 0;
    pthread_create(&t, NULL, run, (void *)16L); pthread_join(t, &r);
    ClockTime(CLOCK_MONOTONIC, NULL, &now);
    printf("result=%ld clock=%d qnx=%d clang=%d\n", (long)r, now > 0, __QNX__ > 0, __clang__);
    return 0;
}
CEOF
for cc in cc gcc; do
  $cc -Wall -o hello-$cc hello.c > err.txt 2>&1; out=$(./hello-$cc 2>&1)
  [ "$out" = "result=40 clock=1 qnx=1 clang=1" ] && [ ! -s err.txt ] && ok "$cc: C with a thread, libm and ClockTime: $out" \
    || bad "$cc hello.c: '$out' $(say err.txt)"
done
cc -O2 -g -pthread -o hello-O2 hello.c -lm > err.txt 2>&1; out=$(./hello-O2 2>&1)
[ "$out" = "result=40 clock=1 qnx=1 clang=1" ] && ok "cc -O2 -g -pthread ... -lm: same answer" || bad "cc -O2: '$out' $(say err.txt)"
cc -c hello.c -o hello.o > err.txt 2>&1 && nm hello.o | grep -q ' T main$' && ok "cc -c, then nm: T main" || bad "cc -c / nm: $(say err.txt)"

cat > hello.cpp <<'CEOF'
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
int main() {
    std::vector<int> v(10); std::iota(v.begin(), v.end(), 1);
    int sum = 0; std::thread t([&] { sum = std::accumulate(v.begin(), v.end(), 0); }); t.join();
    std::string caught;
    try { throw std::runtime_error("boom"); } catch (const std::exception &e) { caught = e.what(); }
    std::cout << "sum=" << sum << " caught=" << caught << " libcxx="
#ifdef _LIBCPP_VERSION
              << 1
#else
              << 0
#endif
              << " std=" << __cplusplus << "\n";
}
CEOF
for cxx in c++ g++; do
  $cxx -std=c++20 -Wall -o hello-$cxx hello.cpp > err.txt 2>&1; out=$(./hello-$cxx 2>&1)
  [ "$out" = "sum=55 caught=boom libcxx=1 std=202002" ] && [ ! -s err.txt ] && ok "$cxx -std=c++20: $out" \
    || bad "$cxx hello.cpp: '$out' $(say err.txt)"
done
llvm-readelf -d hello-c++ 2>/dev/null | grep -q 'libc++.so.2' && ok "the C++ program needs libc++.so.2 (the SDP's libc++)" \
  || bad "hello-c++ does not need libc++.so.2: $(llvm-readelf -d hello-c++ 2>&1 | grep NEEDED | tr '\n' ' ')"

cat > sq.c <<'CEOF'
int square(int x) { return x * x; }
CEOF
cat > usesq.c <<'CEOF'
#include <stdio.h>
int square(int);
int main(void) { printf("%d\n", square(12)); return 0; }
CEOF
cc -fPIC -shared -o libsq.so sq.c > err.txt 2>&1 && cc -o usesq-so usesq.c -L. -lsq >> err.txt 2>&1
out=$(LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./usesq-so 2>&1)
[ "$out" = 144 ] && ok "shared library (-fPIC -shared), linked and loaded: square(12) = 144" || bad "shared library: '$out' $(say err.txt)"
cc -c sq.c -o sq.o > err.txt 2>&1 && ar rcs libsqa.a sq.o >> err.txt 2>&1 && cc -o usesq-a usesq.c libsqa.a >> err.txt 2>&1
out=$(./usesq-a 2>&1)
[ "$out" = 144 ] && ok "static archive (ar rcs), linked: square(12) = 144" || bad "static archive: '$out' $(say err.txt)"
cc -fuse-ld=lld -o usesq-lld usesq.c sq.c > err.txt 2>&1; out=$(./usesq-lld 2>&1)
[ "$out" = 144 ] && ok "cc -fuse-ld=lld links too" || bad "-fuse-ld=lld: '$out' $(say err.txt)"

cat > usefmt.cpp <<'CEOF'
#include <fmt/format.h>
#include <iostream>
#include <string>
int main() { std::string s = fmt::format("{}-{}", 6 * 7, std::string("qnx")); std::cout << s << "\n"; }
CEOF
fl=$(pkg-config --cflags --libs fmt 2>err.txt)
[ -n "$fl" ] && ok "pkg-config --cflags --libs fmt: $fl" || bad "pkg-config fmt: $(say err.txt)"
c++ -std=c++17 -o usefmt usefmt.cpp $fl > err.txt 2>&1; out=$(./usefmt 2>&1)
[ "$out" = "42-qnx" ] && ok "a ported C++ library (fmt) through pkg-config: $out" || bad "fmt: '$out' $(say err.txt)"

mkdir mk && cp usesq.c sq.c mk/ && printf 'CFLAGS = -O2 -Wall\nusesq: usesq.o sq.o\n\t$(CC) -o $@ usesq.o sq.o\nclean:\n\trm -f usesq *.o\n' > mk/Makefile
make -C mk > err.txt 2>&1; out=$(./mk/usesq 2>&1)
[ "$out" = 144 ] && grep -q '^cc -O2 -Wall' err.txt && ok "make: the built-in rules call cc; the program prints 144" || bad "make: '$out' $(say err.txt)"

if [ -x "$P/bin/cmake" ] && [ -x "$P/bin/ninja" ]; then
  mkdir cm && cp usefmt.cpp usesq.c sq.c cm/ && cat > cm/CMakeLists.txt <<'CEOF'
cmake_minimum_required(VERSION 3.20)
project(ntc C CXX)
find_package(fmt REQUIRED)
add_library(sq SHARED sq.c)
add_executable(usesq usesq.c)
target_link_libraries(usesq sq)
add_executable(usefmt usefmt.cpp)
target_link_libraries(usefmt fmt::fmt)
CEOF
  cmake -G Ninja -S cm -B cm/b > err.txt 2>&1 && ok "cmake -G Ninja configures ($(sed -n 's/^-- The CXX compiler identification is //p' err.txt))" || bad "cmake: $(tail -5 err.txt | tr '\n' ' ')"
  ninja -C cm/b > err.txt 2>&1 && ok "ninja builds a shared library and two programs" || bad "ninja: $(tail -5 err.txt | tr '\n' ' ')"
  o1=$(./cm/b/usesq 2>&1); o2=$(./cm/b/usefmt 2>&1)
  [ "$o1" = 144 ] && [ "$o2" = "42-qnx" ] && ok "the cmake-built programs run: $o1, $o2" || bad "cmake-built programs: '$o1' '$o2'"
else
  bad "cmake or ninja is not installed"
fi

cd /; rm -rf "$T"
[ $fail -eq 0 ] && { echo "PASS native-toolchain"; exit 0; }
echo "FAIL native-toolchain ($fail check(s) failed)"; exit 1
