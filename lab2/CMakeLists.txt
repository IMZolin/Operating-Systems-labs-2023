cmake_minimum_required(VERSION 3.22)
project(host_client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror -DCMAKE_EXE_LINKER_FLAGS=-latomic)
find_package (Threads)
find_library (LIBRT rt)

set(CLIENT client)
set(HOST host)
set(CONNECTION connections)
set(UTILS utils)

foreach(CONN mq fifo sock)
    add_executable(host_${CONN} ${HOST}/hostHandler.cpp ${HOST}/hostHandler.h ${HOST}/host.cpp
            ${CLIENT}/client.cpp ${CLIENT}/client.h
            ${UTILS}/configuration.h ${UTILS}/message.h
            ${CONNECTION}/conn.h ${CONNECTION}/conn_${CONN}.cpp ${CONNECTION}/conn_${CONN}.h)
    target_link_libraries(host_${CONN} PUBLIC rt pthread)
endforeach(CONN mq fifo sock)