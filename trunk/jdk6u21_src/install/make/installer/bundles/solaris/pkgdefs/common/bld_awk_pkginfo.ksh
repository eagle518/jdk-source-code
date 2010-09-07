#!/usr/bin/ksh -p
#
# @(#)bld_awk_pkginfo.ksh	1.16 05/11/17
#
# Copyright 2006 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Simple script which builds the awk_pkginfo awk script.  This awk script
# is used to convert the pkginfo.tmpl files into pkginfo files
# for the build.
#


usage()
{
   echo "usage: bld_awk_pkginfo -p <prodver> -o <awk_script> [-m <pkg_arch>]"
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

# The ARCH value in pkginfo must be the base "isa" architecture for the
# system being built, i.e. , sparc or i386.  This is the same value returned
# by `uname -p`.  We cannot use a "sub-isa", such value such as sparcv9 or
# amd64, or any the "non-isa" values commonly used in the Java packages,
# such as i586.
#
# This code will use the default value obtained from `uname -p`, and allow
# a caller to override this value using a command line option.
#
pkg_arch=`uname -p`

#
# initialize other arguments
#
prodver=""
awk_script=""

#
# parse command line
#
while getopts o:p:m: c
do
   case $c in
   o)
      awk_script=$OPTARG
      ;;
   m)
      #
      # We need to map the ARCH value which may be a "sub-isa" value in the
      # case of 64-bit architectures to the underlying "isa" for that
      # architecture for insertion into the ARCH field of the pkginfo file.
      #
      # Note: Any mapping from i568 or i486 to the "Solaris proper" i386
      # is assumed to have already been done.  (If it hasn't the
      # result won't be right anyway.)
      #
      if [ "$OPTARG" = "sparc" ] || [ "$OPTARG" = "sparcv9" ]
      then
        pkg_arch=sparc
      elif [ "$OPTARG" = "i386" ] || [ "$OPTARG" = "amd64" ]
      then
        pkg_arch=i386
      else
        echo "Unrecognized ARCH (sub-isa) value"
        usage
        exit 1
      fi
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

if [ -z "$prodver" ] || [ -z "$awk_script" ]
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
      printf "ARCH=\"%s\"\n", "$pkg_arch"
      next
   }
{ print }
EOF

