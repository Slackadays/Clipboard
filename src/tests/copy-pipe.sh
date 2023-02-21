#!/bin/sh
. ./resources.sh
setup_dir copy-pipe

echo "Foobar" | clipboard

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

pass_test copy-pipe