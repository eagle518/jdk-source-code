/*
 * @(#)Gen.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javah.oldjavah;

import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.File;
import java.io.PrintWriter;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import sun.tools.java.Constants;
import sun.tools.java.Type;
import sun.tools.java.MemberDefinition;
import sun.tools.java.ClassNotFound;
import sun.tools.tree.Expression;

/**
 * An abstraction for generating support files required by native methods.
 * Subclasses are for specific native interfaces. At the time of its
 * original writing, this interface is rich enough to support JNI and the
 * old 1.0-style native method interface.
 *
 * @version 1.5, 12/03/01
 */
public abstract class Gen implements Constants {

    /**
     * Override this abstract method, generating content for the named
     * class into the outputstream.
     */
    protected abstract void write(OutputStream o, String clazz)
	throws ClassNotFoundException;

    /**
     * Override this method to provide a list of #include statements
     * required by the native interface.
     */
    protected abstract String getIncludes();

    /*
     * Gateway to type information.
     */
    protected JavahEnvironment env;
    
    public JavahEnvironment getEnvironment() {
	return env;
    }


    /*
     * Output location.
     */
    protected String outDir;

    public void setOutDir(String outDir) {
	/* Check important, otherwise concatenation of two null strings
	 * produces the "nullnull" String. */
	if (outDir != null) {
	    this.outDir = outDir + System.getProperty("file.separator");
	    File d = new File(outDir);
	    if (!d.exists())
		if (!d.mkdirs())
		    Util.error("cant.create.dir", d.toString());
	}
    }

    protected String outFile;

    public void setOutFile(String outFile) {
	this.outFile = outFile;
    }


    /*
     * List of classes for which we must generate output.
     */
    protected String[] classes;

    public void setClasses(String[] classes) {
	this.classes = classes;
    }


    /*
     * Where do we look for .class files?
     */
    protected String classpath;

    public void setClasspath(String classpath) {
	this.env = new JavahEnvironment(classpath);
    }


    /*
     * Smartness with generated files.
     */
    protected boolean force = false;
    
    public void setForce(boolean state) {
	force = state;
    }

    /**
     * We explicitly need to write ASCII files because that is what C
     * compilers understand.
     */
    protected PrintWriter wrapWriter(OutputStream o) {
	try {
	    return new
		PrintWriter(new OutputStreamWriter(o, "ISO8859_1"), true);
	} catch (UnsupportedEncodingException use) {
	    Util.bug("encoding.iso8859_1.not.found");
	    return null; /* dead code */
	}
    }


    /**
     * After initializing state of an instance, use this method to start
     * processing.
     *
     * Buffer size chosen as an approximation from a single sampling of:
     *         expr `du -sk` / `ls *.h | wc -l`
     */
    public void run() throws IOException, ClassNotFoundException {
	int i = 0;
	if (outFile != null) {
	    /* Everything goes to one big file... */
	    ByteArrayOutputStream bout = new ByteArrayOutputStream(8192);
	    writeFileTop(bout); /* only once */
	    for (i = 0; i < classes.length; i++) {
		write(bout, classes[i]);
	    }
	    writeIfChanged(bout.toByteArray(), outFile);
	} else {
	    /* Each class goes to its own file... */
	    for (i = 0; i < classes.length; i++) {
		ByteArrayOutputStream bout = new ByteArrayOutputStream(8192);
		writeFileTop(bout);
		String clazz = classes[i];
		write(bout, clazz);
		writeIfChanged(bout.toByteArray(), getFileName(clazz));
	    }
	}
    }
    

   /*
    * Write the contents of byte[] b to a file named file.  Writing
    * is done if either the file doesn't exist or if the contents are
    * different.
    */
    private void writeIfChanged(byte[] b, String file) throws IOException {
	File f = new File(file);
	boolean mustWrite = false;
	String event = "[No need to update file ";
	
	if (force) {
	    mustWrite = true;
	    event = "[Forcefully writing file ";
	} else {
	    if (!f.exists()) {
		mustWrite = true;
		event = "[Creating file ";
	    } else {
		int l = (int)f.length();
		if (b.length != l) {
		    mustWrite = true;
		    event = "[Overwriting file ";
		} else {
		    /* Lengths are equal, so read it. */
		    byte[] a = new byte[l];
		    FileInputStream in = new FileInputStream(f);
		    if (in.read(a) != l) {
		    in.close();
		    /* This can't happen, we already checked the length. */
		    Util.error("not.enough.bytes", Integer.toString(l),
			       f.toString());
		    }
		    in.close();
		    while (--l >= 0) {
		    if (a[l] != b[l]) {
			mustWrite = true;
			event = "[Overwriting file ";
		    }
		    }
		}
	    }
	}
	if (Util.verbose)
	    Util.log(event + file + "]");
	if (mustWrite) {
	    OutputStream out = new FileOutputStream(file);
	    out.write(b); /* No buffering, just one big write! */
	    out.close();
	}
    }
    
