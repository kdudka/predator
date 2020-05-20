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

if(APPLE)
    # do not fail with undefined references while linking the plug-ins on Darwin
    set(CMAKE_SHARED_LINKER_FLAGS "-flat_namespace -undefined suppress")
    # because of MACOSX_RPATH or better used cmake >=3.0
    set(CMAKE_MACOSX_RPATH 1)
endif()

# CMake on Darwin would otherwise use .dylib suffix, which breaks GCC arg parser
if(NOT ENABLE_LLVM)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
endif()

# Check Boost availability
set(Boost_USE_STATIC_LIBS ON)
set(Boost_ADDITIONAL_VERSIONS "1.46" "1.47" "1.48" "1.49")
find_package(Boost 1.37)
if(Boost_FOUND)
    link_directories(${Boost_LIBRARY_DIRS})
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif()

# Find llvm devel-headers and libs
if(ENABLE_LLVM)
    find_package(LLVM REQUIRED CONFIG)
    if(LLVM_FOUND)
        message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")
        message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
        message(STATUS "LLVM library directories: ${LLVM_LIBRARY_DIRS}")
        link_directories(${LLVM_LIBRARY_DIRS})
        message(STATUS "Including LLVM directories: ${LLVM_INCLUDE_DIRS}")
        include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
        add_definitions(${LLVM_DEFINITIONS})
        message(STATUS "LLVM binary dir: ${LLVM_TOOLS_BINARY_DIR}")
    endif()
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

# we use c99 to compile *.c and c++11/c++14 to compile *.cc
ADD_C_ONLY_FLAG(  "STD_C99"              "-std=c99")
if(ENABLE_LLVM)
    ADD_CXX_ONLY_FLAG("STD_CXX_14"       "-std=c++14")
else()
    ADD_CXX_ONLY_FLAG("STD_CXX_11"       "-std=c++11")
endif()

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
    set(INT3_BIN "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/int3")
    file(WRITE "${INT3_SRC}" "int main(void) { __asm__(\"int3\"); }\n")
    try_compile(INT3_COMPILE "${CMAKE_BINARY_DIR}"
        "${INT3_SRC}" COPY_FILE "${INT3_BIN}")
    execute_process(COMMAND "${INT3_BIN}" RESULT_VARIABLE INT3)
    file(REMOVE "${INT3_BIN}")

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

if(ENABLE_LLVM)
    if("$ENV{CLANG_HOST}" STREQUAL "")
        get_filename_component(def_clang_host "${LLVM_TOOLS_BINARY_DIR}/clang" ABSOLUTE)
    else()
        set(def_clang_host "$ENV{CLANG_HOST}")
    endif()

    set(CLANG_HOST "${def_clang_host}" CACHE STRING "absolute path to host clang(1)")

    if("$ENV{OPT_HOST}" STREQUAL "")
        get_filename_component(def_opt_host "${LLVM_TOOLS_BINARY_DIR}/opt" ABSOLUTE)
    else()
        set(def_opt_host "$ENV{OPT_HOST}")
    endif()

    set(OPT_HOST "${def_opt_host}" CACHE STRING "absolute path to host opt/LLVM")
else()
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
endif()

option(TEST_WITH_VALGRIND "Set to ON to enable valgrind tests" OFF)

if(NOT ENABLE_LLVM)
    # CMake cannot build shared libraries consisting of static libraries only
    set(EMPTY_C_FILE ${PROJECT_BINARY_DIR}/empty.c)
    if (NOT EXISTS ${EMPTY_C_FILE})
        # this will recursively pull all needed symbols from the static libraries
        file(WRITE ${EMPTY_C_FILE} "struct plugin_name;
    struct plugin_gcc_version;
    extern int plugin_init(struct plugin_name *, struct plugin_gcc_version *);\n
    void __cl_easy_stub(void)
    {
        plugin_init((struct plugin_name *)0, (struct plugin_gcc_version *)0);
    }\n")
    endif()
endif()

# build compiler plug-in PLUGIN from static lib ANALYZER using CL from LIBCL_PATH
macro(CL_BUILD_COMPILER_PLUGIN PLUGIN ANALYZER LIBCL_PATH)
    if(ENABLE_LLVM)
        # CMake cannot build shared libraries consisting of static libraries
        # only and we need set correct name for opt
        set(NAME_CC_FILE "${PROJECT_BINARY_DIR}/${PLUGIN}_name.cc")
        file(WRITE ${NAME_CC_FILE} "#include<string>\nstd::string plugName = \"${PLUGIN}\";\n")
        add_library(${PLUGIN} SHARED ${NAME_CC_FILE})
    else()
        # build GCC plug-in named lib${PLUGIN}.so
        add_library(${PLUGIN} SHARED ${EMPTY_C_FILE})
    endif()

    if("${LIBCL_PATH}" STREQUAL "")
        set(CL_LIB cl)
        set(CLGCC_LIB clgcc)
        set(CLLLVM_LIB clllvm)
    else()
        find_library(CL_LIB cl PATHS ${LIBCL_PATH} NO_DEFAULT_PATH)
        find_library(CLGCC_LIB clgcc PATHS ${LIBCL_PATH} NO_DEFAULT_PATH)
        find_library(CLLLVM_LIB clllvm PATHS ${LIBCL_PATH} NO_DEFAULT_PATH)
    endif()

    if(ENABLE_LLVM)
        # this will recursively pull all needed symbols from the static libraries
        if(NOT APPLE)
            set_target_properties(${PLUGIN} PROPERTIES LINK_FLAGS -Wl,--entry=plugin_init)
        else()
            # on Darwin entry symbol doesn't work properly, load everything
            set_target_properties(${PLUGIN} PROPERTIES LINK_FLAGS -Wl,-all_load)
        endif()
    endif()

    # link the static libraries all together
    if(ENABLE_LLVM)
        target_link_libraries(${PLUGIN} ${CLLLVM_LIB})
    else()
        target_link_libraries(${PLUGIN} ${CLGCC_LIB})
    endif()
    target_link_libraries(${PLUGIN} ${CL_LIB} ${ANALYZER})
endmacro()
