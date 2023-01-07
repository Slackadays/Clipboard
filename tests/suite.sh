#!/bin/sh

sh tests/copy.sh
sh tests/cut.sh
sh tests/files_and_redirection.sh
sh tests/redirection.sh

rm -r copy_test
rm -r cut_test
rm -r fandr_test
rm -r redir_test