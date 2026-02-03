#!/bin/bash

# Copyright (C) 2020-2022 Kamil Dudka <kdudka@redhat.com>
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

SELF="$0"

PKG="predator"

die() {
    echo "$SELF: error: $1" >&2
    exit 1
}

match() {
    grep "$@" > /dev/null
}

DST="`readlink -f "$PWD"`"

REPO="`git rev-parse --show-toplevel`"
test -d "$REPO" || die "not in a git repo"

NV="`git describe --tags`"
echo "$NV" | match "^$PKG-" || die "release tag not found"

VER="`echo "$NV" | sed "s/^$PKG-//"`"

TIMESTAMP="`git log --pretty="%cd" --date=iso -1 \
    | tr -d ':-' | tr ' ' . | cut -d. -f 1,2`"

VER="`echo "$VER" | sed "s/-.*-/.$TIMESTAMP./"`"

BRANCH="`git rev-parse --abbrev-ref HEAD`"
test -n "$BRANCH" || die "failed to get current branch name"
test master = "${BRANCH}" || VER="${VER}.${BRANCH//[\/-]/_}"
test -z "`git diff HEAD`" || VER="${VER}.dirty"

NV="${PKG}-${VER}"
printf "\n%s: preparing a release of \033[1;32m%s\033[0m\n\n" "$SELF" "$NV"

if [[ "$1" != "--generate-spec" ]]; then
    TMP="`mktemp -d`"
    trap "rm -rf '$TMP'" EXIT
    cd "$TMP" >/dev/null || die "mktemp failed"

    # clone the repository
    git clone "$REPO" "$PKG"                || die "git clone failed"
    cd "$PKG"                               || die "git clone failed"
    patch -p1 < build-aux/distro-install.patch || die "failed to patch soource code"
    ./switch-host-gcc.sh /usr/bin/gcc       || die "'make distcheck' has failed"
    mv -v cl/version_cl.h sl/version.h .    || dir "failed to export version file"

    SRC_TAR="${NV}.tar"
    SRC="${SRC_TAR}.xz"
    git archive --prefix="$NV/" --format="tar" HEAD -- . > "$SRC_TAR" \
                                            || die "failed to export sources"

    xz -c "$SRC_TAR" > "$SRC"               || die "failed to compress sources"

    RELEASE="$1"
else
    make version_cl.h -C cl
    make version.h -C sl
    mv -v cl/version_cl.h sl/version.h .    || dir "failed to export version file"
    RELEASE="$2"
fi

SPEC="./$PKG.spec"
cat > "$SPEC" << EOF
Name:       $PKG
Version:    $VER
Release:    ${RELEASE:-1}%{?dist}
Summary:    A Shape Analyzer Based on Symbolic Memory Graphs

License:    GPLv3+
URL:        https://github.com/kdudka/%{name}
Source0:    https://github.com/kdudka/%{name}/releases/download/%{name}-%{version}/%{name}-%{version}.tar.xz
Source1:    version_cl.h
Source2:    version.h

BuildRequires: boost-devel
BuildRequires: cmake
BuildRequires: gcc-c++
BuildRequires: gcc-plugin-devel

Requires: gcc

%description
Predator is a tool for automated formal verification of sequential C programs
operating with pointers and linked lists. The core algorithms of Predator were
originally inspired by works on *separation logic* with higher-order list
predicates, but they are now purely graph-based and significantly extended to
support various forms of low-level memory manipulation used in system-level
code.  Such operations include pointer arithmetic, safe usage of invalid
pointers, block operations with memory, reinterpretation of the memory contents,
address alignment, etc.  The tool can be loaded into *GCC* as a *plug-in*.  This
way you can easily analyse C code sources, using the existing build system,
without manually preprocessing them first.  The analysis itself is, however, not
ready for complex projects yet.

# temporarily disable LTO on Fedora 44 to avoid a crash while unwinding
# the std::runtime_error exception
%if 0%{?fedora} == 44
%define _lto_cflags %{nil}
%endif

%prep
%setup -q
install -pv %{SOURCE1} cl/
install -pv %{SOURCE2} sl/
patch -p1 < build-aux/distro-install.patch
%if 0%{?rhel} && 0%{?rhel} < 9
patch -p1 < build-aux/gcc-8.3.0.patch
%endif
%if 0%{?rhel} == 7
patch -p1 < build-aux/gcc-6.5.0.patch
patch -p1 < build-aux/gcc-5.4.0.patch
%endif

%build
mkdir cl_build
cd cl_build
%cmake -S../cl ../cl -B. -DGCC_HOST=/usr/bin/gcc -Wno-dev
make %{?_smp_mflags} VERBOSE=yes

mkdir ../sl_build
cd ../sl_build
%cmake -S../sl ../sl -B. -DGCC_HOST=/usr/bin/gcc -Wno-dev
make %{?_smp_mflags} VERBOSE=yes

%install
%global plugin_dir %(gcc --print-file-name=plugin)
mkdir -p %{buildroot}%{plugin_dir}
install -m0755 sl_build/libsl.so %{buildroot}%{plugin_dir}/predator.so

%check
make check -C cl
make check -C sl

%files
%{plugin_dir}/predator.so
EOF

if [[ "$1" != "--generate-spec" ]]; then
    rpmbuild -bs "$SPEC"                            \
        --define "_sourcedir ."                     \
        --define "_specdir ."                       \
        --define "_srcrpmdir $DST"
fi
