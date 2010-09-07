#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.linux-i586.sh

DEPLOY=$JAVA_HOME/lib/deploy.jar
#DEPLOY=$JAVA_HOME/tmp/deploy/common/lib/deploy.jar
#DEPLOY=$JAVA_HOME/tmp/deploy/javaws/lib/deploy.jar
#DEPLOY=$JAVA_HOME/j2re-image/lib/deploy.jar

JAVAWS=$JAVA_HOME/lib/javaws.jar
#JAVAWS=$JAVA_HOME/tmp/deploy/javaws/lib/javaws.jar
#JAVAWS=$JAVA_HOME/j2re-image/lib/javaws.jar

PLUGIN=$JAVA_HOME/lib/plugin.jar
#PLUGIN=$JAVA_HOME/tmp/deploy/plugin/classes

export JPI_PLUGIN2_DEBUG=1
export JPI_PLUGIN2_VERBOSE=1

java -Xbootclasspath/a:$DEPLOY:$JAVAWS:$PLUGIN \
    sun.plugin2.applet.viewer.JNLP2Viewer  \
    file://$(pwd)/$1

