#setup
mkdir fandr_test

cd fandr_test

echo "Foobar" > copyme.txt

echo "Foobar" > cutme.txt

#test copying a file
CLIPBOARD_FORCETTY=1 clipboard copy copyme.txt
if [ ! -f "$TMPDIR"/Clipboard/0/data/copyme.txt ]; then
  echo did not copy file
  exit 1
fi

#test piping out contents of the copied file
if [ "$(clipboard paste)" != "Foobar" ]; then
  echo "contents: $(clipboard paste)"
  exit 1
fi

#test cutting a file
CLIPBOARD_FORCETTY=1 clipboard cut cutme.txt
if [ ! -f "$TMPDIR"/Clipboard/0/data/cutme.txt ]; then
  echo did not cut file
  exit 1
fi

#test piping out contents of the cut file
if [ "$(clipboard paste)" != "Foobar" ]; then
  echo "contents: $(clipboard paste)"
  exit 1
fi

echo "FandR test passed"