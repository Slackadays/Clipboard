#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Remove files"

make_files

clipboard copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

clipboard remove testfile

item_is_not_in_cb 0 testfile

clipboard remove testdir

item_is_not_in_cb 0 testdir/testfile

clipboard copy testfile testdir

clipboard remove "test.*"

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile

clipboard copy testfile testdir

clipboard remove "testfile" "testdir"

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile

clipboard copy testfile testdir

clipboard remove ".*file" ".*dir"

item_is_not_in_cb 0 testfile

item_is_not_in_cb 0 testdir/testfile

assert_fails clipboard remove "foo.*"

assert_fails clipboard remove ".*bar"

assert_fails clipboard remove "baz"

assert_fails clipboard remove foo bar baz