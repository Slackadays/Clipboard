#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show help message"

output="$(clipboard help)"

content_is_shown "$output" "Clipboard"

output="$(clipboard -h)"

content_is_shown "$output" "Clipboard"

output="$(clipboard --help)"

content_is_shown "$output" "Clipboard"

clipboard --ee