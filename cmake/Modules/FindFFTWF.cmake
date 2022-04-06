INCLUDE(FindPkgConfig)
#PKG_CHECK_MODULES(PC_FFTWF fftwf)

FIND_PATH(
    FFTWF_INCLUDE_DIRS
    NAMES fftw3.f
    HINTS $ENV{FFTWF_DIR}/include
        ${PC_FFTWF_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    FFTWF_LIBRARIES
    NAMES fftw3f
    HINTS $ENV{SOC_DIR}/lib
        ${PC_FFTWF_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          /usr/x86_64-linux-gnu
          /lib/x86_64-linux-gnu
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FFTWF DEFAULT_MSG FFTWF_LIBRARIES FFTWF_INCLUDE_DIRS)
