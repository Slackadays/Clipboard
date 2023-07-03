#!/bin/sh
export CLIPBOARD_FORCETTY=1
. ./resources.sh
start_test "Copy files"

make_files

cb copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

mkdir dir1

mkdir dir2

cb copy dir1 dir2

item_is_in_cb 0 dir1

item_is_in_cb 0 dir2

echo "Foobar" > file1

echo "Foobar" > file2

cb copy file1 dir1 dir2

item_is_in_cb 0 file1

item_is_in_cb 0 dir1

item_is_in_cb 0 dir2

cb copy file1 file2 dir1

item_is_in_cb 0 file1

item_is_in_cb 0 file2

item_is_in_cb 0 dir1

cb copy file1 file2 dir1 dir2

item_is_in_cb 0 file1

item_is_in_cb 0 file2

item_is_in_cb 0 dir1

item_is_in_cb 0 dir2

cb copy ../TurnYourClipboardUp.png

cb paste

items_match TurnYourClipboardUp.png ../TurnYourClipboardUp.png

cb clear

rm -f TurnYourClipboardUp.png

cb --fast-copy copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

cb --fast-copy copy ../TurnYourClipboardUp.png

cb --fast-copy paste

items_match TurnYourClipboardUp.png ../TurnYourClipboardUp.png

assert_fails cb copy testfile foo bar baz