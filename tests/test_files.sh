#setup
echo "Foobar" > copyme.txt
mkdir dummydir
echo "Foobar" > dummydir/copyme.txt

echo "Foobar" > cutme.txt
mkdir cutdummydir
echo "Foobar" > cutdummydir/cutme.txt

#test copying a file and a directory
clipboard copy copyme.txt dummydir
if [ ! -f "$TMPDIR"/Clipboard/0/copyme.txt ]; then
echo did not copy file
  exit 1
fi
if [ ! -f "$TMPDIR"/Clipboard/0/dummydir/copyme.txt ]; then
  echo did not copy directory
  exit 1
fi

#test pasting a file and a directory
mkdir dummydir2
cd dummydir2
clipboard paste
if [ ! -f copyme.txt ]; then
  echo did not paste file
  exit 1
fi
if [ ! -f dummydir/copyme.txt ]; then
  echo did not paste file in directory
  exit 1
fi

#test contents of the files
contents=$(cat copyme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi
contents=$(cat dummydir/copyme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

#test cutting a file and a directory
cd ..
clipboard cut cutme.txt cutdummydir
if [ ! -f "$TMPDIR"/Clipboard/0/cutme.txt ]; then
  echo did not cut file
  exit 1
fi
if [ ! -f "$TMPDIR"/Clipboard/0/cutdummydir/cutme.txt ]; then
  echo did not cut directory
  exit 1
fi

#test pasting a file and a directory that were cut
cd dummydir2
clipboard paste
if [ ! -f cutme.txt ]; then
  echo did not paste file
  exit 1
fi
if [ ! -f cutdummydir/cutme.txt ]; then
  echo did not paste file in directory
  exit 1
fi

#test contents of the files
contents=$(cat cutme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi
contents=$(cat cutdummydir/cutme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

#setup for next tests
cd ..
rm -rf dummydir2

#test copying a file and a directory to clipboard 1
clipboard copy1 copyme.txt dummydir
if [ ! -f "$TMPDIR"/Clipboard/1/copyme.txt ]; then
  echo did not copy file into cb 1
  exit 1
fi
if [ ! -f "$TMPDIR"/Clipboard/1/dummydir/copyme.txt ]; then
  echo did not copy directory into cb 1
  exit 1
fi

#test pasting a file and a directory from clipboard 1
mkdir dummydir2
cd dummydir2
clipboard paste1
if [ ! -f copyme.txt ]; then
  echo did not paste file from cb 1
  exit 1
fi
if [ ! -f dummydir/copyme.txt ]; then
  echo did not paste file in directory from cb 1
  exit 1
fi

#test contents of the files
contents=$(cat copyme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi
contents=$(cat dummydir/copyme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

#setup for more cut tests
cd ..
echo "Foobar" > cutme.txt
mkdir cutdummydir
echo "Foobar" > cutdummydir/cutme.txt

#test cutting a file and a directory to clipboard 1
clipboard cut1 cutme.txt cutdummydir
if [ ! -f "$TMPDIR"/Clipboard/1/cutme.txt ]; then
  echo did not cut file into cb 1
  exit 1
fi
if [ ! -f "$TMPDIR"/Clipboard/1/cutdummydir/cutme.txt ]; then
  echo did not cut directory into cb 1
  exit 1
fi

#test pasting a file and a directory that were cut from clipboard 1
cd dummydir2
clipboard paste1
if [ ! -f cutme.txt ]; then
  echo did not paste file from cb 1
  exit 1
fi
if [ ! -f cutdummydir/cutme.txt ]; then
  echo did not paste file in directory from cb 1
  exit 1
fi

#test contents of the files
contents=$(cat cutme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi
contents=$(cat cutdummydir/cutme.txt)
if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

echo "Test passed"