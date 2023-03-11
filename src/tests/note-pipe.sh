#!/bin/sh
. ./resources.sh
start_test "Note piped in text"

echo "Foobar" | clipboard note

# check CI is true and the runner is macOS
if [ "$CI" = "true" ] && [ "$RUNNER_OS" = "macOS" ] || [ "$RUNNER_OS" = "Windows" ];
then
    pass_test "Skipping test on macOS CI due to Not A TTY bug"
fi

assert_equals "Foobar" "$(clipboard note)"