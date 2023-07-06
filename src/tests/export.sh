#!/bin/sh
. ./resources.sh
start_test "Export clipboards"

export CLIPBOARD_FORCETTY=1

cb copy "Some text"

entryfor0="$(get_current_entry_name 0)"

unset CLIPBOARD_FORCETTY

cb copy2 < ../TurnYourClipboardUp.png

entryfor2="$(get_current_entry_name 2)"

cb copy3 < ../"Exosphere 2.0.mp3"

entryfor3="$(get_current_entry_name 3)"

export CLIPBOARD_FORCETTY=1

make_files

cb copy4 testfile testdir

entryfor4="$(get_current_entry_name 4)"

message="$(cb export 2>&1)"

content_is_shown "$message" "4"

item_exists Exported_Clipboards/0/data/"$entryfor0"/rawdata.clipboard "Some text"

item_exists Exported_Clipboards/2/data/"$entryfor2"/rawdata.clipboard "$(cat ../TurnYourClipboardUp.png)"

item_exists Exported_Clipboards/3/data/"$entryfor3"/rawdata.clipboard "$(cat ../"Exosphere 2.0.mp3")"

item_exists Exported_Clipboards/4/data/"$entryfor4"/testfile

item_exists Exported_Clipboards/4/data/"$entryfor4"/testdir/testfile