cmake_minimum_required(VERSION 3.10)
project(prisma)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -O2")


#--- LIBRARY

# Set PRISMA_HEADER_FILES var
set(PRISMA_HEADER_FILES prisma.h file_info.h configs.h file_content.h logger.h profiler.h autotuner.h client.h server.h)

# Set PRISMA_SOURCE_FILES var
set(PRISMA_SOURCE_FILES prisma.cpp file_info.cpp configs.cpp file_content.cpp logger.cpp profiler.cpp autotuner.cpp client.cpp server.cpp)

# Create shared library libprisma.so
add_library(prisma SHARED ${PRISMA_HEADER_FILES} ${PRISMA_SOURCE_FILES})


#--- DEPENDENCIES

include(GNUInstallDirs) # independent of lib/lib64
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/../third_party)
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR})
link_directories(${THIRD_PARTY_DIR}/tbb/${CMAKE_INSTALL_LIBDIR})
include_directories(${THIRD_PARTY_DIR}/tbb/include)

# CTPL
# Include CTPL header files
file(MAKE_DIRECTORY ${THIRD_PARTY_DIR}/ctpl)
file(DOWNLOAD
        https://raw.githubusercontent.com/vit-vit/CTPL/master/ctpl.h
        ${THIRD_PARTY_DIR}/ctpl/ctpl.h)
include_directories(${THIRD_PARTY_DIR}/ctpl)

# Intel TBB
target_link_libraries(prisma ${THIRD_PARTY_DIR}/tbb/${CMAKE_INSTALL_LIBDIR}/libtbb.so)
include(ExternalProject)
ExternalProject_Add(
        tbb
        GIT_REPOSITORY https://github.com/oneapi-src/oneTBB.git
        GIT_TAG master
        INSTALL_DIR ${THIRD_PARTY_DIR}/tbb
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${THIRD_PARTY_DIR}/tbb
        -DTBB_STRICT=OFF
)
add_dependencies(prisma tbb)

# Set EXECUTABLE_FILES var
set(EXECUTABLE_FILES main.cpp)

# Create exacutable
add_executable(prisma-middleware ${EXECUTABLE_FILES})

# Include header files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Declare dependency on libprisma.so
target_link_libraries(prisma-middleware ${CMAKE_CURRENT_BINARY_DIR}/libprisma.so)