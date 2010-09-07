#! /bin/sh

LOGDIR=$1

if [ -z "$LOGDIR" ] ; then
    echo Usage $0 "<logdir>"
fi


JAVALOGDIR=$HOME/.java/deployment/log

if [ -z "$LOGDIR" ] ; then
    echo Usage $0 "<logdir>"
fi

. ./tests_run_auto-func.sh

#set -x 

rm -rf $LOGDIR
mkdir -p $LOGDIR

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/hello0.jnlp hello0-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/hello0.jnlp hello0-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/hello1.jnlp hello1-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/hello1.jnlp hello1-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/jnlpdir2/hello2.jnlp hello2-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/jnlpdir2/hello2.jnlp hello2-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/jnlpdir2/hello3.jnlp hello3-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/codebasedir/jnlpdir2/hello3.jnlp hello3-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello4.jnlp  hello4-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello4.jnlp  hello4-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello5.jnlp  hello5-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello5.jnlp  hello5-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello6.jnlp  hello6-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello6.jnlp  hello6-1

JAVAWSTest1 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello7.jnlp  hello7-0
JAVAWSTest2 http://demo.goethel.localnet/SUN/JDK6/deploy/src/plugin/share/classes/sun/plugin2/test/jnlp/applets/AppletBasics/jnlpdir/hello7.jnlp  hello7-1

