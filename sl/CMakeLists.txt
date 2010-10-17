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
project(sl C CXX)
add_definitions("-I../include")
enable_testing()

include("../CMakeLists.common")

# libsl.so
add_library(sl SHARED
    cl_symexec.cc
    symabstract.cc
    symbin.cc
    symbt.cc
    symcall.cc
    symcut.cc
    symdump.cc
    symexec.cc
    symgc.cc
    symheap.cc
    sympath.cc
    symplot.cc
    symproc.cc
    symseg.cc
    symstate.cc
    symutil.cc
    version.c)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(sl ${CL_LIB})

# make install
install(TARGETS sl DESTINATION lib)

# the following include dirs are needed to compile cl/gcc/clplug.c
set(clplug_incs "-I${sl_SOURCE_DIR}/../cl" "-I${sl_SOURCE_DIR}/../cl/gcc")
set(clplug_incs ${clplug_incs} "-I${sl_SOURCE_DIR}/../include")
set(clplug_incs ${clplug_incs} "-I${sl_SOURCE_DIR}/../include/gcc")

set(examples
    0000
    0001 0002 0003 0004 0005 0006 0007 0008 0009 0010
    0011 0012 0013 0014 0015 0016 0017 0018 0019 0020
    0021 0022 0023 0024 0025 0026 0027 0028 0029 0030
    0031 0032 0033 0034 0035 0036 0037 0038
         0042 0043 0044 0045 0046 0047 0048 0049 0050
         0052 0053 0054 0055 0056 0057 0058 0059 0060
    0061 0062 0063 0064 0065 0066 0067 0068 0069 0070
    0071 0072 0073 0074 0075 0076 0077 0078 0079 0080
    0081 0082 0083 0084 0085 0086 0087 0088 0089 0090
    0091 0092 0093 0094 0095)

# FIXME: define this macro more generically, in particular the count of args
macro(test_sym_exec label tool arg1 arg2 arg3)
    foreach (num ${examples})
        add_test("${label}___example-${num}" ${tool} ${arg1} ${arg2} ${arg3}
            -c ${sl_SOURCE_DIR}/data/test-${num}.c
            -DPREDATOR ${clplug_incs}
            -fplugin=${sl_BINARY_DIR}/libsl.so
            -fplugin-arg-libsl-args=noplot
            -fplugin-arg-libsl-preserve-ec
            -fplugin-arg-libsl-verbose=1)
    endforeach()
endmacro()
test_sym_exec("simple-test" ${GCC_HOST} "" "" "")

# FIXME: define this macro more generically, in particular the count of args
macro(test_sym_exec_regre ext arg1)
    foreach (num ${examples})
        set(cmd "${GCC_HOST}")

        # needed for test-0000
        foreach (inc ${clplug_incs})
            set(cmd "${cmd} ${inc}")
        endforeach()

        # we use the following flag to avoid differences on 32bit vs 64bit archs
        # in the error output, which is checked for exact match
        set(cmd "${cmd} -m32")

        set(cmd "${cmd} -c ${sl_SOURCE_DIR}/data/test-${num}.c")
        set(cmd "${cmd} -DPREDATOR -fplugin=${sl_BINARY_DIR}/libsl.so ${arg1}")
        set(cmd "${cmd} -fplugin-arg-libsl-preserve-ec")
        set(cmd "${cmd} 2>&1")

        # FIXME: the following line significantly decreases coverage of the test
        # suite; we should perhaps use awk or whatever to trim only the location
        # part from messages with internal location
        set(cmd "${cmd} | grep -v '\\\\[internal location\\\\]\$'")

        # FIXME: the doubly-nested triply-escaped sed magic might look a bit
        # scary
        set(cmd "${cmd} | sed s/`echo ${sl_SOURCE_DIR}/data/ ")
        set(cmd "${cmd} | sed 's/\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//g'`//")
        set(cmd "${cmd} | sed s/`echo ${sl_SOURCE_DIR}/ ")
        set(cmd "${cmd} | sed 's/\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//g'`//")

        # ... and finally diff with the expected output
        set(cmd
            "${cmd} | diff -up ${sl_SOURCE_DIR}/data/test-${num}.err${ext} -")
        add_test("regression-test___example-${num}${ext}" sh -c "${cmd}")
    endforeach()
endmacro(test_sym_exec_regre)

test_sym_exec_regre(".fast" "-fplugin-arg-libsl-args=fast")
test_sym_exec_regre("" "")

option(SL_TEST_VALGRIND "Set to ON to enable valgrind tests" OFF)
if(SL_TEST_VALGRIND)
    test_sym_exec("valgrind-test" valgrind
        --trace-children=yes
        --error-exitcode=7
        ${GCC_HOST})
endif()

add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${sl_SOURCE_DIR}/config.h)

add_test("headers_sanity-1" make -j
    -C ${sl_SOURCE_DIR}
    -f ${sl_SOURCE_DIR}/Makefile.chk)
