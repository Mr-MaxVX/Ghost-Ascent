cmake_minimum_required(VERSION 3.16)
project(GhostAscent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Find GLEW (or use pkg-config fallback)
find_package(GLEW QUIET)
if(NOT GLEW_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLEW QUIET glew)
    endif()
endif()

# Source files
set(SOURCES
    GameMain.cpp
    MonitorManager.cpp
    GhostController.cpp
    LevelGenerator.cpp
    WallSystem.cpp
    GameUi.cpp
    BrickRenderer.cpp
    BloodTexture.cpp
)

set(HEADERS
    MonitorManager.hpp
    GhostController.hpp
    LevelGenerator.hpp
    WallSystem.hpp
    GameUi.hpp
    BrickRenderer.hpp
    BloodTexture.hpp
    AssetExtractor.hpp
    stb_image.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
if(GLEW_FOUND AND TARGET GLEW::GLEW)
    target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
elseif(GLEW_FOUND AND GLEW_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${GLEW_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLEW_INCLUDE_DIRS})
else()
    # Fallback: link glew directly
    target_link_libraries(${PROJECT_NAME} PRIVATE GLEW)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
)

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    # Link glew32 on Windows if available
    find_library(GLEW_LIB glew32 PATHS "C:/Program Files (x86)/glew/lib" "C:/glew/lib")
    if(GLEW_LIB)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${GLEW_LIB})
    endif()
endif()

# Copy assets to build directory
set(ASSETS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(ASSETS_DEST ${CMAKE_CURRENT_BINARY_DIR}/assets)

add_custom_command(
    OUTPUT ${ASSETS_DEST}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SOURCE} ${ASSETS_DEST}
    DEPENDS ${ASSETS_SOURCE}
    COMMENT "Copying assets to build directory"
)

add_custom_target(copy_assets ALL DEPENDS ${ASSETS_DEST})
add_dependencies(${PROJECT_NAME} copy_assets)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets DESTINATION bin OPTIONAL)
