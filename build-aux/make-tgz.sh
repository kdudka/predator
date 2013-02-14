#!/bin/sh
SH_NAME="$0"

die(){
    msg=error
    test -n "$1" && msg="error: $1"
    echo "$SH_NAME: $msg" >&2
    exit 1
}

usage(){
    echo "Usage: $SH_NAME code-listener|forester|predator"
    exit 1
}

PRUNE_ALWAYS=".git tests/linux-drivers \
build-aux/make-tgz.sh \
build-aux/README-fedora-release.patch \
build-aux/README-ubuntu-release.patch \
build-aux/make-srpm.sh \
build-aux/update-comments-in-tests.sh \
sl/rank.sh"

chlog_watch=
drop_fwnull=no
drop_fa=no
drop_sl=no

PROJECT="$1"
case "$PROJECT" in
    code-listener)
        chlog_watch="cl fwnull"
        drop_fa=yes
        drop_sl=yes
        ;;

    forester)
        chlog_watch="fa fa_analysis"
        drop_fwnull=yes
        drop_sl=yes
        ;;

    predator)
        chlog_watch="sl"
        drop_fa=yes
        ;;

    *)
        usage
        ;;
esac

test -d build-aux || die "this script needs to be run from \$PREDATOR_ROOT"
test -x build-aux/make-tgz.sh || die "unable to find self"

REPO="`git rev-parse --show-toplevel`" \
    || die "not in a git repo"

printf "%s: considering release of %s using %s...\n" \
    "$SH_NAME" "$PROJECT" "$REPO"

branch="`git status | head -1 | sed 's/^#.* //'`" \
    || die "unable to read git branch"

test xmaster = "x$branch" \
    || die "not in master branch"

test -z "`git diff HEAD`" \
    || die "HEAD dirty"

test -z "`git diff origin/master`" \
    || die "not synced with origin/master"

make -j9 distcheck CTEST='ctest -j9' \
    || die "'make distcheck' has failed"

STAMP="`git log --pretty="%cd-%h" --date=short -1`" \
    || die "git log failed"

# clone the repository
NAME="${PROJECT}-$STAMP"
TGZ="${NAME}.tar.gz"
TMP="`mktemp -d`"           || die "mktemp failed"
cd "$TMP"                   || die "mktemp failed"
git clone "$REPO" "$NAME"   || die "git clone failed"
cd "$NAME"                  || die "git clone failed"

# create all version files (no chance to create them out of a git repo)
make version_cl.h -C cl     || die "failed to create cl/version_cl.h"
make version.h -C fwnull    || die "failed to create fwnull/version.h"
make version.h -C sl        || die "failed to create sl/version.h"

# generate ChangeLog
make ChangeLog "CHLOG_WATCH=$chlog_watch" \
    || die "failed to generate ChangeLog"

# adapt README-{fedora,ubuntu}
patch docs/README-fedora < "build-aux/README-fedora-release.patch"
patch docs/README-ubuntu < "build-aux/README-ubuntu-release.patch"

# adapt Makefile
case "$PROJECT" in
    code-listener)
        sed -i Makefile                         \
            -e 's|fwnull sl fa$|fwnull|'        \
            -e 's|cl/api sl/api|cl/api|'        \
            || die "failed to adapt Makefile"
        ;;

    forester)
        sed -i Makefile                         \
            -e 's|fwnull sl fa$|fa|'            \
            -e 's|cl/api sl/api|cl/api|'        \
            || die "failed to adapt Makefile"
        ;;

    predator)
        sed -i Makefile                         \
            -e 's|fwnull sl fa$|fwnull sl|'     \
            || die "failed to adapt Makefile"
        ;;

    *)
        die "internal error"
        ;;
esac

# remove all directories and files we do not want to distribute
rm -rf $PRUNE_ALWAYS
test xyes = "x$drop_fwnull" && rm -rf fwnull
test xyes = "x$drop_fa" && rm -rf fa
test xyes = "x$drop_sl" && rm -rf sl "README-sv-comp-TACAS-2013"

# make a tarball
cd ..
test -d "$NAME" || die "internal error"
tar c "$NAME" | gzip -c > "${REPO}/${TGZ}" \
    || die "failed to package the release"

# cleanup
rm -rf "$TMP"

# done
cd "$REPO"
echo ==========================================================================
ls -lh "$TGZ"
echo ==========================================================================
