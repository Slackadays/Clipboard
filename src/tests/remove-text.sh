#!/bin/sh
. ./resources.sh
start_test "Remove text"
export CLIPBOARD_FORCETTY=1

clipboard copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

clipboard remove "Foobar"

unset CLIPBOARD_FORCETTY

assert_equals "" "$(clipboard paste)"