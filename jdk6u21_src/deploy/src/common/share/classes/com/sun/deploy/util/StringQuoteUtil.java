/*
 * @(#)StringQuoteUtil.java	1.6 - 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.*;
import java.util.*;

public class StringQuoteUtil {
    private static final boolean DEBUG   = false;

    public final static boolean isEscape(char c) {
        return c=='\\';
    }

    public final static boolean isQuote(char c) {
        return c=='\"'; // we ignore \' here
    }

    public final static boolean isWhiteSpace(char c) {
       return c == ' ' || c== '\t' || c=='\n' || 
              c =='\r' || c== '\f' ;
    }

    public final static boolean needsQuote(String arg) {
        return (   arg.indexOf(' ') != -1
                || arg.indexOf('\t')!= -1
                || arg.indexOf('\"')!= -1
               );
    }
    
    public final static boolean needsQuote(StringBuffer arg) {
        return needsQuote(arg.toString());
    }
    
    // embraces the string with quotes,
    // escape \" 
    // and remove a '\' at the end,
    // so we don't confuse unquote with an ending '\"' pattern,
    //  (the infamous windows path bug)
    public final static String quote(String in) {
        if (in == null) return null;
        return quote(new StringBuffer(in)).toString();
    }

    public final static StringBuffer quote(StringBuffer in) {
        if (in == null) return null;

        StringBuffer sb = new StringBuffer();
        int inLen = in.length();
        sb.append("\"");
        for (int i = 0; i < inLen; i++) {
            char c = in.charAt(i);
            if (i!=inLen-1 || c != '\\') {
                if (c == '\"') {
                    sb.append('\\');
                }
                sb.append(c);
            } // else skip ending '\'
        }
        sb.append("\"");
        return sb;
    }

    // unquotes only one element
    //
    // @param in the input buffer
    // @param startPos begin with startPos
    //
    // @param out used to append the digested quoted part, if not null
    // @param unquote if true unquote the appended part to the unquote sequence to 'out'
    // @param deleteQuoted if true, delete the quoted part
    // @return if 'deleteQuoted' is true,  'in' position after the deleted quote
    //         if 'deleteQuoted' is false, 'in' position after the quote
    //         if no quote was found -1
    public final static int digestNextQuote(StringBuffer in, int startPos, StringBuffer out, 
                                            boolean unquote, boolean deleteQuoted) 
    {
        if (in == null) return -1;
        int inLen = in.length();
        if(startPos>inLen-1) return -1;

        char c; // current char
        int  inQuote = -1, inEscape = -1;
        int  innerQuote = -1; // trying to handle the windows path bug
        int  i;

        for (i=startPos; i < inLen; i++) {
            c   = in.charAt(i);

            // TS: terminal symbol
            if(inQuote<0) {
                if(isQuote(c)) { 
                    // TS: "
                    inQuote=i;
                    if(null!=out && !unquote) {
                        out.append(c);
                    }
                } else {
                    // any other char
                    if(null!=out) {
                        if (i!=inLen-1 || c != '\\') {
                            out.append(c);
                        } // else skip ending '\'
                    }
                }
            } else {
                if(isEscape(c)) {
                    if(inEscape>=0) {
                        // TS: \\
                        inEscape=-1;
                        if(null!=out) {
                            if(!unquote) {
                                out.append('\\');
                            }
                            out.append('\\');
                        }
                    } else {
                        // TS: \
                        // earmark escape
                        inEscape=i;
                    }
                } else if(isQuote(c)) {
                    if (inEscape>=0) {
                        // TS: \"
                        // unquote: \" -> "
                        inEscape=-1;
                        innerQuote = (innerQuote>=0)?-1:i; // earmark ..
                        if(null!=out) {
                            if(!unquote) {
                                out.append('\\');
                            }
                            // add quote
                            out.append('\"');
                        }
                    } else {
                        // end of quote
                        if(null!=out && !unquote) {
                            // add quote
                            out.append(c);
                        }
                        if(deleteQuoted) {
                            in.delete(inQuote, i+1); // delete the quoted part
                            return inQuote; // after deleted quote
                        }
                        return i+1; // after quote
                    }
                } else {
                    if(inEscape>=0) {
                        // add the pending escape
                        inEscape=-1;
                        if(null!=out) {
                            out.append('\\');
                        }
                    }
                    // any char
                    if(null!=out) {
                        out.append(c);        
                    }
                }
            }
        }
        if(innerQuote>=0) {
            if(inQuote>=0) {
                // must be the buggy windows path .. <"C:\lala\"> - fix it
                if (null != out) {
                    // remove \"
                    if(!unquote) {
                        // drop '\' 
                        out.deleteCharAt(out.length()-1);
                    }
                    // drop '\' (here: '\"')
                    out.deleteCharAt(out.length()-1);
                }
                return inLen;
            } else {
                throw new IllegalArgumentException("unclosed inner quote: <"+in.substring(innerQuote, inLen).toString()+
                                                   ">\n\t(innerQuoteStart: "+ innerQuote+", scanPos: "+i+", len: "+inLen+
                                                   ")\n\tres: "+((out!=null)?out.toString():null));
           }
        }
        if(inQuote>=0) {
            throw new IllegalArgumentException("unclosed string literal: <"+in.substring(inQuote, inLen).toString()+
                                               ">\n\t(quoteStart: "+ inQuote+", scanPos: "+i+", len: "+inLen+
                                               ")\n\tres: "+((out!=null)?out.toString():null));
        }

        return -1;
    }

    // unquote if the entity is enclosed in quotes 
    // and if they represent a single entity
    // <"la la"          -> <la la>
    // <"la la" "ba ba"> -> <"la la" "ba ba">
    public final static String unquoteIfEnclosedInQuotes(String in) {
        if (in == null) return null;
        return unquoteIfEnclosedInQuotes(new StringBuffer(in)).toString();
    }

    public final static StringBuffer unquoteIfEnclosedInQuotes(StringBuffer in) {
        if (in == null)  return null;
        int inLen = in.length();
        if (inLen<2) return in;

        if(isQuote(in.charAt(0)) && isQuote(in.charAt(inLen-1))) {
            StringBuffer sb=new StringBuffer();
            int i=0, num=0;
            while ( ( i=digestNextQuote ( in, i, sb, true, false ) ) >= 0 ) {
                num++;
            } 
            // only return unquoted value, 
            // if the enclosure quotes represent a single atom.
            if(num==1) {
                return sb;
            }
        }
        return in;
    }

    public final static String unquote(String in) {
        if (in == null) return null;
        return unquote(new StringBuffer(in)).toString();
    }

    public final static StringBuffer unquote(StringBuffer in) {
        if (in == null) return null;
        StringBuffer sb=new StringBuffer();
        int i=0;

        while ( ( i=digestNextQuote ( in, i, sb, true, false ) ) >= 0 ) {
            // nothing to do ..
        } 
        return sb;
    }

    public final static String removeQuotedPart(String in) {
        if (in == null) return null;
        return removeQuotedPart(new StringBuffer(in)).toString();
    }

    public final static StringBuffer removeQuotedPart(StringBuffer in) {
        if (in == null) return null;
        StringBuffer sb=new StringBuffer();
        int i=0;

        while ( ( i=digestNextQuote ( in, i, null, false, true ) ) >= 0 ) {
            // nothing to do ..
        } 
        return in;
    }

    public final static String quoteIfNeeded(String in) {
        if (in == null) return null;
        return quoteIfNeeded(new StringBuffer(in)).toString();
    }

    public final static StringBuffer quoteIfNeeded(StringBuffer in) {
        if (in == null)  return null;
        if(!needsQuote(in)) {
            // no quoting necessary at all
            return in;
        }
        int inLen = in.length();
        if (inLen<2) {
            // not quoted -> quote
            return quote(in);
        }

        // remove quoted part and see if quoting is necessary for the remainder
        StringBuffer in2=new StringBuffer(in.toString());
        int i=0, num=0;
        while ( ( i=digestNextQuote ( in2, i, null, false, true ) ) >= 0 ) {
            // nop
        } 
        if(needsQuote(in2)) {
            return quote(in);
        }
        // no quoting necessary at all
        return in;
    }

    // delivers a list of strings,
    // each containing one command-line argument - unquoted.
    public final static List/*String*/ getCommandLineArgs(String commandline) {
        List/*String*/ args = new ArrayList();
        if(commandline==null) {
            return args;
        }
        StringBuffer in = new StringBuffer(commandline);
        int inLen = in.length();
        StringBuffer sb=null;

        int i;
        char c; // current char
        boolean inArgument = false;

        for (i=0; i < inLen; i++) {
            c   = in.charAt(i);

            if ( !inArgument ) {
                if(sb!=null) {
                    throw new IllegalArgumentException("ooops");
                }
                if ( !isWhiteSpace(c) ) { 
                    // start of argument
                    inArgument=true;
                    sb = new StringBuffer();
                    // push back last char to inArgument processing
                    i--;
                } // else just skip white spaces
            } else {
                if(sb==null) {
                    throw new IllegalArgumentException("ooops");
                }
                if(isWhiteSpace(c)) {
                    inArgument=false;
                } else if(isQuote(c)) { 
                    // digest quoted ..
                    int j = digestNextQuote ( in, i, sb, true, false );
                    if(i<0) {
                        throw new IllegalArgumentException("Quote Error: "+i+".."+j+", size: "+inLen+
                                    ",\n\t input: "+in+
                                    ",\n\tincomplete argument: "+sb);
                    }
                    i = j - 1; // correct for increment at end of loop
                } else {
                    // any char
                    sb.append(c);        
                }
                if(!inArgument || i==inLen-1) {
                    args.add(sb.toString());
                    inArgument=false;
                    sb=null;
                }
            }
        }
        if(sb!=null) {
            throw new IllegalArgumentException("Quote Error: at end: "+i+", size: "+inLen+
                    ",\n\t input: "+in+",\n\t incomplete argument: "+sb);
        }
        return args;
    }

    public final static String getStringByCommandList(List/*String*/ args) {
        StringBuffer buf = new StringBuffer();
        String stmp;
        boolean addSpace = false;
        for (Iterator iter = args.iterator(); iter.hasNext(); ) {
            if (addSpace) {
                buf.append(" ");
            } else {
                addSpace = true;
            }
            stmp = (String) iter.next();
            buf.append(quoteIfNeeded(stmp));
        }
        return buf.toString();
    }

    public static void main(String[] args) {
        System.exit(test()?0:1);
    }

    // special quoting:

    protected final static String _winPathBugVal       = "C:\\windows\\Documents, and, Stuff\\" ;
    protected final static String _winPathFixedVal     = "C:\\windows\\Documents, and, Stuff" ;
    protected final static String _winPathQuotedBugVal = "\"C:\\windows\\Documents, and, Stuff\\\"" ;

    protected final static String[] _testCleanVal       =  {
        "C:\\windows\\Documents, and, Stuff",                         // C:\windows\Documents, and, Stuff\
        "C:\\windows\\Documents, and, Stuff\\jre.exe" };

    protected final static String[] _testCmdLine       =  {
        "-Dababa=2222",
        "-Dlala=C:\\windows\\Documents, and, Stuff",
        "-DresponseString.0=yes",
        "-DwinPath=c:\\windows\\Documents, and Stuff\\java.exe",
        "-DexecString=/bin/sh {1} {0} {2}" };

    protected final static String[] _testPropVal       =  {
        "C:\\windows\\Documents, and, Stuff",                     // C:\windows\Documents, and, Stuff
        "-Dlala=C:\\windows\\Documents, and, Stuff",              // -Dlala="C:\windows\Documents, and, Stuff
        "C:\\windows\\Documents, and, Stuff\\jre.exe",
        };

    protected final static String[] _testPropValQuoted = {
        "\"C:\\windows\\Documents, and, Stuff\"",                 // "C:\\windows\\Documents, and, Stuff"
        "\"-Dlala=C:\\windows\\Documents, and, Stuff\"",          // "-Dlala=C:\\windows\\Documents, and, Stuff"
        "\"C:\\windows\\Documents, and, Stuff\\jre.exe\"",
        };


    protected final static String _testJvmArgs = "-DplatformVersion=1.5 -DresponseString.0=yes -Dverbose=true -DinstallerEntry=jre.dat \"-DexecString=/bin/sh {1} {0} {2}\" -DjavaPath=jre1.5.0_11/bin/java -DlicenseEntry=LICENSE \"-DwaitString.0=[yes or no]\" -DinstallerLocation=javaws-1_0_1-j2re-1_5_0_11-data-linux-i586.jar -DjavaVersion=1.5.0_11 -DisSolarisInstall=yes \"-DwinPath=c:\\windows\\Documents, and Stuff\\java.exe\""; // 12
    protected final static String _testJvmArgsQuoted = "\"-DplatformVersion=1.5 -DresponseString.0=yes -Dverbose=true -DinstallerEntry=jre.dat \\\"-DexecString=/bin/sh {1} {0} {2}\\\" -DjavaPath=jre1.5.0_11/bin/java -DlicenseEntry=LICENSE \\\"-DwaitString.0=[yes or no]\\\" -DinstallerLocation=javaws-1_0_1-j2re-1_5_0_11-data-linux-i586.jar -DjavaVersion=1.5.0_11 -DisSolarisInstall=yes \\\"-DwinPath=c:\\windows\\Documents, and Stuff\\java.exe\\\"\"";

    protected final static String _testJvmArgsUnix = "-DplatformVersion=1.5 -DresponseString.0=yes -Dverbose=true -DinstallerEntry=jre.dat -DexecString=\"/bin/sh {1} {0} {2}\" -DjavaPath=jre1.5.0_11/bin/java -DlicenseEntry=LICENSE -DwaitString.0=\"[yes or no]\" -DinstallerLocation=javaws-1_0_1-j2re-1_5_0_11-data-linux-i586.jar -DjavaVersion=1.5.0_11 -DisSolarisInstall=yes -DwinPath=\"c:\\windows\\Documents, and Stuff\\java.exe\""; // 12
    protected final static String _testJvmArgsUnixQuoted = "\"-DplatformVersion=1.5 -DresponseString.0=yes -Dverbose=true -DinstallerEntry=jre.dat -DexecString=\\\"/bin/sh {1} {0} {2}\\\" -DjavaPath=jre1.5.0_11/bin/java -DlicenseEntry=LICENSE -DwaitString.0=\\\"[yes or no]\\\" -DinstallerLocation=javaws-1_0_1-j2re-1_5_0_11-data-linux-i586.jar -DjavaVersion=1.5.0_11 -DisSolarisInstall=yes -DwinPath=\\\"c:\\windows\\Documents, and Stuff\\java.exe\\\"\"";

    /** Small test harness. */
    protected static boolean test() {
        String jvmArgProperty = "-Djnlpx.vmargs="+_testJvmArgs; // 1
        String jvmArgs2 = quoteIfNeeded(jvmArgProperty) + " " + _testJvmArgs; // 12 + 1
        String stmp;
        boolean ok = true;

        // test quote/unquote windows path bug
        stmp = StringQuoteUtil.unquote(StringQuoteUtil.quote(_winPathBugVal));
        if( stmp.equals(_winPathFixedVal)) {
            System.out.println("Test 0.0 passed");
        } else {
            System.out.println("Test 0.0 FAILED");
            System.out.println("\t orig     : <"+_winPathBugVal+">");
            System.out.println("\t expected : <"+_winPathFixedVal+">");
            System.out.println("\t result   : <"+stmp+">");
        }
        stmp = StringQuoteUtil.unquote(_winPathBugVal);
        if( stmp.equals(_winPathFixedVal)) {
            System.out.println("Test 0.1 passed");
        } else {
            System.out.println("Test 0.1 FAILED");
            System.out.println("\t orig     : <"+_winPathBugVal+">");
            System.out.println("\t expected : <"+_winPathFixedVal+">");
            System.out.println("\t result   : <"+stmp+">");
        }

        stmp = StringQuoteUtil.unquoteIfEnclosedInQuotes(_winPathQuotedBugVal);
        if( stmp.equals(_winPathFixedVal)) {
            System.out.println("Test 0.2 passed");
        } else {
            System.out.println("Test 0.2 FAILED");
            System.out.println("\t orig     : <"+_winPathQuotedBugVal+">");
            System.out.println("\t expected : <"+_winPathFixedVal+">");
            System.out.println("\t result   : <"+stmp+">");
        }

        // test unquote with unquoted values
        for(int i = 0; i<_testCleanVal.length; i++) {
            stmp = StringQuoteUtil.unquote(_testCleanVal[i]);
            if(stmp.equals(_testCleanVal[i])) {
                System.out.println("Test 1."+i+" passed");
            } else {
                System.out.println("Test 1."+i+" FAILED");
                System.out.println("\t expected : <"+_testCleanVal[i]+">");
                System.out.println("\t result   : <"+stmp+">");
                ok = false;
            }
        }
        // test quote/unquote reversibility
        for(int i = 0; i<_testPropVal.length; i++) {
            stmp = StringQuoteUtil.quote(_testPropVal[i]);
            if(stmp.equals(_testPropValQuoted[i])) {
                System.out.println("Test 2."+i+".0 passed");
            } else {
                System.out.println("Test 2."+i+".0 FAILED");
                System.out.println("\t orig     : <"+_testPropVal[i]+">");
                System.out.println("\t expected : <"+_testPropValQuoted[i]+">");
                System.out.println("\t result   : <"+stmp+">");
                ok = false;
            }

            stmp = StringQuoteUtil.unquoteIfEnclosedInQuotes(_testPropValQuoted[i]);
            if(stmp.equals(_testPropVal[i])) {
                System.out.println("Test 2."+i+".1 passed");
            } else {
                System.out.println("Test 2."+i+".1 FAILED");
                System.out.println("\t orig     : <"+_testPropValQuoted[i]+">");
                System.out.println("\t expected : <"+_testPropVal[i]+">");
                System.out.println("\t result   : <"+stmp+">");
                ok = false;
            }
        }

        stmp = StringQuoteUtil.quote(_testJvmArgs);
        if(stmp.equals(_testJvmArgsQuoted)) {
            System.out.println("Test 3.1 passed");
        } else {
            System.out.println("Test 3.1 FAILED");
            System.out.println("\t orig     : <"+_testJvmArgs+">");
            System.out.println("\t expected : <"+_testJvmArgsQuoted+">");
            System.out.println("\t result   : <"+stmp+">");
            ok = false;
        }
        stmp = StringQuoteUtil.unquoteIfEnclosedInQuotes(_testJvmArgsQuoted);
        if(stmp.equals(_testJvmArgs)) {
            System.out.println("Test 3.2 passed");
        } else {
            System.out.println("Test 3.2 FAILED");
            System.out.println("\t orig     : <"+_testJvmArgsQuoted+">");
            System.out.println("\t expected : <"+_testJvmArgs+">");
            System.out.println("\t result   : <"+stmp+">");
            ok = false;
        }

        stmp = StringQuoteUtil.quote(_testJvmArgsUnix);
        if(stmp.equals(_testJvmArgsUnixQuoted)) {
            System.out.println("Test 3.3 passed");
        } else {
            System.out.println("Test 3.3 FAILED");
            System.out.println("\t orig     : <"+_testJvmArgsUnix+">");
            System.out.println("\t expected : <"+_testJvmArgsUnixQuoted+">");
            System.out.println("\t result   : <"+stmp+">");
            ok = false;
        }
        stmp = StringQuoteUtil.unquoteIfEnclosedInQuotes(_testJvmArgsUnixQuoted);
        if(stmp.equals(_testJvmArgsUnix)) {
            System.out.println("Test 3.4 passed");
        } else {
            System.out.println("Test 3.4 FAILED");
            System.out.println("\t orig     : <"+_testJvmArgsUnixQuoted+">");
            System.out.println("\t expected : <"+_testJvmArgsUnix+">");
            System.out.println("\t result   : <"+stmp+">");
            ok = false;
        }

        final String _twoQuotes = "\"-Dlala=li li\" -Dlulu=\"da da\"";
        stmp = StringQuoteUtil.unquoteIfEnclosedInQuotes(_twoQuotes);
        if(stmp.equals(_twoQuotes)) {
            System.out.println("Test 3.5 passed");
        } else {
            System.out.println("Test 3.5 FAILED");
            System.out.println("\t expected : <"+_twoQuotes+">");
            System.out.println("\t result   : <"+stmp+">");
            ok = false;
        }

        List/*String*/ argList;

        // test single simple commandline args
        for(int i = 0; i<_testCmdLine.length; i++) {
            argList = StringQuoteUtil.getCommandLineArgs(quoteIfNeeded(_testCmdLine[i]));
            if(argList.size()==1 && _testCmdLine[i].equals((String)argList.get(0))) {
                System.out.println("Test 4.0."+i+" passed");
            } else {
                System.out.println("Test 4.0."+i+" FAILED");
                if(argList.size()!=1) {
                    System.out.println("\t input : <"+_testCmdLine[i]+">");
                    System.out.println("\t cmdline number not 1, is "+argList.size());
                    for(int j=0; j<argList.size(); j++) {
                        System.out.println("\t"+j+": "+(String)argList.get(j));
                    }
                } else {
                    String stmp2 = (String)argList.get(0);
                    System.out.println("\t input : <"+_testCmdLine[i]+">");
                    System.out.println("\t result: <"+stmp2+">");
                }
                ok = false;
            }
        }

        // test concat simple commandline args
        List _testCmdLineList = new ArrayList();
        for(int i = 0; i<_testCmdLine.length; i++) {
            _testCmdLineList.add(_testCmdLine[i]);
        }
        String _testCmdLineString = getStringByCommandList(_testCmdLineList);
        argList = StringQuoteUtil.getCommandLineArgs(_testCmdLineString);
        int j = argList.size();

        if(argList.size()==_testCmdLine.length) {
            System.out.println("Test 4.1._ passed");
        } else {
            System.out.println("Test 4.1._ failed");
            System.out.println("\t input        : "+_testCmdLineString);
            System.out.println("\t expected size: "+_testCmdLine.length);
            System.out.println("\t argList  size: "+argList.size());
            if(_testCmdLine.length<j) j=_testCmdLine.length;
        }
        for(int i = 0; i<j; i++) {
            if(_testCmdLine[i].equals((String)argList.get(i))) {
                System.out.println("Test 4.1."+i+" passed");
            } else {
                System.out.println("Test 4.1."+i+" FAILED");
                String stmp2 = (String)argList.get(i);
                System.out.println("\t expected: <"+_testCmdLine[i]+">");
                System.out.println("\t result  : <"+stmp2+">");
                ok = false;
            }
        }

        argList = StringQuoteUtil.getCommandLineArgs(_testJvmArgs);
        if(argList.size()==12)  {
            System.out.println("Test 4.2.0 passed");
        } else {
            System.out.println("Test 4.2.0 FAILED");
            System.out.println("\t expected number : 12");
            System.out.println("\t result   number : "+argList.size());
            ok = false;
        }
        for(int i = 0; i<argList.size(); i++) {
            String arg = (String) argList.get(i);
            System.out.println("Test 4.2."+(i+1)+" result : <"+arg+">");
        }

        argList = StringQuoteUtil.getCommandLineArgs(_testJvmArgsUnix);
        if(argList.size()==12)  {
            System.out.println("Test 4.3.0 passed");
        } else {
            System.out.println("Test 4.3.0 FAILED");
            System.out.println("\t expected number : 12");
            System.out.println("\t result   number : "+argList.size());
            ok = false;
        }
        for(int i = 0; i<argList.size(); i++) {
            String arg = (String) argList.get(i);
            System.out.println("Test 4.3."+(i+1)+" result : <"+arg+">");
        }

        argList = StringQuoteUtil.getCommandLineArgs(jvmArgs2);
        if(argList.size()==13)  {
            System.out.println("Test 4.4.0 passed");
        } else {
            System.out.println("Test 4.4.0 FAILED");
            System.out.println("\t expected number : 13");
            System.out.println("\t result   number : "+argList.size());
            ok = false;
        }
        for(int i = 0; i<argList.size(); i++) {
            String arg = (String) argList.get(i);
            switch(i) {
                case 0: // jvmArgProperty
                    if(arg.equals(jvmArgProperty)) {
                        System.out.println("Test 4.4.1 passed");
                    } else {
                        System.out.println("Test 4.4.1 FAILED");
                        System.out.println("\t expected : <"+jvmArgProperty+">");
                        System.out.println("\t result   : <"+arg+">");
                        ok = false;
                    }
                    break;
                default: // .. yeah, right 
                    System.out.println("Test 4.4."+(i+1)+" result : <"+arg+">");
                    break;
            }
        }
        return ok;
    }
}
