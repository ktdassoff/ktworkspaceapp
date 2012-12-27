cmake_minimum_required(VERSION 2.8.8)
project("KtWorkspaceApp")
set(PROJECT_VER_MAJOR 2)
set(PROJECT_VER_MINOR 0)
set(PROJECT_VER_REL 0)
set(PROJECT_VERSION ${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_REL})

if(CMAKE_CXX_COMPILER MATCHES "clang")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++0x)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(CMAKE_CXX_COMPILER MATCHES "clang" OR CMAKE_COMPILER_IS_GNUCXX)
        add_definitions(-Wall)
    endif()
endif()

set(CMAKE_DEBUG_POSTFIX d)

find_package(Qt 4.8.0 REQUIRED)
include(${QT_USE_FILE})
set(CMAKE_AUTOMOC ON)
set(QV Q${DESIRED_QT_VERSION})

configure_file(
    "${PROJECT_SOURCE_DIR}/KtwsGlobal.hpp.in"
    "${PROJECT_BINARY_DIR}/KtwsGlobal.hpp"
    @ONLY)
include_directories(${PROJECT_BINARY_DIR})

set(SHLIBNAME Kt${QV}WorkspaceApp${PROJECT_VER_MAJOR})
set(SOURCES "KtwsWorksheet.cpp" "KtwsWorkspace.cpp" "KtwsWorkspaceDialog.cpp")
set(HEADERS "KtwsWorksheet.hpp" "KtwsWorkspace.hpp" "KtwsGlobal.hpp")
set(PRIV_HDRS "KtwsWorksheetImpl.hpp" "KtwsWorkspaceImpl.hpp" "KtwsSessionDialog.hpp")
add_library(${SHLIBNAME} SHARED ${SOURCES} ${HEADERS} ${PRIV_HDRS})
set_target_properties(${SHLIBNAME} PROPERTIES
    VERSION ${PROJECT_VERSION})
set_property(TARGET ${SHLIBNAME} APPEND_STRING PROPERTY COMPILE_DEFINTIONS "KTWORKSPACEAPP_LIBRARY")
target_link_libraries(${SHLIBNAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install(TARGETS ${SHLIBNAME}
    LIBRARY DESTINATION lib/
    ARCHIVE DESTINATION lib/
    RUNTIME DESTINATION bin/)
install(FILES ${HEADERS} DESTINATION include/)

