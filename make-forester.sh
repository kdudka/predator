#!/bin/sh
die(){
    exit 1
}

PROJECT="forester"
REPO="`git rev-parse --show-toplevel`"              || die
BRANCH="`git stat | head -1 | sed 's/^#.* //'`"     || die

printf "%s: considering release of branch '%s' of '%s'...\n" \
    "$0" "$BRANCH" "$REPO"
set -x
test -z "`git diff HEAD`"                           || die
test -z "`git diff origin/$BRANCH`"                 || die
make -j5 distcheck                                  || die

STAMP="`git log --pretty="%cd-%h" --date=short -1`" || die
NAME="${PROJECT}-$STAMP"                            || die
TMP="`mktemp -d`"                                   || die
mkdir "${TMP}/${NAME}"                              || die
cd "${TMP}/${NAME}"                                 || die
cp "$REPO/CMakeLists.common" .				|| die
cp "$REPO/COPYING" .				|| die
cp "$REPO/FAQ" .				|| die
cp "$REPO/HACKING" .				|| die
cp "$REPO/Makefile" .				|| die
cp "$REPO/README" .				|| die
cp -r "$REPO/include" .					|| die 
cp -r "$REPO/cl" .					|| die 
cp "$REPO/fa/*.hh" ./fa/				|| die 
cp "$REPO/fa/*.cc" ./fa/				|| die 
cp "$REPO/fa/CMakeLists.txt" ./fa/			|| die 
cp "$REPO/fa/data/sll-rev.c" ./fa/examples/		|| die 
cp "$REPO/fa/data/sll-mergesort.c" ./fa/examples/	|| die 
cp "$REPO/fa/data/dsw.c" ./fa/examples/			|| die 
make ChangeLog                                      || die
cd ..                                               || die
test -d "$NAME"                                     || die
TGZ="${NAME}.tar.gz"                                || die
tar cv "$NAME" | gzip -c > "${REPO}/${TGZ}"         || die
rm -rfv "$TMP"                                      || die
cd "$REPO"                                          || die
set +x
echo =====================================================
ls -lh "$TGZ"
echo =====================================================
