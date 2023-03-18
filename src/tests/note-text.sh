#!/bin/sh
. ./resources.sh
start_test "Note user provided text"
export CLIPBOARD_FORCETTY=1

clipboard note "Foobar"

assert_fails clipboard note "Foobar" "Baz"

unset CLIPBOARD_FORCETTY

#skip anything in CI if it isn't Linux
set +u
if [ "$CI" = "true" ] && [ "$(uname)" != "Linux" ]
then
    echo "Skipping test on macOS CI due to Not A TTY bug"
    exit 0
fi

assert_equals "Foobar" "$(clipboard note)"

content_is_shown "$(CLIPBOARD_FORCETTY=1 clipboard note)" "Foobar"

clipboard note ""

assert_equals "" "$(clipboard note)"