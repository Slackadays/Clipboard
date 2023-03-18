#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Add text"

cb copy "Foobar"

cb add "Baz"

unset CLIPBOARD_FORCETTY

assert_equals "FoobarBaz" "$(cb paste)"

assert_fails cb add foo bar baz