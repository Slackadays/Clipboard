#compdef cb
local -a actions
actions=(
    "cut:cut something"
    "copy:copy something"
    "paste:paste something"
    "clear:clear clipboard"
    "show:show clipboard content"
    "edit:edit clipboard content"
    "add:add something to clipboard"
    "remove:remove something from clipboard"
    "note:add note to clipboard"
    "swap:swap clipboard content"
    "status:show status"
    "info:show clipboard info"
    "load:load clipboard into other clipboard"
    "import:import clipboard from file"
    "export:export clipboard to file"
    "history:show clipboard history"
    "ignore:ignore content"
    "search:search clipboard content"
    "help:show help for CB"
)
# only put up to one action in front of the cb command
if [ "${#words[@]}" -eq 2 ]; then
    _describe 'command' actions
    return
fi
# now complete files
if [ "${words[2]}" = "cut" ] || [ "${words[2]}" = "ct" ] || [ "${words[2]}" = "copy" ] || [ "${words[2]}" = "cp" ] || [ "${words[2]}" = "add" ] || [ "${words[2]}" = "ad" ]; then
    _files
    return
fi