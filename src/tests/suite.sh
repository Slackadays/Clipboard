#!/bin/sh
set -eu

trap "rm -r test_*" 0

if [ -z "${CLIPBOARD_TMPDIR:-}" ]
then
    printf "\033[1mEnter Clipboard's temp directory (no slash at end):\033[0m "
    read -r CLIPBOARD_TMPDIR
    export CLIPBOARD_TMPDIR
fi

BASEDIR="$(dirname "$0")"

cd "$BASEDIR" || exit 1

rm -rf test_*

sh add-file.sh
sh add-pipe.sh
sh add-text.sh
sh clear-file.sh
sh clear-pipe.sh
sh clear-text.sh
sh copy-file.sh
sh copy-pipe.sh
sh copy-text.sh
sh cut-file.sh
sh cut-pipe.sh
sh cut-text.sh
sh paste-file.sh
sh paste-pipe.sh
sh paste-text.sh
sh show-file.sh
sh show-pipe.sh
sh show-text.sh

echo "🐢 All tests passed!"