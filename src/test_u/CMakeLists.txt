if (UNIX)
# include (CheckLibraryExists)
# check_library_exists (pthread pthread_create "" HAVE_LIB_PTHREAD)
 include (FindPkgConfig)

 find_package (Threads REQUIRED)
endif ()

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/scripts")
include (ffmpeg)
include (ImageMagick)
include (ACE)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/image/scripts")
include (png)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl/scripts")
include (assimp)
include (glew)
include (glm)
include (glut)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/scripts")
include (PreUIFramework)
include (SetUIFramework)
if (GTK_SUPPORT)
 include (SetGTKVersion)
endif (GTK_SUPPORT)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/scripts")
include (NetlinkConfig)
include (SSLConfig)

if (UNIX)
 include (FindGettext)

# *NOTE*: libCommon requires libcap
 pkg_check_modules (PKG_CAP REQUIRED libcap)

 set (BUILD_SUB_DIR "gcc/${CMAKE_BUILD_TYPE}")
elseif (WIN32)
 set (BUILD_SUB_DIR "msvc")
endif ()

find_package (Common
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES Common/build/${BUILD_SUB_DIR}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${Common_DIR})
 message (FATAL_ERROR "could not find package \"Common\", aborting")
endif ()

find_package (ACEStream
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACEStream/build/${BUILD_SUB_DIR}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACEStream_DIR})
 message (FATAL_ERROR "could not find package \"ACEStream\", aborting")
endif ()

find_package (ACENetwork
              REQUIRED
              CONFIG
              PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../..modules ${CMAKE_CURRENT_SOURCE_DIR}/../../..
              PATH_SUFFIXES ACENetwork/build/${BUILD_SUB_DIR}
              NO_DEFAULT_PATH)
if (NOT EXISTS ${ACENetwork_DIR})
 message (FATAL_ERROR "could not find package \"ACENetwork\", aborting")
endif ()

include_directories (${GLEW_INCLUDE_DIRS})
include_directories (${GLM_INCLUDE_DIRS})
include_directories (${GLUT_INCLUDE_DIRS})
include_directories (${GTK_INCLUDE_DIRS})
include_directories (${ACE_INCLUDE_DIR})
if (UNIX)
 include_directories (/usr/share/gettext)
 include_directories (${PKG_CAP_INCLUDE_DIRS})

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/gl)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/Common/src/ui/gtk)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACEStream/src/modules/net)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../modules/ACENetwork/src/client_server)

 include_directories (${CMAKE_CURRENT_BINARY_DIR}/../..)
elseif (WIN32)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/gettext)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/gl)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/signal)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../Common/src/ui/gtk)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/file)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/misc)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACEStream/src/modules/net)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src)
 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../../../ACENetwork/src/client_server)

 include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../include)
endif ()

############## next target ################
set (mod_mpu6050_EXE_SRCS
     olimex_mod_mpu6050.cpp
     olimex_mod_mpu6050_gtk_callbacks.cpp
     olimex_mod_mpu6050_gtk_callbacks.h
     olimex_mod_mpu6050_gtk_gl_callbacks.cpp
     olimex_mod_mpu6050_gtk_gl_callbacks.h
     olimex_mod_mpu6050_common.cpp
     olimex_mod_mpu6050_common.h
     olimex_mod_mpu6050_defines.h
     olimex_mod_mpu6050_eventhandler.cpp
     olimex_mod_mpu6050_eventhandler.h
     olimex_mod_mpu6050_macros.h
     olimex_mod_mpu6050_message.cpp
     olimex_mod_mpu6050_message.h
     olimex_mod_mpu6050_modules_common.h
     olimex_mod_mpu6050_network.h
     olimex_mod_mpu6050_opengl.cpp
     olimex_mod_mpu6050_opengl.h
     olimex_mod_mpu6050_sessionmessage.cpp
     olimex_mod_mpu6050_sessionmessage.h
     olimex_mod_mpu6050_signalhandler.cpp
     olimex_mod_mpu6050_signalhandler.h
     olimex_mod_mpu6050_stream.cpp
     olimex_mod_mpu6050_stream.h
     olimex_mod_mpu6050_stream.inl
     olimex_mod_mpu6050_stream_common.h
     olimex_mod_mpu6050_types.h
     stdafx.cpp
     stdafx.h)

set (EXECUTABLE mod_mpu6050)
add_executable (${EXECUTABLE} ${mod_mpu6050_EXE_SRCS} etc/olimex_mod_mpu6050.gtk2)

#add_dependencies (${ACESTREAM_TARGET_NAME} ACE)
#add_dependencies (${PROJECT_NAME} Common)
#add_dependencies (${PROJECT_NAME} ACEStream)
#add_dependencies (${PROJECT_NAME} ACENetwork)

if (UNIX)
 target_link_options (${EXECUTABLE} PUBLIC "-rdynamic")
endif (UNIX)

