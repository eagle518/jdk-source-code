#! /bin/sh

rm -rf scodebasedir
mkdir scodebasedir
cd scodebasedir/
cp ../codebasedir/*jnlp .
/usr/local/projects/SUN/keydir/signjogl ../codebasedir/*.jar
 
