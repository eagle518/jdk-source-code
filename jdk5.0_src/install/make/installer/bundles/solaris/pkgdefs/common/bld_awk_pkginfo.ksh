#!/usr/bin/ksh -p
#
# @(#)bld_awk_pkginfo.ksh	1.14 04/05/26
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Simple script which builds the awk_pkginfo awk script.  This awk script
# is used to convert the pkginfo.tmpl files into pkginfo files
# for the build.
#


usage()
{
   echo "usage: bld_awk_pkginfo -p <prodver> -m <mach> -o <awk_script>"
}
#
# Awk strings
#
# two VERSION patterns: one for Dewey decimal, one for Dewey plus ,REV=n
# the first has one '=' the second has two or more '='
#
VERSION1="VERSION=[^=]*$"
VERSION2="VERSION=[^=]*=.*$"
PRODVERS="^SUNW_PRODVERS="
ARCH='ARCH=\"ISA\"'

#
# parse command line
#
mach=""
prodver=""
awk_script=""

while getopts o:p:m: c
do
   case $c in
   o)
      awk_script=$OPTARG
      ;;
   m)
      mach=$OPTARG
      ;;
   p)
      prodver=$OPTARG
      ;;
   \?)
      usage
      exit 1
      ;;
   esac
done

if [[ ( -z $prodver ) || ( -z $mach ) || ( -z $awk_script ) ]]
then
   usage
   exit 1
fi

if [[ -f $awk_script ]]
then
	rm -f $awk_script
fi

#
# Build REV= field based on date
#
if [ -z "$rev" ] ; then	
	rev=$(date "+%Y.%m.%d.%H.%M")
	export rev
fi

# expecting $prodver to be in the format "RELEASE/VERSION"
# "VERSION" may have an optional _xx value that should be removed.
full_release="${prodver%%/*}"
release="${full_release%%_*}"
case "$release" in
?*.?*.?*)	release_1dot="${release#*.}";;
*)		release_1dot="$release";;
esac

#
# Build awk script which will process all the
# pkginfo.tmpl files.
#
# the first VERSION pattern is replaced with a leading quotation mark
#
rm -f $awk_script
cat << EOF > $awk_script
/$VERSION1/ {
      sub(/RELEASE/,"$release")
      sub(/\=[^=]*$/,"=\"$rev\"")
      print
      next
   }
/$VERSION2/ {
      sub(/RELEASE/,"$release")
      sub(/\=[^=]*$/,"=$rev\"")
      print
      next
   }
/^NAME=|^SUNW_PRODNAME=/ {
      sub(/RELEASE/,"$release_1dot")
      sub(/FULLREL/,"$full_release")
      print
      next
}
/$PRODVERS/ { 
      printf "SUNW_PRODVERS=\"%s\"\n", "$prodver" 
      next
   }
/$ARCH/ {
      printf "ARCH=\"%s\"\n", "$mach"
      next
   }
{ print }
EOF

