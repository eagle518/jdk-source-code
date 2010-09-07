#! /bin/sh

BASEDIR=$(dirname $0)

. $BASEDIR/setenv.test.linux-i586.sh
# . $BASEDIR/setenv.test.windows-i586.sh

PATH=$JAVA_HOME/bin:$PATH

JAVAC=$JAVA_HOME/bin/javac

JAVAWS=$J2RE_HOME/lib/javaws.jar
#JAVAWS=$JAVA_HOME/tmp/deploy/javaws/lib/javaws.jar
#JAVAWS=$JAVA_HOME/j2re-image/lib/javaws.jar
PLUGIN=$J2RE_HOME/lib/plugin.jar

THISDIR=$(pwd)

CLASSPATH=$JAVAWS$CP_SEP$PLUGIN$CP_SEP$CLASSPATH
export CLASSPATH

cd codebasedir
echo $JAVAC $CLASSPATH
$JAVAC -source 1.4 -target 1.4 -classpath $CLASSPATH *.java
jar cf hellojavaws.jar hellojavaws.class
jar cf SimpleApplet.jar SimpleAppletTestIf1.class SimpleAppletBase.class SimpleAppletUtil.class
jar cf SimpleAppletNonExistendPart.jar SimpleAppletTestIf1.class SimpleAppletUtil.class
jar cf SimpleApplet1.jar SimpleApplet1.class
jar cf SimpleApplet2.jar SimpleApplet2.class
jar cf JNLPBasicService1.jar JNLPBasicService1.class
jar cf JNLPSingleInstanceService1.jar JNLPSingleInstanceService1*.class
jar cf JNLPExtendedService1.jar JNLPExtendedService1*.class
jar cf JNLPPersistenceService1.jar JNLPPersistenceService1*.class
jar cf JRERelaunch1.jar JRERelaunch1*.class
jar cfm JRERelaunch1vers.jar ../JRERelaunchV2.0.Manifest JRERelaunch1*.class

rm -f JNLPJARVersion.java JNLPJARVersion*class
cp version/JNLPJARVersion1p1.java JNLPJARVersion.java
$JAVAC -source 1.4 -target 1.4 -classpath $CLASSPATH JNLPJARVersion.java
jar cf JNLPJARVersion__V1.1.jar JNLPJARVersion*class
rm -f JNLPJARVersion.java JNLPJARVersion*class

cp version/JNLPJARVersion1p2.java JNLPJARVersion.java
$JAVAC -source 1.4 -target 1.4 -classpath $CLASSPATH JNLPJARVersion.java
jar cf JNLPJARVersion__V1.2.jar JNLPJARVersion*class
rm -f JNLPJARVersion.java JNLPJARVersion*class

cp version/JNLPJARVersion2p0.java JNLPJARVersion.java
$JAVAC -source 1.4 -target 1.4 -classpath $CLASSPATH JNLPJARVersion.java
jar cf JNLPJARVersion__V2.0.jar JNLPJARVersion*class
rm -f JNLPJARVersion.java JNLPJARVersion*class

cp SimpleApplet.jar  jnlpdir2/jnlpdir3/SimpleApplet-2.1.jar

cd $THISDIR

