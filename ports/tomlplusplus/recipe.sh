VERSION=3.5.1; GIT=https://github.com/marzer/tomlplusplus.git; REF=v3.5.1
# Header-only (the default; tomlplusplus_BUILD_LIB stays off). From a git
# checkout 3.5 reruns tools/regenerate.py at configure time to rewrite the
# shipped toml.hpp with whatever python3 cmake finds: install the headers as
# tagged instead.
CMAKE_ARGS_EXTRA=( -Dtomlplusplus_REGENERATE=OFF )
