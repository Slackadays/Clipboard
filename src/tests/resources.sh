#!/bin/sh
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
  else
    printf "💥 \033[1m%s\033[0m failed\n" "$testname"
  fi
}

start_test() {
  export testname="$1"
  cols=$(tput cols 2>/dev/null || echo 80)
  i=0
  while [ $i -lt "$cols" ]; do printf "━"; i=$((i+1)); done
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
        truncated1=$(printf "%s" "$1" | head -c 64)
        truncated2=$(printf "%s" "$2" | head -c 64)
        fail "😕 $truncated1 doesn't equal $truncated2"
    fi
}

assert_fails() {
    echo "⏩ The following should fail:"
    if "$@"
    then
        fail "😕 This command didn't fail"
    else
        return 0
    fi
}

verify_contents() {
    contents=$(cat "$1")
    baseline="$2"
    if [ "$contents" != "$baseline" ]
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

get_current_entry_name() {
  # Get the highest numbered entry folder, where 10 is the entry name in /tmp/Clipboard/0/data/10
  ls "$CLIPBOARD_TMPDIR"/Clipboard/"$1"/data/ | sort -n | tail -n 1
}

item_is_in_cb() {
  clipboard="$1"
  item="$2"
  entry="$(get_current_entry_name "$clipboard")"
  if [ -f "$CLIPBOARD_TMPDIR"/Clipboard/"$clipboard"/data/"$entry"/"$item" ]
  then
    verify_contents "$CLIPBOARD_TMPDIR"/Clipboard/"$clipboard"/data/"$entry"/"$item" "Foobar"
    return 0
  elif [ -d "$CLIPBOARD_TMPDIR"/Clipboard/"$clipboard"/data/"$entry"/"$item" ]
  then
    return 0
  else
    fail "😕 The item $item doesn't exist in clipboard $clipboard"
  fi
}

item_exists() {
  item="$1"
  content="${2:-Foobar}"
  if [ -f "$item" ] || [ -d "$item" ]
  then
      verify_contents "$item" "$content"
      return 0
  else
      fail "😕 The item $item doesn't exist here"
  fi
}

item_is_not_here() {
  item="$1"
  if [ -f "$item" ]
  then
      fail "😕 The item $item exists here"
  else
      return 0
  fi
}

item_is_not_in_cb() {
  clipboard="$1"
  item="$2"
  entry="$(get_current_entry_name "$clipboard")"
  if [ -f "$CLIPBOARD_TMPDIR"/Clipboard/"$clipboard"/data/"$entry"/"$item" ]
  then
    fail "😕 The item $item exists in clipboard $clipboard"
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