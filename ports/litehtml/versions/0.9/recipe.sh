# litehtml: a lightweight HTML/CSS rendering engine (layout of HTML 4/5 and
# CSS 2.1 with parts of CSS3); the application supplies drawing and fonts
# through a document_container. Used by Qt Creator's help viewer, Notepad++
# plugins, the Haiku/Gemini browsers ...
VERSION=0.9; GIT=https://github.com/litehtml/litehtml.git; REF=v0.9
DEPS="gumbo-parser"
BUILDSYS=cmake
# The staged gumbo-parser instead of the bundled copy (one HTML5 parser on
# the target); a shared library; no tests (they download GoogleTest).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DEXTERNAL_GUMBO=ON -DLITEHTML_BUILD_TESTING=OFF -DBUILD_TESTING=OFF )
