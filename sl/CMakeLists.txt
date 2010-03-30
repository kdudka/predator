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
project(sl C CXX)
enable_testing()

# Check Boost availability
set(Boost_ADDITIONAL_VERSIONS "1.40" "1.41" "1.42" "1.43")
find_package(Boost 1.37 REQUIRED
    iostreams
    system
    unit_test_framework)
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
add_definitions("-I.. -I../gcc -I../ssd/src -fPIC")

# Static library sl
add_library(sl STATIC
    cl_chain.cc
    cl_dotgen.cc
    cl_easy.cc
    cl_factory.cc
    cl_locator.cc
    cl_pp.cc
    cl_symexec.cc
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
    location.cc
    ssd.cc
    storage.cc
    symcall.cc
    symdump.cc
    symexec.cc
    symheap.cc
    symplot.cc
    symproc.cc
    symstate.cc
    version.c)

target_link_libraries(sl
    boost_iostreams
    boost_system)

add_library(slplug SHARED slplug.c)
target_link_libraries(slplug sl)

add_executable(test-dll test-dll.cc data/dll.c)
target_link_libraries(test-dll boost_unit_test_framework)
add_test("data/dll.c-self_test" test-dll)

set(GCC45 "${sl_BINARY_DIR}/../gcc-install/bin/gcc"
    CACHE STRING "absolute path to executable of gcc-4.5")

add_test("compile-self" ${GCC45}
    ${CFLAGS}
    -I${sl_SOURCE_DIR}/..
    -I${sl_SOURCE_DIR}/../gcc
    -c ${sl_SOURCE_DIR}/slplug.c
    -fplugin=${sl_BINARY_DIR}/libslplug.so
    -fplugin-arg-libslplug-bypass-symexec)

set(examples
    0002 0003 0004 0005 0006 0007 0008 0009
    0010 0011 0012 0013 0014 0016 0017 0018
    0019 0020 0021 0022 0023 0024 0025 0026
    0027 0028 0029 0030 0031 0032 0033 0034
    0035)

# FIXME: define this macro more generically, in particular the count of args
macro(test_sym_exec label tool arg1 arg2)
    foreach (num ${examples})
        add_test("${label}___example-${num}" ${tool} ${arg} ${arg2}
            -c ${sl_SOURCE_DIR}/data/test-${num}.c
            -fplugin=${sl_BINARY_DIR}/libslplug.so
            -fplugin-arg-libslplug-verbose=1)
    endforeach()
endmacro()
test_sym_exec("simple-test" ${GCC45} "" "")

foreach (num ${examples})
    set(cmd "${GCC45}")

    # we use the following flag to avoid differences on 32bit vs 64bit archs
    # in the error output, which is checked for exact match
    set(cmd "${cmd} -m32")

    set(cmd "${cmd} -c ${sl_SOURCE_DIR}/data/test-${num}.c")
    set(cmd "${cmd} -fplugin=${sl_BINARY_DIR}/libslplug.so")
    set(cmd "${cmd} 2>&1")

    # FIXME: the following line significantly decreases coverage of the test
    # suite; we should perhaps use awk or whatever to trim only the location
    # part from messages with internal location
    set(cmd "${cmd} | grep -v '\\\\[internal location\\\\]\$'")

    # FIXME: the doubly-nested triply-escaped sed magic might look a bit scary
    set(cmd "${cmd} | sed s/`echo ${sl_SOURCE_DIR}/data/ ")
    set(cmd "${cmd} | sed 's/\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//g'`//")
    set(cmd "${cmd} | sed s/`echo ${sl_SOURCE_DIR}/ ")
    set(cmd "${cmd} | sed 's/\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//g'`//")

    # ... and finally diff with the expected output
    set(cmd "${cmd} | diff -up ${sl_SOURCE_DIR}/data/test-${num}.err -")
    add_test("regression-test___example-${num}" sh -c "${cmd}")
endforeach()

set(self_extra_opts
    ${CFLAGS}
    -I${sl_SOURCE_DIR}/..
    -I${sl_SOURCE_DIR}/../gcc
    -c ${sl_SOURCE_DIR}/slplug.c
    -fplugin=${sl_BINARY_DIR}/libslplug.so
    -fplugin-arg-libslplug-dump-pp=/dev/null
    -fplugin-arg-libslplug-dump-types
    -fplugin-arg-libslplug-gen-dot=/dev/null
    -fplugin-arg-libslplug-type-dot=/dev/null
    -fplugin-arg-libslplug-verbose=15
    -fplugin-arg-libslplug-bypass-symexec)

add_test("compile-self-extra" ${GCC45} ${self_extra_opts})
add_test("compile-self-valgrind" valgrind
    --error-exitcode=7
    --suppressions=${sl_SOURCE_DIR}/gcc.valgrind
    ${GCC45}
    ${self_extra_opts})

test_sym_exec("valgrind-test" valgrind
    --error-exitcode=7
    ${GCC45})

add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${sl_SOURCE_DIR}/config.h)

add_test("headers_sanity-1" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${sl_SOURCE_DIR}/code_listener.h)

add_test("headers_sanity-2" make -j
    -C ${sl_SOURCE_DIR}
    -f ${sl_SOURCE_DIR}/Makefile.chk)
