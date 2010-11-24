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

option(FA_DEBUG "Set to OFF to enable optimized build" ON)
if(FA_DEBUG)
    add_definitions("-O0 -g")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# libfa.a
#add_library(fa treeaut.cc timbuk.cc forestaut.cc symexec.cc symctx.cc builtintable.cc)
# libfa_analysis.so
add_library(fa SHARED treeaut.cc timbuk.cc forestaut.cc symexec.cc symctx.cc builtintable.cc cl_fa.cc)
# avoid a failure while linking libfa_analysis.so (may be gcc specific):
#set_target_properties(fa PROPERTIES COMPILE_FLAGS -fPIC)
# test
#add_executable(fa_test test.cc)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fa ${CL_LIB})
#target_link_libraries(fa_test fa)
