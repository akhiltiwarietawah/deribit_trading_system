# websocket_client/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(websocket_client)

set(CMAKE_CXX_STANDARD 17)

# Include necessary libraries
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)

# Manually set LZ4 include and library directories 
link_directories(/usr/lib/x86_64-linux-gnu)
include_directories(/usr/include) 

include_directories(/usr/local/include/websocketpp)  # Ensure this path is correct

# Find and include nlohmann_json
find_package(nlohmann_json REQUIRED)
include_directories(${nlohmann_json_INCLUDE_DIRS})


add_executable(websocket_client websocket_client.cpp)

# Link necessary libraries
target_link_libraries(websocket_client ${Boost_LIBRARIES} 
                        ZLIB::ZLIB
                        lz4
                        ${OPENSSL_LIBRARIES} pthread ${CURL_LIBRARIES} )



# Print a message to indicate successful configuration 
message("Configuration successful!")