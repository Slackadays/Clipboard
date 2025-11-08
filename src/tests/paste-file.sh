#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Paste files"

make_files

cb copy testfile testdir

setup_dir pastehere

cb paste

item_exists testfile

item_exists testdir/testfile

cb clear

cb paste