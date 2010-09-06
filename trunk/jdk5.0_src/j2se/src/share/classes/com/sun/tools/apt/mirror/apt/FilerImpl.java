/*
 * @(#)FilerImpl.java	1.6 04/06/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.apt;


import java.io.*;
import java.util.Collection;
import java.util.EnumMap;
import java.util.HashSet;
import java.util.Set;

import com.sun.mirror.apt.Filer;
import com.sun.tools.apt.mirror.declaration.DeclarationMaker;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Options;
import com.sun.tools.javac.util.Position;
import com.sun.tools.apt.util.Bark;

import static com.sun.mirror.apt.Filer.Location.*;


/**
 * Implementation of Filer.
 */

public class FilerImpl implements Filer {

    private final Options opts;
    private final DeclarationMaker declMaker;
    private final com.sun.tools.apt.main.JavaCompiler comp;

    // Platform's default encoding
    private final static String DEFAULT_ENCODING =
	    new OutputStreamWriter(new ByteArrayOutputStream()).getEncoding();

    private String encoding;	// name of charset used for source files

    private final EnumMap<Location, File> locations;	// where new files go


    private static final Context.Key<FilerImpl> filerKey =
	    new Context.Key<FilerImpl>();

    // Set of files opened.
    private Collection<Flushable> wc;

    private Bark bark;

    // All created files.
    private final Set<File> filesCreated;

    // Names of newly created source files
    private HashSet<String> sourceFileNames = new HashSet<String>();

    // Names of newly created class files
    private HashSet<String> classFileNames  = new HashSet<String>();

    private boolean roundOver;

    public static FilerImpl instance(Context context) {
	FilerImpl instance = context.get(filerKey);
	if (instance == null) {
	    instance = new FilerImpl(context);
	}
	return instance;
    }

    // flush all output streams;
    public void flush() {
	for(Flushable opendedFile: wc) {
	    try {
		opendedFile.flush();
	    } catch (IOException e) {}
	}
    }

    private FilerImpl(Context context) {
	context.put(filerKey, this);
	opts = Options.instance(context);
	declMaker = DeclarationMaker.instance(context);
	bark = Bark.instance(context);
	comp = com.sun.tools.apt.main.JavaCompiler.instance(context);
	roundOver = false;
	this.filesCreated = comp.getAggregateGenFiles();

	// Encoding
	encoding = opts.get("-encoding");
	if (encoding == null) {
	    encoding = DEFAULT_ENCODING;
	}

	wc = new HashSet<Flushable>();

	// Locations
	locations = new EnumMap<Location, File>(Location.class);
	String s = opts.get("-s");	// location for new source files
	String d = opts.get("-d");	// location for new class files
	locations.put(SOURCE_TREE, new File(s != null ? s : "."));
	locations.put(CLASS_TREE,  new File(d != null ? d : "."));
    }


    /**
     * {@inheritDoc}
     */
    public PrintWriter createSourceFile(String name) throws IOException {
	File file = new File(locations.get(SOURCE_TREE),
			     nameToPath(name, ".java"));
	PrintWriter pw = getPrintWriter(file, encoding);
	sourceFileNames.add(file.getPath());
	return pw;
    }

    /**
     * {@inheritDoc}
     */
    public OutputStream createClassFile(String name) throws IOException {
	String pathname = nameToPath(name, ".class");
	OutputStream os = getOutputStream(CLASS_TREE, pathname);
	classFileNames.add(pathname);
	return os;
    }

    /**
     * {@inheritDoc}
     */
    public PrintWriter createTextFile(Location loc,
				      String pkg,
				      File relPath,
				      String charsetName) throws IOException {
	File file = (pkg.length() == 0)
			? relPath
			: new File(nameToPath(pkg), relPath.getPath());
	if (charsetName == null) {
	    charsetName = encoding;
	}
	return getPrintWriter(loc, file.getPath(), charsetName);
    }

    /**
     * {@inheritDoc}
     */
    public OutputStream createBinaryFile(Location loc,
					 String pkg,
					 File relPath) throws IOException {
	File file = (pkg.length() == 0)
			? relPath
			: new File(nameToPath(pkg), relPath.getPath());
	return getOutputStream(loc, file.getPath());
    }


    /**
     * Converts the canonical name of a top-level type or package to a
     * pathname.  Suffix is ".java" or ".class" or "".
     */
    private String nameToPath(String name, String suffix) throws IOException {
        if (!DeclarationMaker.isJavaIdentifier(name.replace('.', '_'))) {
	    bark.warning(Position.NOPOS, "IllegalFileName", name);
	    throw new IOException();
	}
	return name.replace('.', File.separatorChar) + suffix;
    }

    private String nameToPath(String name) throws IOException {
	return nameToPath(name, "");
    }

    /**
     * Returns a writer for a text file given its location, its
     * pathname relative to that location, and its encoding.
     */
    private PrintWriter getPrintWriter(Location loc, String pathname,
				       String encoding) throws IOException {
	File file = new File(locations.get(loc), pathname);
	return getPrintWriter(file, encoding);
    }

    /**
     * Returns a writer for a text file given its encoding.
     */
    private PrintWriter getPrintWriter(File file,
				       String encoding) throws IOException {
	prepareFile(file);
	PrintWriter pw = 
	    new PrintWriter(
		    new BufferedWriter(
			 new OutputStreamWriter(new FileOutputStream(file),
						encoding)));
	wc.add(pw);
	return pw;
    }

    /**
     * Returns an output stream for a binary file given its location
     * and its pathname relative to that location.
     */
    private OutputStream getOutputStream(Location loc, String pathname)
							throws IOException {
	File file = new File(locations.get(loc), pathname);
	prepareFile(file);
	OutputStream os = new FileOutputStream(file);
	wc.add(os);
	return os;
    }
     
    public Set<String> getSourceFileNames() {
	return sourceFileNames;
    }

    public Set<String> getClassFileNames() {
	return classFileNames;
    }

    public void roundOver() {
	roundOver = true;
    }

    /**
     * Checks that the file has not already been created during this
     * invocation.  If not, creates intermediate directories, and
     * deletes the file if it already exists.
     */
    private void prepareFile(File file) throws IOException {
	if (roundOver) {
	    bark.warning(Position.NOPOS, "NoNewFilesAfterRound", file.toString());
	    throw new IOException();
	}
	if (!filesCreated.add(file)) {
	    bark.warning(Position.NOPOS, "FileReopening", file.toString());
	    throw new IOException();
	}
	if (file.exists()) {
	    file.delete();
	} else {
	    File parent = file.getParentFile();
	    if (parent != null && !parent.exists()) {
		parent.mkdirs();
	    }
	}
    }
}
