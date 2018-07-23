#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(mwax_db2fits)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

include_directories(${CMAKE_SOURCE_DIR}/include) # -I flags for compiler
link_directories(${CMAKE_SOURCE_DIR}/lib)        # -L flags for linker

set(PROGSRC src/main.c src/args.c src/dada_dbfits.c src/fitswriter.c src/global.c src/health.c src/utils.c)            # define sources

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

add_executable(mwax_db2fits ${PROGSRC})       # define executable target prog, specify sources
target_link_libraries(mwax_db2fits pthread cfitsio psrdada m)   # -l flags for linking target
