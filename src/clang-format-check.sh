#!/bin/sh
set -eu

find \
    '(' \
        -name '*.cpp' \
        -o -name '*.hpp' \
    ')' \
    -exec \
    clang-format --Werror -i --verbose --dry-run '{}' +
