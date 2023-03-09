#!/bin/bash
. ./resources.sh
start_test "Add piped data"

printf "Foobar" | clipboard

printf "Foobar" | clipboard add

assert_equals "FoobarFoobar" "$(clipboard paste)" # add paste to work around gha bug