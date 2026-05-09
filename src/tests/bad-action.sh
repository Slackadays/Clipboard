#!/bin/sh
. ./resources.sh
start_test "Give a junk action"
export CLIPBOARD_FORCETTY=1

assert_fails cb FoobarActionThatDoesNotExist