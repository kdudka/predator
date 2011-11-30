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
include("../CMakeLists.common")
add_definitions("-I../include")
enable_testing()

option(FA_DEBUG "Set to OFF to enable optimized build" OFF)
if(FA_DEBUG)
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

# format of tests:
#
#    <test file>:<boxes directory>
#
set (forester_tests
  "cdll.c:genericdll.boxes"
  "dll-insert.c:genericdll.boxes"
  "dll-insertsort.c:genericdll.boxes"
  "dll-insertsort_v2.c:genericdll.boxes"
  "dll-listofclists.c:listofclists.dll.boxes"
  "dll-rev.c:genericdll.boxes"
  "dsw.c:"
  "recursion.c:"
  "sll-bubblesort.c:"
  "sll-delete.c:"
  "sll-headptr.c:"
  "sll-insertsort.c:"
  "sll-listofclists.c:listofclists.sll.boxes"
  "sll-mergesort.c:"
  "sll-rev.c:"
  "test-0080.c:"
  "test-0098.c:test-0098.boxes"
  "test-0124.inline.c:"
  "test-0128.c:"
  "treecons.c:"
  "tree_stack.c:"
  "treetraversal.c:treewpp.boxes"

  # currently not working test cases
  #
  #"listeven.c:"
  #"listref.c:"
  #"test-0061.c:genericdll.boxes"
  #"test-0113.c:"
  #"treecraziness.c:"
  #"rb_insert_rebalance.c:"
  #"dfs.c:treewpp.boxes"
)

foreach (testcase ${forester_tests})
  string(REPLACE ":" ";" testcase_list ${testcase})
  list(GET testcase_list 0 testfile)
  list(GET testcase_list 1 boxdir)

  if (NOT ${boxdir} STREQUAL "")
    set(box_arg "-fplugin-arg-libfa-args=db-root:${CMAKE_CURRENT_SOURCE_DIR}/data/${boxdir}")
  else()
    set(box_arg "")
  endif()

  add_test(
    ${testfile} timeout 90 ${GCC_HOST} -o /dev/null -m32 -DFORESTER
    -fplugin=${CMAKE_CURRENT_BUILD_DIR}../fa_build/libfa.so
    -c ${CMAKE_CURRENT_SOURCE_DIR}/data/${testfile} ${box_arg}
  )
endforeach(testcase)

