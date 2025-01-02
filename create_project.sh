#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo 'Usage: ./make_example "ProjectName"'
    exit 1
fi

mkdir Source/"$1"
mkdir Projects/"$1"

cp Source/Test/main.cpp Source/"$1"/main.cpp
sed "s/Test/$1/" Projects/Test/Test.vcxproj > Projects/"$1"/"$1".vcxproj
sed "s/Test/$1/" Projects/Test/Test.vcxproj.filters > Projects/"$1"/"$1".vcxproj.filters
echo "Don't forget to add the created project to the 1gam solution in Visual Studio"
