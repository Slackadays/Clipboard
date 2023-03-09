#!/bin/sh
export CLIPBOARD_FORCETTY=1
. ./resources.sh
start_test "Copy files"

make_files

clipboard copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

clipboard copy ../TurnYourClipboardUp.png

clipboard paste

items_match TurnYourClipboardUp.png ../TurnYourClipboardUp.png