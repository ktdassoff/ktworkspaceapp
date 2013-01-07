cmake_minimum_required(VERSION 2.8.8)
project("KtWorkspaceApp")
set(PROJECT_VER_MAJOR 2)
set(PROJECT_VER_MINOR 0)
set(PROJECT_VER_REL 0)
set(PROJECT_VERSION ${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_REL})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
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
set(SOURCES "KtwsWorksheet.cpp" "KtwsWorkspace.cpp" "KtwsSession.cpp" "KtwsSessionDialog.cpp" "KtwsSerialization.cpp" "KtwsWorksheetHandler.cpp")
set(HEADERS "KtwsWorksheet.hpp" "KtwsWorkspace.hpp" "KtwsSession.hpp" "KtwsSessionDialog.hpp" "KtwsWorksheetHandler.hpp" "${PROJECT_BINARY_DIR}/KtwsGlobal.hpp")
set(PRIV_HDRS "KtwsWorksheet_p.hpp" "KtwsWorkspace_p.hpp" "KtwsSession_p.hpp" "KtwsSessionDialog_p.hpp" "KtwsSerialization_p.hpp")
add_library(${SHLIBNAME} SHARED ${SOURCES} ${HEADERS} ${PRIV_HDRS})
set_target_properties(${SHLIBNAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
	PUBLIC_HEADER "${HEADERS}")
set_property(TARGET ${SHLIBNAME} APPEND PROPERTY COMPILE_DEFINITIONS "KTWORKSPACEAPP_LIBRARY")
target_link_libraries(${SHLIBNAME} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

## Exporting and installation ##
install(TARGETS ${SHLIBNAME}
    LIBRARY DESTINATION lib/
    ARCHIVE DESTINATION lib/
    RUNTIME DESTINATION bin/
	PUBLIC_HEADER DESTINATION include/
	)
