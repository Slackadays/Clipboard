#!/bin/sh
. ./resources.sh
start_test "Add piped data"

printf "Foobar" | cb

printf "Foobar" | cb add

assert_equals "FoobarFoobar" "$(cb paste)" # add paste to work around gha bug

cb add foo bar baz