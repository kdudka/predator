#!/bin/sh
die(){
    exit 1
}

PROJECT="forester"
REPO="`git rev-parse --show-toplevel`"              || die
BRANCH="`git status | head -1 | sed 's/^#.* //'`"     || die

printf "%s: considering release of branch '%s' of '%s'...\n" \
    "$0" "$BRANCH" "$REPO"
set -x
#test -z "`git diff HEAD`"                           || die
#test -z "`git diff origin/$BRANCH`"                 || die
make -j5 -C fa distcheck                            || die

STAMP="`git log --pretty="%cd-%h" --date=short -1`" || die
NAME="${PROJECT}-$STAMP"                            || die
TMP="`mktemp -d`"                                   || die
mkdir "${TMP}/${NAME}"                              || die
mkdir "${TMP}/GIT"                              || die
cd "${TMP}/GIT"                                 || die
git init                                            || die
git pull "$REPO" "$BRANCH"                          || die
git log --pretty="format:%ad  %an%n%n%w(80,8,8)%B%n" --date=short -- fa > ${TMP}/${NAME}/ChangeLog || die
cd "${TMP}/${NAME}"                                 || die
cp ${TMP}/GIT/fa_release/* .				|| die
cp -r ${TMP}/GIT/include .					|| die 
cp -r ${TMP}/GIT/cl .					|| die 
mkdir ./fa					|| die
cp ${TMP}/GIT/fa/*.hh ./fa/				|| die 
cp ${TMP}/GIT/fa/*.cc ./fa/				|| die 
cp ${TMP}/GIT/fa/CMakeLists.txt ./fa/			|| die 
mkdir ./fa/examples				|| die
cp ${TMP}/GIT/fa/data/sll-rev.c ./fa/examples/		|| die 
cp ${TMP}/GIT/fa/data/sll-mergesort.c ./fa/examples/	|| die 
cp ${TMP}/GIT/fa/data/dsw.c ./fa/examples/			|| die 
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
