#! /bin/sh

#J2RE_HOME=/java/devtools/linux/jdk1.6.0_10/jre
#JAVA_HOME=/java/devtools/linux/jdk1.6.0_10
#J2RE_HOME=/usr/local/projects/SUN/JDK6/control/build/linux-i586/j2re-image
J2RE_HOME=$ALT_OUTPUTDIR/j2re-image
JAVA_HOME=$ALT_OUTPUTDIR
CP_SEP=:

export LIBXCB_ALLOW_SLOPPY_LOCK=1

PATH=$JAVA_HOME/bin:$PATH
export PATH


