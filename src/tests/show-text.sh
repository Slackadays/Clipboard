#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1

clipboard copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

output=$(clipboard show)

content_is_shown "$output" "Foobar"

pass_test show-text