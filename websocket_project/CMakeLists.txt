cmake_minimum_required(VERSION 3.10)
project(websocket_project)

set(CMAKE_CXX_STANDARD 17)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

# Find OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Find cURL
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)

# Manually set LZ4 include and library directories 
include_directories(/usr/include) 
link_directories(/usr/lib/x86_64-linux-gnu)

# Include WebSocket++ directory
include_directories(/usr/local/include/websocketpp)  # Ensure this path is correct

# Specify the source file
add_executable(websocket_project src/websocket_server.cpp)

# Link Boost libraries, OpenSSL, pthread, and cURL
target_link_libraries(websocket_project ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} 
                        ZLIB::ZLIB
                       lz4
                        pthread ${CURL_LIBRARIES})


# Print a message to indicate successful configuration 
message("Configuration successful!")