    static private final boolean isWindows = 
	System.getProperty("os.name").startsWith("Windows");

    protected String defineForStatic(String c, MemberDefinition f) {
	String cname = Mangle.mangle(c, Mangle.Type.CLASS);
	String fname = Mangle.mangle(f.getName().toString(),
				   Mangle.Type.FIELDSTUB);

	if (!f.isStatic())
	    Util.bug("tried.to.define.non.static");
	
	if (f.isFinal()) {
	    Object value = null;
	    try {
		Expression constExpression = 
		    (Expression)f.getValue(getEnvironment());
		if (constExpression != null) {
		    value = constExpression.getValue();
		}
	    } catch (ClassNotFound cnf) {
		/* REMIND: why is this generated, and how do we
		 * gracefully bail when it happens? */
	    }
	    if (value != null) { /* so it is a ConstantExpression */
		String constString = null;
		if (value instanceof Integer) {
		    /* covers byte, boolean, char, short, int */
		    constString = value.toString() + "L";
		} else if (value instanceof Long) {
		    // Visual C++ supports the i64 suffix, not LL.
		    if (isWindows)
			constString = value.toString() + "i64";
		    else
			constString = value.toString() + "LL";
		} else if (value instanceof Float) {
		    /* bug for bug */
		    float fv = ((Float)value).floatValue();
		    if (Float.isInfinite(fv))
			constString = ((fv < 0) ? "-" : "") + "Inff";
		    else
			constString = value.toString() + "f";
		} else if (value instanceof Double) {
		    /* bug for bug */
		    double d = ((Double)value).doubleValue();
		    if (Double.isInfinite(d))
			constString = ((d < 0) ? "-" : "") + "InfD";
		    else
			constString = value.toString();
		}
		if (constString != null) {
		    StringBuffer s = new StringBuffer("#undef ");
		    s.append(cname); s.append("_"); s.append(fname);
		    s.append("\n#define "); s.append(cname); s.append("_");
		    s.append(fname); s.append(" "); s.append(constString);
		    return s.toString();
		}
	    }
	}
	/* bug for bug. */
	if (f.getType() != Type.tString ||
	    (f.getType() == Type.tString && !f.isFinal()))
	    return "/* Inaccessible static: " + fname + " */";
	return null;
    }

    /*
     * Deal with the C pre-processor.
     */
    protected String cppGuardBegin() {
	return "#ifdef __cplusplus\nextern \"C\" {\n#endif";
    }

    protected String cppGuardEnd() {
	return "#ifdef __cplusplus\n}\n#endif";
    }

    protected String guardBegin(String cname) {
	return "/* Header for class " + cname + " */\n\n" + 
	    "#ifndef _Included_" + cname + "\n" + 
	    "#define _Included_" + cname;
    }

    protected String guardEnd(String cname) {
	return "#endif";
    }

    /*
     * File name and file preamble related operations.
     */
    protected void writeFileTop(OutputStream o) {
	PrintWriter pw = wrapWriter(o);
	pw.println("/* DO NOT EDIT THIS FILE - it is machine generated */\n" +
		   getIncludes());
    }
    
    protected String baseFileName(String clazz) {
	StringBuffer f = 
	    new StringBuffer(Mangle.mangle(clazz,
					   Mangle.Type.CLASS));
	if (outDir != null) {
	    f.insert(0, outDir);
	}
	return f.toString();
    }

    protected String getFileName(String clazz) {
	return baseFileName(clazz) + getFileSuffix();
    }

    protected String getFileSuffix() {
	return ".h";
    }
}
