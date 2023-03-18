#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show file clipboard content"

make_files

cb copy testfile testdir

output="$(cb show)"

content_is_shown "$output" testfile

content_is_shown "$output" testdir