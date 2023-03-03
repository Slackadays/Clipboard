#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1

testname="Paste files"

start_test

setup_dir paste-file

make_files

clipboard copy testfile testdir

setup_dir pastehere

clipboard paste

item_is_here testfile

item_is_here testdir/testfile

pass_test