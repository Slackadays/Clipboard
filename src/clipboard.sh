#!/bin/bash
set -e

#get number of arguments
if [ $# -lt 1 ]
then
    if [ $ACTION = "copy" ]
    then
        echo -e "\033[38:5:196m╳\033[91m You need to choose a file to copy.\033[0m"
        exit 1
    fi
    if [ $ACTION = "cut" ]
    then
        echo -e "\033[38:5:196m╳\033[91m You need to choose a file to cut.\033[0m"
        exit 1
    fi
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

filepath="/tmp/Clipboard/files"

if [ ! -d $filepath ]
then
    mkdir -p $filepath
fi

rm -rf $filepath/*

if [ $ACTION = "copy" ]
then
    for f in ${files[@]}
    do
        cp -r ${flags[@]} ${f} $filepath
    done
elif [ $ACTION = "cut" ]
then
    for f in ${files[@]}
    do
        mv ${flags[@]} ${f} $filepath
    done
else
    echo -e "\033[38:5:196m╳\033[91m Invalid action. If you wanted to copy or cut, then this is a bug.\033[0m"
    exit 1
fi


#get the number of regular files in the files array
num_regular_files=0
for f in ${files[@]}
do
    if [ -f $f ]
    then
        num_regular_files=$((num_regular_files+1))
    fi
done

#get the number of directories in the files array
num_directories=0
for f in ${files[@]}
do
    if [ -d $f ]
    then
        num_directories=$((num_directories+1))
    fi
done

num_files_success=0
num_directories_success=0
num_files_failed=0
num_directories_failed=0

if [ ${#files[@]} -gt 1 ]
then
    for f in ${files[@]}
    do
        if [ ! -e $filepath/${f} ]
        then
            #check if is file or directory
            if [ -f $f ]
            then
                num_files_failed=$((num_files_failed+1))
            fi
            if [ -d $f ]
            then
                num_directories_failed=$((num_directories_failed+1))
            fi
        fi
        if [ -e $filepath/${f} ]
        then
            #check if is file or directory
            if [ -f $f ]
            then
                num_files_success=$((num_files_success+1))
            fi
            if [ -d $f ]
            then
                num_directories_success=$((num_directories_success+1))
            fi
        fi
    done

    #echo $num_files_success
    #echo $num_directories_success
    #echo $num_files_failed
    #echo $num_directories_failed

    if [ $num_files_success -gt 1 ] || [ $num_directories_success -gt 1 ]
    then
        if [ $ACTION = "copy" ]
        then
            if [ $num_directories_success -eq 0 ]
            then
                echo -e "\033[38:5:40m✔ Copied ${num_files_success} files\033[0m"
            elif [ $num_files_success -eq 0 ]
            then
                echo -e "\033[38:5:40m✔ Copied ${num_directories_success} directories\033[0m"
            else
                echo -e "\033[38:5:40m✔ Copied ${num_files_success} files and ${num_directories_success} directories\033[0m"
            fi
        fi
        if [ $ACTION = "cut" ]
        then
            if [ $num_directories_success -eq 0 ]
            then
                echo -e "\033[38:5:40m✔ Cut ${num_files_success} files\033[0m"
            elif [ $num_files_success -eq 0 ]
            then
                echo -e "\033[38:5:40m✔ Cut ${num_directories_success} directories\033[0m"
            else
                echo -e "\033[38:5:40m✔ Cut ${num_files_success} files and ${num_directories_success} directories\033[0m"
            fi
        fi
    fi
    if [ $num_files_failed -gt 1 ] || [ $num_directories_failed -gt 1 ]
    then
        if [ $ACTION = "copy" ]
        then
            if [ $num_directories_failed -eq 0 ]
            then
                echo -e "\033[38:5:196m╳\033[91m Failed to copy ${num_files_failed} files\033[0m"
            elif [ $num_files_failed -eq 0 ]
            then
                echo -e "\033[38:5:196m╳\033[91m Failed to copy ${num_directories_failed} directories\033[0m"
            elif [ $num_files_failed -gt 1 ] && [ $num_directories_failed -gt 1 ]
            then
                echo -e "\033[38:5:196m╳\033[91m Failed to copy ${num_files_failed} files and ${num_directories_failed} directories\033[0m"
            fi
        fi
        if [ $ACTION = "cut" ]
        then
            if [ $num_directories_failed -eq 0 ]
            then
                echo -e "\033[38:5:196m╳\033[91m Failed to cut ${num_files_failed} files\033[0m"
            elif [ $num_files_failed -eq 0 ]
            then
                echo -e "\033[38:5:196m╳\033[91m Failed to cut ${num_directories_failed} directories\033[0m"
            elif [ $num_files_failed -gt 1 && $num_directories_failed -gt 1 ]
            then
                echo -e "\033[38:5:196m╳\033[91m Failed to cut ${num_files_failed} files and ${num_directories_failed} directories\033[0m"
            fi
        fi
    fi
    exit 0
fi

for f in ${files[@]}
do
    if [ ! -e $filepath/${f} ]
    then
        if [ $ACTION = "copy" ]
        then
            echo -e "\033[38:5:196m╳\033[91m Failed to copy ${f}\033[0m"
        fi
        if [ $ACTION = "cut" ]
        then
            echo -e "\033[38:5:196m╳\033[91m Failed to cut ${f}\033[0m"
        fi
    fi
    if [ -e $filepath/${f} ]
    then
        if [ $ACTION = "copy" ]
        then
            echo -e "\033[38:5:40m√ Copied ${f}\033[0m"
        fi
        if [ $ACTION = "cut" ]
        then
            echo -e "\033[38:5:40m√ Cut ${f}\033[0m"
        fi
    fi
done
exit 0