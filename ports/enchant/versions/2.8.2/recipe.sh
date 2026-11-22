VERSION=2.8.2
# The release tarball, not the git tag.  Enchant's git tree carries gnulib and
# gl-mod/bootstrap as submodules and expects ./bootstrap to run gnulib-tool;
# the tarball ships a generated configure and the gnulib sources it uses, so
# no submodule clone (and no gnulib-tool here) is needed.  README hazard 74.
TARBALL=https://github.com/AbiWord/enchant/releases/download/v2.8.2/enchant-2.8.2.tar.gz
SHA256=8f19535adb5577b83b00e02f330fe9b9eb40dd21f19e2899636fc4d3a7696375
DEPS="glib aspell"
BUILDSYS=autotools
# --enable-relocatable: enchant dlopens its providers from a path it composes
# from the compiled-in PKGLIBDIR ("/usr/local/lib/enchant-2").  Without
# relocation that path is absolute, so the library finds no provider at all
# unless the tree is installed at exactly the configured prefix -- and the
# on-target test bundle is deliberately relocatable.  gnulib's relocate() then
# derives the directory from where the library actually is.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --enable-relocatable
                  --with-aspell --without-hunspell
                  --without-nuspell --without-voikko --without-hspell --without-applespell )

# lib/Makefile.am forces config.h in with GCC's long-form "--include config.h".
# qcc parses its own command line first and rejects the long form outright
# ("cc: unknown option: '--include'"); -include is the universally accepted
# spelling and is exactly equivalent.  Applied to Makefile.in too so the
# tarball builds without re-running automake.
# Two build-machine fixes, both in lib/Makefile.{am,in} (patched in the .in too
# so the release tarball builds without re-running automake):
#
#  1. AM_CPPFLAGS forces config.h in with GCC's long-form "--include config.h".
#     qcc parses its own command line before handing the rest to gcc and rejects
#     the long form outright ("cc: unknown option: '--include'").  -include is
#     the universally accepted spelling and is exactly equivalent.
#  2. nodist_doc_DATA runs "groff -mandoc -Thtml" to render the man page.  The
#     html device needs groff's full install, which the build machine need not
#     have, and the output is documentation for the build host -- nothing the
#     target uses.  The man page itself is still installed.
do_patch() {
  cd "$SRCROOT" || return 1
  # 1. AM_CPPFLAGS forces config.h in with GCC's long-form "--include config.h".
  #    qcc parses its own command line before handing the rest to gcc and
  #    rejects the long form outright ("cc: unknown option: '--include'").
  #    -include is the universally accepted spelling and is equivalent.
  # 2. nodist_doc_DATA renders the man pages to HTML with "groff -Thtml".  The
  #    html device is not in groff-base, it is documentation for the *build*
  #    machine, and nothing on the target reads it.  The man pages themselves
  #    are still installed.  Patched in Makefile.in as well as Makefile.am so
  #    the release tarball builds without re-running automake.
  sed -i -e 's/--include config\.h/-include config.h/' \
         -e 's/^nodist_doc_DATA = .*/nodist_doc_DATA =/' \
         lib/Makefile.am lib/Makefile.in src/Makefile.am src/Makefile.in
}
