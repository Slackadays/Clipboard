#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Add files"

make_files

echo "Foobar" > dummyfile

clipboard copy dummyfile

clipboard add testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

echo "Foobar" > addedfile

clipboard add addedfile

item_is_in_cb 0 addedfile

mkdir addeddir

echo "Foobar" > addeddir/addedfile

clipboard add addeddir

item_is_in_cb 0 addeddir/addedfile

assert_fails clipboard add foo bar baz

assert_fails clipboard add "Some text"

echo "Some text" | assert_fails clipboard add