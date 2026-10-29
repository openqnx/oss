# Included after project() (CMAKE_PROJECT_INCLUDE). Cap'n Proto names its C++
# sources *.c++, which qcc/q++ do not recognise as C++: they produce no object
# and report nothing. Insert "-x c++" into CMake's (QCC) compile rule so the
# language is explicit; link rules are left alone.
string(REPLACE "<FLAGS>" "<FLAGS> -x c++" CMAKE_CXX_COMPILE_OBJECT "${CMAKE_CXX_COMPILE_OBJECT}")
