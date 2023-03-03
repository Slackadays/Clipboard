#!/bin/sh
. ./resources.sh

export testname="Paste piped data"

start_test

setup_dir paste-pipe

echo "Foobar" | clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

pass_test