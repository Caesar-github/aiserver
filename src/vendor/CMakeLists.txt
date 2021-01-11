cmake_minimum_required( VERSION 2.8.8 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(SRC_DEPEND_LIBS rockit dl)

# vendor custom node
option(ENABLE_SAMPLE_NODE  "enable sample node" ON)
if (${ENABLE_SAMPLE_NODE})
    include_directories(samples)
    set(SRC_FILES_VENDOR
        samples/SampleNode.cpp
    )
endif()

# node samples
option(ENABLE_SAMPLE_NODE_EPTZ  "enable sample node eptz" ON)
if (${ENABLE_SAMPLE_NODE_EPTZ})
    include_directories(samples/filter/eptz)
    link_directories(samples/filter/eptz)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/samples/filter/eptz/libeptz.so)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/samples/filter/eptz/libeptz.so DESTINATION ../../oem/usr/lib/)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples/filter/eptz/eptz_zoom.conf")
        set(ETC_DST "${PROJECT_SOURCE_DIR}/../../target/etc/")
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/samples/filter/eptz/eptz_zoom.conf DESTINATION ${ETC_DST})
    endif()
    set(SRC_FILES_VENDOR
        ${SRC_FILES_VENDOR}
        samples/filter/eptz/RTNodeVFilterEptzDemo.cpp
    )
endif()

option(ENABLE_SAMPLE_NODE_ROCKX  "enable sample node rockx" OFF)
if (${ENABLE_SAMPLE_NODE_ROCKX})
    if (${USE_ROCKX})
        set(SRC_FILES_VENDOR
            ${SRC_FILES_VENDOR}
            samples/filter/rockx/RTVFilterRockx.cpp
            samples/filter/rockx/RTNodeVFilterRockx.cpp
        )
        message(STATUS "Build WITH rockx sample")
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/samples/filter/rockx/aicamera.json DESTINATION ../../oem/usr/share/aiserver/)
    else()
        message(STATUS "Build NONE rockx sample")
    endif()
endif()

add_library(${VENDOR_STATIC} STATIC ${SRC_FILES_VENDOR})
set_target_properties(${VENDOR_STATIC} PROPERTIES FOLDER "vendor")
target_link_libraries(${VENDOR_STATIC} ${SRC_DEPEND_LIBS})
