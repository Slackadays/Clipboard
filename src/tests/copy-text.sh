#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Copy user provided text"

cb copy "Foobar"

item_is_in_cb 0 rawdata.clipboard