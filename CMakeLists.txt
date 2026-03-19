cmake_minimum_required(VERSION 3.14)
project(xfind C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Strict warnings for GCC/Clang (Ubuntu target)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(SRC ${CMAKE_SOURCE_DIR}/src)

# Uppercase .C extension: force C language (otherwise CMake maps .C -> C++)
set(ALL_SRCS
    ${SRC}/MXFIDX.C
    ${SRC}/MXFIND.C
    ${SRC}/FSWALK.C
    ${SRC}/INDEX.C
    ${SRC}/MATCH.C
    ${SRC}/ACTIONS.C
    ${SRC}/CONFIG.C
    ${SRC}/UITUI.C
)
set_source_files_properties(${ALL_SRCS} PROPERTIES LANGUAGE C)

# xfidx
add_executable(xfidx
    ${SRC}/MXFIDX.C
    ${SRC}/FSWALK.C
    ${SRC}/INDEX.C
    ${SRC}/CONFIG.C
)
set_target_properties(xfidx PROPERTIES LINKER_LANGUAGE C)
target_include_directories(xfidx PRIVATE ${SRC})

# xfind
add_executable(xfind
    ${SRC}/MXFIND.C
    ${SRC}/INDEX.C
    ${SRC}/MATCH.C
    ${SRC}/ACTIONS.C
    ${SRC}/CONFIG.C
    ${SRC}/UITUI.C
)
set_target_properties(xfind PROPERTIES LINKER_LANGUAGE C)
target_include_directories(xfind PRIVATE ${SRC})

# Install
install(TARGETS xfidx xfind
    RUNTIME DESTINATION bin
)
