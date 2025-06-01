cmake_minimum_required(VERSION 3.3)

project (uLogger)

if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ../")
  set(CMAKE_INSTALL_PREFIX "../" CACHE STRING "The path to use for make install" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(INSTALL_WIN_APP_DIR      "bin/windows")
SET(INSTALL_LINUX_APP_DIR    "bin/linux")

add_subdirectory(sources)

if(MSVC OR MSYS OR MINGW)

    # --- Binaries ---
    install ( TARGETS testapp
        DESTINATION ${INSTALL_WIN_APP_DIR}
    )

    # --- Plugins ---
    install ( TARGETS plugin
        RUNTIME DESTINATION ${INSTALL_WIN_APP_DIR}
    )

else()

    # --- Binaries ---
    install ( TARGETS testapp
        DESTINATION ${INSTALL_LINUX_APP_DIR}
    )

    # --- Plugins ---
    install ( TARGETS plugin
        LIBRARY DESTINATION ${INSTALL_LINUX_APP_DIR}
    )

endif()
