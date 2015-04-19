add_definitions(-Wextra -Wpedantic)
set(SRC Server.cpp
        Storage.cpp
        Session.cpp
)
set(HEDERS Server.h
           Storage.h
           Session.h
)

add_library(core ${SRC} ${HEADERS})
target_link_libraries(core leveldb_static)

add_executable(tmdb main.cpp)
target_link_libraries(tmdb core ${Boost_LIBRARIES} glog)