#!/bin/sh
. ./resources.sh
setup_dir cut-file

make_files

clipboard cut testfile testdir

setup_dir pastehere

clipboard paste

cd ..

item_is_not_here testfile

item_is_not_here testdir/testfile

pass_test cut-file