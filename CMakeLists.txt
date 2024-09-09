cmake_minimum_required(VERSION 3.10)
project(market_data_archiver)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(OpenSSL REQUIRED)
add_executable(market_data_archiver main.cpp) 
target_link_libraries(market_data_archiver
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)
