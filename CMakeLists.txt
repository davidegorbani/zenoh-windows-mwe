cmake_minimum_required(VERSION 3.16)

# Set the project name
project(zenoh-cpp-example)

include(FetchContent)

FetchContent_Declare(zenohc GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-c GIT_TAG main)
FetchContent_MakeAvailable(zenohc)
FetchContent_Declare(zenohcxx GIT_REPOSITORY https://github.com/eclipse-zenoh/zenoh-cpp GIT_TAG main)
FetchContent_MakeAvailable(zenohcxx)

add_executable(pub pub.cxx)
target_link_libraries(pub PRIVATE zenohcxx::zenohc::lib)

add_executable(sub sub.cxx)
target_link_libraries(sub PRIVATE zenohcxx::zenohc::lib)
