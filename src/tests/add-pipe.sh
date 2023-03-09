#!/bin/sh
. ./resources.sh
start_test "Add piped data"

printf "Foobar" | clipboard copy

printf "Foobar" | clipboard add

assert_equals "FoobarFoobar" "$(clipboard)"