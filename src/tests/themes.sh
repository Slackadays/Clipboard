#!/bin/sh
. ./resources.sh
start_test "Show CB themes"
export CLIPBOARD_FORCETTY=1

make_files

CLIPBOARD_THEME=dark cb copy testfile testdir

CLIPBOARD_THEME=light cb copy testfile testdir

CLIPBOARD_THEME=amber cb copy testfile testdir

CLIPBOARD_THEME=green cb copy testfile testdir

CLIPBOARD_THEME=lighthighcontrast cb copy testfile testdir

CLIPBOARD_THEME=darkhighcontrast cb copy testfile testdir

CLIPBOARD_THEME=ansi cb copy testfile testdir

NO_COLOR=1 cb copy testfile testdir

NO_COLOR=1 FORCE_COLOR=1 cb copy testfile testdir