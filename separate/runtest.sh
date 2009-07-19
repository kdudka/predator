#!/bin/bash
#
# Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
#
# This file is part of cgt (Call Graph Tools).
#
# cgt is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# cgt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cgt.  If not, see <http://www.gnu.org/licenses/>.

VALGRIND=valgrind
VALGRIND_EC=3
VALGRIND_EC_LEAK=4
VALGRIND_OPTS="--leak-check=full --error-exitcode=$VALGRIND_EC"

set -x
test -n "$1" || exit 1
TEMP=`mktemp` || exit 1
$VALGRIND $VALGRIND_OPTS --log-fd=3 "./test-$1" 3>"$TEMP"; EC=$?
if test $EC = 0; then
    grep LEAK "$TEMP" && EC=$VALGRIND_EC_LEAK
fi
test $EC = 0 || cat "$TEMP" 1>&2
rm -fv "$TEMP"
exit $EC
