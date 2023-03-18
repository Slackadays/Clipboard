#!/bin/sh
export CLIPBOARD_FORCETTY=1
. ./resources.sh
start_test "Copy files"

make_files

clipboard copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

mkdir dir1

mkdir dir2

clipboard copy dir1 dir2

item_is_in_cb 0 dir1

item_is_in_cb 0 dir2

echo "Foobar" > file1

echo "Foobar" > file2

clipboard copy file1 dir1 dir2

item_is_in_cb 0 file1

item_is_in_cb 0 dir1

item_is_in_cb 0 dir2

clipboard copy file1 file2 dir1

item_is_in_cb 0 file1

item_is_in_cb 0 file2

item_is_in_cb 0 dir1

clipboard copy file1 file2 dir1 dir2

item_is_in_cb 0 file1

item_is_in_cb 0 file2

item_is_in_cb 0 dir1

item_is_in_cb 0 dir2

clipboard copy ../TurnYourClipboardUp.png

clipboard paste

items_match TurnYourClipboardUp.png ../TurnYourClipboardUp.png

clipboard clear

rm -f TurnYourClipboardUp.png

clipboard --fast-copy copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

clipboard --fast-copy copy ../TurnYourClipboardUp.png

clipboard --fast-copy paste

items_match TurnYourClipboardUp.png ../TurnYourClipboardUp.png

assert_fails clipboard copy foo bar baz