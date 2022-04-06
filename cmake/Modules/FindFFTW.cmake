INCLUDE(FindPkgConfig)
#PKG_CHECK_MODULES(PC_FFTW fftw)

FIND_PATH(
    FFTW_INCLUDE_DIRS
    NAMES fftw.h
    HINTS $ENV{FFTW_DIR}/include
        ${PC_FFTW_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    FFTW_LIBRARIES
    NAMES fftw3
    HINTS $ENV{SOC_DIR}/lib
        ${PC_FFTW_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          /usr/x86_64-linux-gnu
          /lib/x86_64-linux-gnu
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTW DEFAULT_MSG FFTW_LIBRARIES FFTW_INCLUDE_DIRS)
