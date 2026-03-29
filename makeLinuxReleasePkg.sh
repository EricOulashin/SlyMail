#!/usr/bin/bash

cd docs
./generate_docs.sh
cd ..

rm -rf SlyMail_Linux
mkdir -p SlyMail_Linux/config_files
mkdir -p SlyMail_Linux/dictionary_files
mkdir -p SlyMail_Linux/tagline_files
mkdir -p SlyMail_Linux/docs
cp config_files/* SlyMail_Linux/config_files
cp dictionary_files/* SlyMail_Linux/dictionary_files
cp tagline_files/* SlyMail_Linux/tagline_files
cp slymail.ini SlyMail_Linux
#cp README.md SlyMail_Linux
#cp CHANGELOG.md SlyMail_Linux
cp slymail SlyMail_Linux
cp config SlyMail_Linux
cd SlyMail_Linux/docs
cp ../../docs/SlyMail_User_Manual.pdf .
cp -rf ../../docs/html .
cd ../..


# Create the FILE_ID.DIZ for the release package
version=$(cat src/program_info.h |grep PROGRAM_VERSION |sed -E 's/^.*PROGRAM_VERSION "(.*)"/\1/g')
sed "s/<VERSION>/$version/g" FILE_ID_Template.DIZ |sed "s/<OS>/Linux/g" |sed "s/<DATE>/$(date '+%Y-%m-%d')/g" >FILE_ID.DIZ

# Make the zip file
zip -r -9 "SlyMail_${version}_Linux.zip" FILE_ID.DIZ SlyMail_Linux
rm -rf SlyMail_Linux
rm FILE_ID.DIZ
