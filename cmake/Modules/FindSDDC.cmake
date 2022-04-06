INCLUDE(FindPkgConfig)
#PKG_CHECK_MODULES(PC_SDDC sddc)

FIND_PATH(
    SDDC_INCLUDE_DIRS
    NAMES libsddc.h
    HINTS $ENV{SDDC_DIR}/include
        ${PC_SDDC_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SDDC_LIBRARIES
    NAMES sddc
    HINTS $ENV{SOC_DIR}/lib
        ${PC_SDDC_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          /usr/x86_64-linux-gnu
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDDC DEFAULT_MSG SDDC_LIBRARIES SDDC_INCLUDE_DIRS)
