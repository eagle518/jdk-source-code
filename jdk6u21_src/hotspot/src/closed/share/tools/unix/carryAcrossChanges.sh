#! /bin/sh
# 
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Purpose:  To help carry across recent changes on one platform
# (Intel) into the machine-dependent files of another platform (SPARC).

USAGE="carryAcrossChanges [-f parallel_file] [-v] [-c|-Cn|-bitw] [ file ... ]"

# The program examines each file of the form "*_sparc.[ch]pp",
# and produces for each such file F, if needed, another file F.diff.
# This is a set of context differences between recent versions
# of another "parallel" file PF of the form "*_i486.[ch]pp".
# (The exact name of PF and recent version information is
# extracted from F or its SCCS history.)

# More specifically, the file F.diff contains the differences
# the last "reconciled" version of PF, and the latest version
# of PF.  (If these versions are the same, the diff file is
# not generated.)  This shell script notes the diff files as
# they are produced, and asks the programmer to apply the diffs
# to the SPARC file F.  At the end of each F.diff file is a
# diff record specifying an addition to the "reconciliation history"
# of F, noting that F is now consistent with the latest version of PF.

# After the user brings F into consistency with the latest version
# of PF, the reconciliation history must be updated also, and this
# advertises that all recent changes to PF have been taken account of.

# This shell script creates a number of files and directories
# (it assumes that you will not be using their names):
#    F.diff			diffs to carry across
#    ./CARRY-ACROSS-DIFFS	list of all F.diff files
#    PF.rec/dddddd-tttttt	cached copies of relevant PF revisions
# The PF.rec directories are intended to serve as a supply of
# reconciliation baselines when SCCS histories are not available.
# Currently, they are created unconditionally.  They may be
# deleted harmlessly if SCCS directories are present; this
# script will simply re-create them from SCCS histories.

# The history of previous reconciliations may be found in a segment
# of each source file which looks like this:
#	//Reconciliation History
#	// 1.50 97/11/12 11:55:12 basicCodeEmit_i486.cpp
#	// 1.56 97/11/21 12:43:44 basicCodeEmit_i486.cpp
#	//End
# Each record in this list refers to a specific revision
# of a parallel file, with which this file has (at one time
# or another) been "reconciled" with.  That is, someone has
# manually examined the parallel file and carried across
# (translated) all relevant code.  This may have been done
# either incrementally or from scratch.  This shell script
# helps this process by producing a report of incremental
# modifications to the parallel file since the last
# reconciliation.

# The four fields in each reconciliation history record
# are SCCS revision id (which is ignored), a date and time
# of the parallel version (which is used to locate the
# last-reconciled version), and the name of the parallel file.
# Only the latest reconciliation history line is used;
# the rest are ignored.

# You may prune the histories at any time, by removing all
# but the most recent entry.  Only the most recent entry
# is needed to create the next round of diffs.

set -eu

# Agenda file:
AGENDA=CARRY-ACROSS-DIFFS
ADDED_TO_AGENDA=false

# Default diff flags:
DFLAGS_DEFAULT='-c -w'

# The target architecture we are building for:
: ${VM_ARCH=sparc}

# The OS system we are building for:
: ${VM_OS_FAMILY=solaris}

# The SCCS keywords must include both date and time, separated by a space:
GOOD_SCCS_KEYWORDS=%W\%\ %E\%\ %U\% # careful:  hide it from SCCS

# ^A, the SCCS record tag (avoid the actual char in this file).
A=`echo x | tr x '\\1'`

top_level() {
  process_arguments "${@-}"

  for F in $FILES
  do
    [ -d $F ] && {
      for FF in ` expand_directory $F `
      do
        process_file $FF
      done
      continue
    }
    process_file $F
  done

  $ADDED_TO_AGENDA && {
    >&2 echo "The file $AGENDA contains a list of diffs."
    >&2 echo "Please apply each one, and delete it from the list."
  }

  rm -f $TMP.*
}


