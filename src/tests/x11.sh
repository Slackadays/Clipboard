#!/bin/sh
. ./resources.sh
start_test "Test X11 functionality"
export CLIPBOARD_FORCETTY=1
set +u

# Test if we have xclip installed
if ! command -v xclip >/dev/null 2>&1;
then
    echo "⏭️ Skipping X11 tests due to missing xclip"
    exit 0
fi

if [ -n "$CLIPBOARD_NOGUI" ]
then
    echo "⏭️ Skipping X11 tests due to CLIPBOARD_NOGUI"
    exit 0
fi

if [ "$CLIPBOARD_REQUIREX11" = "0" ]
then
    echo "⏭️ Skipping X11 tests due to CLIPBOARD_REQUIREX11=0"
    exit 0
fi

make_files

clipboard copy "Some text"

assert_equals "Some text" "$(xclip -o -selection clipboard)"

unset CLIPBOARD_FORCETTY

clipboard copy < ../TurnYourClipboardUp.png

sleep 5

assert_equals "$(cat ../TurnYourClipboardUp.png)" "$(until xclip -o -selection clipboard; do sleep 1; done)"