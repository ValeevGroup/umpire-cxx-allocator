#; -*-CMake-*-

cmake_minimum_required (VERSION 3.14.0)  # for FetchContent_MakeAvailable

set(VRGUmpireCXXAllocator_TRACKED_VGCMAKEKIT_TAG 4c949fd7ccfe4b4f0e103288a5c0f557c6e740c0)
set(VRGUmpireCXXAllocator_TRACKED_UMPIRE_TAG 8c85866107f78a58403e20a2ae8e1f24c9852287)

# Safety net for dev workflow: accidental install will not affect FindOrFetch*
if (NOT DEFINED CACHE{CMAKE_FIND_NO_INSTALL_PREFIX})
    set(CMAKE_FIND_NO_INSTALL_PREFIX ON CACHE BOOL "Whether find_* commands will search CMAKE_INSTALL_PREFIX and CMAKE_STAGING_PREFIX; see https://cmake.org/cmake/help/latest/variable/CMAKE_FIND_NO_INSTALL_PREFIX.html#variable:CMAKE_FIND_NO_INSTALL_PREFIX")
endif()

# Bring ValeevGroup cmake toolkit ==============================================
include(FetchContent)
if (DEFINED PROJECT_BINARY_DIR)
    set(VG_CMAKE_KIT_PREFIX_DIR PROJECT_BINARY_DIR)
else ()
    set(VG_CMAKE_KIT_PREFIX_DIR CMAKE_CURRENT_BINARY_DIR)
endif()
FetchContent_Declare(
        vg_cmake_kit
        QUIET
        GIT_REPOSITORY      https://github.com/ValeevGroup/kit-cmake.git
        GIT_TAG             ${VRGUmpireCXXAllocator_TRACKED_VGCMAKEKIT_TAG}
        SOURCE_DIR ${${VG_CMAKE_KIT_PREFIX_DIR}}/cmake/vg
        BINARY_DIR ${${VG_CMAKE_KIT_PREFIX_DIR}}/cmake/vg-build
        SUBBUILD_DIR ${${VG_CMAKE_KIT_PREFIX_DIR}}/cmake/vg-subbuild
)
FetchContent_MakeAvailable(vg_cmake_kit)
list(APPEND CMAKE_MODULE_PATH "${vg_cmake_kit_SOURCE_DIR}/modules")

project(umpire-cxx-allocator
        VERSION 1.0.0
        DESCRIPTION "C++ standard-compliant allocator for LLNL/Umpire"
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/ValeevGroup/umpire-cxx-allocator")

# Set install paths ============================================================

include(GNUInstallDirs)
set(${PROJECT_NAME}_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}"
        CACHE PATH "${PROJECT_NAME} binary install directory")
set(${PROJECT_NAME}_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}"
        CACHE PATH "${PROJECT_NAME} INCLUDE install directory")
set(${PROJECT_NAME}_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}"
        CACHE PATH "${PROJECT_NAME} LIB install directory")
set(${PROJECT_NAME}_INSTALL_DATADIR "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/${PROJECT_VERSION}"
        CACHE PATH "${PROJECT_NAME} DATA install directory")
set(${PROJECT_NAME}_INSTALL_DOCDIR "${${PROJECT_NAME}_INSTALL_DATADIR}/doc"
        CACHE PATH "${PROJECT_NAME} DOC install directory")
set(${PROJECT_NAME}_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        CACHE PATH "${PROJECT_NAME} CMAKE install directory")

add_library(umpire-cxx-allocator INTERFACE)
install( FILES umpire_cxx_allocator.hpp DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDEDIR} COMPONENT ${PROJECT_NAME})

