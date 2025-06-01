
cmake_minimum_required(VERSION 3.10)
project(testapp)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_executable(${PROJECT_NAME} src/main_linux.cpp)

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_executable(${PROJECT_NAME} src/main_windows.cpp)

else()
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
endif()


target_link_libraries(${PROJECT_NAME}
    uLogger
    iPlugin
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME}
        dl
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_link_libraries(${PROJECT_NAME}
        pthread
    )
else()
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
endif()

