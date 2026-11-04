# md4c: a fast CommonMark Markdown parser in C (SAX-like callbacks, with
# GitHub extensions: tables, strikethrough, task lists, autolinks), plus
# libmd4c-html (Markdown to HTML) and the md2html program.
VERSION=0.6.0; GIT=https://github.com/mity/md4c.git; REF=v0.6.0
BUILDSYS=cmake
# The upstream defaults: shared libraries, libmd4c-html and md2html built.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_HTML_LIBRARY=ON -DBUILD_MD2HTML_EXECUTABLE=ON )
