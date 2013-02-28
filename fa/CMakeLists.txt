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

# Add some more useful compiler flags
ADD_C_FLAG(       "PEDANTIC_ERRORS"      "-pedantic-errors")
ADD_CXX_ONLY_FLAG("W_CTOR_DTOR_PRIVACY"  "-Wctor-dtor-privacy")
ADD_CXX_ONLY_FLAG("W_OLD_STYLE_CAST"     "-Wold-style-cast")
ADD_CXX_ONLY_FLAG("W_EFFCXX"             "-Weffc++")

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
	backward_run.cc
	box.cc
	boxman.cc
	call.cc
	cl_fa.cc
	comparison.cc
	compiler.cc
	connection_graph.cc
	fixpoint.cc
	folding.cc
	forestaut.cc
	forestautext.cc
	integrity.cc
	jump.cc
	label.cc
	memplot.cc
	microcode.cc
	microcode_rev.cc
	normalization.cc
	plotenum.cc
	programconfig.cc
	sequentialinstruction.cc
	splitting.cc
	streams.cc
	symctx.cc
	symexec.cc
	symstate.cc
	symstate_isect.cc
	timbuk.cc
	treeaut.cc
	virtualmachine.cc
)
set_target_properties(fa PROPERTIES LINK_FLAGS -lrt)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fa ${CL_LIB})

# get the full path of libfa.so
get_property(GCC_PLUG TARGET fa PROPERTY LOCATION)
message (STATUS "GCC_PLUG: ${GCC_PLUG}")

option(TEST_ONLY_FAST "Set to OFF to boost test coverage" ON)

set(GCC_EXEC_PREFIX "timeout 120"
    CACHE STRING "Set to empty string if not sure")

set(testdir "${fa_SOURCE_DIR}/../tests/forester-regre")

# basic tests
set(tests
          f0001 f0002 f0003 f0004 f0005 f0006 f0007       f0009
    f0010 f0011 f0012 f0013 f0014       f0016             f0019
    f0020       f0022       f0024 f0025       f0027 f0028 f0029
    f0030                               f0036 f0037       f0039
    f0040       f0042       f0044 f0045       f0047 f0048
                f0052             f0055

    f0100 f0101 f0102 f0103       f0105 f0106
    f0110 f0111             f0114             f0117 f0118

# Predator tests
          p0001       p0003
                      p0023                   p0027 p0028 p0029
                                              p0037
                p0042             p0045 p0046
                                  p0075 p0076
                                  p0085
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
          f0021
          f0031 f0032 f0033
                                                          p0019
                                  p0025 p0026      
          p0041  
                                  
                                                          p0159
    p0160
                                                    p0188
                                                    p0198
)

set(tests-failing-due-to-abstraction
                                                    f0008
                                              f0017
                                  f0035             f0038
                      f0043             f0046             f0049
    f0050 f0051       f0053 f0054
    f0060
                                              p0507
)

set(tests-failing-due-to-timeout
                      f0023
)

set(tests-failing-due-to-unsupported-feature
                                        f0026
                            f0034            
                                              p0007 p0008
    p0010       p0012 p0013 p0014 p0015 p0016 p0017 p0018
                p0022       p0024
                p0032 p0033 p0034 p0035             p0038 p0039
    p0040             p0043                   p0047 p0048 p0049
    p0050 p0051 p0052 p0053 p0054 p0055 p0056 p0057 p0058 p0059
    p0060 p0061 p0062 p0063 p0064 p0065 p0066 p0067 p0068 p0069
    p0070 p0071 p0072 p0073                               p0079
    p0080 p0081 p0082       p0084       p0086 p0087 p0088 p0089
    p0090       p0092 p0093 p0094       p0096 p0097 p0098 p0099
    p0100 p0101 p0102 p0103 p0104 p0105 p0106 p0107 p0108 p0109
    p0110 p0111 p0112 p0113 p0114 p0115 p0116 p0117 p0118
    p0120 p0121 p0122 p0123 p0124 p0125       p0127 p0128 p0129
    p0130       p0132 p0133 p0134 p0135 p0136 p0137 p0138 p0139
    p0140 p0141 p0142       p0144 p0145 p0146 p0147 p0148 p0149
    p0150 p0151 p0152 p0153 p0154 p0155 p0156 p0157
          p0161 p0162 p0163 p0164 p0165 p0166 p0167 p0168 p0169
          p0171 p0172 p0173 p0174             p0177 p0178 p0179
    p0180 p0181 p0182 p0183 p0184       p0186             p0189
          p0191 p0192 p0193 p0194       p0196             p0199
          p0201 p0202 p0203 p0204 p0205 p0206 p0207 
                p0402 p0403 p0404 p0405 p0406 p0407 p0408 p0409
    p0410 p0411 p0412 p0413 p0414 p0415 p0416 p0417 p0418 p0419
                p0432
    p0500 p0501 p0502 p0503 p0504 p0505 p0506       p0508 p0509
    p0510 p0511 p0512 p0513 p0514 p0515 p0516 p0517 p0518 p0519
    p0520 p0521
)

set(tests-failing-due-to-internal-error
          f0041
                p0002       p0004 p0005 p0006             p0009
          p0011
    p0020 p0021           
                            p0044
                                              p0077 p0078 
                                  p0095
                 
                                                    p0158
                                        p0176
                                  p0185       p0187
    p0190                         p0195       p0197
    p0200                                           p0208
    p0300 p0301 p0302 p0303
    p0400 p0401       
                            p0464 p0465 p0466 p0467 p0468
)

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# PRIORITY: check these tests and if OK, add them into the suite!!!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
set(tests-with-unknown-results
                                  f0015             f0018
    p0030 p0031                         p0036
                            p0074
                      p0083
)

set(cost 1)

macro(test_forester_regre name_suff ext arg1)
    foreach (num ${tests})
        set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_EXEC_PREFIX} ${GCC_HOST}")

        # we use the following flag to avoid differences on 32bit vs 64bit archs
        # in the error output, which is checked for exact match
        set(cmd "${cmd} -m32")

        set(cmd "${cmd} -S ${testdir}/test-${num}.c -o /dev/null")
        set(cmd "${cmd} -I../include/forester-builtins -DFORESTER")
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
