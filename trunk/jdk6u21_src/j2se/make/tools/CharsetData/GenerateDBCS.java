/*
 * @(#)GenerateDBCS.java	1.2 10/03/23
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.io.*;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.Formatter;
import java.util.regex.*;
import java.nio.charset.*;
import static sun.nio.cs.CharsetMapping.*;

public class GenerateDBCS {
    public static void main(String args[]) throws Exception {
        if (args.length < 3) {
            System.err.println("Usage: java GenerateDBCS srcDir dstDir config");
            System.exit(1);
        }

        Scanner s = new Scanner(new File(args[0], args[2]));
        while (s.hasNextLine()) {
            String line = s.nextLine();
            if (line.startsWith("#") || line.length() == 0)
                continue;
            String[] fields = line.split("\\s+");
            if (fields.length < 11) {
                System.err.println("Misconfiged sbcs line <" + line + ">?");
                continue;
            }
            String clzName = fields[0];
            String csName  = fields[1];
            String hisName = ("null".equals(fields[2]))?null:fields[2];
            String baseClz = fields[3];    //Doublebyte, EBCDIC...
            String pkgName  = fields[4];
            int    b1MinDB1 = toInteger(fields[5]);
            int    b1MaxDB1 = toInteger(fields[6]);
            int    b1MinDB2 = toInteger(fields[7]);
            int    b1MaxDB2 = toInteger(fields[8]);
            int    b2Min    = toInteger(fields[9]);
            int    b2Max    = toInteger(fields[10]);

            StringBuilder b2c = new StringBuilder();
            int c2bLen = genB2C(new File(args[0], clzName+".map"), 
                                b2c,
                                mappingPattern,
                                b1MinDB1, b1MaxDB1, b1MinDB2, b1MaxDB2, b2Min, b2Max);

            String b2cNR = genNR(new File(args[0], clzName+".nr"), mappingPattern, "b2cNR");
            String c2bNR = genNR(new File(args[0], clzName+".c2b"), mappingPattern, "c2bNR");

            genClass(args[0], args[1], "DBCS-X.java",
                     clzName, csName, hisName, pkgName,
                     false, baseClz,
                     b2c.toString(), b2cNR, c2bNR, c2bLen,
                     b2Min, b2Max);
        }
    }

    private static int toInteger(String s) {
        if (s.startsWith("0x") || s.startsWith("0X"))
            return Integer.valueOf(s.substring(2), 16);
        else
            return Integer.valueOf(s);
    }

    private static void outString(Formatter out, 
                                  char[] cc, int off, int end,
                                  String closure) {
        while (off < end) {
            out.format("            \"");
            for (int j = 0; j < 8; j++) {
                if (off == end)
                    break;
                char c = cc[off++];
                switch (c) {
                case '\b':
                    out.format("\\b"); break;
                case '\t':
                    out.format("\\t"); break;
                case '\n':
                    out.format("\\n"); break;
                case '\f':
                    out.format("\\f"); break;
                case '\r':
                    out.format("\\r"); break;
                case '\"':
                    out.format("\\\""); break;
                case '\'':
                    out.format("\\'"); break;
                case '\\':
                    out.format("\\\\"); break;
                default:
                    out.format("\\u%04X", c & 0xffff);
                }
            }
            if (off == end)
                out.format("\" %s%n", closure);
            else
                out.format("\" + %n");
        }
    }

    private static void outString(Formatter out,
                                  char[] db,
                                  int b1Min, int b1Max,
                                  int b2Min, int b2Max,
                                  String closure) {
        char[] cc = new char[(b1Max - b1Min + 1) * (b2Max - b2Min + 1) + 1];
        int off = 0;
        for (int b1 = b1Min; b1 <= b1Max; b1++) {
            int b2 = b2Min;
            while (b2 <= b2Max) {
                cc[off++] = db[(b1 << 8) | (b2++)];
            }
        }
	outString(out, cc, 0, cc.length, closure);
    }

    static Pattern mappingPattern = Pattern.compile("(\\p{XDigit}++)\\s++(\\p{XDigit}++)(\\s++#.*)?");
    private static int genB2C(File f,
                              StringBuilder out,
                              Pattern mPattern,
                              int b1MinDB1, int b1MaxDB1,
                              int b1MinDB2, int b1MaxDB2,
                              int b2Min, int b2Max)
        throws Exception
    {
        FileInputStream in = new FileInputStream(f);
        char[] db = new char[0x10000];
        char[] indexC2B = new char[0x100];
        Arrays.fill(db, UNMAPPABLE_DECODING);
        Arrays.fill(indexC2B, UNMAPPABLE_DECODING);
        int    off = 0x100;    // first 0x100 for unmappable segs

        // parse the b2c mapping table
        Parser p = new Parser(in, mPattern);
        Entry  e = null;
        while ((e = p.next()) != null) {
            db[e.bs] = (char)e.cp;
            if (indexC2B[e.cp>>8] == UNMAPPABLE_DECODING) {
                off += 0x100;
                indexC2B[e.cp>>8] = 1;
            }
        }

        Formatter fm = new Formatter(out);
        fm.format("%n        static final String SB = %n");
        outString(fm, db, 0x00, 0x100,  ";");

        fm.format("%n        static final String[] DB = {%n");
        for (int i = 0; i < 0x100; i += 0x10 ) {
	    if ((i < b1MinDB1) ||
                (i > b1MaxDB1 && i < b1MinDB2) ||
                (i > b1MaxDB2)) {
                fm.format("            null,%n");    // unmappable segments
            } else {
                outString(fm, db, i, i + 0xf, b2Min, b2Max, ",");
                fm.format("%n");
            }
        }
        fm.format("        };%n");

        fm.close();
        return off;
    }

    // generate non-roundtrip entries from xxx.nr and xxx.c2b file
    private static String genNR(File f, Pattern mPattern, String name)
        throws Exception
    {
        if (!f.exists())
            return "static final String " + name + " = null;";
        InputStream in = new FileInputStream(f);
        StringBuilder sb = new StringBuilder();
        Parser p = new Parser(in, mPattern);
        Entry  e = null;
        while ((e = p.next()) != null) {
	    // A <b,c> pair
            sb.append((char)e.bs);
            sb.append((char)e.cp);
        }
        char[] nr = sb.toString().toCharArray();
        Formatter fm = new Formatter(sb.delete(0, sb.length()));
        fm.format("static final String %s = %n", name);
        outString(fm, nr, 0, nr.length,  ";");
        fm.close();
        return sb.toString();
    }

    private static void genClass(String srcDir, String dstDir,
                                 String template,
                                 String clzName,
                                 String csName,
                                 String hisName,
                                 String pkgName,
                                 boolean isASCII,
                                 String baseClz,
                                 String b2c, String b2cNR, String c2bNR, int c2blen,
                                 int b2Min, int b2Max)
        throws Exception
    {
        Scanner s = new Scanner(new File(srcDir, template));
        PrintStream out = new PrintStream(new FileOutputStream(
                              new File(dstDir, clzName + ".java")));

        while (s.hasNextLine()) {
            String line = s.nextLine();
            int i = line.indexOf("$");
            if (i == -1) {
                out.println(line);
                continue;
            }
            if (line.indexOf("$PACKAGE$", i) != -1) {
                line = line.replace("$PACKAGE$", pkgName);
            }
            if (line.indexOf("$NAME_CLZ$", i) != -1) {
                line = line.replace("$NAME_CLZ$", clzName);
            }
            if (line.indexOf("$IMPLEMENTS$", i) != -1) {
                if (hisName == null)
                    line = "";
                else
                    line = line.replace("$IMPLEMENTS$",
                                        "implements HistoricallyNamedCharset");
            }
            if (line.indexOf("$NAME_CS$", i) != -1) {
                line = line.replace("$NAME_CS$", csName);
            }
            if (line.indexOf("$NAME_ALIASES$", i) != -1) {
                if ("sun.nio.cs".equals(pkgName))
                    line = line.replace("$NAME_ALIASES$",
                                        "StandardCharsets.aliases_" + clzName);
                else
                    line = line.replace("$NAME_ALIASES$",
                                        "ExtendedCharsets.aliasesFor(\"" + csName + "\")");
            }
            if (line.indexOf("$HISTORICALNAME$", i) != -1) {
                if (hisName != null)
                    line = "    public String historicalName() { return" + hisName + "; }";
                else
                    line = "";
            }
            if (line.indexOf("$CONTAINS$", i) != -1) {
                if (isASCII)
                    line = "        return ((cs.name().equals(\"US-ASCII\")) || (cs instanceof " + clzName + "));";
                else
                    line = "        return (cs instanceof " + clzName + ");";
            }
            if (line.indexOf("$DBCS_BASE$") != -1) {
                line = line.replace("$DBCS_BASE$", baseClz);
            }
            if (line.indexOf("$B2CTABLE$") != -1) {
                line = line.replace("$B2CTABLE$", b2c);
            }
            if (line.indexOf("$C2BLENGTH$") != -1) {
                line = line.replace("$C2BLENGTH$", "0x" + Integer.toString(c2blen, 16));
            }
            if (line.indexOf("$NONROUNDTRIP_B2C$") != -1) {
                if (b2cNR == null)
                    continue;
                line = line.replace("$NONROUNDTRIP_B2C$", b2cNR);
            }
            if (line.indexOf("$NONROUNDTRIP_C2B$") != -1) {
                if (c2bNR == null)
                    continue;
                line = line.replace("$NONROUNDTRIP_C2B$", c2bNR);
            }

            if (line.indexOf("$B2MIN$") != -1) {
                line = line.replace("$B2MIN$", "0x" + Integer.toString(b2Min, 16));
            }

            if (line.indexOf("$B2MAX$") != -1) {
                line = line.replace("$B2MAX$", "0x" + Integer.toString(b2Max, 16));
            }

            out.println(line);
        }
        out.close();
    }
}
