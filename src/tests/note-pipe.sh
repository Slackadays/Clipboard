#!/bin/sh
. ./resources.sh
start_test "Note piped in text"

echo "Foobar" | cb note

# check CI is true and the runner is macOS
set +u
if [ "$CI" = "true" ] && [ "$(uname)" != "Linux" ]
then
    echo "⏭️ Skipping test on macOS CI due to Not A TTY bug"
    exit 0
fi

assert_equals "Foobar" "$(cb note)"

echo "" | cb note

assert_equals "" "$(cb note)"