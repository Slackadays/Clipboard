#!/bin/sh
. ./resources.sh

testname="Cut piped data"

start_test

setup_dir cut-pipe

echo "Foobar" | clipboard cut

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

item_is_not_in_cb 0 rawdata.clipboard

pass_test