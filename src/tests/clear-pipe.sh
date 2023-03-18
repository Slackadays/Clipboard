#!/bin/sh
. ./resources.sh
start_test "Clear piped data"

echo "Foobar" | cb

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

cb clear

item_is_not_in_cb 0 rawdata.clipboard