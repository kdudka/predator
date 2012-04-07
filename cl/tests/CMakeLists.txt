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

# compile libcl_test.so
add_library(cl_test SHARED cl_test.cc)
target_link_libraries(cl_test cl)

# basic set of the options to compile gcc/clplug.c with libc_test.so loaded
set(cmd "LC_ALL=C CCACHE_DISABLE=1 ${GCC_HOST}")
set(cmd "${cmd} ${CFLAGS}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}")
set(cmd "${cmd} -I${cl_SOURCE_DIR}/../include")
set(cmd "${cmd} -I${GCC_INC_DIR}")
set(cmd "${cmd} -c ${cl_SOURCE_DIR}/gcc/clplug.c")
set(cmd "${cmd} -o /dev/null")
set(cmd "${cmd} -fplugin=${cl_BINARY_DIR}/tests/libcl_test.so")
set(cmd_base "${cmd}")

# smoke test #1
set(cmd "${cmd} -fplugin-arg-libcl_test-dry-run")
add_test("compile-self" sh -c "${cmd}")

# smoke test #2
set(cmd "${cmd_base}")
set(cmd "${cmd} | diff -up ${cl_SOURCE_DIR}/cl_test.out -")
add_test("cl_test" sh -c "${cmd}")

# smoke test #3
set(cmd "${cmd_base}")
set(cmd "${cmd} -fplugin-arg-libcl_test-dump-pp=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_test-dump-types")
set(cmd "${cmd} -fplugin-arg-libcl_test-gen-dot=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_test-type-dot=/dev/null")
set(cmd "${cmd} -fplugin-arg-libcl_test-verbose=15")
set(cmd "${cmd} -fplugin-arg-libcl_test-dry-run")
add_test("compile-self-extra" sh -c "${cmd}")

# smoke test #4 runs through valgrind and it is optional
if(TEST_WITH_VALGRIND)
    set(cmd "valgrind --trace-children=yes --error-exitcode=7 ${cmd}")
    add_test("compile-self-valgrind" sh -c "${cmd}")
endif()

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
