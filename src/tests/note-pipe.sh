#!/bin/sh
. ./resources.sh
start_test "Note piped in text"

echo "Foobar" | clipboard note

# check CI is true and the runner is macOS
set +u
if [ "$CI" = "true" ] && [ "$(uname)" != "Linux" ]
then
    echo "Skipping test on macOS CI due to Not A TTY bug"
    exit 0
fi

assert_equals "Foobar" "$(clipboard note)"