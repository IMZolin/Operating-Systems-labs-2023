cmake_minimum_required(VERSION 3.22)
project(lab3 VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Werror")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_COMPILE_FEATURES "-latomic")
find_package (Threads REQUIRED)
find_library (LIBRT rt)

add_executable(lab3 main.cpp) 
include_directories(test)
include_directories(queue)
target_sources(lab3 PRIVATE test/test.cpp)

target_link_libraries(lab3 PUBLIC pthread rt atomic)