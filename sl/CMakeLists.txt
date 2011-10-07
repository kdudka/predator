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

option(SL_DEBUG "Set to ON to enable debugging support" OFF)
if(SL_DEBUG)
    add_definitions("-O0 -g")
    message (STATUS "Debugging support enabled...")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# libsl.so
add_library(sl SHARED
    cl_symexec.cc
    memdebug.cc
    sigcatch.cc
    symabstract.cc
    symbin.cc
    symbt.cc
    symcall.cc
    symcmp.cc
    symcut.cc
    symdiscover.cc
    symdump.cc
    symexec.cc
    symgc.cc
    symheap.cc
    symjoin.cc
    symneq.cc
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

option(TEST_ONLY_FAST "Set to OFF to boost test coverage" ON)

set(GCC_EXEC_PREFIX "timeout 64"
    CACHE STRING "Set to empty string if not sure")

set(examples
         0001 0002 0003 0004 0005 0006 0007 0008 0009
    0010 0011 0012 0013 0014 0015 0016 0017 0018 0019
    0020 0021 0022 0023 0024 0025 0026 0027 0028 0029
    0030 0031 0032 0033 0034 0035 0036 0037 0038 0039
    0040 0041 0042 0043 0044 0045 0046 0047 0048 0049
    0050 0051 0052 0053 0054 0055 0056 0057 0058 0059
    0060 0061 0062 0063 0064 0065 0066 0067 0068 0069
    0070 0071 0072 0073 0074 0075 0076 0077 0078 0079
    0080 0081 0082 0083 0084 0085 0086 0087 0088 0089
    0090 0091 0092 0093 0094 0095 0096 0097 0098 0099
    0100 0101 0102 0103 0104 0105      0107 0108 0109
    0110 0111 0112      0114 0115 0116 0117 0118 0119
    0120 0121 0122 0123      0125 0126 0127 0128 0129
    0130 0131 0132 0133 0134 0135 0136 0137 0138 0139
    0140 0141 0142 0143 0144 0145 0146 0147 0148 0149
    0150 0151 0152 0153 0154 0155 0156      0158 0159
    0160 0161 0162 0163 0164      0166      0168
    0170 0171 0172 0173 0174 0175 0176 0177 0178 0179
    0180 0181 0182 0183 0184 0185 0186 0187 0188 0189
    0190
    0300      0302
    0500 0501 0502 0503 0504 0505           0508 0509
    0510 0511 0512 0513 0514 0515 0516 0517 0518
    0520
    0167 0157 0124 0521)

set(cost 1048576)

# FIXME: define this macro more generically, in particular the count of args
macro(test_sym_exec label tool arg1 arg2 arg3)
    foreach (num ${examples})
        set(test_name "${label}___example-${num}")
        add_test(${test_name} ${tool} ${arg1} ${arg2} ${arg3}
            -c ${sl_SOURCE_DIR}/data/test-${num}.c
            -DPREDATOR ${clplug_incs}
            -fplugin=${sl_BINARY_DIR}/libsl.so
            -fplugin-arg-libsl-args=noplot
            -fplugin-arg-libsl-preserve-ec
            -fplugin-arg-libsl-verbose=1)

        SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
        MATH(EXPR cost "${cost} - 1")
    endforeach()
endmacro()
if(TEST_ONLY_FAST)
else()
    test_sym_exec("smoke-test" ${GCC_HOST} "" "" "")
endif()

# FIXME: define this macro more generically, in particular the count of args
macro(test_sym_exec_regre name_suff ext arg1)
    foreach (num ${examples})
        set(cmd "LC_ALL=C ${GCC_EXEC_PREFIX} ${GCC_HOST}")

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

        # filter out messages that are unrelated to our plug-in
        set(cmd "${cmd} | grep -E '\\\\[-fplugin=libsl.so\\\\]\$|compiler error'")
        set(cmd "${cmd} | sed 's/ \\\\[-fplugin=libsl.so\\\\]\$//'")

        # filter out NOTE messages with internal location
        set(cmd "${cmd} | grep -v 'note: .*\\\\[internal location\\\\]'")

        # FIXME: the doubly-nested triply-escaped sed magic might look a bit
        # scary
        set(cmd "${cmd} | sed s/`echo ${sl_SOURCE_DIR}/data/ ")
        set(cmd "${cmd} | sed 's/\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//g'`//")
        set(cmd "${cmd} | sed s/`echo ${sl_SOURCE_DIR}/ ")
        set(cmd "${cmd} | sed 's/\\\\\\\\//\\\\\\\\\\\\\\\\\\\\\\\\//g'`//")

        # ... and finally diff with the expected output
        set(cmd
            "${cmd} | diff -up ${sl_SOURCE_DIR}/data/test-${num}.err${ext} -")
        set(test_name "test-${num}.c${name_suff}")
        add_test(${test_name} sh -c "${cmd}")

        SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
        MATH(EXPR cost "${cost} - 1")
    endforeach()
endmacro(test_sym_exec_regre)

test_sym_exec_regre("" "" "-fplugin-arg-libsl-args=error_label:ERROR")
if(TEST_ONLY_FAST)
else()
    test_sym_exec_regre("-OOM" ".oom" "-fplugin-arg-libsl-args=oom")
endif()

if(TEST_WITH_VALGRIND)
    message (STATUS "valgrind enabled for testing...")
    test_sym_exec("valgrind-test" valgrind
        --trace-children=yes
        --error-exitcode=7
        ${GCC_HOST})
endif()

if(TEST_ONLY_FAST)
else()
    add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
        -o /dev/null
        -c ${sl_SOURCE_DIR}/config.h)

    add_test("headers_sanity-1" make -j
        -C ${sl_SOURCE_DIR}
        -f ${sl_SOURCE_DIR}/Makefile.chk)
endif()
