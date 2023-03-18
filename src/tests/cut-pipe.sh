#!/bin/sh
. ./resources.sh
start_test "Cut piped data"

echo "Foobar" | cb cut

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

cb paste

item_is_here rawdata.clipboard

item_is_not_in_cb 0 rawdata.clipboard