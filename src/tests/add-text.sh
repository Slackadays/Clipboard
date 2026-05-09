#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Add text"

cb copy "Foobar"

cb add "Baz"

unset CLIPBOARD_FORCETTY

assert_equals "FoobarBaz" "$(cb paste)"

touch foofile

assert_fails cb add foofile bar baz