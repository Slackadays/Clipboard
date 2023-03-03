#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1

testname="Clear user provided text"

start_test "$testname"

clipboard copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

clipboard clear

item_is_not_in_cb 0 rawdata.clipboard

pass_test "$testname"