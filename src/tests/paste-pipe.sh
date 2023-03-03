#!/bin/sh
. ./resources.sh

testname="Paste piped data"

start_test "$testname"

setup_dir paste-pipe

echo "Foobar" | clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

pass_test "$testname"