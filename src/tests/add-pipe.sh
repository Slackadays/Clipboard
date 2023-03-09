#!/bin/sh
. ./resources.sh
start_test "Add piped data"

echo "Foobar" | clipboard

echo "Foobar" | clipboard add

assert_equals "Foobar
Foobar" "$(clipboard)"