process_file() {
  F=$1
  F_DIR=`dirname $F`
  F_NAME=`basename $F`

  verbose echo "# $F"

  # Get the history of reconciliations with parallel versions:
  RHIST=` extract_reconciliation_history $F `

  case $FARG in
  '') RTAB=`  extract_parallel_revisions     $F "$RHIST" `;;
  ?*) RTAB="0 0 0 $FARG";;
  esac

  case $RTAB in
  '') verbose \
      echo "*** No reconciliation history found for $F."
      return
  esac
  set -$- ` echo "$RTAB" | head -1 `
  PF_NAME=$4
  # Search for parallel version.
  PF=$F_DIR/$PF_NAME
  SRCH_DIR=$F_DIR
  SRCH_PAT=
  while [ $SRCH_DIR != . -a ! -f $PF ]
  do
    SRCH_DIR=`dirname $SRCH_DIR`
    SRCH_PAT="$SRCH_PAT/*"
    PF=`(ls $SRCH_DIR$SRCH_PAT/$PF_NAME 2>/dev/null || echo $PF) | head -1`
  done
  REC_PF_STAMP="$1 $2 $3"
  # (Ignore SCCS revision; it is sometimes adjusted by Teamware.)
  [ -f $PF ] || {
    >&2 echo "*** Cannot locate the parallel file for $F: $PF"
    return
  }

  # Look for the previously-reconciled version of the parallel file.
  NEW_PF_STAMP=` extract_sccs_header $PF `
  version_matches $NEW_PF_STAMP $REC_PF_STAMP && {
    verbose echo "# Latest version of $PF_NAME matches reconciliation of $F_NAME."
    # Create a cached copy of this version, for later use.
    CC=` make_cached_copy_name $PF $REC_PF_STAMP `
    [ -f $CC ] || {
      verbose_eval \
      cp -p $PF $CC
    }
    return
  }

  # The previously-reconciled version is not the most recent.
  # So, we need to generate some diffs, which will help the programmer
  # reconcile against the most recent version of the parallel file.

  # First, find a copy of the previously-reconciled version.
  REC_PF=` discover_parallel_version $PF $REC_PF_STAMP `
  case $REC_PF in
  '') CC=` make_cached_copy_name $PF $REC_PF_STAMP `
      M="and there is no SCCS history for $F"
      [ -f `sccsfilename $F` ] &&
        M="and there is no SCCS version for $F matching $REC_PF_STAMP"
      >&2 echo "*** Cannot find $CC, $M."
      return
  esac

  DF=$F.diff
  verbose_eval \
  diff $DFLAGS $REC_PF $PF \> $DF

  # Add the reconciliation history update to the end of the diff!
  update_reconciliation_history >> $DF \
	$F "$RHIST" "$RTAB" "// $NEW_PF_STAMP $PF_NAME"

  # Tell the user about this set of diffs.
  add_to_agenda $DF
}

version_matches() {
  # Usage: version_matches sid1 date1 time1 sid2 date2 time2
  [ $1 != unexpanded ] &&
  [ $2 = $5 -a \( $3 = $6 -o notime = $3 -o notime = $6 \) ]
}

make_cached_copy_name() {
  # Usage: make_cached_copy_name parallel_file sid date time
  CDIR=$1.rec
  [ -d $CDIR ] || mkdir $CDIR
  DTN=` echo $3-$4 | sed 's,[/:],,g' `
  echo $CDIR/$DTN
}

update_reconciliation_history() {
  F=$1 RHIST=$2 RTAB=$3 NEW_ENTRY=$4
  case $RHIST in
  '') LC=` wc -l < $F `
      LN=` expr $LC + 1 `
      LM=` echo "$RTAB" | wc -l `
      LE=` expr $LN + 3 + $LM `
      echo "***************"
      echo "***" $LC "****"
      echo "--- $LN,$LE ----"
      echo "+ "
      echo "+ //Reconciliation History"
      echo "$RTAB" \
      |sort +1     \
      |grep -v '0 0 0' \
      |sed 's:^:+ // :'
      echo "+ $NEW_ENTRY"
      echo "+ //End"
      ;;
  ?*) L1=` echo "$RHIST" | head -1 `
      LN=` fgrep -n "$L1" < $F | sed 's/:.*//' `
      LC=` echo "$RHIST" | wc -l `
      LE=` expr $LN + $LC - 1 `
      L2=` expr $LE + 1 `
      echo "***************"
      echo "*** $LN,$LE ****"
      echo "--- $LN,$L2 ----"
      echo "$RHIST" \
      |sed "
	s/^/  /
	${LC}{d;q;}
      "
      echo "+ $NEW_ENTRY"
      echo "$RHIST" \
      |sed -n "
	s/^/  /
	${LC}p
      "
      ;;
  esac
}

add_to_agenda() {
  >&2 echo "Adding to $AGENDA: $1"
  ([ -f $AGENDA ] && cat $AGENDA) \
  | fgrep -x -s "$1" ||
  echo $1 >> $AGENDA
  ADDED_TO_AGENDA=true
}

