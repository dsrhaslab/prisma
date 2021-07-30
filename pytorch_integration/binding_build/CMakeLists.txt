cmake_minimum_required(VERSION 3.16)
project("prisma_binding")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")
set(CMAKE_CXX_STANDARD 17)
set(LIBRARY_SOURCE_FILES
    src/module_binding.cpp 
    src/prisma.h 
    src/prisma.cpp 
    src/file_info.cpp 
    src/file_info.h 
    src/configs.cpp 
    src/configs.h 
    src/file_content.cpp 
    src/file_content.h 
    src/logger.cpp 
    src/logger.h 
    src/profiler.cpp 
    src/profiler.h 
    src/server.cpp 
    src/server.h 
    src/client.cpp 
    src/client.h 
    src/autotuner.cpp
    src/autotuner.h)

# Boost
set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/boost_root")
set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")

# Pybind11
add_subdirectory(pybind11)

# Module
pybind11_add_module (
    ${PROJECT_NAME}
    SHARED
    ${LIBRARY_SOURCE_FILES}
)

include(GNUInstallDirs) # independent of lib/lib64
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
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
target_link_libraries(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_DIR}/tbb/${CMAKE_INSTALL_LIBDIR}/libtbb.so)
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
add_dependencies(${PROJECT_NAME} tbb)