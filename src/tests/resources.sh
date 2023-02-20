#!/bin/sh
set -eu

make_files() {
  echo "Foobar" > testfile
  mkdir testdir
  echo "Foobar" > testdir/testfile
}

verify_contents() {
    contents=$(cat "$1")
    if [ "$contents" != "Foobar" ]
    then
        echo "❌ The contents don't match: $contents"
        exit 1
    fi
}

item_is_in_cb() {
  if [ -f "$CLIPBOARD_TMPDIR"/Clipboard/"$1"/data/"$2" ]
  then
    verify_contents "$CLIPBOARD_TMPDIR"/Clipboard/"$1"/data/"$2"
    return 0
  else
    echo "❌ The file $2 doesn't exist in clipboard $1"
    exit 1
  fi
}

item_is_here() {
    if [ -f "$1" ]
    then
verify_contents "$1"
        return 0
    else
        echo "❌ The file $1 doesn't exist here"
        exit 1
    fi
}

item_is_not_here() {
    if [ -f "$1" ]
    then
        echo "❌ The file $1 exists here"
        exit 1
    else
        return 0
    fi
}

item_is_not_in_cb() {
  if [ -f "$CLIPBOARD_TMPDIR/Clipboard/$1/data/$2" ]
  then
    echo "❌ The file $2 exists in clipboard $1"
    exit 1
  else
    return 0
  fi
}

setup_dir() {
    mkdir test_"$1"
    cd test_"$1"
}

pass_test() {
  echo "🧪 The $1 test passed"
}