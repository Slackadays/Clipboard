#!/bin/sh
. ./resources.sh
setup_dir paste-pipe

echo "Foobar" | clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

pass_test paste-pipe