#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Paste user provided text"

cb copy "Foobar"

cb paste

item_exists clipboard0-0.txt