add_custom_target(vrg-build-external-projects)
if (NOT TARGET umpire)

    find_package(umpire CONFIG)

    # if user provides UMPIRE, use it
    if(TARGET umpire)

        message(STATUS "umpire found at ${umpire_CONFIG_DIR}")

    else()

        # to pass CMAKE_C_* vars to external project
        enable_language(C)

        # set source and build path for umpire in the TiledArray project
        set(EXTERNAL_SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/VRG_umpire-src)
        set(EXTERNAL_BUILD_DIR ${FETCHCONTENT_BASE_DIR}/VRG_umpire-build)
        set(EXTERNAL_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

        if (NOT UMPIRE_URL)
            set(UMPIRE_URL https://github.com/LLNL/Umpire.git)
        endif (NOT UMPIRE_URL)
        if (NOT UMPIRE_TAG)
            set(UMPIRE_TAG ${VRGUmpireCXXAllocator_TRACKED_UMPIRE_TAG})
        endif (NOT UMPIRE_TAG)

        message("** Will clone umpire from ${UMPIRE_URL}")

        option(UMPIRE_ENABLE_ASSERTS "Enables Umpire asserts" OFF)
        option(UMPIRE_ENABLE_LOGGING "Enables Umpire logging" OFF)

        # as of now BLT only supports up to C++20, so limit CMAKE_CXX_STANDARD
        if (NOT CMAKE_CXX_STANDARD)
            set(BLT_CXX_STD 17)
        else()
            set(BLT_CXX_STD ${CMAKE_CXX_STANDARD})
        endif()
        set(BLT_CXX_STD_MAX 20)
        if (BLT_CXX_STD GREATER ${BLT_CXX_STD_MAX})
            set(BLT_CXX_STD ${BLT_CXX_STD_MAX})
        endif()

        if (CMAKE_PREFIX_PATH)
            set(UMPIRE_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
        endif()

        set(UMPIRE_CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_DIR}
                -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
                -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DCMAKE_PREFIX_PATH=${UMPIRE_CMAKE_PREFIX_PATH}
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                -DCMAKE_C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
                -DCMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}
                -DCMAKE_C_FLAGS_RELWITHDEBINFO=${CMAKE_C_FLAGS_RELWITHDEBINFO}
                -DCMAKE_C_FLAGS_MINSIZEREL=${CMAKE_C_FLAGS_MINSIZEREL}
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
                -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
                -DCMAKE_CXX_FLAGS_RELWITHDEBINFO=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
                -DCMAKE_CXX_FLAGS_MINSIZEREL=${CMAKE_CXX_FLAGS_MINSIZEREL}
                -DCMAKE_CXX_STANDARD=${BLT_CXX_STD}
                -DCMAKE_AR=${CMAKE_AR}
                -DBLT_CXX_STD=c++${BLT_CXX_STD}
                -DENABLE_BENCHMARKS=OFF
                -DENABLE_OPENMP=OFF
                -DENABLE_TESTS=OFF
                -DENABLE_EXAMPLES=OFF
                -DENABLE_LOGGING=${UMPIRE_ENABLE_LOGGING}
                -DENABLE_ASSERTS=${UMPIRE_ENABLE_ASSERTS}
                -DENABLE_CLANGFORMAT=OFF
        )

        if (DEFINED CMAKE_CXX_EXTENSIONS)
            list(APPEND UMPIRE_CMAKE_ARGS
                    -DCMAKE_CXX_EXTENSIONS=${CMAKE_CXX_EXTENSIONS}
            )
        endif()

        # caveat: on recent Ubuntu default libstdc++ provides filesystem, but if using older gcc (gcc-8) must link against
        # libstdc++fs: https://bugs.launchpad.net/ubuntu/+source/gcc-8/+bug/1824721 ... skip the use of std::filesystem altogether with pre-9 gcc!!!
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
            # disable by populating cache with compile test result variable
            list(APPEND UMPIRE_CMAKE_ARGS
                    -DUMPIRE_ENABLE_FILESYSTEM=OFF)
        endif()

        if (UMPIRE_ENABLE_CUDA)
            list(APPEND UMPIRE_CMAKE_ARGS
                    -DENABLE_CUDA=ON
                    -DCMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER}
                    -DCMAKE_CUDA_STANDARD=${CMAKE_CUDA_STANDARD}
                    -DCMAKE_CUDA_EXTENSIONS=${CMAKE_CUDA_EXTENSIONS}
                    -DCMAKE_CUDA_HOST_COMPILER=${CMAKE_CUDA_HOST_COMPILER}
                    -DCUDA_TOOLKIT_ROOT_DIR=${CUDAToolkit_ROOT}
            )
            if (DEFINED CMAKE_CUDA_ARCHITECTURES)
                list(APPEND UMPIRE_CMAKE_ARGS "-DCMAKE_CUDA_ARCHITECTURES=${CMAKE_CUDA_ARCHITECTURES}")
            endif(DEFINED CMAKE_CUDA_ARCHITECTURES)
            # BLT will need FindCUDA until https://github.com/LLNL/blt/pull/585 is merged
            # with CMake 3.28.1 needs to set CMP0146 to OLD
            if (POLICY CMP0146)
                list(APPEND UMPIRE_CMAKE_ARGS -DCMAKE_POLICY_DEFAULT_CMP0146=OLD)
            endif()
            # as of CMake 3.28+ FindCUDA seems to require CUDA_TOOLKIT_ROOT_DIR to be defined
            if (DEFINED CUDA_TOOLKIT_ROOT_DIR)
                list(APPEND UMPIRE_CMAKE_ARGS "-DCUDA_TOOLKIT_ROOT_DIR=${CUDA_TOOLKIT_ROOT_DIR}")
            endif()
        endif()
        if (UMPIRE_ENABLE_HIP)
            list(APPEND UMPIRE_CMAKE_ARGS
                    -DENABLE_HIP=ON
                    -DCMAKE_HIP_COMPILER=${CMAKE_HIP_COMPILER}
                    -DCMAKE_HIP_STANDARD=${CMAKE_HIP_STANDARD}
                    -DCMAKE_HIP_EXTENSIONS=${CMAKE_HIP_EXTENSIONS}
            )
            if (DEFINED CMAKE_HIP_ARCHITECTURES)
                list(APPEND UMPIRE_CMAKE_ARGS "-DCMAKE_HIP_ARCHITECTURES=${CMAKE_HIP_ARCHITECTURES}")
            endif(DEFINED CMAKE_HIP_ARCHITECTURES)
        endif()
        if (CMAKE_TOOLCHAIN_FILE)
            set(UMPIRE_CMAKE_ARGS "${UMPIRE_CMAKE_ARGS}"
                    "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            )
        endif(CMAKE_TOOLCHAIN_FILE)
        if (DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
            set(UMPIRE_CMAKE_ARGS "${UMPIRE_CMAKE_ARGS}"
                    "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}")
        endif(DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        if (DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION_${CMAKE_BUILD_TYPE})
            set(UMPIRE_CMAKE_ARGS "${UMPIRE_CMAKE_ARGS}"
                    "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION_${CMAKE_BUILD_TYPE}=${CMAKE_INTERPROCEDURAL_OPTIMIZATION_${CMAKE_BUILD_TYPE}}")
        endif(DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION_${CMAKE_BUILD_TYPE})

        foreach(lang C CXX CUDA)
            if (DEFINED CMAKE_${lang}_COMPILER_LAUNCHER)
                list(APPEND UMPIRE_CMAKE_ARGS
                        "-DCMAKE_${lang}_COMPILER_LAUNCHER=${CMAKE_${lang}_COMPILER_LAUNCHER}")
            endif()
        endforeach()

        if (BUILD_SHARED_LIBS)
            set(UMPIRE_DEFAULT_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
        else(BUILD_SHARED_LIBS)
            set(UMPIRE_DEFAULT_LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
        endif(BUILD_SHARED_LIBS)

        # N.B. Ninja needs spelling out the byproducts of custom targets, see https://cmake.org/cmake/help/v3.3/policy/CMP0058.html
        set(UMPIRE_BUILD_BYPRODUCTS "${EXTERNAL_BUILD_DIR}/lib/libumpire${UMPIRE_DEFAULT_LIBRARY_SUFFIX}")
        message(STATUS "custom target umpire is expected to build these byproducts: ${UMPIRE_BUILD_BYPRODUCTS}")

        include(VRGImportExternalProject)
        VRGImportExternalProject(
                NAME            VRG_umpire
                GIT_REPOSITORY  ${UMPIRE_URL}
                GIT_TAG         ${UMPIRE_TAG}
                TARGETS         umpire
                BUILD_BYPRODUCTS "${UMPIRE_BUILD_BYPRODUCTS}"
                EXTRA_CMAKE_ARGS
                ${UMPIRE_CMAKE_ARGS}
        )

        # VRG_umpire target depends on existence of these directories to be usable from the build tree at configure time
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNAL_SOURCE_DIR}/src/tpl/umpire/camp/include")
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNAL_BUILD_DIR}/src/tpl/umpire/camp/include")
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNAL_BUILD_DIR}/include")

        # do install of Umpire as part of building host project's install target
        install(CODE
                "execute_process(
               COMMAND \"${CMAKE_COMMAND}\" \"--build\" \".\" \"--target\" \"install\"
               WORKING_DIRECTORY \"${EXTERNAL_BUILD_DIR}\"
               RESULT_VARIABLE error_code)
               if(error_code)
                 message(FATAL_ERROR \"Failed to install Umpire\")
               endif()
            ")

        # Add Umpire dependency to External
        add_dependencies(vrg-build-external-projects umpire)

        set(_UMPIRE_INSTALL_DIR ${EXTERNAL_INSTALL_DIR})

        set_target_properties(
                umpire
                PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES
                "$<BUILD_INTERFACE:${EXTERNAL_SOURCE_DIR}/src>;$<BUILD_INTERFACE:${EXTERNAL_SOURCE_DIR}/src/tpl>;$<BUILD_INTERFACE:${EXTERNAL_SOURCE_DIR}/src/tpl/umpire/camp/include>;$<BUILD_INTERFACE:${EXTERNAL_SOURCE_DIR}/src/tpl/umpire/fmt/include>;$<BUILD_INTERFACE:${EXTERNAL_BUILD_DIR}/src/tpl/umpire/camp/include>;$<BUILD_INTERFACE:${EXTERNAL_BUILD_DIR}/include>;$<INSTALL_INTERFACE:${_UMPIRE_INSTALL_DIR}/include>"
                INTERFACE_LINK_LIBRARIES
                "$<BUILD_INTERFACE:${UMPIRE_BUILD_BYPRODUCTS}>;$<INSTALL_INTERFACE:${_UMPIRE_INSTALL_DIR}/lib/libumpire${UMPIRE_DEFAULT_LIBRARY_SUFFIX}>"
                INTERFACE_LINK_OPTIONS
                "LINKER:-rpath,${EXTERNAL_BUILD_DIR}/lib"
                INTERFACE_COMPILE_DEFINITIONS
                FMT_HEADER_ONLY=1
        )

        install(TARGETS umpire EXPORT ${PROJECT_NAME} COMPONENT umpire)

    endif()

endif(NOT TARGET umpire)

target_link_libraries(umpire-cxx-allocator INTERFACE umpire)
target_include_directories(umpire-cxx-allocator INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>;$<INSTALL_INTERFACE:${${PROJECT_NAME}_INSTALL_INCLUDEDIR}>")
add_dependencies(umpire-cxx-allocator vrg-build-external-projects)

install(TARGETS umpire-cxx-allocator EXPORT ${PROJECT_NAME})

# Create the version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion)

# Create the targets file
export(EXPORT ${PROJECT_NAME}
       FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake")

# Create the configure file
configure_package_config_file(cmake/${PROJECT_NAME}-config.cmake.in
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION "${${PROJECT_NAME}_INSTALL_CMAKEDIR}"
        PATH_VARS CMAKE_INSTALL_PREFIX ${PROJECT_NAME}_INSTALL_BINDIR
        ${PROJECT_NAME}_INSTALL_INCLUDEDIR ${PROJECT_NAME}_INSTALL_LIBDIR
        ${PROJECT_NAME}_INSTALL_DOCDIR ${PROJECT_NAME}_INSTALL_CMAKEDIR)

# Install config, version, and target files
install(EXPORT ${PROJECT_NAME}
        FILE "${PROJECT_NAME}-targets.cmake"
        DESTINATION "${${PROJECT_NAME}_INSTALL_CMAKEDIR}"
        COMPONENT ${PROJECT_NAME})
install(FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION "${${PROJECT_NAME}_INSTALL_CMAKEDIR}"
        COMPONENT ${PROJECT_NAME})
