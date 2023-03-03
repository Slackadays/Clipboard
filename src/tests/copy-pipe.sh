#!/bin/sh
. ./resources.sh

testname="Copy piped data"

start_test "$testname"

setup_dir copy-pipe

echo "Foobar" | clipboard

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

unset CLIPBOARD_FORCETTY

clipboard < ../TurnYourClipboardUp.png

clipboard paste > temp # work around github actions tty bug

items_match temp ../TurnYourClipboardUp.png

pass_test "$testname"