#setup
set -eu
export CLIPBOARD_FORCETTY=1

mkdir copy_test
cd copy_test

echo "Foobar" > copyme.txt

mkdir copydir
echo "Foobar" > copydir/copyme.txt

#test copying a file and a directory
clipboard copy copyme.txt copydir

if [ ! -f "$TMPDIR"/Clipboard/0/data/copyme.txt ]; then
echo did not copy file
  exit 1
fi

if [ ! -f "$TMPDIR"/Clipboard/0/data/copydir/copyme.txt ]; then
  echo did not copy directory
  exit 1
fi

#test pasting a file and a directory
mkdir dummydir
cd dummydir
clipboard paste

if [ ! -f copyme.txt ]; then
  echo did not paste file
  exit 1
fi

if [ ! -f copydir/copyme.txt ]; then
  echo did not paste file in directory
  exit 1
fi

#test contents of the files
contents=$(cat copyme.txt)

if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

contents=$(cat copydir/copyme.txt)

if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

#setup for more copy tests
mkdir dummydir
cd dummydir

echo "Foobar" > copyme.txt

mkdir copydir
echo "Foobar" > copydir/copyme.txt

#test copying a file and a directory to clipboard 1
clipboard copy1 copyme.txt copydir

if [ ! -f "$TMPDIR"/Clipboard/1/data/copyme.txt ]; then
  echo did not copy file into cb 1
  exit 1
fi

if [ ! -f "$TMPDIR"/Clipboard/1/data/copydir/copyme.txt ]; then
  echo did not copy directory into cb 1
  exit 1
fi

#test pasting a file and a directory from clipboard 1
mkdir dummydir
cd dummydir
clipboard paste1

if [ ! -f copyme.txt ]; then
  echo did not paste file from cb 1
  exit 1
fi

if [ ! -f copydir/copyme.txt ]; then
  echo did not paste file in directory from cb 1
  exit 1
fi

#test contents of the files
contents=$(cat copyme.txt)

if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

contents=$(cat copydir/copyme.txt)

if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

#setup to repeat copy tests, but with "--copy" instead of "copy"
clipboard --clear

mkdir dummydir
cd dummydir

echo "Foobar" > copyme.txt

mkdir copydir
echo "Foobar" > copydir/copyme.txt

#test copying a file and a directory with "--copy"
clipboard --copy copyme.txt copydir

if [ ! -f "$TMPDIR"/Clipboard/0/data/copyme.txt ]; then
  echo did not copy file
  exit 1
fi

if [ ! -f "$TMPDIR"/Clipboard/0/data/copydir/copyme.txt ]; then
  echo did not copy directory
  exit 1
fi

#test pasting a file and a directory with "--paste"
mkdir dummydir
cd dummydir

clipboard --paste

if [ ! -f copyme.txt ]; then
  echo did not paste file
  exit 1
fi

if [ ! -f copydir/copyme.txt ]; then
  echo did not paste file in directory
  exit 1
fi

#test contents of the files
contents=$(cat copyme.txt)

if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

contents=$(cat copydir/copyme.txt)

if [ "$contents" != "Foobar" ]; then
  echo "contents: $contents"
  exit 1
fi

echo "Copy test passed"