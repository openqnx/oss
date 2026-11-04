# openexr: the OpenEXR high-dynamic-range image file format (ILM/ASWF) -- the
# C core library, the C++ IlmImf/OpenEXR library, IlmThread and Iex, and the
# exr* tools (exrinfo, exrheader, exrmaketiled, exrenvmap, exrstdattr ...).
VERSION=3.4.1; GIT=https://github.com/AcademySoftwareFoundation/openexr.git; REF=v3.4.1
DEPS="imath libdeflate openjph"
BUILDSYS=cmake
# Built against the staged Imath, libdeflate and OpenJPH (HTJ2K compression):
# without them OpenEXR's configure downloads its own copies. Threading on;
# no examples, tests, Python bindings, docs or website.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF
                   -DOPENEXR_FORCE_INTERNAL_IMATH=OFF -DOPENEXR_FORCE_INTERNAL_DEFLATE=OFF
                   -DOPENEXR_FORCE_INTERNAL_OPENJPH=OFF
                   -DOPENEXR_BUILD_TOOLS=ON -DOPENEXR_INSTALL_TOOLS=ON
                   -DOPENEXR_BUILD_EXAMPLES=OFF -DOPENEXR_BUILD_PYTHON=OFF
                   -DOPENEXR_INSTALL_DOCS=OFF -DBUILD_WEBSITE=OFF -DOPENEXR_ENABLE_THREADING=ON )
