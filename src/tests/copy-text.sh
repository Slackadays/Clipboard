#!/bin/sh
. ./resources.sh
setup_dir copy-text

clipboard copy "Foobar"

item_is_in_cb 0 rawdata.clipboard

pass_test copy-text