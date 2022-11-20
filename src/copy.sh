#!/bin/bash
set -e

#get number of arguments
if [ $# -lt 1 ]; then
    echo -e "\033[91m× You need to choose a file to copy.\033[0m"
    exit 1
fi

for f in "$@"
do
    first=`echo $f | cut -c1-1`
    if [ $first = '-' ]
    then
        flags+=($f)
    else
        files+=($f)
    fi
done

#echo ${flags[@]}
#echo ${files[@]}

if [ ! -d "/tmp/copy" ]
then
    mkdir /tmp/copy
fi

for f in ${files[@]}
do
    cp ${flags[@]} ${f} /tmp/copy/
done

# get the number of elements in files
num_files=${#files[@]}

if [ $num_files -gt 1 ]
then
    num_success=0
    num_failed=0
    for f in ${files[@]}
    do
        if [ ! -e /tmp/copy/${f} ]
        then
           num_failed=$((num_failed+1))
        fi
        if [ -e /tmp/copy/${f} ]
        then
            num_success=$((num_success+1))
        fi
    done
    if [ $num_success -gt 0 ]
    then
        echo -e "\033[92m√ Copied ${num_success} files\033[0m"
    fi
    if [ $num_failed -gt 0 ]
    then
        echo -e "\033[91m× Failed to copy ${num_failed} files\033[0m"
    fi
    exit 0
fi

for f in ${files[@]}
do
    if [ ! -e /tmp/copy/${f} ]
    then
        echo -e "\033[91m✖ Couldn't copy ${f}\033[0m"
    fi
    if [ -e /tmp/copy/${f} ]
    then
        echo -e "\033[92m√ Copied ${f}\033[0m"
    fi
done
exit 0