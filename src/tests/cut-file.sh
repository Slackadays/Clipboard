#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Cut files"

make_files

cb cut testfile testdir

setup_dir pastehere

cb paste

cd ../

item_is_not_here testfile

item_is_not_here testdir/testfile