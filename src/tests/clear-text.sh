#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Clear user provided text"

cb copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

cb clear

item_is_not_in_cb 0 rawdata.clipboard