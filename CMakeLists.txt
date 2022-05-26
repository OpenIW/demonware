cmake_minimum_required(VERSION 3.17)

# Compatibility with Microsoft Visual Studio 2008
set(CMAKE_GENERATOR_TOOLSET v90)
set(CMAKE_GENERATOR_PLATFORM win32)

project(demonware LANGUAGES CXX)

set(MASTER_PROJECT OFF)
# Determine if we're built as a subproject (using add_subdirectory)
# or if this is the master project.
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MASTER_PROJECT ON)
endif()

# source_group following the directory hierarchy
function(add_virtual_path target_name)
    get_target_property(target_sources  "${target_name}" SOURCES)
    foreach(file_name IN LISTS target_sources)
        get_filename_component(dir_name "${file_name}" PATH)
        string(REPLACE "/" "\\" group_name "${dir_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()

include_directories(. 
    external/libtomcrypt/src/headers 
)

add_definitions(-DLTM_DESC)

add_compile_options(
    /MP                      # Build with Multiple Processes
    $<$<CONFIG:Debug>:/Zi>   # The /Zi option produces a separate PDB file that contains all the symbolic debugging information for use with the debugger.
)

file(GLOB_RECURSE SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.inl)
list(FILTER SRC EXCLUDE REGEX "external")
list(FILTER SRC EXCLUDE REGEX "build")

add_library(demonware STATIC
    ${SRC}
)

target_compile_definitions(demonware PUBLIC
    "_DEBUG"
    "_WINDOWS"
    "_CRT_SECURE_NO_WARNINGS"
)

target_link_libraries(demonware ws2_32.lib libtomcrypt libtommath)

add_virtual_path(demonware)

add_subdirectory(external/libtommath)
add_subdirectory(external/libtomcrypt)
