#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show help message"

output="$(cb help)"

content_is_shown "$output" "cb"

output="$(cb -h)"

content_is_shown "$output" "cb"

output="$(cb --help)"

content_is_shown "$output" "cb"

cb --bachata