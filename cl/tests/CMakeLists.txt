# Copyright (C) 2010-2012 Kamil Dudka <kdudka@redhat.com>
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

# tweak include dirs, etc.
add_definitions("-I../../include -fPIC")

# compile libchk_var_killer.so
add_library(chk_var_killer_core STATIC chk_var_killer.cc)
CL_BUILD_COMPILER_PLUGIN(chk_var_killer chk_var_killer_core "")

# compile libchk_pt.so
add_library(chk_pt_core STATIC chk_pt.cc)
CL_BUILD_COMPILER_PLUGIN(chk_pt chk_pt_core "")

# compile libcl_smoke_test.so
add_library(cl_smoke_test_core STATIC cl_smoke_test.cc)
CL_BUILD_COMPILER_PLUGIN(cl_smoke_test cl_smoke_test_core "")

# get the full paths of plugins
get_property(VK_PLUG TARGET chk_var_killer PROPERTY LOCATION)
get_property(PT_PLUG TARGET chk_pt         PROPERTY LOCATION)
set(PLUG "${VK_PLUG}")

message(STATUS "VK_PLUG: ${VK_PLUG}")
message(STATUS "PT_PLUG: ${PT_PLUG}")

set(PRED_INCL_DIR "${cl_SOURCE_DIR}/../include/predator-builtins/")

macro(add_test_wrap test_name cmd)
    add_test("${test_name}" bash -o pipefail -c
        "LC_ALL=C CCACHE_DISABLE=1 MALLOC_PERTURB_=170 ${cmd}")
endmacro()

if(ENABLE_LLVM)
set(cmd_cc1 "-S -emit-llvm")
#set(cmd_cc1 "${cmd_cc1} -femit-all-decls")
set(CFLAGS "-std=c++11 -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS")

# basic set of the options to compile llvm/clplug.cc with libc_test.so loaded
set(cmd "${CLANG_HOST} ${CFLAGS} ${cmd_cc1}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} ${cl_SOURCE_DIR}/llvm/clplug.cc")
set(cmd "${cmd} -o - | ${OPT_HOST} -o /dev/null -lowerswitch")
set(cmd "${cmd} -load ${cl_BINARY_DIR}/tests/libcl_smoke_test.so -cl")
set(cmd_base "${cmd}")

# smoke test #1
set(cmd "${cmd_base} -dry-run")
#add_test_wrap("smoke-test-01" "${cmd}")

# smoke test #2
set(cmd "${cmd_base} | diff -up ${cl_SOURCE_DIR}/tests/cl_smoke_test.out -")
#add_test_wrap("smoke-test-02" "${cmd}")

# compile self #1
set(cmd "${cmd_base}")
set(cmd "${cmd} -dump-pp=/dev/null")
set(cmd "${cmd} -dump-types")
set(cmd "${cmd} -gen-dot=/dev/null")
set(cmd "${cmd} -type-dot=/dev/null")
set(cmd "${cmd} -verbose=2")
set(cmd "${cmd} -dry-run")
set(cmd_all "${cmd}")
#add_test_wrap("compile-self-01-dry-run" "${cmd}")

# compile self #2
set(cmd "${cmd_base} -load ${cl_BINARY_DIR}/tests/libchk_var_killer.so -cl")
#add_test_wrap("compile-self-02-var-killer" "${cmd}")

# compile self #3 runs both plug-ins through valgrind and it is optional
if(TEST_WITH_VALGRIND)
    set(cmd "valgrind --trace-children=yes --error-exitcode=7 ${cmd}")
#    add_test_wrap("compile-self-03-valgrind" "${cmd}")
endif()

# generic template for var-killer tests
macro(add_vk_test id)
    set(cmd "${CLANG_HOST} ${cmd_cc1}")
    set(cmd "${cmd} -I${PRED_INCL_DIR}")
    set(cmd "${cmd} ${cl_SOURCE_DIR}/tests/data/vk-${id}.c")
    set(cmd "${cmd} -o - | ${OPT_HOST} -o /dev/null -lowerswitch")
    set(cmd "${cmd} -load ${VK_PLUG} -chk_var_killer")
    add_test_wrap("var-killer-${id}" "${cmd}")
endmacro()

