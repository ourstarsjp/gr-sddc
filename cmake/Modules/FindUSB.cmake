INCLUDE(FindPkgConfig)
#PKG_CHECK_MODULES(PC_USB usb)

FIND_PATH(
    USB_INCLUDE_DIRS
    NAMES libusb-1.0/libusb.h
    HINTS $ENV{USB_DIR}/include
        ${PC_USB_INCLUDEDIR}
    PATHS /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    USB_LIBRARIES
    NAMES usb-1.0
    HINTS $ENV{SOC_DIR}/lib
        ${PC_USB_LIBDIR}
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          /usr/x86_64-linux-gnu
          /lib/x86_64-linux-gnu
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(USB DEFAULT_MSG USB_LIBRARIES USB_INCLUDE_DIRS)
