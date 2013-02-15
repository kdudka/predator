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

# uncomment this on Darwin if linking the plug-ins fails on undefined references
#set(CMAKE_SHARED_LINKER_FLAGS "-flat_namespace -undefined suppress")

# CMake on Darwin would otherwise use .dylib suffix, which breaks GCC arg parser
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

# Check Boost availability
set(Boost_USE_STATIC_LIBS ON)
set(Boost_ADDITIONAL_VERSIONS "1.46" "1.47" "1.48" "1.49")
find_package(Boost 1.37)
if(Boost_FOUND)
    link_directories(${Boost_LIBRARY_DIRS})
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif()

# Check for a C compiler flag
include(CheckCCompilerFlag)
macro(ADD_C_FLAG opt_name opt)
    check_c_compiler_flag(${opt} HAVE_${opt_name})
    if(HAVE_${opt_name})
        add_definitions(${opt})
    endif()
endmacro()
macro(ADD_C_ONLY_FLAG opt_name opt)
    check_c_compiler_flag(${opt} C_HAVE_${opt_name})
    if(C_HAVE_${opt_name})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${opt}")
    endif()
endmacro()

# Check for a C++ compiler flag
include(CheckCXXCompilerFlag)
macro(ADD_CXX_ONLY_FLAG opt_name opt)
    check_cxx_compiler_flag(${opt} CXX_HAVE_${opt_name})
    if(CXX_HAVE_${opt_name})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${opt}")
    endif()
endmacro()

# treat Code Listener headers as system headers when scanning dependencies
include_directories(SYSTEM ../include)

# make sure the linking works appropriately
ADD_C_FLAG(       "fPIC"                 "-fPIC")
ADD_C_FLAG(       "hidden_visibility"    "-fvisibility=hidden")

# we use c99 to compile *.c and c++0x to compile *.cc
ADD_C_ONLY_FLAG(  "STD_C99"         "-std=c99")
ADD_CXX_ONLY_FLAG("STD_CXX_0X"      "-std=c++0x")

# tweak warnings
ADD_C_FLAG(       "PEDANTIC"             "-pedantic")
ADD_C_FLAG(       "W_ALL"                "-Wall")
ADD_C_FLAG(       "W_FLOAT_EQUAL"        "-Wfloat-equal")
ADD_C_ONLY_FLAG(  "W_UNDEF"              "-Wundef")
ADD_CXX_ONLY_FLAG("W_NO_DEPRECATED"      "-Wno-deprecated")
ADD_CXX_ONLY_FLAG("W_OVERLOADED_VIRTUAL" "-Woverloaded-virtual")

option(USE_WEXTRA "Set to ON to use -Wextra (recommended)" ON)
if(USE_WEXTRA)
    ADD_C_FLAG("W_EXTRA" "-Wextra")
endif()

option(USE_WERROR "Set to ON to use -Werror (recommended)" OFF)
if(USE_WERROR)
    ADD_C_FLAG("W_ERROR" "-Werror")
endif()

# if __asm__("int3") raises SIGTRAP, use it for breakpoints (SIGTRAP otherwise)
if("${INT3_RESPONSE}" STREQUAL "")
    message(STATUS "checking whether INT3 raises SIGTRAP")
    set(INT3_SRC "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/int3.c")
    file(WRITE "${INT3_SRC}" "int main(void) { __asm__(\"int3\"); }\n")
    try_run(INT3_RUN_RESULT INT3_COMPILE_RESULT
            "${CMAKE_BINARY_DIR}" "${INT3_SRC}"
            RUN_OUTPUT_VARIABLE INT3)

    # cache the result for next run
    set(INT3_RESPONSE "${INT3}" CACHE STRING "response to INT3")
    mark_as_advanced(INT3_RESPONSE)
endif()
if("${INT3_RESPONSE}" MATCHES "SIGTRAP")
    message(STATUS "INT3 will be used for trigerring breakpoints")
    add_definitions("-DUSE_INT3_AS_BRK")
else()
    message(STATUS "raise(SIGTRAP) will be used for trigerring breakpoints")
endif()

# FIXME: the use of $GCC_HOST from the environment should be better documented
if("$ENV{GCC_HOST}" STREQUAL "")
    get_filename_component(def_gcc_host "../gcc-install/bin/gcc" ABSOLUTE)
else()
    set(def_gcc_host "$ENV{GCC_HOST}")
endif()

set(GCC_HOST "${def_gcc_host}" CACHE STRING "absolute path to host gcc(1)")

if("$ENV{GCC_HOST}" STREQUAL "")
else()
    execute_process(COMMAND "${GCC_HOST}" "-print-file-name=plugin"
        RESULT_VARIABLE GCC_HOST_STATUS OUTPUT_QUIET)
    if (NOT "${GCC_HOST_STATUS}" EQUAL 0)

        # the current GCC_HOST does not work, try to fall-back to $ENV{GCC_HOST}
        set(GCC_HOST "$ENV{GCC_HOST}"
            CACHE STRING "absolute path to host gcc(1)" FORCE)
    endif()
endif()

option(TEST_WITH_VALGRIND "Set to ON to enable valgrind tests" OFF)