macro(add_pt_test id)
    set(cmd "${CLANG_HOST} ${cmd_cc1} ${cl_SOURCE_DIR}/tests/data/pt-${id}.c")
    set(cmd "${cmd} -I${cl_SOURCE_DIR}")
    set(cmd "${cmd} -DNDEBUG")
    set(cmd "${cmd} -I${PRED_INCL_DIR}")
    set(cmd "${cmd} -o - | ${OPT_HOST} -o /dev/null -lowerswitch")
    set(cmd "${cmd} -load ${PT_PLUG} -chk_pt")

    add_test_wrap("points-to-${id}" "${cmd}")
endmacro()
else()
# basic set of the options to compile gcc/clplug.c with libc_test.so loaded
set(cmd "${GCC_HOST} ${CFLAGS}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} -I${GCC_INC_DIR}")
set(cmd "${cmd} -xc++ -c ${cl_SOURCE_DIR}/gcc/clplug.c")
set(cmd "${cmd} -o /dev/null")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/tests/libcl_smoke_test.so")
set(cmd_base "${cmd}")

# smoke test #1
set(cmd "${cmd_base} -fplugin-arg-libcl_smoke_test-dry-run")
add_test_wrap("smoke-test-01" "${cmd}")

# smoke test #2
set(cmd "${cmd_base} | diff -up ${cl_SOURCE_DIR}/tests/cl_smoke_test.out -")
add_test_wrap("smoke-test-02" "${cmd}")

# compile self #1
set(cmd "${cmd_base}")
set(cmd "${cmd} -fplugin-arg-libcl_smoke_test-dump-pp=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_smoke_test-dump-types")
set(cmd "${cmd} -fplugin-arg-libcl_smoke_test-gen-dot=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_smoke_test-type-dot=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_smoke_test-verbose=15")
set(cmd "${cmd} -fplugin-arg-libcl_smoke_test-dry-run")
set(cmd_all "${cmd}")
add_test_wrap("compile-self-01-dry-run" "${cmd}")

# compile self #2
set(cmd "${cmd_base} -fplugin=${cl_BINARY_DIR}/tests/libchk_var_killer.so")
add_test_wrap("compile-self-02-var-killer" "${cmd}")

# compile self #3 runs both plug-ins through valgrind and it is optional
if(TEST_WITH_VALGRIND)
    set(cmd "valgrind --trace-children=yes --error-exitcode=7 ${cmd}")
    add_test_wrap("compile-self-03-valgrind" "${cmd}")
endif()

# generic template for var-killer tests
macro(add_vk_test id)
    set(cmd "${GCC_HOST} -c ${cl_SOURCE_DIR}/tests/data/vk-${id}.c")
    set(cmd "${cmd} -o /dev/null")
    set(cmd "${cmd} -I${PRED_INCL_DIR}")
    set(cmd "${cmd} -fplugin=${VK_PLUG}")
    add_test_wrap("var-killer-${id}" "${cmd}")
endmacro()

macro(add_pt_test id)
    set(cmd "${GCC_HOST} -c ${cl_SOURCE_DIR}/tests/data/pt-${id}.c")
    set(cmd "${cmd} -o /dev/null")
    set(cmd "${cmd} -I${cl_SOURCE_DIR}")
    set(cmd "${cmd} -DNDEBUG")
    set(cmd "${cmd} -I${PRED_INCL_DIR}")
    set(cmd "${cmd} -fplugin=${PT_PLUG}")

    add_test_wrap("points-to-${id}" "${cmd}")
endmacro()

# Get the command to call right version of g++ and store it in CXX_HOST:
execute_process(COMMAND "basename" "${GCC_HOST}" COMMAND "tr" "c" "+"
    OUTPUT_VARIABLE CXX_HOST OUTPUT_STRIP_TRAILING_WHITESPACE)
if ("${CXX_HOST}" STREQUAL "")
    message(FATAL_ERROR "failed to obtain CXX_HOST")
endif()

