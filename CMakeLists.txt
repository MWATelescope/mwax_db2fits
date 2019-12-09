#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(mwax_db2fits)

add_definitions(-D_GNU_SOURCE=1)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include ../mwax_common /opt/psrdada/src /opt/cfitsio/cfitsio-3.47) # -I flags for compiler
link_directories(${CMAKE_SOURCE_DIR}/lib /usr/local/cuda/lib64)        # -L flags for linker

set(PROGSRC src/main.c src/args.c src/dada_dbfits.c src/fitswriter.c src/global.c src/health.c src/utils.c)            # define sources

IF(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")    
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

add_executable(mwax_db2fits ${PROGSRC})       # define executable target prog, specify sources
target_link_libraries(mwax_db2fits pthread cfitsio psrdada cudart m)   # -l flags for linking target
