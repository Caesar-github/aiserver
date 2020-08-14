set(CMAKE_DBUSXX_XML2CPP_TOOLS ${BR2_SDK_PATH}/bin/dbusxx-xml2cpp)
add_custom_command(OUTPUT dbus_meida_control_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_media_control.xml
                   --proxy=dbus_media_control_proxy.h
                   --adaptor=dbus_media_control_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/control
                   COMMENT "build dbus_meida_control xml2cpp head file"
)
add_custom_target(dbus_meida_control_xml2cppTask ALL DEPENDS dbus_meida_control_xml2cpp)

add_custom_command(OUTPUT dbus_dbserver_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_dbserver.xml
                   --proxy=dbus_dbserver_proxy.h
                   --adaptor=dbus_dbserver_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database
                   COMMENT "build dbus_dbserver xml2cpp head file"
)
add_custom_target(dbus_dbserver_xml2cppTask ALL DEPENDS dbus_dbserver_xml2cpp)


add_custom_command(OUTPUT dbus_storage_manager_xml2cpp
                   COMMAND ${CMAKE_DBUSXX_XML2CPP_TOOLS}
                   dbus_storage_manager.xml
                   --proxy=dbus_storage_manager_proxy.h
                   --adaptor=dbus_storage_manager_adaptor.h
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dbus/storage_manager
                   COMMENT "build dbus_storage_manager xml2cpp head file"
)
add_custom_target(dbus_storage_manager_xml2cppTask ALL DEPENDS dbus_storage_manager_xml2cpp)

set(AI_SERVER_INC
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/server/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/thread/
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/osd/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/control/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/database/
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/storage_manager/)

#     glib_2::glib_2
set(AISERVER_LIB
    dbus-1
    dbus_cpp::dbus_cpp
    ${CMAKE_THREAD_LIBS_INIT})

set(AI_SERVER_SRC aiserver.cpp ai_scene_director.cpp)
aux_source_directory(utils/thread AI_SERVER_SRC)
aux_source_directory(task_graph   AI_SERVER_SRC)
aux_source_directory(dbus         AI_SERVER_SRC)
aux_source_directory(dbus/control AI_SERVER_SRC)
aux_source_directory(dbus/database        AI_SERVER_SRC)
aux_source_directory(dbus/storage_manager AI_SERVER_SRC)

if (DEFINED MEDIASERVER_CONF_PREFIX)
add_definitions(-DMEDIASERVER_CONF_PREFIX=${MEDIASERVER_CONF_PREFIX})
endif()

option(ENABLE_OSD_SERVER  "enbale osd server" OFF)
if (${ENABLE_OSD_SERVER_ON})
    find_package(FreeType2 REQUIRED)
    add_definitions(-DENABLE_OSD_SERVER)
    set(AISERVER_LIB ${AISERVER_LIB} Freetype_2::Freetype_2)
    aux_source_directory(utils/osd/ AI_SERVER_SRC)
endif()

option(ENABLE_SHM_SERVER  "enbale shm server" ON)
if (${ENABLE_SHM_SERVER})
    add_definitions(-DENABLE_SHM_SERVER)
    set(AI_SERVER_INC
        ${AI_SERVER_INC}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/shmc/)
    aux_source_directory(utils/shmc/ AI_SERVER_SRC)
    find_package(Protobuf REQUIRED)
    if(PROTOBUF_FOUND)
        message(STATUS "protobuf library found")
        set(AISERVER_LIB ${AISERVER_LIB} protobuf)
        include_directories(${PROTOBUF_INCLUDE_DIRS})
        include_directories(${CMAKE_CURRENT_BINARY_DIR})
        protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/utils/shmc/nn_data.proto)
        set(AI_SERVER_INC ${AI_SERVER_INC} ${PROTO_HDRS})
        set(AI_SERVER_SRC ${AI_SERVER_SRC} ${PROTO_SRCS} ${PROTO_HDRS})
    endif()
endif()

option(ENABLE_EXIV2_LIB  "enbale exiv2 library" OFF)
if (${ENABLE_EXIV2_LIB_ON})
    find_package(Exiv2 REQUIRED)
    add_definitions(-DENABLE_EXIV2_LIB)
    set(AISERVER_LIB ${AISERVER_LIB} Exiv2::Exiv2)
    set(AI_SERVER_INC ${AI_SERVER_INC} utils/exiv2/)
    aux_source_directory(utils/exiv2/ AI_SERVER_SRC)
endif()

option(USE_ROCKFACE  "enbale rockface" OFF)
if (${USE_ROCKFACE})
add_definitions(-DUSE_ROCKFACE)
endif()

option(USE_ROCKX  "enbale rockx" ON)
if (${USE_ROCKX})
add_definitions(-DUSE_ROCKX)
add_definitions(-DWITH_ROCKX)
set(AISERVER_LIB ${AISERVER_LIB} rockx)
endif()

set(AISERVER_LIB ${AISERVER_LIB} rockit)

add_executable(aiserver ${AI_SERVER_SRC})
target_include_directories(aiserver PUBLIC ${AI_SERVER_INC})
target_link_libraries(aiserver ${AISERVER_LIB})
add_dependencies(aiserver dbus_meida_control_xml2cppTask)
add_dependencies(aiserver dbus_dbserver_xml2cppTask)
add_dependencies(aiserver dbus_storage_manager_xml2cppTask)
install(TARGETS aiserver RUNTIME DESTINATION bin)
install(FILES dbus/rockchip.aiserver.control.conf DESTINATION share/dbus-1/system.d)
install(FILES conf/camera_nv12_rkrga_300.json DESTINATION share/aiserver/)
install(FILES conf/camera_nv12_rkrga_300_rknn_facedetect.json DESTINATION share/aiserver/)
install(FILES conf/camera_nv12_rkrga_300_rknn_face_landmark.json DESTINATION share/aiserver/)
install(FILES conf/camera_nv12_rkrga_300_rknn_graph.json DESTINATION share/aiserver/)
install(FILES conf/camera_nv12_rkrga_300_rknn_mixed.json DESTINATION share/aiserver/)
install(FILES conf/camera_nv12_rkrga_300_rknn_posebody.json DESTINATION share/aiserver/)
install(FILES conf/rknn_facedetect.json DESTINATION share/aiserver/)
install(FILES conf/rknn_face_landmark.json DESTINATION share/aiserver/)
install(FILES conf/rknn_posebody.json DESTINATION share/aiserver/)

if (${ENABLE_OSD_SERVER})
    install(DIRECTORY resource/ DESTINATION share/aiserver FILES_MATCHING PATTERN "*.bmp")
    install(DIRECTORY resource/ DESTINATION share/aiserver FILES_MATCHING PATTERN "*.ttc")
    install(DIRECTORY resource/ DESTINATION share/aiserver FILES_MATCHING PATTERN "*.ttf")
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/aiserver.h DESTINATION include/aiserver)
