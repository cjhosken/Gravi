cmake_minimum_required(VERSION 3.26)

# Project
project(hdGravi VERSION 1.0.0 LANGUAGES CXX)

# Add a relative target path to the HDGRAVI_RPATH.
function(_add_target_to_rpath CURRENT_RPATH target)
    get_target_property(LIBRARY_FILE ${target} LOCATION)

    get_filename_component(LIBRARY_PATH ${LIBRARY_FILE} DIRECTORY)

    file(RELATIVE_PATH
            LIBRARY_PATH_REL


            ${LIBRARY_PATH}
    )
    set (HDGRAVI_RPATH "${CURRENT_RPATH}:$ORIGIN/${LIBRARY_PATH_REL}" PARENT_SCOPE)
endfunction()

add_library(hdGravi SHARED
        include/Common.h
        include/HdGraviPrim.h
        src/HdGraviPrim.cpp
        include/RenderParam.h
        include/Mesh.h
        src/Points.cpp
        include/Points.h
        src/BasisCurves.cpp
        include/BasisCurves.h
        src/Mesh.cpp
        include/Renderer.h
        src/Renderer.cpp
        include/RenderBuffer.h
        src/RenderBuffer.cpp
        include/RenderPass.h
        src/RenderPass.cpp
        include/Scene.h
        src/Scene.cpp
        include/Light.h
        src/Light.cpp
        include/RenderDelegate.h
        src/RenderDelegate.cpp
        include/RendererPlugin.h
        src/RendererPlugin.cpp
        plugInfo.json
        src/HdGravityWell.cpp
        include/HdGravityWell.h
        src/GraviAdapter.cpp
        src/GraviRenderSettingsAdapter.cpp
        include/GraviRenderSettingsAdapter.h
        include/GraviAdapter.h
        ../usdGravi/gravityWell.h
        ../usdGravi/gravityWell.cpp
        ../usdGravi/wrapGravityWell.cpp
        ../usdGravi/graviRenderSettings.h
        ../usdGravi/graviRenderSettings.cpp
        ../usdGravi/wrapGraviRenderSettings.cpp
        ../usdGravi/tokens.cpp
        ../usdGravi/wrapTokens.cpp
        src/Light.cpp
        src/RenderSettings.cpp
        include/RenderSettings.h
        src/GraviRenderSettingsAdapter.cpp
)

set_property(TARGET hdGravi PROPERTY CXX_STANDARD 17)
set_property(TARGET hdGravi PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET hdGravi PROPERTY CXX_EXTENSIONS OFF)

target_include_directories(hdGravi PRIVATE ${PXR_INCLUDE_DIRS} ${PXR_CMAKE_DIR}/include/ include ../usdGravi/)
target_link_libraries(hdGravi PRIVATE ${PXR_LIBRARIES})

install(TARGETS hdGravi DESTINATION plugin/usd/hdGravi)
set(PLUG_INFO_LIBRARY_PATH "./hdGravi${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUG_INFO_RESOURCE_PATH "resources")
set(PLUG_INFO_ROOT "..")
configure_file(plugInfo.json plugInfo.json @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json" DESTINATION plugin/usd/hdGravi/resources)
set (HDGRAVI_RPATH "$ORIGIN")
set (HDGRAVI_RPATH "${HDGRAVI_RPATH}:/../../lib")
set (HDGRAVI_RPATH "${HDGRAVI_RPATH}:${pxr_DIR}/lib")

set_target_properties(hdGravi PROPERTIES
        PREFIX ""
        INSTALL_RPATH ${HDGRAVI_RPATH}
)