#!/bin/sh
. ./resources.sh
start_test "Add piped data"

echo -n "Foobar" | clipboard

echo -n "Foobar" | clipboard add

assert_equals "FoobarFoobar" "$(clipboard)"