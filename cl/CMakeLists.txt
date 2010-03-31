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
cmake_minimum_requireD(VERSION 2.8)
project(cl C CXX)
enable_testing()

# Check Boost availability
set(Boost_ADDITIONAL_VERSIONS "1.40" "1.41" "1.42" "1.43")
find_package(Boost 1.37 REQUIRED
    iostreams
    system)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Check for isatty(3)
include(CheckFunctionExists)
check_function_exists("isatty" HAVE_ISATTY)
if (HAVE_ISATTY)
    add_definitions("-DHAVE_ISATTY=1")
else()
    add_definitions("-DHAVE_ISATTY=0")
endif()

# Check for a C compiler flag
include(CheckCCompilerFlag)
macro(ADD_C_FLAG opt_name opt)
    check_c_compiler_flag(${opt} HAVE_${opt_name})
    if(HAVE_${opt_name})
        add_definitions(${opt})
    endif()
endmacro()
macro(ADD_C_ONLY_FLAG opt_name opt)
    check_c_compiler_flag(${opt} C_HAVE_${opt_name})
    if(C_HAVE_${opt_name})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${opt}")
    endif()
endmacro()

# Check for a C++ compiler flag
include(CheckCXXCompilerFlag)
macro(ADD_CXX_ONLY_FLAG opt_name opt)
    check_cxx_compiler_flag(${opt} CXX_HAVE_${opt_name})
    if(CXX_HAVE_${opt_name})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${opt}")
    endif()
endmacro()

# we use c99 to compile *.c and c++0x to copmile *.cc
ADD_C_ONLY_FLAG(  "STD_C99"         "-std=c99")
ADD_CXX_ONLY_FLAG("STD_CXX_0X"      "-std=c++0x")

# tweak warnings
ADD_C_FLAG(       "PEDANTIC"        "-pedantic")
ADD_C_FLAG(       "W_ALL"           "-Wall")
ADD_CXX_ONLY_FLAG("W_NO_DEPRECATED" "-Wno-deprecated")

option(SL_WEXTRA "Set to ON to use -Wextra (recommended)" ON)
if(SL_WEXTRA)
    ADD_C_FLAG("W_EXTRA" "-Wextra")
endif()

option(SL_WERROR "Set to ON to use -Werror (recommended)" OFF)
if(SL_WERROR)
    ADD_C_FLAG("W_ERROR" "-Werror")
endif()

option(SL_DEBUG "Set to OFF to boost performance" ON)
if(SL_DEBUG)
    add_definitions("-O0 -g")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# tweak include dirs, etc.
add_definitions("-I../include -I../include/gcc -fPIC")

# Static library cl
add_library(cl SHARED
    cl_chain.cc
    cl_dotgen.cc
    cl_easy.cc
    cl_factory.cc
    cl_locator.cc
    cl_pp.cc
    cl_storage.cc
    cl_typedot.cc
    cld_argsub.cc
    cld_intchk.cc
    cld_opchk.cc
    cld_optrans.cc
    cld_unilabel.cc
    cld_uniregs.cc
    cld_univars.cc
    cld_unswitch.cc
    code_listener.cc
    gcc/clplug.c
    location.cc
    ssd.cc
    storage.cc
    version.c)

target_link_libraries(cl
    boost_iostreams
    boost_system)

add_library(cl_test SHARED cl_test.cc)
target_link_libraries(cl_test cl)

set(GCC45 "${cl_BINARY_DIR}/../gcc-install/bin/gcc"
    CACHE STRING "absolute path to executable of gcc-4.5")

add_test("compile-self" ${GCC45}
    ${CFLAGS}
    -I${cl_SOURCE_DIR}/../include
    -I${cl_SOURCE_DIR}/../include/gcc
    -c ${cl_SOURCE_DIR}/gcc/clplug.c
    -o /dev/null
    -fplugin=${cl_BINARY_DIR}/libcl_test.so
    -fplugin-arg-libcl_test-bypass-symexec)

set(cmd "${GCC45}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include -I${cl_SOURCE_DIR}/../include/gcc")
set(cmd "${cmd} -c ${cl_SOURCE_DIR}/gcc/clplug.c -o /dev/null")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/libcl_test.so")
set(cmd "${cmd} | diff -up ${cl_SOURCE_DIR}/cl_test.out -")
add_test("cl_test" sh -c "${cmd}")

set(self_extra_opts
    ${CFLAGS}
    -I${cl_SOURCE_DIR}/../include
    -I${cl_SOURCE_DIR}/../include/gcc
    -c ${cl_SOURCE_DIR}/gcc/clplug.c
    -fplugin=${cl_BINARY_DIR}/libcl_test.so
    -fplugin-arg-libcl_test-dump-pp=/dev/null
    -fplugin-arg-libcl_test-dump-types
    -fplugin-arg-libcl_test-gen-dot=/dev/null
    -fplugin-arg-libcl_test-type-dot=/dev/null
    -fplugin-arg-libcl_test-verbose=15
    -fplugin-arg-libcl_test-bypass-symexec)

add_test("compile-self-extra" ${GCC45} ${self_extra_opts})
add_test("compile-self-valgrind" valgrind
    --error-exitcode=7
    --suppressions=${cl_SOURCE_DIR}/gcc/gcc.valgrind
    ${GCC45}
    ${self_extra_opts})

add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/config.h)

add_test("headers_sanity-1" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/../include/cl/code_listener.h)

add_test("headers_sanity-2" make -j
    -C ${cl_SOURCE_DIR}/../include/cl
    -f ${cl_SOURCE_DIR}/Makefile.chk)

add_test("headers_sanity-3" make -j
    -C ${cl_SOURCE_DIR}
    -f ${cl_SOURCE_DIR}/Makefile.chk)
