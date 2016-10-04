#!/bin/bash

mkdir dir1
mkdir dir2

sudo mount --bind ./dir1 ./dir2
sudo mount --bind ./dir2 ./dir1

#Test 1 -- OK!
echo "Hi there dir2!" > ./dir1/testfile1
cat ./dir2/testfile1
echo "Hi there dir1!" > ./dir2/testfile2
cat ./dir1/testfile2

rm ./dir1/testfile1
rm ./dir2/testfile2

#Test 2 -- OK!
echo "I wrote from dir1!" >> ./dir1/testfile
echo "i wrote from dir2!" >> ./dir2/testfile
cat ./dir1/testfile
cat ./dir2/testfile

rm -f ./dir1/testfile
rm -f ./dir2/testfile

sudo umount ./dir1
sudo umount ./dir2

rm -r ./dir1
rm -r ./dir2
