#!/bin/sh
set -eu

make_files() {
  echo "Foobar" > testfile
  mkdir testdir
  echo "Foobar" > testdir/testfile
}

start_test() {
  printf "⏩ Starting test \033[1m%s\033[0m\n" "$1"
}

fail() {
  printf "%s\n" "$1"
  exit 1
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
    if diff -c --text "$1" "$2"
    then
        return 0
    else
        fail "😕 The files don't match"
    fi
}

setup_dir() {
    mkdir test_"$1"
    cd test_"$1"
}

pass_test() {
  printf "🎉 The test \033[1m%s\033[0m passed\n" "$1"
}