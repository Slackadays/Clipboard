#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show clipboard status"

make_files

cb copy testfile testdir

# check CI is true and the runner is not Linux
set +u
if [ "$CI" = "true" ] && [ "$(uname)" != "Linux" ]
then
    echo "⏭️ Skipping test on this platform CI due to Not A TTY bug"
    exit 0
fi

content="$(cb 2>&1)"

content_is_shown "$content" "testfile"

content_is_shown "$content" "testdir"

content_is_shown "$content" "0"

content="$(cb status 2>&1)"

content_is_shown "$content" "testfile"

content_is_shown "$content" "testdir"

content_is_shown "$content" "0"