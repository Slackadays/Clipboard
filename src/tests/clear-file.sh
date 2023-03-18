#!/bin/sh
export CLIPBOARD_FORCETTY=1
. ./resources.sh
start_test "Clear files"

make_files

cb copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

cb clear

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile