#!/bin/sh
set -eu

clear_tempdirs() {
    rm -rf test_*
}

trap clear_tempdirs 0

if [ -z "${CLIPBOARD_TMPDIR:-}" ]
then
    printf "\033[1mEnter CB's temp directory (no slash at end):\033[0m "
    read -r CLIPBOARD_TMPDIR
    export CLIPBOARD_TMPDIR
fi

BASEDIR="$(dirname "$0")"

cd "$BASEDIR" || exit 1

clear_tempdirs

run_one() {
    if ! sh "$1"
    then
        sh "$1"
    fi
}

run_all_tests() {
    printf "\033[1m\033[33mRunning all tests\033[0m\n"
    printf "\033[1m\033[33mColor theme: \033[0m\033[33m%s\033[0m\n" "${CLIPBOARD_THEME:-default}"
    run_one export.sh
    run_one history.sh
    run_one ignore.sh
    run_one add-file.sh
    run_one add-pipe.sh
    run_one add-text.sh
    run_one bad-action.sh
    run_one clear-file.sh
    run_one clear-pipe.sh
    run_one clear-text.sh
    run_one copy-file.sh
    run_one copy-pipe.sh
    run_one copy-text.sh
    run_one cut-file.sh
    run_one cut-pipe.sh
    run_one cut-text.sh
    run_one paste-file.sh
    run_one paste-pipe.sh
    run_one paste-text.sh
    run_one show-file.sh
    run_one show-pipe.sh
    run_one show-text.sh
    run_one remove-file.sh
    run_one remove-pipe.sh
    run_one remove-text.sh
    run_one note-pipe.sh
    run_one note-text.sh
    run_one status.sh
    run_one help.sh
    run_one themes.sh
    run_one languages.sh
    run_one x11.sh
    run_one wayland.sh
    clear_tempdirs
}

run_all_themes() {
    run_all_tests
    export CLIPBOARD_THEME=dark
    run_all_tests
    export CLIPBOARD_THEME=light
    run_all_tests
    export CLIPBOARD_THEME=darkhighcontrast
    run_all_tests
    export CLIPBOARD_THEME=lighthighcontrast
    run_all_tests
    export CLIPBOARD_THEME=amber
    run_all_tests
    export CLIPBOARD_THEME=green
    run_all_tests
    unset CLIPBOARD_THEME
}

run_all_themes

echo "🐢 All tests passed!"