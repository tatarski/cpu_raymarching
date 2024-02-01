project(raymarching)

set(SOURCES
    "utils.inl"
    "main.cpp"
 "raymarching.h")

set(HEADERS
    "utils.h"
 "Image.h" "raymarching.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} "Image.h" "raymarching.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET raymarching PROPERTY CXX_STANDARD 20)
endif()