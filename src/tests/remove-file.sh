#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Remove files"

make_files

cb copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

cb remove testfile

item_is_not_in_cb 0 testfile

cb remove testdir

item_is_not_in_cb 0 testdir/testfile

cb copy testfile testdir

cb remove "test.*"

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile

cb copy testfile testdir

cb remove "testfile" "testdir"

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile

cb copy testfile testdir

cb remove ".*file" ".*dir"

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile

assert_fails cb remove "foo.*"

assert_fails cb remove ".*bar"

assert_fails cb remove "baz"

assert_fails cb remove foo bar baz