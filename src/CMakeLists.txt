# Copyright (c) Glyn Matthews 2012-2017.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.5.1)
project(NetworkUri CXX)
set(PROJECT_NAME "networkuri")
set(PROJECT_LIB "networkuri" )

###############################################################################
## VERSION
set( URI_MAJOR_VERSION 0  )
set( URI_MINOR_VERSION 13 )
set( URI_PATCH_VERSION 0  )
set( URI_VERSION ${URI_MAJOR_VERSION}.${URI_MINOR_VERSION}.${URI_PATCH_VERSION})

################################################################################
## Including GNUInstallDirs for definition of CMAKE_INSTALL_<dir>
include ( GNUInstallDirs )

set ( INSTALL_PATH_LIB     ${CMAKE_INSTALL_LIBDIR}       )
set ( INSTALL_PATH_HEADERS ${CMAKE_INSTALL_INCLUDEDIR}   )

################################################################################
## rpath stuff
set( CMAKE_INSTALL_NAME_DIR  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

################################################################################
## Flags
set ( CMAKE_CXX_STANDARD 11)
set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra" )
set ( CMAKE_POSITION_INDEPENDENT_CODE ON)


find_package(Threads REQUIRED)

find_package(
        Boost 1.64
        COMPONENTS
        program_options
        filesystem
        system
        thread
        REQUIRED)
include_directories( ${Boost_INCLUDE_DIRS} )
link_directories   ( ${Boost_LIBRARY_DIRS} )

################################################################################
## Using custom cmake utils functions
include ( func.cmake )


include_directories(src)

add_subdirectory(src)


################################################################################
## Installation of package config file

configure_file( NetworkUri.pc.in
        "${CMAKE_BINARY_DIR}/lib/pkgconfig/${PROJECT_NAME}.pc" @ONLY )
install( FILES "${CMAKE_BINARY_DIR}/lib/pkgconfig/${PROJECT_NAME}.pc"
        DESTINATION "${INSTALL_PATH_LIB}/pkgconfig")


