#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Cut user provided text"

cb cut "Foobar"

item_is_in_cb 0 rawdata.clipboard

cb paste

item_exists clipboard0-0.txt

item_is_not_in_cb 0 rawdata.clipboard