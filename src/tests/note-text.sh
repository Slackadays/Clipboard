#!/bin/sh
. ./resources.sh
start_test "Note user provided text"
export CLIPBOARD_FORCETTY=1

clipboard note "Foobar"

unset CLIPBOARD_FORCETTY

assert_equals "Foobar" "$(clipboard note)"