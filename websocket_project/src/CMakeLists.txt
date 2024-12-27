# src/CMakeLists.txt
add_executable(websocket_server websocket_server.cpp)

# Include necessary libraries
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

include_directories(/usr/local/include/websocketpp)  # Ensure this path is correct

# Link necessary libraries
target_link_libraries(websocket_server ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} pthread ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)
