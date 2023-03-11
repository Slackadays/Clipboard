#!/bin/sh
. ./resources.sh
start_test "Note user provided text"
export CLIPBOARD_FORCETTY=1

clipboard note "Foobar"

unset CLIPBOARD_FORCETTY

#skip macOS and Windows GHA
if [ "$CI" = "true" ] && [ "$RUNNER_OS" = "macOS" ] || [ "$RUNNER_OS" = "Windows" ];
then
    echo "Skipping test on macOS CI due to Not A TTY bug"
    exit 0
fi

assert_equals "Foobar" "$(clipboard note)"