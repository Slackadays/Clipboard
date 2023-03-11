#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show clipboard status"

make_files

clipboard copy testfile testdir

content="$(clipboard)"

content_is_shown "$content" "testfile"

content_is_shown "$content" "testdir"

content_is_shown "$content" "0"