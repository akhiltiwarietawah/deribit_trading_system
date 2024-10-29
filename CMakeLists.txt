

cmake_minimum_required(VERSION 3.10)

project(deribit_trading_system)



set(CMAKE_CXX_STANDARD 14)


# Integrate vcpkg

# Set Boost paths


# Set jsoncpp path
set(jsoncpp_DIR "/home/akhil/dev-tools/vcpkg/installed/x64-linux/share/jsoncpp")

# Set websocketpp path

# Include standard C++ library paths explicitly
include_directories("/usr/include/c++/11")
include_directories("/usr/include/x86_64-linux-gnu/c++/11")


find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ZLIB REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)


add_executable(deribit_trading_system main.cpp auth.cpp order_management.cpp utils.cpp)

target_link_libraries(deribit_trading_system 
                    OpenSSL::Crypto OpenSSL::SSL 
                    CURL::libcurl 
                    nlohmann_json::nlohmann_json 
                    ZLIB::ZLIB
                    jsoncpp_static)