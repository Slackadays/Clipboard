#!/bin/sh
export CLIPBOARD_FORCETTY=1
. ./resources.sh
setup_dir copy-file

make_files

clipboard copy testfile testdir

item_is_in_cb 0 testfile

item_is_in_cb 0 testdir/testfile

pass_test copy-file