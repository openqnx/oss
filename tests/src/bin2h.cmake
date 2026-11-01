# cmake -DIN=<file> -DOUT=<header> -DNAME=<c name> -P bin2h.cmake
# Writes <file> as a C array (the n6-libs pybind11 test carries its module).
file(READ "${IN}" hex HEX)
string(LENGTH "${hex}" n)
math(EXPR len "${n} / 2")
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," body "${hex}")
string(REGEX REPLACE "((0x..,){32})" "\\1\n" body "${body}")
file(WRITE "${OUT}" "static const unsigned char ${NAME}[] = {\n${body}\n};\nstatic const size_t ${NAME}_len = ${len};\n")
