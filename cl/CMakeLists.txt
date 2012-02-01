# Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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
cmake_minimum_required(VERSION 2.6)
project(cl C CXX)
enable_testing()

include("../build-aux/common.cmake")

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
    add_definitions("-DHAVE_ISATTY=1")
else()
    add_definitions("-DHAVE_ISATTY=0")
endif()

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

# set the path to GCC headers for buildign plug-ins
set_source_files_properties(gcc/clplug.c PROPERTIES
    COMPILE_FLAGS "-I${GCC_INC_DIR}")

# tweak include dirs, etc.
add_definitions("-I../include -fPIC")

# libcl.so
add_library(cl STATIC
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
    code_listener.cc
    gcc/clplug.c
    killer.cc
    loopscan.cc
    ssd.cc
    stopwatch.cc
    storage.cc
    version.c)

add_library(cl_test SHARED cl_test.cc)
target_link_libraries(cl_test cl)

# make install
install(TARGETS cl DESTINATION lib)
install(DIRECTORY ../include/cl DESTINATION include)

set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_HOST}")
set(cmd "${cmd} ${CFLAGS}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} -I${GCC_INC_DIR}")
set(cmd "${cmd} -c ${cl_SOURCE_DIR}/gcc/clplug.c")
set(cmd "${cmd} -o /dev/null")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/libcl_test.so")
set(cmd "${cmd} -fplugin-arg-libcl_test-dry-run")
add_test("compile-self" sh -c "${cmd}")

set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_HOST}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} -I${GCC_INC_DIR}")
set(cmd "${cmd} -c ${cl_SOURCE_DIR}/gcc/clplug.c -o /dev/null")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/libcl_test.so")
set(cmd "${cmd} | diff -up ${cl_SOURCE_DIR}/cl_test.out -")
add_test("cl_test" sh -c "${cmd}")

set(cmd "${GCC_HOST} ${CFLAGS}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} -I${GCC_INC_DIR}")
set(cmd "${cmd} -c ${cl_SOURCE_DIR}/gcc/clplug.c")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/libcl_test.so")
set(cmd "${cmd} -fplugin-arg-libcl_test-dump-pp=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_test-dump-types")
set(cmd "${cmd} -fplugin-arg-libcl_test-gen-dot=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_test-type-dot=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_test-verbose=15")
set(cmd "${cmd} -fplugin-arg-libcl_test-dry-run")

add_test("compile-self-extra" sh -c "LC_ALL=C CCACHE_DISABLE=1 ${cmd}")
if(TEST_WITH_VALGRIND)
    set(cmd "valgrind --trace-children=yes --error-exitcode=7 ${cmd}")
    add_test("compile-self-valgrind" sh -c "${cmd}")
endif()

add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/config_cl.h)

add_test("headers_sanity-1" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/../include/cl/code_listener.h)

add_test("headers_sanity-2" make -j
    -C ${cl_SOURCE_DIR}/../include/cl
    -f ${cl_SOURCE_DIR}/Makefile.chk)

add_test("headers_sanity-3" make -j
    -C ${cl_SOURCE_DIR}
    -f ${cl_SOURCE_DIR}/Makefile.chk)
