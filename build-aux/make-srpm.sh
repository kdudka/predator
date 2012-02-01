#/bin/bash
SELF="$0"

PKG="predator"

die(){
    echo "$SELF: error: $1" >&2
    exit 1
}

test -d build-aux || die "this script needs to be run from \$PREDATOR_ROOT"
test -x build-aux/make-srpm.sh || die "unable to find self"

REPO="`git rev-parse --show-toplevel`" \
    || die "not in a git repo"

printf "%s: considering release of %s using %s...\n" \
    "$SELF" "$PKG" "$REPO"

branch="`git status | head -1 | sed 's/^#.* //'`" \
    || die "unable to read git branch"

test xmaster = "x$branch" \
    || die "not in master branch"

test -z "`git diff HEAD`" \
    || die "HEAD dirty"

test -z "`git diff origin/master`" \
    || die "not synced with origin/master"

VER="`git log --pretty="%cd_%h" --date=short -1`" || die "git log failed"
PKG_VER="0.`echo "$VER" | tr -d -`"
VER="`echo "$VER" | tr _ -`"

TGZ="predator-$VER.tar.gz"
test -e "$TGZ" && die "`readlink -f $TGZ` already exists, please remove it"

"$REPO/build-aux/make-tgz.sh" predator || die "make-tgz.sh failed"
test -r "$TGZ" || die "make-tgz.sh failed"

TMP="`mktemp -d`"
trap "echo --- $SELF: removing $TMP... 2>&1; rm -rf '$TMP'" EXIT
cd "$TMP" >/dev/null || die "mktemp failed"

NV="${PKG}-$PKG_VER"
mv -v "${REPO}/$TGZ" "$TMP"                       || die "internal error"
cp -v "${REPO}/build-aux/cl-config.patch" "$TMP"  || die "internal error"

SPEC="./$PKG.spec"
cat > "$SPEC" << EOF
Name:       $PKG
Version:    $PKG_VER
Release:    1%{?dist}
Summary:    A Separation Logic-based GCC plug-in.

Group:      VeriFIT
License:    GPLv3+
URL:        http://www.fit.vutbr.cz/research/groups/verifit/tools/predator
Source0:    $TGZ
Patch0:     cl-config.patch

BuildRequires: boost-devel
BuildRequires: cmake
BuildRequires: gcc-plugin-devel

# we need 32bit glibc-devel in order to pass the test-suite
BuildRequires: /usr/include/gnu/stubs-32.h

%description
Predator is a practical tool for checking manipulation of dynamic data
structures using *separation logic*.  It can be loaded directly into *GCC* as a
*plug-in*.  This way you can easily analyse C code sources, using the existing
build system, without any manual preprocessing of them etc.  The analysis itself
is, however, not ready for complex projects yet.  The plug-in is based on Code
Listener infrastructure (included).

%prep
%setup -q -n $PKG-$VER
%patch0 -p1

# patch the expected output of the regression tests (if a patch is available)
gcc_ver="\$(gcc --version | head -1 | sed -e 's|^.* ||' -e 's|-.*\$||')"
tests_patch="build-aux/gcc-\$gcc_ver.patch"
test -e "\$tests_patch" && patch -p1 < "\$tests_patch"

%build
make %{?_smp_mflags} CMAKE='cmake -D GCC_HOST=/usr/bin/gcc' VERBOSE=yes

%install
install -m0755 -d \$RPM_BUILD_ROOT%{_libdir}/
install -m0755 fwnull_build/libfwnull.so \$RPM_BUILD_ROOT%{_libdir}/
install -m0755 sl_build/libsl.so \$RPM_BUILD_ROOT%{_libdir}/

%check
make %{?_smp_mflags} check CTEST='ctest %{?_smp_mflags}'

%files
%defattr(-,root,root,-)
%{_libdir}/libfwnull.so
%{_libdir}/libsl.so
EOF

set -v
rpmbuild -bs "$SPEC"            \
    --define "_sourcedir ."     \
    --define "_specdir ."       \
    --define "_srcrpmdir $REPO"
