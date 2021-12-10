# - Try to find PSRDADA code.
# Variables used by this module:
#  PSRDADA_ROOT_DIR     - PSRDADA root directory
# Variables defined by this module:
#  PSRDADA_FOUND        - system has PSRDADA
#  PSRDADA_INCLUDE_DIR  - the PSRDADA include directory (cached)
#  PSRDADA_INCLUDE_DIRS - the PSRDADA include directories
#                         (identical to PSRDADA_INCLUDE_DIR)
#  PSRDADA_LIBRARY      - the PSRDADA library (cached)
#  PSRDADA_LIBRARIES    - the PSRDADA libraries

message("Finding PSRDADA")

set(PSRDADA_ROOT_DIR $ENV{PSRDADA})

if(NOT DEFINED PSRDADA_ROOT_DIR)
    message(STATUS "Warning PSRDADA_ROOT_DIR not set: will try and find it ")
else(NOT DEFINED PSRDADA_ROOT_DIR)
    message(STATUS "PSRDADA_ROOT_DIR = ${PSRDADA_ROOT_DIR}")
endif(NOT DEFINED PSRDADA_ROOT_DIR)

if(NOT PSRDADA_FOUND)

  find_path(PSRDADA_INCLUDE_DIR dada_cuda.h
    HINTS ${PSRDADA_ROOT_DIR} PATH_SUFFIXES /include /include/psrdada)
  find_library(PSRDADA_LIBRARY psrdada
    HINTS ${PSRDADA_ROOT_DIR} PATH_SUFFIXES lib )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PSRDADA DEFAULT_MSG
    PSRDADA_LIBRARY PSRDADA_INCLUDE_DIR)

  set(PSRDADA_INCLUDE_DIRS ${PSRDADA_INCLUDE_DIR})
  set(PSRDADA_LIBRARIES ${PSRDADA_LIBRARY})

endif(NOT PSRDADA_FOUND)

if (PSRDADA_FOUND)
    message (STATUS "Found PSRDADA (${PSRDADA_LIBRARIES})")
endif (PSRDADA_FOUND)

