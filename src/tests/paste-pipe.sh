#!/bin/sh
. ./resources.sh
setup_dir paste-pipe

echo "Foobar" | clipboard

clipboard paste

item_is_here rawdata.clipboard

pass_test paste-pipe