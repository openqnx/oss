VERSION=0.22.5
TARBALL=https://ftp.gnu.org/gnu/gettext/gettext-0.22.5.tar.gz
SHA256=ec1705b1e969b83a9f073144ec806151db88127f5e40fe5a94cb6c8fa48996a0
BUILDSYS=autotools
# ncurses: libtextstyle links it for terminal styling. Undeclared, gettext built
# before ncurses in a from-scratch run and picked up the SDP's prebuilt copy
# (soname libncursesw.so.1 against our .so.6). ncurses itself needs nothing here,
# so there is no cycle.
# NOT libxml2: this builds --with-included-libxml, so gettext uses its own
# bundled copy and never links ours. Declaring it created the workspace's only
# dependency cycle -- xz -> gettext -> libxml2 -> xz -- which was invisible
# while everything was already built (the stamps short-circuit the recursion)
# and recursed until the shell segfaulted on the first from-scratch build on a
# new machine.
# Also linked at run time (found by scripts/gen-dependency-index): libiconv libunistring
DEPS="ncurses libiconv libunistring"
# --enable-nls (was --disable-nls): with NLS off, gettext installs no libintl at
# all, and 56 staged libraries silently linked the SDP's prebuilt libintl.so.1
# instead -- the one thing this workspace must never do.  scripts/check-sdp-leak
# could not see it, because it only flagged sonames we had also staged; it now
# works from an allowlist of genuine OS libraries.
#
# gt_cv_func_printf_posix=yes: gettext asks whether printf supports positional
# arguments ("%2$s") by RUNNING a probe, which a cross build cannot do, so it
# assumes no and its <libintl.h> then does "#define printf libintl_printf".
# That macro renames members of any C++ class with a printf method -- WebKit's
# WTF::PrintStream fails to compile against the SDP's header for exactly this
# reason.  QNX does support positional arguments; verified on the target:
#   snprintf(b, n, "%2$s-%1$s", "a", "b")  ->  "b-a"
CONF_ENV=( gt_cv_func_printf_posix=yes )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-java --disable-csharp
                  --enable-nls --without-emacs --disable-openmp --with-included-libxml )
