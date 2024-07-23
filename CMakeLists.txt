cmake_minimum_required(VERSION 3.16)

# Set the project name
project(zenoh-cpp-example)

find_package(spdlog REQUIRED)

include(FetchContent)

FetchContent_Declare(zenohc GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-c GIT_TAG 0.11.0.3)
FetchContent_MakeAvailable(zenohc)
FetchContent_Declare(zenohcxx GIT_REPOSITORY https://github.com/ami-iit/zenoh-cpp GIT_TAG 0.11.0.3-fixwin)
FetchContent_MakeAvailable(zenohcxx)

add_executable(pub pub.cxx)
target_link_libraries(pub PRIVATE zenohcxx::zenohc::lib)
target_compile_features(pub PRIVATE cxx_std_17)

add_executable(sub sub.cxx)
target_link_libraries(sub PRIVATE zenohcxx::zenohc::lib spdlog::spdlog)
target_compile_features(sub PRIVATE cxx_std_17)
