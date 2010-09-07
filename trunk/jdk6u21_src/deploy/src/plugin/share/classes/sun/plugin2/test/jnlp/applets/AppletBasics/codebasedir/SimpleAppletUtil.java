/**
 */

import java.applet.*;
import java.lang.*;
import java.util.*;

public class SimpleAppletUtil 
{
    protected static void printAppletInfo(Applet a)
    {
        if(a==null) return;

        int w = a.getWidth();
        int h = a.getHeight();

        try {
            String name = a.getParameter("NAME"); // the std in applet name

            System.out.println("Applet: "+name+" ["+w+"x"+h+"]");
            System.out.println("\tgetDocumentBase(): "+a.getDocumentBase());
            System.out.println("\tgetCodeBase()    : "+a.getCodeBase());
            System.out.println("\tgetAppletInfo()  : "+a.getAppletInfo());
            System.out.println("\tisActive()       : "+a.isActive());
            System.out.println("\tjava properties  :");
            System.out.println("\t\tvendor           : "+System.getProperty("java.vendor"));
            System.out.println("\t\tversion product  : "+System.getProperty("java.version"));
            System.out.println("\t\tversion platform : "+System.getProperty("java.specification.version"));
            System.out.println("\t\tmax heap size    : "+Runtime.getRuntime().maxMemory());
        } catch (Exception e) {
            System.out.println("Applet: is not a true applet");
        }

        System.out.println("");
    }

    protected static void printAppletsInfo(AppletContext ac)
    {
        if (ac==null) return;

        try {
            Enumeration    applets = ac.getApplets();
            for (int i = 1; applets.hasMoreElements() ; i++) {
                 printAppletInfo((Applet)applets.nextElement());
            }
        } catch (Exception e) {e.printStackTrace();}
    }

    // Helper to parse memory size specifications within the whole JVM arguments
    // ("256m", which is the tail of "-Xmx256m")
    public static long parseMemorySpec(String jvm_args) throws IllegalArgumentException {
        long maxHeapSize = -1;
        if(jvm_args!=null) {
            StringTokenizer tok = new StringTokenizer(jvm_args);
            while (tok.hasMoreTokens()) {
                String arg = tok.nextToken();
                if (arg.startsWith("-Xmx")) {
                    try {
                        maxHeapSize = parseMemorySpecSingleArg(arg.substring("-Xmx".length()));
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        return maxHeapSize;
    }

    private static long parseMemorySpecSingleArg(String spec) throws IllegalArgumentException {
        char sizeSpecifier = (char) 0;
        String origSpec = spec;
        for (int i = 0; i < spec.length(); i++) {
            if (!Character.isDigit(spec.charAt(i))) {
                if (i != spec.length() - 1) {
                    throw new IllegalArgumentException("Too many characters after heap size specifier");
                } else {
                    sizeSpecifier = spec.charAt(i);
                    spec = spec.substring(0, spec.length() - 1);
                    break;
                }
            }
        }
        try {
            long val = Long.parseLong(spec);
            if (sizeSpecifier != (char) 0) {
                switch (sizeSpecifier) {
                    case 'T': case 't':
                        val = val * TB;
                        break;
                    case 'G': case 'g':
                        val = val * GB;
                        break;
                    case 'M': case 'm':
                        val = val * MB;
                        break;
                    case 'K': case 'k':
                        val = val * KB;
                        break;
                    default:
                        throw new IllegalArgumentException("Illegal heap size specifier " + sizeSpecifier + " in " + origSpec);
                }
            }
            return val;
        } catch (NumberFormatException e) {
            throw (IllegalArgumentException)
                new IllegalArgumentException().initCause(e);
        }
    }

    private static final int  KB = 1024;
    private static final int  MB = KB * KB;
    private static final int  GB = KB * MB;
    private static final long TB = KB * GB;
}

