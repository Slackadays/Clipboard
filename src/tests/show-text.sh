#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show user provided text"

cb copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

output=$(cb show 2>&1)

content_is_shown "$output" "Foobar"