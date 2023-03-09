#!/bin/sh
export CLIPBOARD_FORCETTY=1
. ./resources.sh
start_test "Clear files"

make_files

clipboard copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

clipboard clear

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile