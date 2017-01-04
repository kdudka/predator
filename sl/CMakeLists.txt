# Copyright (C) 2010-2013 Kamil Dudka <kdudka@redhat.com>
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
enable_testing()

include("../build-aux/common.cmake")

option(SL_DEBUG "Set to ON to enable debugging support" OFF)
if(SL_DEBUG)
    add_definitions("-O0 -g")
    message (STATUS "Debugging support enabled...")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# check whether the long long type is available and greater than long
include(CheckTypeSize)
check_type_size("long"      SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)
if("${SIZEOF_LONG_LONG}" AND "${SIZEOF_LONG_LONG}" GREATER "${SIZEOF_LONG}")
    add_definitions("-DUSE_LONG_LONG")
    message (STATUS "Use of long long enabled...")
endif()

# libpredator.a
add_library(predator STATIC
    adt_op.cc
    adt_op_def.cc
    adt_op_match.cc
    adt_op_meta.cc
    adt_op_replace.cc
    cl_symexec.cc
    cont_shape.cc
    cont_shape_seq.cc
    cont_shape_var.cc
    fixed_point.cc
    fixed_point_proxy.cc
    fixed_point_rewrite.cc
    glconf.cc
    intrange.cc
    plotenum.cc
    prototype.cc
    shape.cc
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
    symplot.cc
    symproc.cc
    symseg.cc
    symstate.cc
    symtrace.cc
    symutil.cc
    version.c)


# build compiler plug-in (libsl.so)
CL_BUILD_COMPILER_PLUGIN(sl predator ../cl_build)

# get the full path of libsl.so
get_property(SL_PLUG TARGET sl PROPERTY LOCATION)
message (STATUS "SL_PLUG: ${SL_PLUG}")

if(ENABLE_LLVM)
    #get path of libpasses.so
    if("${LIBPASSES_PATH}" STREQUAL "")
        set(PASSES_LIB ${PROJECT_SOURCE_DIR}/../passes-src/passes_build/libpasses.so)
    else()
        find_library(PASSES_LIB passes PATHS ${LIBPASSES_PATH} NO_DEFAULT_PATH)
    endif()
    message (STATUS "PASSES_LIB: ${PASSES_LIB}")

    # helping scripts
    configure_file(${PROJECT_SOURCE_DIR}/slscript.in  ${PROJECT_BINARY_DIR}/slllvm    @ONLY)
    configure_file(${PROJECT_SOURCE_DIR}/slllvmv.in   ${PROJECT_BINARY_DIR}/slllvmv   @ONLY)
else()
    # helping scripts
    configure_file(${PROJECT_SOURCE_DIR}/slscript.in  ${PROJECT_BINARY_DIR}/slgcc     @ONLY)
    configure_file(${PROJECT_SOURCE_DIR}/slgccv.in    ${PROJECT_BINARY_DIR}/slgccv    @ONLY)
    configure_file(${PROJECT_SOURCE_DIR}/slgdb.in     ${PROJECT_BINARY_DIR}/slgdb     @ONLY)
    configure_file(${PROJECT_SOURCE_DIR}/probe.sh.in  ${PROJECT_BINARY_DIR}/probe.sh  @ONLY)

    configure_file(${PROJECT_SOURCE_DIR}/register-paths.sh.in
        ${PROJECT_BINARY_DIR}/register-paths.sh                                       @ONLY)
endif()
configure_file(${PROJECT_SOURCE_DIR}/check-property.sh.in
    ${PROJECT_BINARY_DIR}/check-property.sh                                           @ONLY)

# make install
install(TARGETS sl DESTINATION lib)

option(TEST_ONLY_FAST "Set to OFF to boost test coverage" ON)

set(GCC_EXEC_PREFIX "" CACHE STRING
    "Set to \"timeout N\" to limit each test to run at most N seconds.")

set(testdir "${sl_SOURCE_DIR}/../tests/predator-regre")

# basic tests
if(ENABLE_LLVM) # -----------------------------------------------------------

