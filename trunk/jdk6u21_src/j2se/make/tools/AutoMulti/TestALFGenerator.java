/*
 * @(#)TestALFGenerator.java	1.7 10/04/01
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
import java.lang.reflect.*;
import java.util.*;
import java.io.*;

/**
 * Automatically generates an auxiliary look and feel to be
 * used for testing the Multiplexing look and feel.
 * <p>
 * To use, type 'java TestALFGenerator <plafdir> [<package>]' where <plafdir>
 * is the directory containing the source for Swing's UI classes.
 * <package> is an optional argument that specifies the package
 * of the TestALF classes.  If it's omitted, the classes are in
 * the default package.
 * For example:
 *
 * <pre>
 * ../../../../build/solaris-sparc/bin/java TestALFGenerator ../../../../src/share/classes/javax/swing/plaf com.myco.myalaf
 * </pre>
 *
 * TestALFGenerator will scour the plaf directory for *UI.java files and
 * generate TestALF*UI.java files.
 * <p>
 * NOTE:  This tool depends upon the existence of <plafdir> and on the
 * compiled classes from <plafdir> being somewhere in the class path.
 *
 * @version 1.7 04/01/10 16:14:30
 * @author Willie Walker
 */
public class TestALFGenerator {
    static String importLines;
    static String packageName;
    static String classPrefix = "TestALF";

    /**
     * A silly list of parameter names to use.  Skips "i" because we use
     * it as a 'for' loop counter.  If you want to get fancy, please feel
     * to change how parameter names are obtained.  This will break if
     * someone decides to create a UI method that takes more than 8
     * parameters.  Which one is a bug (this breaking or having a method
     * with more than eight parameters) is a subjective thing.
     */
    public static String[] paramNames = {"a","b","c","d","e","f","g","h"};

    /**
     * Removes the package names (e.g., javax.swing) from the name.
     */
    public static String unqualifyName(String name) {
        StringTokenizer parser = new StringTokenizer(name,".");
        String unqualifiedName = null;
        while (parser.hasMoreTokens()) {
            unqualifiedName = parser.nextToken();
        }
        return removeDollars(unqualifiedName);
    }

    /**
     * Strips the extension from the filename.
     */
    public static String stripExtension(String name) {
        StringTokenizer parser = new StringTokenizer(name,".");
        return parser.nextToken();
    }

    /**
     * Adds some spaces.
     */
    public static void indent(StringBuffer s, int i) {
        while (i > 0) {
            s.append(" ");
	    i--;
        }
    }

    /**
     * Spits out all the beginning stuff.
     */
    public static StringBuffer createPreamble(String prefixName) {
	StringBuffer s = new StringBuffer();
	s.append("/*\n");
        s.append(" * %W" + "% %" + "E%\n"); // We don't want SCCS expanding this yet
        s.append(" *\n");
        s.append(" * Copyright (c) 1997, 2000, Oracle and/or its affiliates. All rights reserved.\n");
        s.append(" * \n");
        s.append(" * This software is the proprietary information of Oracle.\n");
        s.append(" * Use is subject to license terms.\n");
        s.append(" * \n");
        s.append(" */\n");
	if (packageName != null) {
            s.append("package " + packageName + ";\n");
            s.append("\n");
	}
	return s;
    }

    /**
     * Replaces 'Xxx$Yyy' with "Xxx'.  Used by addImport because you
     * can't import nested classes directly.
     */
    public static String removeNestedClassName(String s) {
        int dollarPosition = s.indexOf('$');

        if (dollarPosition >= 0) {    // s contains '$'
            StringBuffer sb = new StringBuffer(s);
            sb.setLength(dollarPosition);
            return sb.toString();
        } else {                      // no '$'
            return s;
	}
    }

