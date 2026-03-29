#!/usr/bin/bash

# Arguments:
# 1: OS

if [ "$1" == "" ]; then
   echo "Missing 1st argumen: Operating system"
   exit
fi

#datetime=$(date '+%Y-%m-%d_%H%M')
#date=$(date '+%Y-%m-%d')
version=$(cat src/program_info.h |grep PROGRAM_VERSION |sed -E 's/^.*PROGRAM_VERSION "(.*)"/\1/g')

sed "s/<VERSION>/$version/g" FILE_ID_Template.DIZ |sed "s/<OS>/${1}/g" |sed "s/<DATE>/$(date '+%Y-%m-%d')/g" >FILE_ID.DIZ