# Get the directory of the g++:
execute_process(COMMAND "dirname" "${GCC_HOST}"
    OUTPUT_VARIABLE CXX_HOST_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if ("${CXX_HOST}" STREQUAL "")
    message(FATAL_ERROR "failed to obtain directory of CXX_HOST")
endif()

# Create the full path for the g++:
set(CXX_HOST "${CXX_HOST_DIR}/${CXX_HOST}")

macro(add_gcc_adapt_test compiler dir id name)
    # Use the correct command for invoking the requested compiler:
    if ("${compiler}" STREQUAL "cc1plus")
        set(cmd "${CXX_HOST}")    # g++
    else()
        set(cmd "${GCC_HOST}")    # gcc
    endif()

    set(cmd "${cmd} -c ${cl_SOURCE_DIR}/tests/gcc-adapter/${dir}/${id}-${name}")

    # Add the file extension corresponding to the test folder:
    if ("${dir}" STREQUAL "C++")
        set(cmd "${cmd}.cc")
    else()
        set(cmd "${cmd}.c")
    endif()

    set(cmd "${cmd} -o /dev/null")
    set(cmd "${cmd} -I${cl_SOURCE_DIR}")

    set(cmd "${cmd} -DPREDATOR")
    set(cmd "${cmd} -DNDEBUG")
    set(cmd "${cmd} -m32 -O0 -Wall -Wextra")

    # Request the minimal supported standard by GCC adapter:
    # NOTE: Since we're using both C and C++ language compiler and supporting
    #       the GNU extension, the '-pedantic' flag is therefore undesirable.
    if ("${compiler}" STREQUAL "cc1plus")
        # FIXME: Remove the '-fno-rtti' & '-fno-exceptions' after the support
        #        for these features was implemented.
        set(cmd "${cmd} -std=c++98 -fno-rtti -fno-exceptions")
    else()
        set(cmd "${cmd} -std=gnu89")
    endif()

    set(cmd "${cmd} -fplugin=${VK_PLUG}")
    set(cmd "${cmd} -fplugin-arg-libchk_var_killer-dump-pp")
    set(cmd "${cmd} -fplugin-arg-libchk_var_killer-verbose=1")


    if ("${compiler}" STREQUAL "cc1plus")
        add_test_wrap("g++-adapter-test-of-${dir}/${id}" "${cmd}")
    else()
        add_test_wrap("gcc-adapter-test-of-${dir}/${id}" "${cmd}")
    endif()
endmacro()
endif()

# Wrapping macro to reduce the number of lines:
macro(add_gcc_adapt_C_tests compiler)
  add_gcc_adapt_test(${compiler} C 00 Empty-main)
  add_gcc_adapt_test(${compiler} C 01 Global-variable-without-initialization)
  add_gcc_adapt_test(${compiler} C 02 Global-variable-with-initial-value)
  add_gcc_adapt_test(${compiler} C 03 Global-variable-with-initialization)
  add_gcc_adapt_test(${compiler} C 04 Local-variable-without-initialization)
  add_gcc_adapt_test(${compiler} C 05 Local-variable-with-initialization)
  add_gcc_adapt_test(${compiler} C 06 Types-char)
  add_gcc_adapt_test(${compiler} C 07 Types-int)
  add_gcc_adapt_test(${compiler} C 08 Types-real)
  add_gcc_adapt_test(${compiler} C 09 Types-enum)
  add_gcc_adapt_test(${compiler} C 10 Types-union)
  add_gcc_adapt_test(${compiler} C 11 Types-struct)
  add_gcc_adapt_test(${compiler} C 12 Operator-sizeof)
  add_gcc_adapt_test(${compiler} C 13 Types-typedef)
  add_gcc_adapt_test(${compiler} C 14 Types-pointers)
  add_gcc_adapt_test(${compiler} C 15 Memory-class-modificators)
  add_gcc_adapt_test(${compiler} C 16 Type-modificators)
  add_gcc_adapt_test(${compiler} C 17 Pointers-reference-dereference)
  add_gcc_adapt_test(${compiler} C 18 Type-casting)
  add_gcc_adapt_test(${compiler} C 19 Operators-unary)
  add_gcc_adapt_test(${compiler} C 20 Operators-binary)
  add_gcc_adapt_test(${compiler} C 21 Operators-relational-and-equality)
  add_gcc_adapt_test(${compiler} C 22 Operators-logical)
  add_gcc_adapt_test(${compiler} C 23 Operators-bitwise)
  add_gcc_adapt_test(${compiler} C 24 Operators-assignment)
  add_gcc_adapt_test(${compiler} C 25 Operators-ternary-and-comma)
  add_gcc_adapt_test(${compiler} C 26 Arrays-definitions)
  add_gcc_adapt_test(${compiler} C 27 Functions-definitions)
  add_gcc_adapt_test(${compiler} C 28 Operators-other-plus-calls-types)
  add_gcc_adapt_test(${compiler} C 29 Declarations)
  add_gcc_adapt_test(${compiler} C 30 String-literals)
  add_gcc_adapt_test(${compiler} C 31 Bitfields)
  add_gcc_adapt_test(${compiler} C 32 Pointer-arithmetics)
  add_gcc_adapt_test(${compiler} C 33 Compound-literals)
  add_gcc_adapt_test(${compiler} C 34 Control-flow-if-conditions)
  add_gcc_adapt_test(${compiler} C 35 Control-flow-while-loop)
  add_gcc_adapt_test(${compiler} C 36 Control-flow-do-while-loop)
  add_gcc_adapt_test(${compiler} C 37 Control-flow-for-loop)
  add_gcc_adapt_test(${compiler} C 38 Control-flow-switch-statement)
  add_gcc_adapt_test(${compiler} C 39 Control-flow-break-and-continue)
  add_gcc_adapt_test(${compiler} C 40 Control-flow-goto)
  add_gcc_adapt_test(${compiler} C 41 Standard-header-files-C89)
  add_gcc_adapt_test(${compiler} C 42 Header-setjmp)
  add_gcc_adapt_test(${compiler} C 43 Standard-header-files-C99)
  add_gcc_adapt_test(${compiler} C 44 Types-complex)
  add_gcc_adapt_test(${compiler} C 45 Standard-header-files-C11)
endmacro()

####################################
# append test-cases for var-killer #
####################################

add_vk_test(0001)
add_vk_test(0002)
add_vk_test(0003)

add_vk_test(0100)  # show mayBePointed breakage
add_vk_test(0101)

################################
# append tests points-to graph #
################################

# -> basic pointer operations
add_pt_test(0001)
add_pt_test(0002)
add_pt_test(0003)

# -> struct pointer operations
add_pt_test(0201)
add_pt_test(0202)
add_pt_test(0203)

# -> void pointer tricks
add_pt_test(0401)
add_pt_test(0402)
add_pt_test(0490) # regr. test
add_pt_test(0491) # regr. test

# -> global variables
add_pt_test(0801)
add_pt_test(0802)
add_pt_test(0803)
add_pt_test(0804)
add_pt_test(0850) # BindLoc
add_pt_test(0851)

# -> function parameters
add_pt_test(0901)
add_pt_test(0902)
add_pt_test(0903)
add_pt_test(0904)
add_pt_test(0905) # passing structs as parameters
add_pt_test(0906)
add_pt_test(0907) # binding return value
add_pt_test(0950) # ugly hacks // checks are disabled

add_pt_test(1100) # external function -> black hole
add_pt_test(1101) # whitelisted fnc -> no-black hole!

# -> more complicated examples
add_pt_test(1200) # PT-info downward propagation
add_pt_test(1201) # ^^^ + check for non-propagation with returning operand
add_pt_test(1202) # recursive loop in call-graph
add_pt_test(1203) # simple tail recuresion (from forester-regre/test-f0019.c)
add_pt_test(1204) # caught a USE_AFTER_FREE (from predator-regre/test-0221.c)

add_pt_test(1300) # predator-regre test-0167.c

# headers sanity #0
add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/config_cl.h)

