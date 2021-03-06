# get from https://code.google.com/r/antmakpub-leveldb/
cmake_minimum_required(VERSION 2.8)
PROJECT(leveldb)

ENABLE_TESTING()

set(VERSION_MAJOR 1)
set(VERSION_MINOR 18)
set(VERRION_PATCH 0)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} include)

# Set a default build type for single-configuration
# CMake generators if no build type is set.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE)
  #set(CMAKE_BUILD_TYPE DEBUG)
endif()

message("Configuration: ${CMAKE_BUILD_TYPE}")

## needs define:
##  COMPILER_MSVC
##  LEVELDB_PLATFORM_POSIX
##  LEVELDB_CSTDATOMIC_PRESENT
##  SNAPPY
##  OS_****

if(WIN32 AND MSVC)
  add_definitions(-DOS_WIN)
  add_definitions(-DLEVELDB_PLATFORM_WINDOWS)
  add_definitions(-DCOMPILER_MSVC)

  add_definitions(-DWINVER=0x0501 -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
  add_definitions(-Dssize_t=ptrdiff_t) #workaround for ssize_t

  set(PORT_SRCS port/port_win.cc util/env_win.cc util/win_logger.cc)
  set(PORT_HDRS port/port_win.h port/win/stdint.h util/win_logger.h)
  include_directories(port/win) #workaround for stdint.h

#  set(CAPI_SRC db/c.cc)
#  set(CAPI_HDRS include/leveldb/c.h)
  set(CAPI_SRC)
  set(CAPI_HDRS)
  set(CAPI_TEST)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /Oi /EHsc")
  #set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /MD")
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox /Ob2 /Ot /DNDEBUG /MD")
  set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /Ob0 /RTC1 /D_DEBUG /MDd")

  set(LIBS "${LIBS}" shlwapi)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-DOS_LINUX)
  add_definitions(-DLEVELDB_PLATFORM_POSIX)
  add_definitions(-DLEVELDB_ATOMIC_PRESENT)
  add_definitions(-fno-builtin-memcmp)

  set(PORT_SRCS port/port_posix.cc util/env_posix.cc)
  set(PORT_HDRS port/port_posix.h util/posix_logger.h)

  set(CAPI_SRC db/c.cc)
  set(CAPI_HDRS include/leveldb/c.h)
  set(CAPI_TEST db/c_test.c)

  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")

else()
  error("Unknown platform")
endif()

###
# embedded snappy
###
#add_definitions(-DSNAPPY)
#include_directories(snappy)
#set(SNAPPY_SRCS
#  snappy/snappy.cc
#  snappy/snappy-sinksource.cc
#  snappy/snappy-stubs-internal.cc
#  )
#set(SNAPPY_HDRS
#  snappy/snappy.h
#  snappy/snappy-internal.h
#  snappy/snappy-sinksource.h
#  snappy/snappy-stubs-internal.h
#  snappy/snappy-stubs-public.h
#  )

set(SRCS
  db/builder.cc
  db/db_impl.cc
  db/db_iter.cc
  db/dbformat.cc
  db/filename.cc
  db/log_reader.cc
  db/log_writer.cc
  db/memtable.cc
  db/repair.cc
  db/table_cache.cc
  db/version_edit.cc
  db/version_set.cc
  db/write_batch.cc
  helpers/memenv/memenv.cc
  table/block_builder.cc
  table/block.cc
  table/filter_block.cc
  table/format.cc
  table/iterator.cc
  table/merger.cc
  table/table_builder.cc
  table/table.cc
  table/two_level_iterator.cc
  util/arena.cc
  util/bloom.cc
  util/cache.cc
  util/coding.cc
  util/comparator.cc
  util/crc32c.cc
  util/env.cc
  util/filter_policy.cc
  util/hash.cc
  util/histogram.cc
  util/logging.cc
  util/options.cc
  util/status.cc
  ${PORT_SRCS}
  ${CAPI_SRC}
  )

set(HDRS
  db/builder.h
  db/db_impl.h
  db/db_iter.h
  db/dbformat.h
  db/filename.h
  db/log_format.h
  db/log_reader.h
  db/log_writer.h
  db/memtable.h
  db/skiplist.h
  db/snapshot.h
  db/table_cache.h
  db/version_edit.h
  db/version_set.h
  db/write_batch_internal.h
  helpers/memenv/memenv.h
  include/leveldb/cache.h
  include/leveldb/comparator.h
  include/leveldb/db.h
  include/leveldb/env.h
  include/leveldb/filter_policy.h
  include/leveldb/iterator.h
  include/leveldb/options.h
  include/leveldb/slice.h
  include/leveldb/status.h
  include/leveldb/table.h
  include/leveldb/table_builder.h
  include/leveldb/write_batch.h
  port/atomic_pointer.h
  port/port.h
  port/thread_annotations.h
  table/block.h
  table/block_builder.h
  table/filter_block.h
  table/format.h
  table/iterator_wrapper.h
  table/merger.h
  table/two_level_iterator.h
  util/arena.h
  util/coding.h
  util/crc32c.h
  util/hash.h
  util/histogram.h
  util/logging.h
  util/mutexlock.h
  util/random.h
  ${PORT_HDRS}
  ${CAPI_HDRS}
  )

add_library(leveldb SHARED ${SRCS} ${HDRS} ${SNAPPY_SRCS} ${SNAPPY_HDRS})
set_target_properties(leveldb PROPERTIES
  VERSION ${VERSION_STRING}
  SOVERSION ${VERSION_MAJOR}
  )

add_library(leveldb_static STATIC ${SRCS} ${HDRS} ${SNAPPY_SRCS} ${SNAPPY_HDRS})

find_package(Threads)
target_link_libraries(leveldb ${CMAKE_THREAD_LIBS_INIT} ${LIBS})
target_link_libraries(leveldb_static ${CMAKE_THREAD_LIBS_INIT} ${LIBS})

