#!/bin/sh
. ./resources.sh
setup_dir paste-text

clipboard copy "Foobar"

clipboard paste

item_is_here rawdata.clipboard

pass_test paste-text