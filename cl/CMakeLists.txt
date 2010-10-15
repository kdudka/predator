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
project(cl C CXX)
enable_testing()

include("../CMakeLists.common")

# Check for isatty(3)
include(CheckFunctionExists)
check_function_exists("isatty" HAVE_ISATTY)
if (HAVE_ISATTY)
    add_definitions("-DHAVE_ISATTY=1")
else()
    add_definitions("-DHAVE_ISATTY=0")
endif()

# tweak include dirs, etc.
add_definitions("-I../include -I../include/gcc -I../cl -I../cl/gcc -fPIC")

# libcl.so
add_library(cl STATIC
    cl_chain.cc
    cl_dotgen.cc
    cl_easy.cc
    cl_factory.cc
    cl_locator.cc
    cl_pp.cc
    cl_storage.cc
    cl_typedot.cc
    cldebug.cc
    clf_intchk.cc
    clf_opchk.cc
    clf_unilabel.cc
    clf_unswitch.cc
    clutil.cc
    code_listener.cc
    gcc/clplug.c
    location.cc
    ssd.cc
    storage.cc
    version.c)

add_library(cl_test SHARED cl_test.cc)
target_link_libraries(cl_test cl)

# make install
install(TARGETS cl DESTINATION lib)
install(DIRECTORY ../include/cl DESTINATION include)

add_test("compile-self" ${GCC_HOST}
    ${CFLAGS}
    -I${cl_SOURCE_DIR}
    -I${cl_SOURCE_DIR}/../include
    -I${cl_SOURCE_DIR}/../include/gcc
    -I${cl_SOURCE_DIR}/gcc
    -c ${cl_SOURCE_DIR}/gcc/clplug.c
    -o /dev/null
    -fplugin=${cl_BINARY_DIR}/libcl_test.so
    -fplugin-arg-libcl_test-dry-run)

set(cmd "${GCC_HOST}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include/gcc")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/gcc")
set(cmd "${cmd} -c ${cl_SOURCE_DIR}/gcc/clplug.c -o /dev/null")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/libcl_test.so")
set(cmd "${cmd} | diff -up ${cl_SOURCE_DIR}/cl_test.out -")
add_test("cl_test" sh -c "${cmd}")

set(self_extra_opts
    ${CFLAGS}
    -I${cl_SOURCE_DIR}
    -I${cl_SOURCE_DIR}/../include
    -I${cl_SOURCE_DIR}/../include/gcc
    -I${cl_SOURCE_DIR}/gcc
    -c ${cl_SOURCE_DIR}/gcc/clplug.c
    -fplugin=${cl_BINARY_DIR}/libcl_test.so
    -fplugin-arg-libcl_test-dump-pp=/dev/null
    -fplugin-arg-libcl_test-dump-types
    -fplugin-arg-libcl_test-gen-dot=/dev/null
    -fplugin-arg-libcl_test-type-dot=/dev/null
    -fplugin-arg-libcl_test-verbose=15
    -fplugin-arg-libcl_test-dry-run)

add_test("compile-self-extra" ${GCC_HOST} ${self_extra_opts})
add_test("compile-self-valgrind" valgrind
    --trace-children=yes
    --error-exitcode=7
    ${GCC_HOST}
    ${self_extra_opts})

add_test("headers_sanity-0" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/config_cl.h)

add_test("headers_sanity-1" gcc -ansi -Wall -Wextra -Werror -pedantic
    -o /dev/null
    -c ${cl_SOURCE_DIR}/../include/cl/code_listener.h)

add_test("headers_sanity-2" make -j
    -C ${cl_SOURCE_DIR}/../include/cl
    -f ${cl_SOURCE_DIR}/Makefile.chk)

add_test("headers_sanity-3" make -j
    -C ${cl_SOURCE_DIR}
    -f ${cl_SOURCE_DIR}/Makefile.chk)
