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

cb copy "Some text"

assert_equals "Some text" "$(xclip -o -selection clipboard)"

unset CLIPBOARD_FORCETTY

cb copy < ../TurnYourClipboardUp.png

sleep 6

assert_equals "$(cat ../TurnYourClipboardUp.png)" "$(until xclip -o -selection clipboard; do sleep 1; done)"

cb copy < ../"Exosphere 2.0.mp3"

sleep 6

assert_equals "$(cat ../"Exosphere 2.0.mp3")" "$(until xclip -o -selection clipboard; do sleep 1; done)"

xclip -selection clipboard -t image/png < ../TurnYourClipboardUp.png

sleep 6

assert_equals "$(cat ../TurnYourClipboardUp.png)" "$(cb paste)"

xclip -selection clipboard -t audio/mpeg < ../"Exosphere 2.0.mp3"

sleep 6

assert_equals "$(cat ../"Exosphere 2.0.mp3")" "$(cb paste)"