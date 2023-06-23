#/usr/bin/env bash
complete_cb() {
    if [ "${#COMP_WORDS[@]}" == "2" ]; then
        COMPREPLY=($(compgen -W "cut copy paste clear show edit add remove note swap status info load import export history ignore search help" ${COMP_WORDS[1]}))
        return
    fi
    if [ "${COMP_WORDS[1]}" == "cut" ] || [ "${COMP_WORDS[1]}" == "ct" ] || [ "${COMP_WORDS[1]}" == "copy" ] || [ "${COMP_WORDS[1]}" == "cp" ] || [ "${COMP_WORDS[1]}" == "add" ] || [ "${COMP_WORDS[1]}" == "ad" ]; then
        COMPREPLY=($(compgen -A file ${COMP_WORDS[COMP_CWORD]}))
        return
    fi
}

complete -F complete_cb cb