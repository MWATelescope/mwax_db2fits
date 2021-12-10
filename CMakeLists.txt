#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(mwax_db2fits)

add_definitions(-D_GNU_SOURCE=1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(OpenMP REQUIRED)
find_package(CUDA REQUIRED)
find_package(CFITSIO)
find_package(XGPU)
find_package(PSRDADA)

if(NOT MWAX_COMMON_ROOT)
    set(MWAX_COMMON_ROOT ../mwax_common)
endif(NOT MWAX_COMMON_ROOT)

include_directories(${CMAKE_SOURCE_DIR}/include ${MWAX_COMMON_ROOT} ${PSRDADA_INCLUDE_DIR} ${CFITSIO_INCLUDE_DIR}) # -I flags for compiler

set(PROGSRC src/main.c src/args.c ${MWAX_COMMON_ROOT}/mwax_global_defs.c src/dada_dbfits.c src/fitswriter.c src/global.c src/health.c src/utils.c)            # define sources

IF(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")    
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

add_executable(mwax_db2fits ${PROGSRC})       # define executable target prog, specify sources
target_link_libraries(mwax_db2fits pthread ${CFITSIO_LIBRARY} ${PSRDADA_LIBRARY} cudart m)   # -l flags for linking target

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mwax_db2fits DESTINATION bin)
