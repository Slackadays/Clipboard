#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1

start_test "Paste user provided text"

clipboard copy "Foobar"

clipboard paste

item_is_here rawdata.clipboard

pass_test