option(LLVM_OK_TEST "" ON)
if(LLVM_OK_TEST)
set(tests
              0002 0003 0004 0005 0006 0007 0008
    0010      0012 0013      0015 0016 0017 0018 0019
    0020           0023 0024           0027      0029
    0030 0031 0032 0033 0034           0037 0038
         0041 0042 0043 0044      0046      0048
         0051 0052 0053           0056
    0060      0062 0063 0064           0067 0068 0069
         0071           0074 0075 0076 0077
                   0083 0084 0085 0086 0087
         0091 0092 0093                          0099
    0100 0101 0102 0103 0104 0105      0107 0108 0109
                        0114                0118 0119
    0120           0123      0125
         0131
         0141      0143                     0148 0149
                   0153 0154 0155           0158 0159
    0160 0161 0162 0163 0164      0166
         0171                0175
                             0185           0188
    0190           0193 0194           0197 0198
         0201 0202 0203 0204 0205
                        0214 0215      0217 0218 0219
    0220                     0225 0226 0227 0228 0229
    0230 0231 0232 0233 0234      0236 0237 0238
         0241 0242      0244 0245           0248


    0400 0401      0403

              0432

    0500 0501           0504 0505                0509
    0510      0512           0515           0518

         0601 0602 0603 0604 0605 0606 0607 0608 0609
    0610 0611 0612 0613 0614 0615)

# test correct, but different output then gcc-plugin
else()
set(tests ${tests}
# track uninit [conditional jump depends on uninitialized value]
    0025 0026 0036 0047
# opposite execution branch (inverted condition) or different cycles count or addr of local variable
    0001 0022 0040 0045 0049 0050 0054 0055 0059 0065
    0066 0070 0089 0096 0097 0098 0111 0115 0121 0122
    0126 0127 0128 0129 0130 0156 0196 0199 0200 0247
    0502 0503 0511 0517 0520
    0176 0404 0408 0411
# bad location info
    0009 0011 0057 0058 0079 0080 0081 0088 0090 0112
    0144 0145 0146 0147 0172 0173 0195 0208 0221 0223
# incompletely location info
    0035 0142 0191 0192 0224 0222 0402
# incorrect output - need analysis
    0014 0028 0039 0061 0072 0073 0078 0082
    0094 0095
    0110 0116 0117
    0132 0133 0134 0135 0136 0137 0138 0139
    0140 0150 0151 0152 0167 0168
    0170 0174 0177 0178 0179
    0180 0181 0182 0183 0184 0186 0187 0189
    0207 0209 0210 0212
    0239 0246 # stack.save .restore
    0240 0243
    0300 0302 0316
    0406
    0508 0513 0514 0516)
endif()

else() # --------------------------------------------------------------------
set(tests
         0001 0002 0003 0004 0005 0006 0007 0008 0009
    0010 0011 0012 0013 0014 0015 0016 0017 0018 0019
    0020      0022 0023 0024 0025 0026 0027 0028 0029
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
    0160 0161 0162 0163 0164      0166 0167 0168
    0170 0171 0172 0173 0174 0175 0176 0177 0178 0179
    0180 0181 0182 0183 0184 0185 0186 0187 0188 0189
    0190 0191 0192 0193 0194 0195 0196 0197 0198 0199
    0200 0201 0202 0203 0204 0205      0207 0208 0209
    0210      0212      0214 0215      0217 0218 0219
    0220 0221 0222 0223 0224 0225 0226 0227 0228 0229
    0230 0231 0232 0233 0234      0236 0237 0238 0239
    0240 0241 0242 0243 0244 0245 0246 0247 0248
    0300      0302
                                  0316
    0400 0401 0402 0403 0404      0406      0408
         0411
              0432
    0500 0501 0502 0503 0504 0505           0508 0509
    0510 0511 0512 0513 0514 0515 0516 0517 0518
    0520
         0601 0602 0603 0604 0605 0606 0607 0608 0609
    0610 0611 0612 0613 0614 0615)
endif()

