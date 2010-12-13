#!/bin/sh
die(){
    exit 1
}

PROJECT="predator"
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
git init                                            || die
git pull "$REPO" "$BRANCH"                          || die
make ChangeLog                                      || die
rm -rf .git make-tgz.sh                             || die
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
