cmake_minimum_required(VERSION 3.22)
project(demonware LANGUAGES CXX)
file(GLOB_RECURSE demonware RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.inl)

add_library(demonware
  ${demonware}
)

target_include_directories(demonware
PUBLIC
  include/demonware
)

target_compile_definitions(demonware
PRIVATE
  LTM_DESC LTC_SOURCE USE_LTM
  "_CRT_SECURE_NO_WARNINGS"
)

target_link_libraries(demonware
PUBLIC
  ws2_32.lib libtomcrypt libtommath
)

add_subdirectory(externals)
