#setup
echo "Blah!" > dummy.txt
mkdir dummydir
echo "Blah!" > dummydir/foo.txt

#test copying a file and a directory
clipboard copy dummy.txt dummydir
if [ ! -f /tmp/Clipboard/0/dummy.txt ]; then
echo did not copy file
  exit 1
fi
if [ ! -f /tmp/Clipboard/0/dummydir/foo.txt ]; then
  echo did not copy directory
  exit 1
fi

#test pasting a file and a directory
mkdir dummydir2
cd dummydir2
clipboard paste
if [ ! -f dummy.txt ]; then
  echo did not paste file
  exit 1
fi
if [ ! -f dummydir/foo.txt ]; then
  echo did not paste file in directory
  exit 1
fi

#test copying contents piped in
echo "Bleh" | clipboard
if [ ! -f /tmp/Clipboard/0/clipboard.txt ]; then
  echo did not copy contents piped in
  exit 1
fi
clipboard paste > dummy.txt
contents=$(cat dummy.txt)
if [ "$contents" != "Bleh" ]; then
  echo "contents: $contents"
  exit 1
fi

#setup for next tests
cd ..
rm -rf dummydir2

#test copying a file and a directory to clipboard 1
clipboard copy1 dummy.txt dummydir
if [ ! -f /tmp/Clipboard/1/dummy.txt ]; then
  echo did not copy file into cb 1
  exit 1
fi
if [ ! -f /tmp/Clipboard/1/dummydir/foo.txt ]; then
  echo did not copy directory into cb 1
  exit 1
fi

#test pasting a file and a directory from clipboard 1
mkdir dummydir2
cd dummydir2
clipboard paste1
if [ ! -f dummy.txt ]; then
  echo did not paste file from cb 1
  exit 1
fi
if [ ! -f dummydir/foo.txt ]; then
  echo did not paste file in directory from cb 1
  exit 1
fi

#test copying contents piped in to clipboard 1
echo "Bleh" | clipboard copy1
if [ ! -f /tmp/Clipboard/1/clipboard.txt ]; then
  echo did not copy contents piped in to cb 1
  exit 1
fi

echo "Test passed"