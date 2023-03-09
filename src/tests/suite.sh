#!/bin/bash
set -u

clear_tempdirs() {
    rm -rf test_*
}

trap clear_tempdirs 0

if [ -z "${CLIPBOARD_TMPDIR:-}" ]
then
    printf "\033[1mEnter Clipboard's temp directory (no slash at end):\033[0m "
    read -r CLIPBOARD_TMPDIR
    export CLIPBOARD_TMPDIR
fi

BASEDIR="$(dirname "$0")"

cd "$BASEDIR" || exit 1

clear_tempdirs

#locales=(en_US.UTF-8 es_ES.UTF-8 pt_BR.UTF-8 tr_TR.UTF-8)
themes=(dark light darkhighcontrast lighthighcontrast amber green)

run_tests() {
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
}

#for locale in "${locales[@]}"
#do 
#    export LC_ALL="$locale"
    for theme in "${themes[@]}"
    do
        export CLIPBOARD_THEME="$theme"
        run_tests
        clear_tempdirs
    done
#done

echo "🐢 All tests passed!"