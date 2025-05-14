cmake_minimum_required(VERSION 3.26)

# Project
project(Gravi VERSION 1.0.0 LANGUAGES CXX)

# Qt
# For simpler class generation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)
# For compatibility with USD
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

# Define source files
add_executable(Gravi src/main.cpp)

find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets REQUIRED)

set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)

qt_add_resources(RESOURCE_FILES
        ${RESOURCES_DIR}/resources.qrc
)

target_sources(Gravi PRIVATE
        include/MainWindow.h
        src/MainWindow.cpp
        include/HdWidget.h
        src/HdWidget.cpp
        include/TimelineWidget.h
        src/TimelineWidget.cpp
        include/Tabs.h
        src/Tabs.cpp
        ${RESOURCE_FILES}

        # Schema Stuff
        ../usdGravi/gravityWell.cpp
        ../usdGravi/wrapGravityWell.cpp

        ../usdGravi/graviRenderSettings.cpp
        ../usdGravi/wrapGraviRenderSettings.cpp

        ../usdGravi/wrapTokens.cpp
        ../usdGravi/tokens.cpp
)

# Include directories
target_include_directories(Gravi PRIVATE
        include/
        ../usdGravi/
        ../hdGravi/include
        ${PXR_INCLUDE_DIRS}
        ${PXR_CMAKE_DIR}/include/
)

# Link libraries
target_link_libraries(Gravi PRIVATE
        ${PXR_LIBRARIES}
        Qt6::Core
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        Qt6::Widgets
        util
)

target_link_directories(Gravi PRIVATE
    ${PXR_CMAKE_DIR}/lib/
    ${PXR_CMAKE_DIR}/lib64/
)

# Set RPATH to find dependencies at runtime
set_target_properties(Gravi PROPERTIES
        BUILD_RPATH "\$ORIGIN/../lib:\$ORIGIN/../lib64"
        INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/../lib64"
        CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(TARGETS Gravi
    DESTINATION bin
)

install(DIRECTORY ${PXR_CMAKE_DIR}/lib/
        DESTINATION lib
        PATTERN "cmake" EXCLUDE
        PATTERN "pkgconfig" EXCLUDE
        PATTERN "libpng" EXCLUDE
)

install(DIRECTORY ${PXR_CMAKE_DIR}/lib64/
        DESTINATION lib64
        PATTERN "cmake" EXCLUDE
        PATTERN "pkgconfig" EXCLUDE
)

install(DIRECTORY ${PXR_CMAKE_DIR}/plugin/usd/
    DESTINATION plugin/usd
)

install(DIRECTORY images
        DESTINATION images
)

file(COPY ${RESOURCES_DIR}/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

# Display configuration summary
message(STATUS "Gravi v${PROJECT_VERSION} configured successfully")