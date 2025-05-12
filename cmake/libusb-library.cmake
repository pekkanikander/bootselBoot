# cmake/libusb-library.cmake

# Locate libusb via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

find_library(LIBUSB_LIB_PATH
  NAMES usb-1.0
  PATHS ${LIBUSB_LIBRARY_DIRS}
  NO_DEFAULT_PATH
)

# Define imported target
add_library(LIBUSB::LIBUSB SHARED IMPORTED)
set_target_properties(LIBUSB::LIBUSB PROPERTIES
  IMPORTED_LOCATION "${LIBUSB_LIB_PATH}"
  INTERFACE_INCLUDE_DIRECTORIES "${LIBUSB_INCLUDE_DIRS}"
)
