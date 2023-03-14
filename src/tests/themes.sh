#!/bin/sh
. ./resources.sh
start_test "Show Clipboard themes"
export CLIPBOARD_FORCETTY=1

make_files

CLIPBOARD_THEME=dark clipboard copy testfile testdir

CLIPBOARD_THEME=light clipboard copy testfile testdir

CLIPBOARD_THEME=amber clipboard copy testfile testdir

CLIPBOARD_THEME=green clipboard copy testfile testdir

CLIPBOARD_THEME=lighthighcontrast clipboard copy testfile testdir

CLIPBOARD_THEME=darkhighcontrast clipboard copy testfile testdir

NO_COLOR=1 clipboard copy testfile testdir

NO_COLOR=1 FORCE_COLOR=1 clipboard copy testfile testdir