discover_parallel_version() {
  # Usage: discover_parallel_version parallel_file sid date time
  case $2 in
  0*) echo /dev/null; return;;
  esac

  # First, look for it in the cache.
  CC=` make_cached_copy_name "$@" `
  [ -f $CC ] && {
    echo $CC
    return
  }

  # Retrieve it from SCCS, if possible.
  DV=` discover_version_for_sid_and_date $1 $2 $3 $4 `
  case $DV in
  '') return
  esac

  set -$- $1 $DV
  CC=` make_cached_copy_name "$@" `
  [ -f $CC ] ||
  verbose_eval \
  sccs get -r$2 -s -G$CC $1
  echo $CC
}

discover_version_for_sid_and_date() {
  # Usage: discover_version_for_sid_and_date file sid date time
  # Returns the sid/date/time of the version with a matching date & time.
  # Prefers to return the given sid, but must match the date.
  SF=` sccsfilename $1 `
  [ -f $SF ] || {
    return
  }
  # Here is the pattern to match in the SCCS file, e.g., 01/02/03 04:05:06
  DT=` echo "$3 $4" | sed -e 's:/:\\\\/:g' -e 's/notime/[0-9][0-9:]*/' `
  # Here is the sid pattern, with embedded dots regexp-quoted:
  SP=` echo $2 | sed 's/[.]/[.]/g' `
  # Output a list of all matching dates to $TMP.tab.
  # Output to $TAB a matching date also with a matching sid.
  rm -f $TMP.tab
  XSID=` sed -n < $SF '
	/^'${A}'I/q
	/^'${A}'d D \([0-9][0-9.]*\) \([0-9][0-9\/]*\) \([0-9][0-9:]*\) .*$/{
	  s//\1 \2 \3/
	  /^[0-9][0-9.]* '"$DT"'$/!d
	  # An exact match of the sid?
	  /^'"$SP"' /p
	  w '$TMP.tab'
	}
  '`
  TAB0=` head -1 $TMP.tab `
  rm -f $TMP.tab

  # Was there an exact match with date, time, and sid?
  case $XSID in
  ?*) echo "$XSID"; return
  esac

  # Was there an exact match with date and time?
  case $TAB0 in
  ?*) echo "$TAB0"; return
  esac

  # Find a recent delta no later than the requested date.
  # Ignore branch deltas.
  CARG=` echo $3$4 | sed -e 's/notime//' `
  sccs prs -e -c"$CARG" -d':I: :D: :T:' $SF \
  | grep -v '^[0-9]*[.][0-9]*[.]' \
  | head -1
}

extract_sccs_header() {
  PF=$1
  n1=`basename $PF`
  S=` sed -n < $PF '
	/.*@(#) *'$n1'[ 	]*\([0-9][0-9.]*\) \([0-9][0-9\/]*\) \([0-9][0-9:]*\).*/ {
	  s//\1 \2 \3/p
	  q
	}
	# Deficient format (time missing):
	/.*@(#) *'$n1'[ 	]*\([0-9][0-9.]*\) \([0-9][0-9\/]*\).*/ {
	  s//\1 \2 notime/p
	  q
	}
	# Perhaps the file is checked out for editing:
	/%[WEUAZ]%/ {
	  s/.*/unexpanded/p
	  q
	}
	'`
  case $S in
  '') >&2 echo "*** Cannot locate timestamp for parallel file: $PF"
      ;;
  ?*) [ -f ` sccsfilename $PF ` ] && {
	# Check for stuck SCCS keywords:
	S2=` sccs prs -d':I: :D: :T:' $PF `
	[ "$S" != "$S2" -a "$S" != unexpanded -a "$VFLAG" = -v ] && {
	  >&2 echo "*** $PF has stuck or deficient SCCS keywords;"
	  >&2 echo "    using \"$S2\" instead of \"$S\""
	  >&2 echo "*** Also, change the header line of $PF"
	  >&2 echo "    from:  `fgrep '@(#)' < $PF | head -1`"
	  >&2 echo "    to:    // $GOOD_SCCS_KEYWORDS"
	}
	S=$S2
      }
  esac
  echo $S
}


extract_reconciliation_history() {
  F=$1

  # First fetch reconciliation history from the file itself.
  sed < $F -n '
	/\/\/ *Reconciliation History/,$ {
	  p
	  /\/\/ *End/q
	}
  '
}


