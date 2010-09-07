#! /bin/sh

THISDIR=$(pwd)

rm -rf scodebasedir

cd codebasedir
rm -f *.jar *.class

rm -f jnlpdir2/*.jar
rm -f jnlpdir2/jnlpdir3/*.jar


cd $THISDIR

