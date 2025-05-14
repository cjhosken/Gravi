cmake_minimum_required(VERSION 3.12)
project(usdGravi VERSION 1.0.0 LANGUAGES CXX)

# Add a relative target path to the HDGRAVI_RPATH.
function(_add_target_to_rpath CURRENT_RPATH target)
    get_target_property(LIBRARY_FILE ${target} LOCATION)

    get_filename_component(LIBRARY_PATH ${LIBRARY_FILE} DIRECTORY)

    file(RELATIVE_PATH
            LIBRARY_PATH_REL


            ${LIBRARY_PATH}
    )
    set (USDGRAVI_RPATH "${CURRENT_RPATH}:$ORIGIN/${LIBRARY_PATH_REL}" PARENT_SCOPE)
endfunction()

add_library(usdGravi SHARED
        api.h
        generatedSchema.usda
        plugInfo.json
        tokens.cpp
        wrapTokens.cpp
)

set_property(TARGET usdGravi PROPERTY CXX_STANDARD 17)
set_property(TARGET usdGravi PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET usdGravi PROPERTY CXX_EXTENSIONS OFF)

target_include_directories(usdGravi PRIVATE ${PXR_INCLUDE_DIRS} ${PXR_CMAKE_DIR}/include/ include)
target_link_libraries(usdGravi PRIVATE ${PXR_LIBRARIES})

#install(TARGETS usdGravi DESTINATION plugin/usd/usdGravi)
set(PLUG_INFO_LIBRARY_PATH "")
set(PLUG_INFO_RESOURCE_PATH "resources")
set(PLUG_INFO_ROOT "..")
configure_file(plugInfo.json plugInfo.json @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json" "${CMAKE_CURRENT_LIST_DIR}/generatedSchema.usda" DESTINATION plugin/usd/usdGravi/resources)