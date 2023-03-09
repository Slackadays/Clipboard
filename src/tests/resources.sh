#!/bin/bash
set -eu

testname="(Unknown)"

make_files() {
  echo "Foobar" > testfile
  mkdir testdir
  echo "Foobar" > testdir/testfile
}

setup_dir() {
    mkdir test_"$1"
    cd test_"$1"
}

pass_test() {
  if [ "$?" -eq "0" ]
  then
    printf "🎉 \033[1m%s\033[0m passed\n" "$testname"
  fi
}

start_test() {
  export testname="$1"
  printf "🏁 Starting \033[1m%s\033[0m (file \033[1m%s\033[0m)\n" "$testname" "$0"
  setup_dir "${0%.sh}"
  trap pass_test 0
}

fail() {
  printf "%s\n" "$1"
  exit 1
}

assert_equals() {
    if [ "$1" = "$2" ]
    then
        return 0
    else
        fail "😕 $1 doesn't equal $2"
    fi
}

verify_contents() {
    contents=$(cat "$1")
    if [ "$contents" != "Foobar" ]
    then
        fail "😕 The contents don't match: $contents"
    fi
}

content_is_shown() {
    if printf "%s" "$1" | grep -q "$2"
    then
        return 0
    else
        fail "😕 The content $2 isn't shown"
    fi
}

item_is_in_cb() {
  if [ -f "$CLIPBOARD_TMPDIR"/Clipboard/"$1"/data/"$2" ]
  then
    verify_contents "$CLIPBOARD_TMPDIR"/Clipboard/"$1"/data/"$2"
    return 0
  else
    fail "😕 The file $2 doesn't exist in clipboard $1"
  fi
}

item_is_here() {
    if [ -f "$1" ]
    then
        verify_contents "$1"
        return 0
    else
        fail "😕 The file $1 doesn't exist here"
    fi
}

item_is_not_here() {
    if [ -f "$1" ]
    then
        fail "😕 The file $1 exists here"
    else
        return 0
    fi
}

item_is_not_in_cb() {
  if [ -f "$CLIPBOARD_TMPDIR/Clipboard/$1/data/$2" ]
  then
    fail "😕 The file $2 exists in clipboard $1"
  else
    return 0
  fi
}

items_match() {
    #use diff to compare the contents of the files
    if diff "$1" "$2"
    then
        return 0
    else
        fail "😕 The files don't match"
    fi
}