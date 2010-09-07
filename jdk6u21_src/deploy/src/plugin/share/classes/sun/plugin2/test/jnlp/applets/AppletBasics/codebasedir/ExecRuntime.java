

public class ExecRuntime {
     public static void main(String[] args) {
String [] cmds0 = {
"C:\\jre1.6.0_10\\bin\\javaw.exe",
"-DminimumJRE=1.4.2",
"\"-DQueryResult.WarningMessage=The query exceeds the allowed number of search terms.NL Results matching the first 40 terms have been returned.NL This may include more or less results than what the full query would have returned.\"",
"-DQueryResult.LimitationCount=40",
"-client",
"-Xbootclasspath/a:C:\\jre1.6.0_10\\lib\\javaws.jar;C:\\jre1.6.0_10\\lib\\deploy.jar",
"-classpath",
"C:\\jre1.6.0_10\\lib\\deploy.jar",
"\"-Djnlpx.vmargs=-DminimumJRE=1.4.2 \\\"-DQueryResult.WarningMessage=The query exceeds the allowed number of search terms.NL Results matching the first 40 terms have been returned.NL This may include more or less results than what the full query would have returned.\\\" -DQueryResult.LimitationCount=40 -client\"",
"-Djnlpx.jvm=C:\\jre1.6.0_10\\bin\\javaw.exe",
"-Djnlpx.splashport=3744",
"-Djnlpx.home=C:\\jre1.6.0_10\\bin",
"-Djnlpx.remove=false",
"-Djnlpx.offline=false",
"-Djnlpx.relaunch=true",
"-Djnlpx.heapsize=NULL,NULL",
"-Djava.security.policy=file:C:\\jre1.6.0_10\\lib\\security\\javaws.policy",
"-DtrustProxy=true",
"-Xverify:remote",
"com.sun.javaws.Main",
"-secure",
"JNLPLongProperties1.2.abs.sec.jnlp" };

String [] cmds1 = {
"/opt-linux-x86/jre-dev/bin/java",
"-DQueryResult.LimitationCount=40",
"-DQueryResult.WarningMessage=\"The query exceeds the allowed number of search terms.NL Results matching the first 40 terms have been returned.NL This may include more or less results than what the full query would have returned.\"",
"-DminimumJRE=1.4.2",
"-client",
"-Xbootclasspath/a:/opt-linux-x86/jre-dev/lib/javaws.jar:/opt-linux-x86/jre-dev/lib/deploy.jar",
"-classpath",
"/opt-linux-x86/jre-dev/lib/deploy.jar",
"-Djnlpx.vmargs=\"-DQueryResult.LimitationCount=40 -DQueryResult.WarningMessage=\\\"The query exceeds the allowed number of search terms.NL Results matching the first 40 terms have been returned.NL This may include more or less results than what the full query would have returned.\\\" -DminimumJRE=1.4.2 -client\"",
"-Djnlpx.jvm=/opt-linux-x86/jre-dev/bin/java",
"-Djnlpx.splashport=45125",
"-Djnlpx.home=/opt-linux-x86/jre-dev/bin",
"-Djnlpx.remove=false",
"-Djnlpx.offline=false",
"-Djnlpx.relaunch=true",
"-Djnlpx.heapsize=NULL,NULL",
"-Djava.security.policy=file:/opt-linux-x86/jre-dev/lib/security/javaws.policy",
"-DtrustProxy=true",
"-Xverify:remote",
"com.sun.javaws.Main",
"-secure",
"../scodebasedir/JNLPLongProperties1.2.abs.sec.jnlp" };

        String[] cmds=null;

        if(args.length>1) {
            cmds=args;
        } else {
            int sel = -1;
            try {
                sel = Integer.valueOf(args[0]).intValue();
            } catch (Exception e) {
                e.printStackTrace();
                System.exit(-1);
            }
            if(sel<0) System.exit(-1);
            switch(sel) {
                case 0: cmds=cmds0; break;
                case 1: cmds=cmds1; break;
                default: System.exit(-1);
            }
        }

        
        for(int i = 0; i < cmds.length; i++) {
            System.out.println("cmd " + i + " : " + cmds[i]);
        }
        try {
            Runtime.getRuntime().exec(cmds);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

