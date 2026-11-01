# riegeli is built with Bazel upstream; this is the same library as one CMake
# shared library, libriegeli.so, plus the describe_riegeli_file command.
# Every C++ package of riegeli/ except tensorflow/ (TensorFlow ops) and the
# benchmark goes in; the Bazel BUILD files list the same sources.
cmake_minimum_required(VERSION 3.16)
project(riegeli CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(GNUInstallDirs)

set(RIEGELI_SOURCE_DIR "" CACHE PATH "riegeli source tree")
set(PROTOC "" CACHE FILEPATH "protoc that runs on the build machine")
set(R "${RIEGELI_SOURCE_DIR}")

find_package(absl REQUIRED CONFIG)
find_package(Protobuf REQUIRED CONFIG)
foreach(l brotlienc brotlidec brotlicommon zstd snappy lzma z bz2 lz4 highwayhash)
  find_library(LIB_${l} NAMES ${l} REQUIRED)
endforeach()

# Generated protobuf code (records metadata; the summary for the tool).
function(riegeli_proto out rel)
  get_filename_component(d "${rel}" DIRECTORY)
  get_filename_component(n "${rel}" NAME_WE)
  set(cc "${CMAKE_BINARY_DIR}/${d}/${n}.pb.cc")
  set(h "${CMAKE_BINARY_DIR}/${d}/${n}.pb.h")
  add_custom_command(OUTPUT "${cc}" "${h}"
    COMMAND "${PROTOC}" -I "${R}" -I "${Protobuf_INCLUDE_DIR}" --cpp_out "${CMAKE_BINARY_DIR}" "${R}/${rel}"
    DEPENDS "${R}/${rel}")
  set(${out} "${cc}" PARENT_SCOPE)
endfunction()
get_target_property(Protobuf_INCLUDE_DIR protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
list(GET Protobuf_INCLUDE_DIR 0 Protobuf_INCLUDE_DIR)
riegeli_proto(META_PB riegeli/records/records_metadata.proto)
riegeli_proto(SUMMARY_PB riegeli/records/tools/riegeli_summary.proto)

set(DIRS base brotli bytes bzip2 chunk_encoding containers csv digests endian
         lines lz4 messages ordered_varint records snappy text varint xz zlib zstd)
set(SRCS)
foreach(d ${DIRS})
  file(GLOB s "${R}/riegeli/${d}/*.cc")
  list(APPEND SRCS ${s})
endforeach()
# ReaderCFile()/WriterCFile() (a Reader or Writer as a stdio FILE*) are made
# with glibc's fopencookie(), which QNX's C library does not have (nor BSD's
# funopen()): left out, with their headers.
list(FILTER SRCS EXCLUDE REGEX "/bytes/(reader|writer)_cfile\\.cc$")

# snappy is built without RTTI (its CMake adds -fno-rtti), so its Source and
# Sink classes have no typeinfo for riegeli's subclasses of them to refer to:
# those (snappy_streams.cc, where their virtual functions are defined) are
# compiled without RTTI as well; they use none.
set_source_files_properties("${R}/riegeli/snappy/snappy_streams.cc" PROPERTIES COMPILE_OPTIONS -fno-rtti)

add_library(riegeli SHARED ${SRCS} ${META_PB})
set_target_properties(riegeli PROPERTIES VERSION 0.0.0 SOVERSION 0)
target_include_directories(riegeli PUBLIC $<BUILD_INTERFACE:${R}> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
                                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(riegeli PUBLIC
  absl::base absl::core_headers absl::dynamic_annotations absl::config absl::strings absl::str_format
  absl::cord absl::status absl::statusor absl::crc32c absl::flat_hash_map absl::flat_hash_set
  absl::node_hash_map absl::node_hash_set absl::inlined_vector absl::hash absl::any_invocable
  absl::function_ref absl::type_traits absl::bits absl::optional absl::variant absl::span
  absl::compare absl::utility absl::synchronization absl::time absl::memory
  protobuf::libprotobuf
  # Public: riegeli headers call the compression libraries inline.
  ${LIB_brotlienc} ${LIB_brotlidec} ${LIB_brotlicommon} ${LIB_zstd} ${LIB_snappy} ${LIB_lzma}
          ${LIB_z} ${LIB_bz2} ${LIB_lz4} ${LIB_highwayhash})

add_executable(describe_riegeli_file "${R}/riegeli/records/tools/describe_riegeli_file.cc" ${SUMMARY_PB})
target_link_libraries(describe_riegeli_file PRIVATE riegeli absl::flags absl::flags_parse absl::flags_usage)

install(TARGETS riegeli describe_riegeli_file
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY "${R}/riegeli" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.proto" PATTERN "tensorflow" EXCLUDE
  PATTERN "reader_cfile.h" EXCLUDE PATTERN "writer_cfile.h" EXCLUDE)
install(FILES "${CMAKE_BINARY_DIR}/riegeli/records/records_metadata.pb.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/riegeli/records)
