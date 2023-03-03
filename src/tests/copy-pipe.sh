#!/bin/sh
. ./resources.sh

start_test copy-pipe

setup_dir copy-pipe

echo "Foobar" | clipboard

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

clipboard paste

item_is_here rawdata.clipboard

unset CLIPBOARD_FORCETTY

cat ../TurnYourClipboardUp.png | clipboard

clipboard > temp

items_match temp ../TurnYourClipboardUp.png

pass_test copy-pipe