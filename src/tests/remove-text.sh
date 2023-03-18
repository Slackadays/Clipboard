#!/bin/sh
. ./resources.sh
start_test "Remove text"
export CLIPBOARD_FORCETTY=1

cb copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

cb remove "Foobar"

unset CLIPBOARD_FORCETTY

assert_equals "" "$(cb paste)"