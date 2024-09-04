set(CMAKE_BINARY_DIR               ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_INSTALL_PREFIX           ${PROJECT_SOURCE_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(SurrogateCommon)

add_executable(${PROJECT_NAME}
    src/main.cc
)

target_link_libraries(${PROJECT_NAME}
    /workspace/libs/libnvram.so
    /workspace/libs/libwlcsm.so
    /workspace/libs/libshared.so
    surrogate
    stdc++
    stdc++fs
)

install(
    DIRECTORY data/etc data/lib
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)

# This target requires 'make install' to be called first.
add_custom_target(dist
    WORKING_DIRECTORY dist
    COMMAND ${CMAKE_COMMAND} -E tar czf ../surrogate.tar.gz .
)

