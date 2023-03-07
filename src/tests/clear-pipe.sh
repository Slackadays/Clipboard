#!/bin/sh
. ./resources.sh

start_test "Clear piped data"

echo "Foobar" | clipboard

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

clipboard clear

item_is_not_in_cb 0 rawdata.clipboard