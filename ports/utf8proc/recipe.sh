VERSION=2.11.3; GIT=https://github.com/JuliaStrings/utf8proc.git; REF=v2.11.3
# 2.11 for Neovim 0.12, which uses ambiguous_width (the version Neovim bundles).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DUTF8PROC_ENABLE_TESTING=OFF
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
