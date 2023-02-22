#!/bin/sh
set -eux

BASEDIR="$(dirname "$0")"

cd "$BASEDIR" || exit 1

rm -rf test_*

sh copy-file.sh
sh copy-pipe.sh
sh copy-text.sh
sh cut-file.sh
sh cut-pipe.sh
sh cut-text.sh
sh paste-file.sh
sh paste-pipe.sh
sh paste-text.sh

rm -r test_*

echo "🐢 All tests passed!"