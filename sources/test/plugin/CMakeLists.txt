
cmake_minimum_required(VERSION 3.10)
project(plugin)

add_library( ${PROJECT_NAME}
    SHARED
        src/plugin.cpp)

target_link_libraries( ${PROJECT_NAME}
    uLogger
    iPlugin
    pthread
)

