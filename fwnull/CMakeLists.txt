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
project(fwnull C CXX)
enable_testing()

include("../build-aux/common.cmake")

option(FWNULL_DEBUG "Set to ON to enable debugging support" OFF)
if(FWNULL_DEBUG)
    add_definitions("-O0 -g")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# libfwnull.so
add_library(fwnull SHARED
    cl_fwnull.cc
    version.c)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fwnull ${CL_LIB})

# make install
install(TARGETS fwnull DESTINATION lib)

set(cost 1048576)
set(testdir "${fwnull_SOURCE_DIR}/../tests/fwnull-regre")

macro(test_fwnull tname)
    set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_HOST}")

    # we use the following flag to avoid differences on 32bit vs 64bit archs
    # in the error output, which is checked for exact match
    set(cmd "${cmd} -m32")

    set(cmd "${cmd} -S ${testdir}/${tname}.c")
    set(cmd "${cmd} -fplugin=${fwnull_BINARY_DIR}/libfwnull.so")
    set(cmd "${cmd} -fplugin-arg-libfwnull-preserve-ec")
    set(cmd "${cmd} 2>&1")

    # filter out messages that are unrelated to our plug-in
    set(cmd "${cmd} | grep -E '\\\\[-fplugin=libfwnull.so\\\\]\$|compiler error|undefined symbol'")
    set(cmd "${cmd} | sed 's/ \\\\[-fplugin=libfwnull.so\\\\]\$//'")

    # filter out NOTE messages with internal location
    set(cmd "${cmd} | grep -v 'note: .*\\\\[internal location\\\\]'")

    # drop absolute paths
    set(cmd "${cmd} | sed 's|^[^:]*/||'")

    # ... and finally diff with the expected output
    set(cmd "${cmd} | diff -up ${testdir}/${tname}.err -")
    set(test_name "${tname}.c")
    add_test(${test_name} sh -c "${cmd}")

    SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
    MATH(EXPR cost "${cost} - 1")
endmacro(test_fwnull)

test_fwnull(fwnull-0001)
test_fwnull(libcurl-rtsp-32bit)
