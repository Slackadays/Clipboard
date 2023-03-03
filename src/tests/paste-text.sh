#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1

testname="Paste user provided text"

start_test

setup_dir paste-text

clipboard copy "Foobar"

clipboard paste

item_is_here rawdata.clipboard

pass_test