/*
 * @(#)CompileProperties.java	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// Compile with "javac CompileProperties.java" using JDK 1.4 or higher.

import java.io.*;
import java.text.*;
import java.util.*;

/** Translates a .properties file into a .java file containing the
 *  definition of a java.util.Properties subclass which can then be
 *  compiled with javac. <P>
 *
 *  Usage: java CompileProperties [path to .properties file] [path to .java file to be output] [super class]
 *
 *  Infers the package by looking at the common suffix of the two
 *  inputs, eliminating "classes" from it.
 *
 * @author Scott Violet
 * @author Kenneth Russell
 */

public class CompileProperties {
    private static final String FORMAT =
              "{0}" +
              "import java.util.ListResourceBundle;\n\n" +
              "public final class {1} extends {2} '{'\n" +
              "    protected final Object[][] getContents() '{'\n" +
              "        return new Object[][] '{'\n" +
              "{3}" +
              "        };\n" +
              "    }\n" +
              "}\n";


    // This comes from Properties
    private static final char[] hexDigit = {
	'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };

    // Note: different from that in Properties
    private static final String specialSaveChars = "\"";

    // This comes from Properties
    private static char toHex(int nibble) {
	return hexDigit[(nibble & 0xF)];
    }

    private static String propfiles[];
    private static String outfiles[] ;
    private static String supers[]   ;
    private static int compileCount = 0;

    private static void parseOptions(String args[])
    {
        if ( compileCount > 0 ) {
	    String new_propfiles[] = new String[compileCount + args.length];
	    String new_outfiles[]  = new String[compileCount + args.length];
	    String new_supers[]    = new String[compileCount + args.length];
	    System.arraycopy(propfiles, 0, new_propfiles, 0, compileCount);
	    System.arraycopy(outfiles, 0, new_outfiles, 0, compileCount);
	    System.arraycopy(supers, 0, new_supers, 0, compileCount);
	    propfiles = new_propfiles;
	    outfiles  = new_outfiles;
	    supers    = new_supers;
	} else {
	    propfiles = new String[args.length];
	    outfiles  = new String[args.length];
	    supers    = new String[args.length];
	}
	for ( int i = 0; i < args.length ; i++ ) {
	    if ( "-compile".equals(args[i]) && i+3 < args.length ) {
		propfiles[compileCount] = args[++i];
		outfiles[compileCount]  = args[++i];
		supers[compileCount]    = args[++i];
		compileCount++;
	    } else if ( "-optionsfile".equals(args[i]) && i+1 < args.length ) {
		String filename = args[++i];
		FileInputStream finput;
		byte contents[];
		try {
	            finput = new FileInputStream(filename);
		    int byteCount = finput.available();
		    if ( byteCount <= 0 ) {
			break;
		    }
		    contents = new byte[byteCount];
		    int bytesRead = finput.read(contents);
		    if ( byteCount != bytesRead ) {
			System.err.println("ERROR: Cannot read all of -optionsfile file");
			usage();
		    }
		} catch ( IOException e ) {
		    System.err.println("ERROR: CompileProperties cannot open " + filename);
                    usage();
		    break;
		}
		String tokens[] = (new String(contents)).split("\\s+");
		if ( tokens.length > 0 ) {
		    parseOptions(tokens);
		}
	    } else {
		System.err.println("ERROR: CompileProperties argument error");
                usage();
	    }
	}
    }
    
    public static void main(String[] args) throws IOException {

	/* Original usage */
        if (args.length == 2 && args[0].charAt(0) != '-' ) {
            createFile(args[0], args[1], "ListResourceBundle");
	    return;
        }
        if (args.length == 3) {
            createFile(args[0], args[1], args[2]);
	    return;
        }
        if (args.length == 0) {
            usage();
        }

	/* New batch usage */
	parseOptions(args);

	/* Need at least one file. */
        if ( compileCount == 0 ) {
            usage();
        }

	/* Process files */
	for ( int i = 0; i < compileCount ; i++ ) {
            createFile(propfiles[i], outfiles[i], supers[i]);
	}
    }

    private static void usage() {
        System.err.println("usage:");
        System.err.println("    java CompileProperties path_to_properties_file path_to_java_output_file [super_class]");
        System.err.println("      -OR-");
        System.err.println("    java CompileProperties {-compile path_to_properties_file path_to_java_output_file super_class} -or- -optionsfile filename");
        System.err.println("");
        System.err.println("Example:");
        System.err.println("    java CompileProperties -compile test.properties test.java ListResourceBundle");
        System.err.println("    java CompileProperties -optionsfile option_file");
        System.err.println("option_file contains: -compile test.properties test.java ListResourceBundle");
        System.exit(1);
    }

    private static void createFile(String propertiesPath, String outputPath, String superClass)
            throws IOException {
        System.out.println("parsing: " + propertiesPath);
        Properties p = new Properties();

        p.load(new FileInputStream(propertiesPath));

        String packageName = inferPackageName(propertiesPath, outputPath);
        System.out.println("inferred package name: " + packageName);

        ArrayList sortedKeys = new ArrayList(p.keySet());
        Collections.sort(sortedKeys);
        Iterator keys = sortedKeys.iterator();

        StringBuffer data = new StringBuffer();

        while (keys.hasNext()) {
            Object key = keys.next();
            data.append("            { \"" + escape((String)key) + "\", \"" +
                        escape((String)p.get(key)) + "\" },\n");
        }

        File file = new File(propertiesPath);
        String name = file.getName();
        int dotIndex = name.lastIndexOf('.');
        String className;
        if (dotIndex == -1) {
            className = name;
        } else {
            className = name.substring(0, dotIndex);
        }
        
        String packageString = "";
        if (packageName != null && !packageName.equals("")) {
          packageString = "package " + packageName + ";\n\n";
        }

        Writer writer = new BufferedWriter(
            new OutputStreamWriter(new FileOutputStream(outputPath), "8859_1"));
        MessageFormat format = new MessageFormat(FORMAT);
        writer.write(format.format(new Object[] { packageString, className, superClass, data }));
        writer.flush();
        writer.close();
        System.out.println("wrote: " + outputPath);
    }

    private static String escape(String theString) {
        boolean escapeSpace = false;
        // This is taken from Properties.saveConvert with changes for Java strings
        int len = theString.length();
        StringBuffer outBuffer = new StringBuffer(len*2);

        for(int x=0; x<len; x++) {
            char aChar = theString.charAt(x);
            switch(aChar) {
                case '\\':outBuffer.append('\\'); outBuffer.append('\\');
                          break;
                case '\t':outBuffer.append('\\'); outBuffer.append('t');
                          break;
                case '\n':outBuffer.append('\\'); outBuffer.append('n');
                          break;
                case '\r':outBuffer.append('\\'); outBuffer.append('r');
                          break;
                case '\f':outBuffer.append('\\'); outBuffer.append('f');
                          break;
                default:
                    if ((aChar < 0x0020) || (aChar > 0x007e)) {
                        outBuffer.append('\\');
                        outBuffer.append('u');
                        outBuffer.append(toHex((aChar >> 12) & 0xF));
                        outBuffer.append(toHex((aChar >>  8) & 0xF));
                        outBuffer.append(toHex((aChar >>  4) & 0xF));
                        outBuffer.append(toHex( aChar        & 0xF));
                    } else {
                        if (specialSaveChars.indexOf(aChar) != -1)
                            outBuffer.append('\\');
                        outBuffer.append(aChar);
                    }
            }
        }
        return outBuffer.toString();
    }

    private static String inferPackageName(String inputPath, String outputPath) {
        // Normalize file names
        inputPath  = new File(inputPath).getPath();
        outputPath = new File(outputPath).getPath();
        // Split into components
        String sep;
        if (File.separatorChar == '\\') {
            sep = "\\\\";
        } else {
            sep = File.separator;
        }
        String[] inputs  = inputPath.split(sep);
        String[] outputs = outputPath.split(sep);
        // Match common names, eliminating first "classes" entry from
        // each if present
        int inStart  = 0;
        int inEnd    = inputs.length - 2;
        int outEnd   = outputs.length - 2;
        int i = inEnd;
        int j = outEnd;
        while (i >= 0 && j >= 0) {
            if (!inputs[i].equals(outputs[j]) ||
                (inputs[i].equals("gensrc") && inputs[j].equals("gensrc"))) {
                ++i;
                ++j;
                break;
            }
            --i;
            --j;
        }
        if (i < 0 || j < 0 || i >= inEnd || j >= outEnd) {
            return "";
        }
        if (inputs[i].equals("classes") && outputs[j].equals("classes")) {
            ++i;
            ++j;
        }
        inStart = i;
        StringBuffer buf = new StringBuffer();
        for (i = inStart; i <= inEnd; i++) {
            buf.append(inputs[i]);
            if (i < inEnd) {
                buf.append('.');
            }
        }
        return buf.toString();
    }
}
