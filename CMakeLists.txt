cmake_minimum_required(VERSION 3.15)
project(terminal_api_poc CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(lib)

file(COPY crt/test.crt DESTINATION .)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(
        terminal_api_poc
        main.cpp
        httpRequest.cpp
        terminalRequest.cpp
        POI.cpp
        nexoCrypto.cpp
        base64.cpp
        escapeJson.cpp
)

include_directories(${CURL_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories("/usr/local/data/include/")

target_link_libraries(
        terminal_api_poc
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        "/usr/local/data/lib/libjsoncpp.a"
)

