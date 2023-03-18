#!/bin/sh
. ./resources.sh
start_test "Show piped data"

echo "Foobar" | cb

export CLIPBOARD_FORCETTY=1

output=$(cb show)

item_is_in_cb 0 rawdata.clipboard

content_is_shown "$output" "Foobar"