target_link_libraries (${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${EXECUTABLE} ${ACE_LIBRARY})
if (UNIX)
 target_link_libraries (${EXECUTABLE} rt)
 target_link_libraries (${EXECUTABLE} ${PKG_CAP_LIBRARIES})
 target_link_libraries (${EXECUTABLE} GL)
endif (UNIX)
target_link_libraries (${EXECUTABLE} ${GLEW_LIBRARY})
target_link_libraries (${EXECUTABLE} ${GLUT_LIBRARY})
target_link_libraries (${EXECUTABLE} ${GTK_LIBRARIES})

target_link_libraries (${EXECUTABLE} Common)
target_link_libraries (${EXECUTABLE} CommonEvent)
target_link_libraries (${EXECUTABLE} CommonLog)
target_link_libraries (${EXECUTABLE} CommonOpenGL)
target_link_libraries (${EXECUTABLE} CommonTimer)
target_link_libraries (${EXECUTABLE} CommonUI)
if (WIN32)
 # *WARNING*: linking CommonUI requires "/FORCE:MULTIPLE""
# set_target_properties (${EXECUTABLE} PROPERTIES LINK_FLAGS "/FORCE:MULTIPLE /SAFESEH:NO")
endif ()
target_link_libraries (${EXECUTABLE} CommonUI_GTK)

target_link_libraries (${EXECUTABLE} ACEStream)
target_link_libraries (${EXECUTABLE} ACEStream_Network)
target_link_libraries (${EXECUTABLE} ACEStream_Statistic)

target_link_libraries (${EXECUTABLE} ACENetwork)
target_link_libraries (${EXECUTABLE} ACENetwork_Client)
target_link_libraries (${EXECUTABLE} ACENetwork_Server)

set (ARGS "-l")
set (PATH_SUFFIX "Release")
if (DEFINED CMAKE_BUILD_TYPE)
 set (PATH_SUFFIX ${CMAKE_BUILD_TYPE})
endif (DEFINED CMAKE_BUILD_TYPE)
if (PATH_SUFFIX STREQUAL Debug)
 set (ARGS "${ARGS} -t")
endif (PATH_SUFFIX STREQUAL Debug)
if (UNIX)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}" COMMAND)
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${GTK_LIB_PATH}
                                              ${ACE_LIB_PATH}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/test_u
                         ENVIRONMENT ${ENVIRONMENT})
elseif (WIN32)
 file (TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${EXECUTABLE}.exe" COMMAND)
 string (REPLACE "\\" "\\\\" COMMAND ${COMMAND})
 file (TO_CMAKE_PATH "${GLEW_LIB_DIR}" GLEW_LIB_PATH)
 file (TO_CMAKE_PATH "${GLUT_LIB_DIR}" GLUT_LIB_PATH)
 file (TO_CMAKE_PATH "${PNG_LIB_DIR}" PNG_LIB_PATH)
 file (TO_CMAKE_PATH "${ASSIMP_LIB_DIR}" ASSIMP_LIB_PATH)
 file (TO_CMAKE_PATH "${GTK_LIB_DIRS}" GTK_LIB_PATH)
 file (TO_CMAKE_PATH "${OPENSSL_LIB_DIR}" SSL_LIB_PATH)
 # *NOTE*: ffmpeg was built with "vcpkg-libpng" support, so now there is a new dependency to zlib1.dll, liblzma.dll, iconv-2.dll :-(
 file (TO_CMAKE_PATH "$ENV{VCPKG_INSTALLATION_ROOT}/installed/x64-windows/bin" VCPKG_LIB_PATH)
 file (TO_CMAKE_PATH "${ffmpeg_LIB_DIR}" FFMPEG_LIB_PATH)
 file (TO_CMAKE_PATH "${ImageMagick_LIB_DIR}" IMAGEMAGICK_LIB_PATH)
 file (TO_CMAKE_PATH "${ACE_LIB_DIR}" ACE_LIB_PATH)
 create_target_launcher (${EXECUTABLE}
                         COMMAND ${COMMAND}
                         ARGS ${ARGS}
#                         FORWARD_ARGS
                         RUNTIME_LIBRARY_DIRS ${GLEW_LIB_PATH}
                                              ${GLUT_LIB_PATH}
                                              ${PNG_LIB_PATH}
                                              ${ASSIMP_LIB_PATH}
                                              ${GTK_LIB_PATH}
                                              ${SSL_LIB_PATH}
                                              ${VCPKG_LIB_PATH}
                                              ${FFMPEG_LIB_PATH}
                                              ${IMAGEMAGICK_LIB_PATH}
                                              ${ACE_LIB_PATH}
                         WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/test_u
                         ENVIRONMENT ${ENVIRONMENT})
endif ()

install (TARGETS ${EXECUTABLE}
         EXPORT ${PROJECT_NAME}Targets
         RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
         COMPONENT dev)
install (FILES etc/olimex_mod_mpu6050.gtk2
         DESTINATION etc/${PROJECT_NAME}/mod_mpu6050 COMPONENT config)
