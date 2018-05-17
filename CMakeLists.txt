#CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(mwa_xc_datacapture)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
include_directories(${CMAKE_SOURCE_DIR}/include) # -I flags for compiler
link_directories(${CMAKE_SOURCE_DIR}/lib)        # -L flags for linker

set(PROGSRC src/main.c src/fitswriter.c)            # define sources

add_executable(mwa_xc_datacapture ${PROGSRC})       # define executable target prog, specify sources
target_link_libraries(mwa_xc_datacapture cfitsio)   # -l flags for linking target
