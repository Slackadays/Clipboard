#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
export CLIPBOARD_NOGUI=1 # temporary GHA workaround for X11
start_test "Show clipboard status"

make_files

clipboard copy testfile testdir

content="$(clipboard)"

echo "$content"

echo "$(clipboard show)"

content_is_shown "$content" "testfile"

content_is_shown "$content" "testdir"

content_is_shown "$content" "0"