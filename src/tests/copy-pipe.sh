#!/bin/sh
. ./resources.sh
setup_dir copy-pipe

echo "Foobar" | clipboard

item_is_in_cb 0 rawdata.clipboard

pass_test copy-pipe