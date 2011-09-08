# Copyright (C) 2010 Jiri Simacek
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

add_test(
	"sll_delete" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/sll-delete.c
)
add_test(
	"sll_reversal" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/sll-rev.c
)
add_test(
	"sll_bubblesort" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/sll-bubblesort.c
)
add_test(
	"sll_mergesort" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/sll-mergesort.c
)
add_test(
	"sll_listofclists" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/sll-listofclists.c
	-fplugin-arg-libfa-args=db-root:../fa/data/listofclists.sll.boxes
)
add_test(
	"recursion" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/recursion.c
)
add_test(
	"cdll" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/cdll.c
	-fplugin-arg-libfa-args=db-root:../fa/data/genericdll.boxes
)
add_test(
	"dll_insert" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/dll-insert.c
	-fplugin-arg-libfa-args=db-root:../fa/data/listofclists.dll.boxes
)
add_test(
	"dll_rev" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/dll-rev.c
	-fplugin-arg-libfa-args=db-root:../fa/data/genericdll.boxes
)
add_test(
	"dll_listofclists" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/dll-listofclists.c
	-fplugin-arg-libfa-args=db-root:../fa/data/listofclists.dll.boxes
)
add_test(
	"dll_insertsort" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/dll-insertsort.c
	-fplugin-arg-libfa-args=db-root:../fa/data/genericdll.boxes
)
add_test(
	"dll_insertsort_v2" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/dll-insertsort_v2.c
	-fplugin-arg-libfa-args=db-root:../fa/data/genericdll.boxes
)
add_test(
	"test_0061" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/test-0080.c
	-fplugin-arg-libfa-args=db-root:../fa/data/genericdll.boxes
)
add_test(
	"test_0080" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/test-0080.c
)
add_test(
	"test_0098" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/test-0098.c
	-fplugin-arg-libfa-args=db-root:../fa/data/test-0098.boxes
)
add_test(
	"test_0128" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/test-0128.c
)
add_test(
	"treecons" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/treecons.c
)
add_test(
	"test_stack" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/tree_stack.c
)
add_test(
	"tree_traversal" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/treetraversal.c
	-fplugin-arg-libfa-args=db-root:../fa/data/treewpp.boxes
)
add_test(
	"dsw" ${GCC_HOST} -o /dev/null -m32 -DFORESTER -fplugin=../fa_build/libfa.so
	-c ../fa/data/dsw.c
)
