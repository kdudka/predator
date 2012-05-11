# Copyright (C) 2010 Jiri Simacek
#
# This file is part of forester.
#
# forester is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# forester is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with forester.  If not, see <http://www.gnu.org/licenses/>.

# project metadata
cmake_minimum_required(VERSION 2.6)
project(fa C CXX)
enable_testing()

include("../build-aux/common.cmake")

option(FA_DEBUG "Set to ON to enable debugging support" OFF)
if(FA_DEBUG)
    add_definitions("-O0 -g")
    message (STATUS "Debugging support enabled...")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

## Check for isatty(3)
#include(CheckFunctionExists)
#check_function_exists("isatty" HAVE_ISATTY)
#if (HAVE_ISATTY)
#    add_definitions("-DHAVE_ISATTY=1")
#else()
#    add_definitions("-DHAVE_ISATTY=0")
#endif()

# helping scripts
configure_file( ${PROJECT_SOURCE_DIR}/fagcc.in     ${PROJECT_BINARY_DIR}/fagcc     @ONLY)
configure_file( ${PROJECT_SOURCE_DIR}/fagccp.in    ${PROJECT_BINARY_DIR}/fagccp    @ONLY)
configure_file( ${PROJECT_SOURCE_DIR}/fagccv.in    ${PROJECT_BINARY_DIR}/fagccv    @ONLY)
configure_file( ${PROJECT_SOURCE_DIR}/fagccvf.in   ${PROJECT_BINARY_DIR}/fagccvf   @ONLY)
configure_file( ${PROJECT_SOURCE_DIR}/fagdb.in     ${PROJECT_BINARY_DIR}/fagdb     @ONLY)

configure_file(${PROJECT_SOURCE_DIR}/register-paths.sh.in
    ${PROJECT_BINARY_DIR}/register-paths.sh                                       @ONLY)

# libfa.so
add_library(fa SHARED
	treeaut.cc
	timbuk.cc
	forestaut.cc
	sequentialinstruction.cc
	jump.cc
	call.cc
	comparison.cc
	fixpoint.cc
	microcode.cc
	compiler.cc
	symctx.cc
	symexec.cc
	cl_fa.cc
)
set_target_properties(fa PROPERTIES LINK_FLAGS -lrt)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fa ${CL_LIB})

option(TEST_ONLY_FAST "Set to OFF to boost test coverage" ON)

set(GCC_EXEC_PREFIX "timeout 120"
    CACHE STRING "Set to empty string if not sure")

set(testdir "${fa_SOURCE_DIR}/../tests/forester-regre")

# basic tests
set(tests
          f0001 f0002 f0003 f0004 f0005 f0006 f0007       f0009
    f0010 f0011 f0012 f0013 f0014       f0016             f0019
    f0020       f0022       f0024 f0025

# Predator tests
          p0001
                      p0023                         p0028 p0029
                                              p0037
                p0042             p0045 p0046
                                  p0075 p0076
          p0091
                                                          p0119
                                        p0126
                      p0143
    p0170                         p0175
)

if(TEST_ONLY_FAST)
else()

# expensive tests
set(tests ${tests}
                                  f0015
)

endif(TEST_ONLY_FAST)


# tests known to fail now (but shouldn't!)
set(tests-known-to-fail
                                                          p0019
                                  p0025 p0026 p0027
          p0041
                            p0074
                      p0083       p0085
                                                          p0159
                                                    p0198
                                              p0507
)

set(cost 1)

macro(test_forester_regre name_suff ext arg1)
    foreach (num ${tests})
        set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_EXEC_PREFIX} ${GCC_HOST}")

        # we use the following flag to avoid differences on 32bit vs 64bit archs
        # in the error output, which is checked for exact match
        set(cmd "${cmd} -m32")

        set(cmd "${cmd} -S ${testdir}/test-${num}.c -o /dev/null")
        set(cmd "${cmd} -I../include/predator-builtins -DFORESTER")
        set(cmd "${cmd} -fplugin=${fa_BINARY_DIR}/libfa.so ${arg1}")
        set(cmd "${cmd} -fplugin-arg-libfa-preserve-ec")
        set(cmd "${cmd} 2>&1")

        # filter out messages that are unrelated to our plug-in
        set(cmd "${cmd} | (grep -E '\\\\[-fplugin=libfa.so\\\\]\$|compiler error|undefined symbol'; true)")
        set(cmd "${cmd} | sed 's/ \\\\[-fplugin=libfa.so\\\\]\$//'")

        # filter out NOTE messages with internal location
        set(cmd "${cmd} | (grep -v 'note: .*\\\\[internal location\\\\]'; true)")

        # drop absolute paths
        set(cmd "${cmd} | sed 's|^[^:]*/||'")

        # drop var UIDs that are not guaranteed to be fixed among runs
        set(cmd "${cmd} | sed -r -e 's|#[0-9]+:||g' -e 's|#[0-9]+|_|g'")

        # ... and finally diff with the expected output
        set(cmd "${cmd} | diff -up ${testdir}/test-${num}.err${ext} -")
        set(test_name "test-${num}.c${name_suff}")
        add_test(${test_name} bash -o pipefail -c "${cmd}")

        SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
        MATH(EXPR cost "${cost} + 1")
    endforeach()
endmacro(test_forester_regre)

# default mode
test_forester_regre("" "" "")