# headers sanity #1
add_test("headers_sanity-1" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/../include/cl/code_listener.h)

# headers sanity #2
add_test("headers_sanity-2" make -j
    -C ${cl_SOURCE_DIR}/../include/cl
    -f ${cl_SOURCE_DIR}/Makefile.chk)

# headers sanity #3
add_test("headers_sanity-3" make -j
    -C ${cl_SOURCE_DIR}
    -f ${cl_SOURCE_DIR}/Makefile.chk)

if(ENABLE_LLVM)
# headers sanity #4
add_test("headers_sanity-4" ${CMAKE_CXX_COMPILER} -x c++ -std=c++11 -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -I${cl_SOURCE_DIR}/../include
    -c ${cl_SOURCE_DIR}/llvm/clplug.hh)
endif()

#####################################
# append test-cases for gcc-adapter #
#####################################

# Adding C language test-cases with both C and C++ compiler:
add_gcc_adapt_C_tests(cc1)
add_gcc_adapt_C_tests(cc1plus)

# Add the specific C++ test-cases only for the C++ compiler:
add_gcc_adapt_test($cc1plus C++ 00 Unnamed-namespace)
add_gcc_adapt_test($cc1plus C++ 01 Named-namespace)
add_gcc_adapt_test($cc1plus C++ 02 Global-namespace)
add_gcc_adapt_test($cc1plus C++ 03 Extension-namespace-definition)
add_gcc_adapt_test($cc1plus C++ 04 Nested-namespaces)
add_gcc_adapt_test($cc1plus C++ 05 Namespace-functions-declarations-and-definitions)
add_gcc_adapt_test($cc1plus C++ 06 Inline-namespaces) # Only dummy test for now.
add_gcc_adapt_test($cc1plus C++ 07 Using-declaration-01)
add_gcc_adapt_test($cc1plus C++ 08 Using-declaration-02)
add_gcc_adapt_test($cc1plus C++ 09 Using-directive-01)
add_gcc_adapt_test($cc1plus C++ 10 Using-directive-02)
add_gcc_adapt_test($cc1plus C++ 11 Namespace-alias)
add_gcc_adapt_test($cc1plus C++ 12 Basic-constructors-and-destructors)
add_gcc_adapt_test($cc1plus C++ 13 Using-declaration-03)
add_gcc_adapt_test($cc1plus C++ 14 Lvalue-references)
add_gcc_adapt_test($cc1plus C++ 15 Lvalue-reference-collapsing-and-typedef)
add_gcc_adapt_test($cc1plus C++ 16 Extern-lvalue-references)
add_gcc_adapt_test($cc1plus C++ 17 Function-pass-by-lvalue-reference)
add_gcc_adapt_test($cc1plus C++ 18 Lvalue-reference-as-return-value)
add_gcc_adapt_test($cc1plus C++ 19 Lvalue-reference-as-data-member)
add_gcc_adapt_test($cc1plus C++ 20 Cv-qualifiers-with-lvalue-references)
add_gcc_adapt_test($cc1plus C++ 21 Lvalue-initializers)
add_gcc_adapt_test($cc1plus C++ 22 Lvalue-references-of-array-function-and-object)
add_gcc_adapt_test($cc1plus C++ 23 Native-type-bool)
add_gcc_adapt_test($cc1plus C++ 24 Functions-overloading)
add_gcc_adapt_test($cc1plus C++ 25 Functions-arguments-default-values)
add_gcc_adapt_test($cc1plus C++ 26 Constant-expressions)
add_gcc_adapt_test($cc1plus C++ 27 Constant-function-arguments)
add_gcc_adapt_test($cc1plus C++ 28 Default-initializations)
add_gcc_adapt_test($cc1plus C++ 29 Zero-and-value-initializations)
add_gcc_adapt_test($cc1plus C++ 30 Direct-initializations)
add_gcc_adapt_test($cc1plus C++ 31 Copy-initializations)
add_gcc_adapt_test($cc1plus C++ 32 List-initializations)
add_gcc_adapt_test($cc1plus C++ 33 Aggregate-initializations)
add_gcc_adapt_test($cc1plus C++ 34 Reference-initializations)
add_gcc_adapt_test($cc1plus C++ 35 Address-of-overloaded-function)
add_gcc_adapt_test($cc1plus C++ 36 Function-member-declaration-context)
add_gcc_adapt_test($cc1plus C++ 37 Malloc-and-free)
add_gcc_adapt_test($cc1plus C++ 38 New-and-delete-fundamental-types)
add_gcc_adapt_test($cc1plus C++ 39 New-and-delete-arrays)
add_gcc_adapt_test($cc1plus C++ 40 New-and-delete-composite-types)
add_gcc_adapt_test($cc1plus C++ 41 POD-class-types)
add_gcc_adapt_test($cc1plus C++ 42 Pointer-to-data-member)


# debugging helpers
configure_file(${cl_SOURCE_DIR}/tests/vkdebug.sh.in
               ${cl_BINARY_DIR}/tests/vkdebug.sh
               @ONLY)

configure_file(${cl_SOURCE_DIR}/tests/ptgccv.in
               ${cl_BINARY_DIR}/tests/ptgccv
               @ONLY)

configure_file(${cl_SOURCE_DIR}/tests/ptgdb.in
               ${cl_BINARY_DIR}/tests/ptgdb
               @ONLY)
