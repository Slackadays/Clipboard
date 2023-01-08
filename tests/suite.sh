#!/bin/sh

BASEDIR="$(dirname "$0")"

sh "$BASEDIR/copy.sh"
sh "$BASEDIR/cut.sh"
sh "$BASEDIR/files_and_redirection.sh"
sh "$BASEDIR/redirection.sh"

rm -r copy_test
rm -r cut_test
rm -r fandr_test
rm -r redir_test