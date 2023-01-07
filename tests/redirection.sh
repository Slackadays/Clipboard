#test copying contents piped in
mkdir redir_test

cd redir_test

echo "Bleh" | clipboard
if [ ! -f "$TMPDIR"/Clipboard/0/clipboard.rawdata ]; then
  echo did not copy contents piped in
  exit 1
fi

clipboard paste > dummy.txt

contents=$(cat dummy.txt)

if [ "$contents" != "Bleh" ]; then
  echo "contents: $contents"
  exit 1
fi

#test copying contents piped in to clipboard 1
echo "Bleh" | clipboard copy1

if [ ! -f "$TMPDIR"/Clipboard/1/clipboard.rawdata ]; then
  echo did not copy contents piped in to cb 1
  exit 1
fi

echo "Redir test passed"