    /**
     * Replaces '$' with ".'.  Needed for printing inner class names
     * for argument and return types.
     */
    public static String removeDollars(String s) {
        int dollarPosition = s.indexOf('$');

        if (dollarPosition >= 0) {    // s contains '$'
            StringBuffer sb = new StringBuffer(s);
            while (dollarPosition >= 0) {
                //XXX: will there ever be more than one '$'?
                sb.replace(dollarPosition, dollarPosition+1, ".");
                dollarPosition = sb.indexOf("$", dollarPosition);
            }
            return sb.toString();
        } else {                     // no $
            return s;
        }
    }

    /**
     * Adds an import line to the String.
     */
    public static void addImport(String s, Class theClass) {
	if (!theClass.isPrimitive() && (theClass != Object.class)) {
	    String className = removeNestedClassName(theClass.getName());
	    String importLine = new String("import " + className + ";\n");
	    if (importLines.indexOf(importLine) == -1) {
	    	importLines += importLine;
	    }
	}
    }

    /**
     * Spits out the class header information.
     */
    public static void addHeader(StringBuffer s, String className) {
	s.append("/**\n");
	s.append(" * An auxiliary UI for <code>" + className + "</code>s.\n");
        s.append(" * \n");
        s.append(" * <p>This file was automatically generated by TestALFGenerator.\n");
        s.append(" *\n");
        s.append(" * @version %" + "I% %" + "G% %" + "U%\n"); 	// Don't let SCCS expand this yet
        s.append(" * @author  Otto Multey\n");			// Get it?  I crack myself up.
        s.append(" */\n");
        s.append("public class " + classPrefix + className + " extends " + className + " {\n");
        s.append("\n");
    }

    /**
     * Prints out the code for a method.
     */
    public static void addMethod(StringBuffer s, Method m, String origName, String className) {

        // Get the method name and the return type.  Be a little careful about arrays.
        //
        String methodName = unqualifyName(m.getName());
        String returnType;

        if (!m.getReturnType().isArray()) {
            returnType = unqualifyName(m.getReturnType().toString());
	    addImport(importLines,m.getReturnType());
        } else {
            returnType = unqualifyName(m.getReturnType().getComponentType().toString())
		 	 + "[]";
	    addImport(importLines,m.getReturnType().getComponentType());
        }

        // Print the javadoc
        //
        s.append("\n");

	if (methodName.equals("createUI")) {
	    s.append("    /**\n");
	    s.append("     * Returns a UI object for this component.\n");
	    s.append("     */\n");
	} else {
	    s.append("    /**\n");
	    s.append("     * Prints a message saying this method has been invoked.\n");
	    s.append("     */\n");
	}

        // Print the method signature
        //
        s.append("    public");
        if (Modifier.isStatic(m.getModifiers())) {
            s.append(" static");
        }
        s.append(" " + returnType);
        s.append(" " + methodName);
        s.append("(");

        Class[] params = m.getParameterTypes();
	Class temp;
	String braces;
        for (int i = 0; i < params.length; i++) {
            if (i > 0) {
                s.append(", ");
            }
	    temp = params[i];
	    braces = new String("");
	    while (temp.isArray()) {
	        braces += "[]";
                temp = temp.getComponentType();
            }
            s.append(unqualifyName(temp.getName()) + braces + " " + paramNames[i]);
	    addImport(importLines,temp);
        }
        s.append(")");

        // Don't forget about exceptions
        //
        Class exceptions[] = m.getExceptionTypes();
	String throwsString = new String("");

        if (exceptions.length > 0) {
	    s.append("\n");
	    indent(s,12);
            s.append("throws ");
            for (int i = 0; i < exceptions.length; i++) {
                if (i > 0) {
                    s.append(", ");
                }
	        s.append(unqualifyName(exceptions[i].getName()));
	        addImport(importLines,exceptions[i]);
	    }
        }
        s.append(throwsString + " {\n");

        // Now print out the contents of the method.  
	indent(s,8);
	s.append("System.out.println(\"In the " + methodName
	                            + " method of the "
		                    + classPrefix + origName + " class.\");\n");
	if (methodName.equals("createUI")) {
	    indent(s,8);
	    s.append("return ui;\n");
	} else {
	    // If we have to return something, do so.
	    if (!returnType.equals("void")) {
	        Class rType = m.getReturnType();
	        indent(s,8);
	        if (!rType.isPrimitive()) {
		    s.append("return null;\n");
	        } else if (rType == Boolean.TYPE) {
		    s.append("return false;\n");
	        } else if (rType == Character.TYPE) {
		    s.append("return '0';\n");
	        } else {  // byte, short, int, long, float, or double
		    s.append("return 0;\n");
	        }
	    }
	}

	indent(s,4);
        s.append("}\n");
    }

