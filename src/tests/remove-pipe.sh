#!/bin/sh
. ./resources.sh
start_test "Remove piped data"

echo "Foobar" | clipboard copy

item_is_in_cb 0 rawdata.clipboard

echo "Baz" | assert_fails clipboard remove

echo "Foobar" | clipboard remove

assert_equals "" "$(clipboard paste)"