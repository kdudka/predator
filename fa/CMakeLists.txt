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

add_test(
	"sll_reversal" ../gcc-install/bin/gcc -c ../fa/data/sll-rev.c -o /dev/null -O0 -m32
    -DFORESTER
    -fplugin=../fa_build/libfa.so
    -fplugin-arg-libfa-verbose=0
)
add_test(
	"sll_mergesort" ../gcc-install/bin/gcc -c ../fa/data/sll-mergesort.c -o /dev/null -O0 -m32
    -DFORESTER
    -fplugin=../fa_build/libfa.so
    -fplugin-arg-libfa-verbose=0
)
add_test(
	"dsw" ../gcc-install/bin/gcc -c ../fa/data/dsw.c -o /dev/null -O0 -m32
    -DFORESTER
    -fplugin=../fa_build/libfa.so
    -fplugin-arg-libfa-verbose=0
)

# libfa.so
add_library(fa SHARED treeaut.cc timbuk.cc forestaut.cc symexec.cc symctx.cc builtintable.cc cl_fa.cc)
set_target_properties(fa PROPERTIES LINK_FLAGS -lrt)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fa ${CL_LIB})
