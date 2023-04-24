#!/bin/sh
set -eu

find \
    '(' \
        -name '*.cpp' \
        -o -name '*.hpp' \
    ')' \
    -exec \
    clang-format-15 --Werror -i --verbose --dry-run '{}' +
