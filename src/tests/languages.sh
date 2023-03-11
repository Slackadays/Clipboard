#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show Clipboard languages"

make_files

CLIPBOARD_LOCALE=en_US.UTF-8 clipboard copy testfile testdir

CLIPBOARD_LOCALE=es_ES.UTF-8 clipboard copiar testfile testdir

CLIPBOARD_LOCALE=pt_BR.UTF-8 clipboard copiar testfile testdir

CLIPBOARD_LOCALE=tr_TR.UTF-8 clipboard kopyala testfile testdir