cmake_minimum_required(VERSION 3.14)

project(a2i LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "User install prefix" FORCE)
endif()

set(SOURCE_DIR "source")
set(INCLUDE_DIR "include")

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.hpp")
include_directories(${INCLUDE_DIR})

include(GNUInstallDirs)

add_library(a2i STATIC ${LIB_SOURCES})

#find_package(PkgConfig REQUIRED)
#pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
#target_link_libraries(a2i PRIVATE PkgConfig::FFTW)
#pkg_check_modules(FFTW REQUIRED fftw3)
#target_include_directories(a2i PRIVATE ${FFTW_INCLUDE_DIRS})
#target_link_libraries(a2i PRIVATE ${FFTW_LIBRARIES})

#find_package(PkgConfig REQUIRED)
#pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
#target_link_libraries(a2i PRIVATE PkgConfig::FFTW)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3)
#pkg_check_modules(FFTW3_THREADS REQUIRED IMPORTED_TARGET fftw3_threads)
target_link_libraries(a2i PRIVATE PkgConfig::FFTW3)



find_package(OpenCV CONFIG REQUIRED)
#target_include_directories(a2i PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(a2i PRIVATE ${OpenCV_LIBS})

find_package(OpenMP REQUIRED)
target_compile_options(a2i PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(a2i PRIVATE OpenMP::OpenMP_CXX)

target_include_directories(a2i PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

install(TARGETS a2i
    EXPORT a2iTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/a2i)

install(EXPORT a2iTargets
    FILE a2iTargets.cmake
    NAMESPACE a2i::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/a2i/cmake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/a2iConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(a2iConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/a2iConfig.cmake" @ONLY)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/a2iConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/a2iConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/a2i/cmake)

export(EXPORT a2iTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/a2iTargets.cmake"
    NAMESPACE a2i::)
export(PACKAGE a2i)