#!/bin/sh
. ./resources.sh
start_test "Note piped in text"

echo "Foobar" | clipboard note

assert_equals "Foobar" "$(clipboard note)"