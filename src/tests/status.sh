#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show clipboard status"

make_files

clipboard copy testfile testdir

# check CI is true and the runner is Windows
set +u
if [ "$CI" = "true" ] && [ "$RUNNER_OS" = "Windows" ]
then
    echo "Skipping test on Windows CI due to Not A TTY bug"
    exit 0
fi

content="$(clipboard)"

content_is_shown "$content" "testfile"

content_is_shown "$content" "testdir"

content_is_shown "$content" "0"