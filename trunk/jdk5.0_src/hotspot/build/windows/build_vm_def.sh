# 
# @(#)build_vm_def.sh	1.9 03/12/23 16:35:32
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This shell script builds a vm.def file for the current VM variant.
# The .def file exports vtbl symbols which allow the Serviceability
# Agent to run on Windows. See build/windows/projectfiles/*/vm.def
# for more information.
#
# The script expects to be executed in the directory containing all of
# the object files.

# Note that we currently do not have a way to set HotSpotMksHome in
# the batch build, but so far this has not seemed to be a problem. The
# reason this environment variable is necessary is that it seems that
# Windows truncates very long PATHs when executing shells like MKS's
# sh, and it has been found that sometimes `which sh` fails.
if [ "x$HotSpotMksHome" != "x" ]; then
 MKS_HOME="$HotSpotMksHome"
else
 SH=`which sh`
 MKS_HOME=`dirname "$SH"`
fi

echo "EXPORTS" > vm1.def

AWK="$MKS_HOME/awk.exe"
GREP="$MKS_HOME/grep.exe"
SORT="$MKS_HOME/sort.exe"
UNIQ="$MKS_HOME/uniq.exe"
CAT="$MKS_HOME/cat.exe"
RM="$MKS_HOME/rm.exe"

dumpbin /symbols *.obj | "$GREP" "??_7.*@@6B@" | "$AWK" '{print $7}' | "$SORT" | "$UNIQ" > vm2.def
"$CAT" vm1.def vm2.def > vm.def
"$RM" -f vm1.def vm2.def
