# Copyright (C) 2010-2025 Kamil Dudka <kdudka@redhat.com>
#
# This file is part of predator.
#
# predator is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# predator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with predator.  If not, see <http://www.gnu.org/licenses/>.

# project metadata
cmake_minimum_required(VERSION 3.5)
project(cl C CXX)
enable_testing()

# fail immediately if no C++ compiler is found
if("${CMAKE_CXX_COMPILER}" MATCHES ".*NOTFOUND")
    message(FATAL_ERROR "
Unable to find system C++ compiler!")
endif()

if(APPLE)
    if(ENABLE_LLVM)
        if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
            # for regular Clang or AppleClang
            message(FATAL_ERROR "LLVM plug-in requires the use of "
                "(Apple)Clang compiler!")
        endif()
    else()
        if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # for GCC
            message(FATAL_ERROR "GCC plug-in requires the use of GNU compiler!")
        endif()
    endif()
endif()

include("../build-aux/common.cmake")

# suppress an unnecessary warning with gcc-4.4.x
ADD_CXX_ONLY_FLAG("W_NO_INVALID_OFFSETOF" "-Wno-invalid-offsetof")

option(CL_DEBUG "Set to ON to enable debugging support" OFF)
if(CL_DEBUG)
    add_definitions("-O0 -g")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# Check for isatty(3)
include(CheckFunctionExists)
check_function_exists("isatty" HAVE_ISATTY)
if (HAVE_ISATTY)
    set_source_files_properties(ssd.cc PROPERTIES
        COMPILE_FLAGS "-DHAVE_ISATTY=1")
else()
    set_source_files_properties(ssd.cc PROPERTIES
        COMPILE_FLAGS "-DHAVE_ISATTY=0")
endif()

if(ENABLE_LLVM)
    # libclllvm.a
    add_library(clllvm STATIC llvm/clplug.cc)
	if(NOT LLVM_ENABLE_RTTI)
		target_compile_options(clllvm PUBLIC "-fno-rtti")
	endif()
else()  # --------------------------------------------------------------begin

# query GCC_INC_DIR using GCC_HOST
message(STATUS "GCC_HOST: ${GCC_HOST}")
execute_process(COMMAND "${GCC_HOST}" "-print-file-name=plugin"
    RESULT_VARIABLE GCC_HOST_STATUS
    OUTPUT_VARIABLE GCC_PLUG_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if (NOT "${GCC_HOST_STATUS}" EQUAL 0)
    message(FATAL_ERROR "failed to validate GCC_HOST

    Run './switch-host-gcc.sh' to build the plug-in against the system GCC.
    If you need a local build of GCC, run 'make build_gcc'.  See the README
    for details.  If anything goes wrong, tell the authors.")
endif()

# check the legibility of GCC_INC_DIR
set(GCC_INC_DIR "${GCC_PLUG_DIR}/include")
message(STATUS "GCC_INC_DIR: ${GCC_INC_DIR}")
if (NOT EXISTS "${GCC_INC_DIR}/gcc-plugin.h")
    message(FATAL_ERROR "unable to find GCC headers for building plug-ins

    Please make sure the appropriate header files are installed.  On some Linux
    distributions a separate package (e.g. gcc-plugin-devel on Fedora) may be
    required for building GCC plug-ins.")
endif()

# query CC1_HOST using GCC_HOST
execute_process(COMMAND "${GCC_HOST}" "-print-prog-name=cc1"
    RESULT_VARIABLE CC1_HOST_STATUS
    OUTPUT_VARIABLE CC1_HOST OUTPUT_STRIP_TRAILING_WHITESPACE)
if ("${CC1_HOST_STATUS}" EQUAL 0)
    message(STATUS "CC1_HOST: ${CC1_HOST}")
else()
    message(FATAL_ERROR "failed to query CC1_HOST")
endif()

# check for readelf/nm
find_program(READELF readelf)
if (EXISTS ${READELF})
    set(SYM_LISTER "${READELF} -s")
else()
    find_program(NM nm)
    if (EXISTS ${NM})
        set(SYM_LISTER "${NM}")
    endif()
endif()

# libclgcc.a
add_library(clgcc STATIC gcc/clplug.c)

# if we have readelf or nm, check whether cc1 symbols are mangled
if (DEFINED SYM_LISTER)
    message(STATUS "checking whether fancy_abort() in ${CC1_HOST} is mangled")
    execute_process(COMMAND "sh" "-c"
        "${SYM_LISTER} ${CC1_HOST} | grep _Z11fancy_abort > /dev/null"
        RESULT_VARIABLE CXX_LINKAGE_STATUS)
    if ("${CXX_LINKAGE_STATUS}" EQUAL 0)
        message(STATUS "GCC plug-ins will be built with C++ linkage")
        remove_definitions("-pedantic")
        set_source_files_properties(gcc/clplug.c PROPERTIES LANGUAGE CXX)
        set_target_properties(clgcc PROPERTIES COMPILE_FLAGS "-fno-rtti")
    else()
        message(STATUS "GCC plug-ins will be built with C linkage")
    endif()
else()
    message(WARNING "readelf/nm not found, building C linkage by default...")
endif()

# set the path to GCC headers for building plug-ins
set_source_files_properties(gcc/clplug.c PROPERTIES
    COMPILE_FLAGS "-isystem ${GCC_INC_DIR}")

endif() # ----------------------------------------------------------------end

# libcl.a
add_library(cl STATIC
    builtins.cc
    callgraph.cc
    cl_chain.cc
    cl_dotgen.cc
    cl_easy.cc
    cl_factory.cc
    cl_locator.cc
    cl_pp.cc
    cl_storage.cc
    cl_typedot.cc
    cldebug.cc
    clf_intchk.cc
    clf_opchk.cc
    clf_unilabel.cc
    clf_unswitch.cc
    clutil.cc
    clplot.cc
    code_listener.cc
    killer.cc
    loopscan.cc
    memdebug.cc
    pointsto.cc
    pointsto_fics.cc
    ssd.cc
    stopwatch.cc
    storage.cc
    version.c)

# load regression tests
add_subdirectory(tests)