extract_parallel_revisions() {
  F=$1
  RHIST=$2

  # First fetch reconciliation history from the file itself.
  extract_reconciliation_history $F > $TMP.rh

  # Parse it into tabular form.
  rm -f $TMP.err
  echo "$RHIST" \
  | sed > $TMP.rt -n '
	1d
	/^[//Reconcile with version "]*\([0-9][0-9.]*\) \([0-9][0-9\/]*\) \([0-9][0-9:]*\)[" of]* \([a-z].*[.][chppad]*\).*/ {
	  # Format:  1.50 97/11/12 11:55:12 basicCodeEmit_i486.cpp
	  #     or:  Reconcile with version "1.50 97/11/12 11:55:12" of basicCodeEmit_i486.cpp
	  s//\1 \2 \3 \4/p
	  d
	}
	/^ *$/d
	/\/\/ *End/d
	w '$TMP.err'
  ' > $TMP.rt
  [ -s $TMP.err ] && {
    >&2 echo "*** Ignored malformed records in reconciliation history of $F:"
    >&2 cat $TMP.err
    >&2 echo "*** The correct format is:  <sccsrid> <date> <time> <name>.[ch]pp"
    >&2 echo "    or the more verbose:  Reconcile with version \"<sccsrid> <date> <time>\" of <name>.[ch]pp"
    rm -f $TMP.err
  }

  # As a backup, fetch reconciliation history from SCCS, if present.
  # This whole paragraph of code can be safely deleted, once reconciliation
  # histories are transferred fully to the clear-files (as they ought to be).
  SF=` sccsfilename $F `
  [ -f $SF -a ! -s $TMP.rt ] && {
    sed < $SF -n '
	/^'${A}'I/q
	/^'${A}'c[ Reconcile with version "]*\([0-9][0-9.]*\) \([0-9][0-9\/]*\) \([0-9][0-9:]*\)[" of]* \([a-z].*[.][ch]pp\).*/ {
	  s//\1 \2 \3 \4/p
	}
    ' > $TMP.rt
    [ -s $TMP.rt ] &&
      verbose echo "# No reconciliation history in $F; found some under SCCS."
  }

  # Sort by date, the youngest first.
  sort $TMP.rt -o $TMP.rt +1r
  cat $TMP.rt
  rm -f $TMP.rt
}

sccsfilename() {
  echo `dirname $1`/SCCS/s.`basename $1`
}

process_arguments() {
  [ -d src/share/vm ] || {
    >&2 echo "Must be in root directory of HotSpot workspace."
    exit 2
  }

  FILES=
  VFLAG=
  FARG=
  DFLAGS=

  while true
  do
    case $# in 0) break;; esac
    case $1 in
      -v) VFLAG=$1;;
      -f) FARG=$2; shift;;
      -c*|-C*|-b*|-w*|-i*|-t*)
          $DFLAGS="$DFLAGS $1";;
      -*|'')
          >&2 echo "Usage: $USAGE"; exit 2;;
      *)  FILES="$FILES $1";;
    esac
    shift
  done

  case $DFLAGS in
  '') DFLAGS=$DFLAGS_DEFAULT
  esac

  TMP=/tmp/cac$$.
  trap "rm -f $TMP.*" 0 2 3 15

  # By default, process all "_sparc" and "_solaris" source files.
  set -$- $FILES
  case $# in
  0) FILES=` expand_directory * -type d -print -prune -o `;;
  1) [ "$FARG" = '' -o `dirname $FARG` = `dirname $1` ] || {
       >&2 echo "$0: The \"-f\" file must be in the same directory as the argument file."
       exit 2
     }
     case $FARG in
     ?*) FARG=`basename $FARG`
     esac;;
  *) case $FARG in
     ?*) >&2 echo "$0: The \"-f file\" option must be followed by a single file."
	 exit 2
     esac;;
  esac
}

expand_directory() {
  find $* \
	-type f -name "$VM_ARCH.ad"                   -print -o \
	-type f -name "[a-z]*_$VM_ARCH.ad"            -print -o \
	-type f -name "[a-z]*_$VM_ARCH.[ch]pp"        -print -o \
	-type f -name "[a-z]*_$VM_ARCH[._]*.[ch]pp"   -print -o \
	-type f -name "[a-z]*_$VM_OS_FAMILY.[ch]pp"   -print -o \
	-type f -name "[a-z]*_$VM_OS_FAMILY.*.[ch]pp" -print -o \
	-name '*.rec'                             -prune -o \
	-name '*[Dd][Ee][Ll][Ee][Tt][Ee][Dd]*'    -prune -o \
	-name SCCS                                -prune \
  2>/dev/null  ||true
  # Throw away error info.
}

verbose() {
  case $VFLAG in
  -v) >&2 "$@"   ||true;;
  esac
}

verbose_eval() {
  verbose echo \
  + "$@"
  eval \
  "$@"     ||true
}

top_level "${@-}"
