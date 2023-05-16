#!/bin/sh
. ./resources.sh
start_test "Ignore regex patterns"

export CLIPBOARD_FORCETTY=1

cb ignore ""

make_files

cb copy testfile testdir

item_is_in_cb 0 "testfile"

item_is_in_cb 0 "testdir"

cb ignore "testfile"

item_is_in_cb 0 "testdir"

item_is_not_in_cb 0 "testfile"

cb ignore "testdir"

item_is_not_in_cb 0 "testdir"

cb ignore ""