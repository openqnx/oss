VERSION=2.1.2; GIT=https://github.com/neovim/unibilium.git; REF=v2.1.2
BUILDSYS=cmake
# TERMINFO_DIRS: the compiled-in search path. Left alone, CMakeLists asks the
# BUILD HOST's ncurses6-config and bakes in the host's directories
# (/etc/terminfo:/lib/terminfo:/usr/share/terminfo), none of which holds this
# tree's database on the target: the ncurses component installs it in
# /usr/local/share/terminfo. TERMINFO / TERMINFO_DIRS still override it.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON
                   -DTERMINFO_DIRS=/usr/local/share/terminfo:/usr/share/terminfo:/etc/terminfo:/usr/lib/terminfo )
