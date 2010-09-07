#!/bin/ksh

# Run on the source-bundles created before and after any source bundling
#   changes. Will verify the bundling is the same as it was, to a degree.

exitCode=0
t=/tmp/compare.$$
rm -f -r $t
mkdir -p $t

error()
{
  echo "ERROR: $1"
  echo "ERROR: $1" >> $t/errorMessages
  exitCode=1
}

listZip()
{
  if [ "`file $1 | fgrep 'ZIP archive'`" != "" ] ; then
    unzip -l $1 | \
       egrep '[0-9][0-9]*[\ ][\ ]*[0-9][0-9]-[0-9][0-9]-[0-9][0-9][\ ][0-9][0-9]:[0-9][0-9]' | \
       egrep -v '.*/$' | \
       cut -c1-10,29- 
  fi
}

listIgnore() # file
{
  #(cat $1 | grep 'jdk/make/redist')
  # Need to do something... 
  echo "dummy" > /dev/null
}

listInterested() 
{
  cat $1 \
  | fgrep -v jdk/make/netbeans  \
  | cat
}

getCount()
{
  c=`cat $1 | wc -l`
  expr ${c} '+' 0
}

# Get the two directories
d1=$1
if [ "$d1" = "" -o ! -d $d1 ] ; then
  error "No first directory supplied, usage: $0 dir1 dir2"
  exit 1
fi
d2=$2
if [ "$d2" = "" -o ! -d $d2 ] ; then
  error "No second directory supplied, usage: $0 dir1 dir2"
  exit 1
fi

# Get the bundles in the two dirs
(cd $d1 && find . -type f | sort) > $t/l1
(cd $d2 && find . -type f | sort) > $t/l2

# Compare the bundle lists
diff $t/l1 $t/l2 > $t/ldiff

# Find the common files to compare
cp $t/l1 $t/list
if [ "`cat $t/ldiff`" != "" ] ; then
  error "The bundle list is different, will use what is in common"
  cat $t/ldiff
  comm -1 -2 $t/l1 $t/l2 > $t/list
fi

for i in `cat $t/list` ; do
  echo "========================================================"
  echo "Comparing $i"
  ls -al $d1/$i
  ls -al $d2/$i
  listZip $d1/$i > $t/oz1
  listIgnore $t/oz1 > $t/ig1
  if [ "`cat $t/ig1`" != "" ] ; then
     echo "Ignoring these files:"
     cat $t/ig1
  fi
  listInterested $t/oz1 > $t/z1
  listZip $d2/$i > $t/oz2
  listIgnore $t/oz2 > $t/ig2
  if [ "`cat $t/ig2`" != "" ] ; then
     echo "Ignoring these files:"
     cat $t/ig2
  fi
  listInterested $t/oz2 > $t/z2
  c1=`getCount $t/z1`
  c2=`getCount $t/z2`
  if [ ${c1} -ne ${c2} ] ; then
    error "Different count of files in bundle, ${c1}!=${c2}: $i"
  fi
  echo "Contains ${c1} files: ${i}"
  cat $t/z1 | cut -c11- | sort > $t/n1
  cat $t/z2 | cut -c11- | sort > $t/n2
  comm -3 $t/n1 $t/n2 > $t/notsame
  if [ "`cat $t/notsame`" != "" ] ; then
      error "Filenames different in bundle: $i"
      echo "Missing Files:"
      comm -2 -3 $t/n1 $t/n2
      echo "Extra Files:"
      comm -1 -3 $t/n1 $t/n2
  fi
  echo " "
done

rm -f -r $t

if [ -f $t/errorMessages ] ; then
  cat $t/errorMessages
fi
exit ${exitCode}

