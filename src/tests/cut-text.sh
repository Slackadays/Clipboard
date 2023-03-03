#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1

export testname="Cut user provided text"

start_test

setup_dir cut-text

clipboard cut "Foobar"

item_is_in_cb 0 rawdata.clipboard

clipboard paste

item_is_here rawdata.clipboard

item_is_not_in_cb 0 rawdata.clipboard

pass_test