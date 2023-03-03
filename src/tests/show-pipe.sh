#!/bin/sh
. ./resources.sh

start_test show-pipe

setup_dir show-pipe

echo "Foobar" | clipboard

export CLIPBOARD_FORCETTY=1

output=$(clipboard show)

item_is_in_cb 0 rawdata.clipboard

content_is_shown "$output" "Foobar"

pass_test show-pipe