#setup
echo "Foobar" > copyme.txt

echo "Foobar" > cutme.txt

#test copying a file
clipboard copy copyme.txt
if [ ! -f "$TMPDIR"/Clipboard/0/copyme.txt ]; then
echo did not copy file
  exit 1
fi

#test piping out contents of the copied file
if [ "$(clipboard paste)" != "Foobar" ]; then
  echo "contents: $(clipboard paste)"
  exit 1
fi

#test cutting a file
clipboard cut cutme.txt
if [ ! -f "$TMPDIR"/Clipboard/0/cutme.txt ]; then
  echo did not cut file
  exit 1
fi

#test piping out contents of the cut file
if [ "$(clipboard paste)" != "Foobar" ]; then
  echo "contents: $(clipboard paste)"
  exit 1
fi

echo "Test passed"