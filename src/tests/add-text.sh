#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Add text"

clipboard copy "Foobar"

clipboard add "Baz"

unset CLIPBOARD_FORCETTY

assert_equals "FoobarBaz" "$(clipboard)"