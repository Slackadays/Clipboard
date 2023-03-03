#!/bin/sh
. ./resources.sh

start_test "Paste piped data"

echo "Foobar" | clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

pass_test