option(TEST_INCLUDE_SLAYER "Include tests distributed with SLAyer" OFF)
if(TEST_INCLUDE_SLAYER)
set(tests ${tests}
    0700 0701 0702 0703 0704 0705 0706 0707 0708 0709
    0710 0711 0712 0713 0714 0715 0716 0717 0718 0719
    0720 0721 0722 0723 0724 0725 0726 0727 0728 0729
    0730 0731 0732 0733 0734 0735 0736 0737 0738 0739
    0740 0741 0742 0743 0744 0745 0746 0747 0748 0749
    0750 0751 0752 0753 0754 0755 0756 0757 0758 0759
    0760 0761 0762 0763 0764 0765 0766 0767 0768 0769
    0770 0771 0772 0773 0774 0775 0776 0777 0778 0779
    0780 0781 0782 0783 0784 0785 0786 0787 0788 0789
    0790 0791 0792 0793 0794 0795 0796 0797 0798 0799
    0800 0801 0802 0803 0804 0805 0806 0807 0808 0809
    0810 0811 0812 0813 0814 0815 0816 0817 0818 0819
    0820 0821 0822 0823 0824 0825 0826 0827 0828 0829
    0830 0831 0832 0833 0834)
endif()

if(TEST_ONLY_FAST)
else()
# expensive tests
set(tests ${tests}
    0124 0157 0521 0235
    0308 0309 0310 0311 0312      0314 0315      0318
    0405 0407 0409 0410 0412 0413 0414 0415 0416 0417 0418
    0469 0471 0474)
endif()

set(cost 1)

if(ENABLE_LLVM) # -----------------------------------------------------------
set(cmd_cc1 "-S -emit-llvm")

# FIXME: define this macro more generically, in particular the count of args
macro(test_predator_regre name_suff ext arg1)
    foreach (num ${tests})
        set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_EXEC_PREFIX} ${CLANG_HOST}")

        # we use the following flag to avoid differences on 32bit vs 64bit archs
        # in the error output, which is checked for exact match
        set(cmd "${cmd} -w -g -m32 ${cmd_cc1}")
        set(cmd "${cmd} -I../include/predator-builtins -DPREDATOR")
        set(cmd "${cmd} ${testdir}/test-${num}.c -o - | ")
        set(cmd "${cmd} ${GCC_EXEC_PREFIX} ${OPT_HOST} -o /dev/null -lowerswitch")

        # eliminate some alloca
#        set(cmd "${cmd} -mem2reg")
        set(cmd "${cmd} -load ${sl_BINARY_DIR}/libsl.so -sl")
        set(cmd "${cmd} ${arg1}")
        set(cmd "${cmd} -preserve-ec")
        set(cmd "${cmd} 2>&1")

        # filter out messages that are unrelated to our plug-in
        set(cmd "${cmd} | (grep -E '\\\\[-sl\\\\]\$|compiler error|undefined symbol|CL_BREAK_IF'; true)")
        set(cmd "${cmd} | sed 's/ \\\\[-sl\\\\]\$//'")

        # filter out NOTE messages with internal location
        set(cmd "${cmd} | (grep -v 'note: .*\\\\[internal location\\\\]'; true)")

		# filter out Code Listener messages about unreachable labels
		set(cmd "${cmd} | (grep -v 'warning: unreachable label .*'; true)")

        # drop absolute paths
        set(cmd "${cmd} | sed 's|^[^:]*/||'")

        # drop column numbers
        set(cmd "${cmd} | sed -r 's|^([^:]+:[0-9]+:)[0-9]+:|\\\\1|'")

        # drop var UIDs that are not guaranteed to be fixed among runs
        set(cmd "${cmd} | sed -r -e 's|#[0-9]+:||g' -e 's|[#.][0-9]+|_|g'")

        # ... and finally diff with the expected output
        set(cmd "${cmd} | diff -up ${testdir}/test-${num}.err${ext} -")
        set(test_name "test-${num}.c${name_suff}")
        add_test(${test_name} bash -o pipefail -c "${cmd}")

        SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
        MATH(EXPR cost "${cost} + 1")
    endforeach()
endmacro(test_predator_regre)

# default mode -> change error_label:ERROR to function __VERIFIER_error()
test_predator_regre("" "" "")

# track_uninit mode
test_predator_regre("-TRACK_UNINIT" ".uninit"
    "-args=track_uninit")

# OOM simulation mode
test_predator_regre("-OOM" ".oom" "-args=oom")



