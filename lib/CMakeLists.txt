# Copyright 2011,2012,2016,2018,2019 Free Software Foundation, Inc.
#
# This file was generated by gr_modtool, a tool from the GNU Radio framework
# This file is a part of gr-sddc
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX
include_directories(${SDDC_INCLUDE_DIRS} ${USB_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS} ${FFTWF_INCLUDE_DIRS})
link_directories(${SDDC_LIBRARY_DIRS} ${USB_LIBRARY_DIRS} ${FFTW_LIBRARY_DIRS} ${FFTWF_LIBRARY_DIRS})


list(APPEND sddc_sources
    sddc_source_impl.cc
)

set(sddc_sources "${sddc_sources}" PARENT_SCOPE)
if(NOT sddc_sources)
    MESSAGE(STATUS "No C++ sources... skipping lib/")
    return()
endif(NOT sddc_sources)

add_library(gnuradio-sddc SHARED ${sddc_sources})
target_link_libraries(gnuradio-sddc 
    ${SDDC_LIBRARIES} 
    ${USB_LIBRARIES} 
    ${FFTW_LIBRARIES} 
    ${FFTWF_LIBRARIES} 
    gnuradio::gnuradio-runtime
)
target_include_directories(gnuradio-sddc
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PUBLIC $<INSTALL_INTERFACE:include>
    ${SDDC_INCLUDE_DIRS}
    ${USB_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIRS}
    ${FFTWF_INCLUDE_DIRS}
  )
set_target_properties(gnuradio-sddc PROPERTIES DEFINE_SYMBOL "gnuradio_sddc_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-sddc PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-sddc)

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

# If your unit tests require special include paths, add them here
#include_directories()
# List all files that contain Boost.UTF unit tests here
list(APPEND test_sddc_sources
)
# Anything we need to link to for the unit tests go here
list(APPEND GR_TEST_TARGET_DEPS gnuradio-sddc)

if(NOT test_sddc_sources)
    MESSAGE(STATUS "No C++ unit tests... skipping")
    return()
endif(NOT test_sddc_sources)

foreach(qa_file ${test_sddc_sources})
    GR_ADD_CPP_TEST("sddc_${qa_file}"
        ${CMAKE_CURRENT_SOURCE_DIR}/${qa_file}
    )
endforeach(qa_file)