    /**
     * Takes a plaf class name (e.g., "MenuUI") and generates the corresponding
     * TestALF UI Java source code (e.g., "TestALFMenuUI.java").
     */
    public static void generateFile(String prefixName, String className) {
        try {
            FileOutputStream fos;
            PrintWriter outFile;

	    importLines = new String();
	    importLines += new String("import java.util.Vector;\n");

	    StringBuffer body = new StringBuffer();
	    Class wee = Class.forName(prefixName + ".swing.plaf." + className);
	    String weeName = unqualifyName(wee.getName());
	    String thisClassName = classPrefix + className;
	    addImport(importLines,wee);

	    // Declare and initialize the shared UI object.
            body.append("\n");
            body.append("////////////////////\n");
	    body.append("// Shared UI object\n");
            body.append("////////////////////\n");
            body.append("private final static " + thisClassName 
			+ " ui = new " + thisClassName + "();\n");

	    while (!weeName.equals("Object")) {
                body.append("\n");
                body.append("////////////////////\n");
	        body.append("// " + weeName + " methods\n");
                body.append("////////////////////\n");
                Method[] methods = wee.getDeclaredMethods();
                for (int i=0; i < methods.length; i++) {
                    if (Modifier.isPublic(methods[i].getModifiers())) {
                        addMethod(body,methods[i],className,weeName);
                    }
                }
		wee = wee.getSuperclass();
                weeName = unqualifyName(wee.getName());
	        addImport(importLines,wee);
            }

	    fos = new FileOutputStream(classPrefix + className + ".java");
            outFile = new PrintWriter(fos);
	    StringBuffer outText = createPreamble(prefixName);
	    outText.append(importLines.toString() + "\n");
	    addHeader(outText,className);
	    outText.append(body.toString());
            outText.append("}\n");
	    outFile.write(outText.toString());
	    outFile.flush();
	    outFile.close();
        } catch (Exception e) {
	    System.err.println(e);
        }
    }

    /**
     * D'Oh!  Something bad happened.
     */
    public static void usage(String s) throws IOException {
	System.err.println("Usage:  java TestALFGenerator <plafdir> [<packageName>]");
	throw new IllegalArgumentException(s);
    }

    /**
     * Takes the plaf directory name and generates the TestALF UI
     * source code.
     */
    public static void main(String[] args) throws IOException {

        if (args.length < 1) {
	    usage("");
        }

        String dirName = args[0];
        File dir = new File(dirName);
	if (!dir.isDirectory()) {
	    System.err.println("No such directory:  " + dirName);
	    usage("");
        }

	if (args.length > 1) {
	    packageName = args[1];
        }

        String plafUIs[] = dir.list(new UIJavaFilter());
        for (int i = 0; i < plafUIs.length; i++) {
	    generateFile("javax",stripExtension(plafUIs[i]));
	}
    }
}

/**
 * Only accepts file names of the form *UI.java.  The one exception
 * is not accepting ComponentUI.java because we don't need to generate
 * a TestALF class for it.
 */
class UIJavaFilter implements FilenameFilter {
    public boolean accept(File dir, String name) {
	if (name.equals("ComponentUI.java")) {
	    return false;
	} else if (name.endsWith("UI.java")) {
	    return true;
	} else {
	    return false;
        }
    }
}
