#!/bin/sh
. ./resources.sh
start_test "Remove piped data"

echo "Foobar" | cb copy

item_is_in_cb 0 rawdata.clipboard

echo "Baz" | assert_fails cb remove

echo "Foobar" | cb remove

assert_equals "" "$(cb paste)"