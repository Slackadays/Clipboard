#!/bin/sh
. ./resources.sh
export CLIPBOARD_FORCETTY=1
start_test "Show CB languages"

make_files

CLIPBOARD_LOCALE=en_US.UTF-8 cb copy testfile testdir

CLIPBOARD_LOCALE=es_ES.UTF-8 cb copiar testfile testdir

CLIPBOARD_LOCALE=pt_BR.UTF-8 cb copiar testfile testdir

CLIPBOARD_LOCALE=tr_TR.UTF-8 cb kopyala testfile testdir