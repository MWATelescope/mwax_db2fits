#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(mwa_xc_datacapture)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(PSRHOME "/home/gsleap/work/cvs/psrdada")

include_directories(${CMAKE_SOURCE_DIR}/include ${PSRHOME}/src) # -I flags for compiler
link_directories(${CMAKE_SOURCE_DIR}/lib ${PSRHOME}/build/lib)        # -L flags for linker

set(PROGSRC src/main.c src/args.c src/log.c src/fitswriter.c src/utils.c src/dada_nicdb.c)            # define sources

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

add_executable(mwa_xc_datacapture ${PROGSRC})       # define executable target prog, specify sources
target_link_libraries(mwa_xc_datacapture pthread cfitsio psrdada)   # -l flags for linking target
