# Copyright (C) 2013 Daniela Ďuričeková <xduric00@stud.fit.vutbr.cz>
#
# This file is part of value-range analyzer.
#
# Value-range analyzer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or any later version.
#
# Value-range analyzer is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# the value-range analyzer. If not, see <http://www.gnu.org/licenses/>.

# project metadata
cmake_minimum_required(VERSION 2.6)
project(vra C CXX)
enable_testing()

include("../build-aux/common.cmake")

# debugging
option(VRA_DEBUG "Set to ON to enable debugging support" ON)
if(VRA_DEBUG)
    add_definitions("-O0 -g")
else()
    add_definitions("-O3 -DNDEBUG")
endif()

# disable some warnings
add_definitions("-Wno-float-equal")

# libvra.so
add_library(vra_core STATIC
    vra.cc
    Number.cc
    Range.cc
    MemoryPlace.cc
    OperandToMemoryPlace.cc
    ValueAnalysis.cc
    Utility.cc
    LoopFinder.cc
    GlobAnalysis.cc
    version.c)

# build compiler plug-in (libvra.so)
CL_BUILD_COMPILER_PLUGIN(vra vra_core ../cl_build)

# helping scripts
configure_file( ${PROJECT_SOURCE_DIR}/tests-overall/tests-config.sh.in
  ${PROJECT_BINARY_DIR}/tests-config.sh     @ONLY)

# link with gmp
find_library(GMP_LIB gmp)
if(NOT GMP_LIB)
  message(FATAL_ERROR "gmp library not found.")
endif()

# link with gmpxx
find_library(GMPXX_LIB gmpxx)
if(NOT GMPXX_LIB)
  message(FATAL_ERROR "gmpxx library not found.")
endif()

target_link_libraries(vra ${CL_LIB} ${GMP_LIB} ${GMPXX_LIB})

# make install
install(TARGETS vra DESTINATION lib)
