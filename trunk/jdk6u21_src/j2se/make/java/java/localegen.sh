#!/bin/sh
# @(#)localegen.sh	1.1 05/08/12
#
# This script is to generate the supported locale list string and replace the
# LocaleDataMetaInfo-XLocales.java in <j2se ws>/src/share/classes/sun/util
# 
# NAWK & SED is passed in as environment variables.
#

# A list of resource base name list;
RESOURCE_NAMES=$1

# A list of European resources;
EURO_FILES_LIST=$2

# A list of non-European resources;
NONEURO_FILES_LIST=$3

INPUT_FILE=$4
OUTPUT_FILE=$5

localelist=
getlocalelist() {
    localelist=""
    localelist=`$NAWK -F$1_ '{print $2}' $2 | sort`
}

sed_script="$SED -e \"s@^#warn .*@// -- This file was mechanically generated: Do not edit! -- //@\" " 

for FILE in $RESOURCE_NAMES 
do
    getlocalelist $FILE $EURO_FILES_LIST
    sed_script=$sed_script"-e \"s/#"$FILE"_EuroLocales#/$localelist/g\" "
    getlocalelist $FILE $NONEURO_FILES_LIST
    sed_script=$sed_script"-e \"s/#"$FILE"_NonEuroLocales#/$localelist/g\" "
done

sed_script=$sed_script"$INPUT_FILE > $OUTPUT_FILE"
eval $sed_script


