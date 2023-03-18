#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Add files"

make_files

echo "Foobar" > dummyfile

cb copy dummyfile

cb add testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

echo "Foobar" > addedfile

cb add addedfile

item_is_in_cb 0 addedfile

mkdir addeddir

echo "Foobar" > addeddir/addedfile

cb add addeddir

item_is_in_cb 0 addeddir/addedfile

assert_fails cb add foo bar baz

assert_fails cb add "Some text"

echo "Some text" | assert_fails cb add