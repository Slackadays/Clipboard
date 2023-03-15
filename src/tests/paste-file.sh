#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Paste files"

make_files

clipboard copy testfile testdir

setup_dir pastehere

clipboard paste

item_is_here testfile

item_is_here testdir/testfile

clipboard clear

clipboard paste