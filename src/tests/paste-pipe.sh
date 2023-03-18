#!/bin/sh
. ./resources.sh
start_test "Paste piped data"

echo "Foobar" | cb

export CLIPBOARD_FORCETTY=1

cb paste

item_is_here rawdata.clipboard