if(TEST_ONLY_FAST)
else()
    add_test("headers_sanity-0" ${CMAKE_CXX_COMPILER} -ansi -Wall -Wextra -Werror -pedantic
        -o /dev/null
        -c ${sl_SOURCE_DIR}/config.h)

    add_test("headers_sanity-1" make -j
        -C ${sl_SOURCE_DIR}
        -f ${sl_SOURCE_DIR}/Makefile.chk)
endif()


else() # --------------------------------------------------------------------
# FIXME: define this macro more generically, in particular the count of args
macro(test_predator_smoke label tool arg1 arg2 arg3)
    foreach (num ${tests})
        set(test_name "${label}___example-${num}")
        add_test(${test_name} ${tool} ${arg1} ${arg2} ${arg3}
            -S "${testdir}/test-${num}.c" -o /dev/null
            -I../include/predator-builtins -DPREDATOR
            -fplugin=${sl_BINARY_DIR}/libsl.so
            -fplugin-arg-libsl-args=noplot
            -fplugin-arg-libsl-preserve-ec
            -fplugin-arg-libsl-verbose=1)

        SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
        MATH(EXPR cost "${cost} + 1")
    endforeach()
endmacro()

option(ENABLE_CC1_PLUS "Set to TRUE to use C++ compiler for the tests." FALSE)

# FIXME: define this macro more generically, in particular the count of args
macro(test_predator_regre name_suff ext arg1)
    foreach (num ${tests})
        set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_EXEC_PREFIX} ${GCC_HOST}")

        # we use the following flag to avoid differences on 32bit vs 64bit archs
        # in the error output, which is checked for exact match
        set(cmd "${cmd} -m32")

        # use C++ compiler if requested:
        if (ENABLE_CC1_PLUS)
            set(cmd "${cmd} -x c++")
        endif()

        set(cmd "${cmd} -S ${testdir}/test-${num}.c -o /dev/null")
        set(cmd "${cmd} -I../include/predator-builtins -DPREDATOR")
        set(cmd "${cmd} -fplugin=${sl_BINARY_DIR}/libsl.so ${arg1}")
        set(cmd "${cmd} -fplugin-arg-libsl-preserve-ec")
        set(cmd "${cmd} 2>&1")

        # filter out messages that are unrelated to our plug-in
        set(cmd "${cmd} | (grep -E '\\\\[-fplugin=libsl.so\\\\]\$|compiler error|undefined symbol|CL_BREAK_IF'; true)")
        set(cmd "${cmd} | sed 's/ \\\\[-fplugin=libsl.so\\\\]\$//'")

        # filter out NOTE messages with internal location
        set(cmd "${cmd} | (grep -v 'note: .*\\\\[internal location\\\\]'; true)")

        # drop absolute paths
        set(cmd "${cmd} | sed 's|^[^:]*/||'")

        # drop column numbers
        set(cmd "${cmd} | sed -r 's|^([^:]+:[0-9]+:)[0-9]+:|\\\\1|'")

        # drop var UIDs that are not guaranteed to be fixed among runs
        set(cmd "${cmd} | sed -r -e 's|#[0-9]+:||g' -e 's|[#.][0-9]+|_|g'")

        # ... and finally diff with the expected output
        set(cmd "${cmd} | diff -up ${testdir}/test-${num}.err${ext} -")
        set(test_name "test-${num}.c${name_suff}")
        add_test(${test_name} bash -o pipefail -c "${cmd}")

        SET_TESTS_PROPERTIES(${test_name} PROPERTIES COST ${cost})
        MATH(EXPR cost "${cost} + 1")
    endforeach()
endmacro(test_predator_regre)

# default mode
test_predator_regre("" "" "-fplugin-arg-libsl-args=error_label:ERROR")

# track_uninit mode
test_predator_regre("-TRACK_UNINIT" ".uninit"
    "-fplugin-arg-libsl-args=track_uninit")

# OOM simulation mode
test_predator_regre("-OOM" ".oom" "-fplugin-arg-libsl-args=oom")

if(TEST_WITH_VALGRIND)
    message (STATUS "valgrind enabled for testing...")
    test_predator_smoke("valgrind-test" valgrind
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

endif() #--------------------------------------------------------------------
