#!/bin/sh
. ./resources.sh
start_test "Copy piped data"

echo "Foobar" | cb

item_is_in_cb 0 rawdata.clipboard

export CLIPBOARD_FORCETTY=1

cb paste

item_exists clipboard0-0.txt

unset CLIPBOARD_FORCETTY

cb < ../TurnYourClipboardUp.png

cb paste > temp # work around github actions tty bug

items_match temp ../TurnYourClipboardUp.png