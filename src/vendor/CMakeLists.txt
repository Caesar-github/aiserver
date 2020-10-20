cmake_minimum_required( VERSION 2.8.8 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

# vendor custom node
option(ENABLE_SAMPLE_NODE  "enable sample node" ON)
if (${ENABLE_SAMPLE_NODE})
    include_directories(samples)
    set(SRC_FILES_VENDOR
        samples/SampleNode.cpp
    )
endif()

# node samples
option(ENABLE_SAMPLE_NODE_EPTZ  "enable sample node eptz" OFF)
if (${ENABLE_SAMPLE_NODE_EPTZ})
    include_directories(samples/filter/eptz)
    link_directories(samples/filter/eptz)
    link_libraries(${CMAKE_CURRENT_SOURCE_DIR}/samples/filter/eptz/libeptz.so)
    set(SRC_FILES_VENDOR
        ${SRC_FILES_VENDOR}
        samples/filter/eptz/RTNodeVFilterEptzDemo.cpp
    )
endif()

set(SRC_DEPEND_LIBS
    rockit
)

add_library(${VENDOR_STATIC} STATIC ${SRC_FILES_VENDOR})
set_target_properties(${VENDOR_STATIC} PROPERTIES FOLDER "vendor")
target_link_libraries(${VENDOR_STATIC} ${SRC_DEPEND_LIBS})
