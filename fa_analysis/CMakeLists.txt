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
project(fa_analysis C CXX)
include("../CMakeLists.common")
add_definitions("-I../include -g -DDEBUG")

# libfa.a
add_library(fa treeaut.cc timbuk.cc box.cc forestaut.cc forestautext.cc ufae.cc symexec.cc label.cc symctx.cc builtintable.cc)
# avoid a failure while linking libfa_analysis.so (may be gcc specific):
set_target_properties(fa PROPERTIES COMPILE_FLAGS -fPIC)
# libfa_analysis.so
add_library(fa_analysis SHARED cl_fa.cc)
# test
add_executable(fa_test test.cc)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fa_analysis fa ${CL_LIB})
target_link_libraries(fa_test fa)
