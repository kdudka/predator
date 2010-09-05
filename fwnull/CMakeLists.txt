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
project(fwnull C CXX)
add_definitions("-I../include")
enable_testing()

include("../CMakeLists.common")

# libfwnull.so
add_library(fwnull SHARED
    cl_fwnull.cc
    version.c)

# link with code_listener
find_library(CL_LIB cl ../cl_build)
target_link_libraries(fwnull ${CL_LIB})
