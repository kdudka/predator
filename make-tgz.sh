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

PRUNE_ALWAYS="dev-util fa_release invader-extras make-forester.sh make-tgz.sh \
    ocaml seplog .git"

chlog_watch=
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
        chlog_watch="fa"
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

make -j5 distcheck \
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

# sync FAQ and Makefile
# TODO: sync README
case "$PROJECT" in
    code-listener)
        sed -i \
            -e 's/\<sl\>/fwnull/' \
            -e 's/\<SL_/FWNULL_/' \
            FAQ || die "failed to adapt FAQ"
        sed -i 's/cl fwnull.*/cl fwnull/' Makefile \
            || die "failed to adapt Makefile"
        ;;

    forester)
        sed -i \
            -e 's/\<sl\>/fa/' \
            -e 's/\<SL_/FA_/' \
            FAQ || die "failed to adapt FAQ"
        sed -i 's/cl fwnull sl/cl fwnull/' Makefile \
            || die "failed to adapt Makefile"
        ;;

    predator)
        sed -i 's/cl fwnull sl fa/cl fwnull sl/' Makefile \
            || die "failed to adapt Makefile"
        ;;

    *)
        die "internal error"
        ;;
esac

# remove all directories and files we do not want to distribute
rm -rf $PRUNE_ALWAYS
test xyes = "x$drop_fa" && rm -rf fa
test xyes = "x$drop_sl" && rm -rf sl

# make a tarball
cd ..
test -d "$NAME" || die "internal error"
tar cv "$NAME" | gzip -c > "${REPO}/${TGZ}" \
    || die "failed to package the release"

# cleanup
rm -rf "$TMP"

# done
cd "$REPO"
echo ==========================================================================
ls -lh "$TGZ"
echo ==========================================================================
