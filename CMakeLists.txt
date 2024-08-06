cmake_minimum_required(VERSION 3.10)

project(ExrToJxr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(${PROJECT_NAME} src/main.cpp) 
find_package(OpenImageIO CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenImageIO::OpenImageIO)
target_link_libraries(${PROJECT_NAME} PRIVATE windowscodecs)