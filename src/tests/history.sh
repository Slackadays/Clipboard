#!/bin/sh
. ./resources.sh
start_test "Show history"

export CLIPBOARD_FORCETTY=1

cb copy "Some text 1"

cb copy "Some text 2"

cb copy "Some text 3"

cb copy "Some text 4"

cb copy "Some text 5"

if [ "$(uname)" = "Darwin" ] || [ "$(uname)" = "FreeBSD" ] || [ "$(uname)" = "OpenBSD" ] || [ "$(uname)" = "NetBSD" ]
then
    echo "⏭️ Skipping test on BSD platform due to AIO redirection bug"
    exit 0
fi

message="$(cb history 2>&1)"

content_is_shown "$message" "Some text 1"

content_is_shown "$message" "Some text 2"

content_is_shown "$message" "Some text 3"

content_is_shown "$message" "Some text 4"

content_is_shown "$message" "Some text 5"

unset CLIPBOARD_FORCETTY

json="$(cb history 2>&1)"

content_is_shown "$json" '"content": "Some text 1"'

content_is_shown "$json" '"content": "Some text 2"'

content_is_shown "$json" '"content": "Some text 3"'

content_is_shown "$json" '"content": "Some text 4"'

content_is_shown "$json" '"content": "Some text 5"'