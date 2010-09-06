/**
 * @(#)Paths.java	1.6 04/07/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;
import java.io.File;
import java.io.IOException;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.util.jar.Attributes;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Iterator;
import java.util.StringTokenizer;
import com.sun.tools.javac.util.Context;
import com.sun.tools.javac.util.Log;
import com.sun.tools.javac.util.Options;
import com.sun.tools.javac.util.Position;

/** This class converts command line arguments, environment variables
 *  and system properties (in File.pathSeparator-separated String form)
 *  into a boot class path, user class path, and source path (in
 *  Collection<String> form).
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Paths {

    /** The context key for the todo list */
    protected static final Context.Key<Paths> pathsKey =
	new Context.Key<Paths>();

    /** Get the Paths instance for this context. */
    public static Paths instance(Context context) {
	Paths instance = context.get(pathsKey);
	if (instance == null)
	    instance = new Paths(context);
	return instance;
    }

    /** The log to use for warning output */
    private final Log log;

    /** Collection of command-line options */
    private final Options options;

    protected Paths(Context context) {
	context.put(pathsKey, this);
	log = Log.instance(context);
	options = Options.instance(context);
    }

    /** Whether to warn about non-existent path elements */
    private boolean warn;

    /** Cached paths */
    private Path bootClassPath;
    private Path userClassPath;
    private Path sourcePath;

    private boolean inited = false;

    protected void lazy() {
	if (!inited) {
	    warn = options.lint("path");

	    bootClassPath = computeBootClassPath();
	    userClassPath = computeUserClassPath();
	    sourcePath    = computeSourcePath();

	    inited = true;

	    if (options.get("-verbose") != null) {
		printVerbose("sourcepath", sourceSearchPath().toString());
		printVerbose("classpath",  classSearchPath().toString());
	    }
	}
    }

    public Collection<String> bootClassPath() {
        lazy();
        return Collections.unmodifiableCollection(bootClassPath);
    }
    public Collection<String> userClassPath() {
        lazy();
        return Collections.unmodifiableCollection(userClassPath);
    }
    public Collection<String> sourcePath() {
        lazy();
        return sourcePath == null
            ? null
            : Collections.unmodifiableCollection(sourcePath);
    }

    private static class PathIterator implements Collection<String> {
	private int pos = 0;
	private final String path;
	private final String emptyPathDefault;

	public PathIterator(String path, String emptyPathDefault) {
	    this.path = path;
	    this.emptyPathDefault = emptyPathDefault;
	}
	public PathIterator(String path) { this(path, null); }
	public Iterator<String> iterator() {
	    return new Iterator<String>() {
		public boolean hasNext() {
		    return pos <= path.length();
		}
		public String next() {
		    int beg = pos;
		    int end = path.indexOf(File.pathSeparator, beg);
		    if (end == -1)
			end = path.length();
		    pos = end + 1;

		    if (beg == end && emptyPathDefault != null)
			return emptyPathDefault;
		    else
			return path.substring(beg, end);
		}
		public void remove() {
		    throw new UnsupportedOperationException();
		}
	    };
	}

        // required for Collection.
        public int size() { throw new UnsupportedOperationException(); }
        public boolean isEmpty() { throw new UnsupportedOperationException(); }
        public boolean contains(Object o) { throw new UnsupportedOperationException(); }
        public Object[] toArray() { throw new UnsupportedOperationException(); }
        public <T> T[] toArray(T[] a) { throw new UnsupportedOperationException(); }
        public boolean add(String o) { throw new UnsupportedOperationException(); }
        public boolean remove(Object o) { throw new UnsupportedOperationException(); }
        public boolean containsAll(Collection<?> c) { throw new UnsupportedOperationException(); }
        public boolean addAll(Collection<? extends String> c) { throw new UnsupportedOperationException(); }
        public boolean removeAll(Collection<?> c) { throw new UnsupportedOperationException(); }
        public boolean retainAll(Collection<?> c) { throw new UnsupportedOperationException(); }
        public void clear() { throw new UnsupportedOperationException(); }
        public boolean equals(Object o) { throw new UnsupportedOperationException(); }
        public int hashCode() { throw new UnsupportedOperationException(); }
    }

    private class Path extends LinkedHashSet<String> {
	private static final long serialVersionUID = 0;

	private boolean expandJarClassPaths = false;

	public Path expandJarClassPaths(boolean x) {
	    expandJarClassPaths = x;
	    return this;
	}

	/** What to use when path element is the empty string */
	private String emptyPathDefault = null;

	public Path emptyPathDefault(String x) {
	    emptyPathDefault = x;
	    return this;
	}

	public Path() { super(); }

	public Path addDirectories(String dirs, boolean warn) {
	    if (dirs != null)
		for (String dir : new PathIterator(dirs))
		    addDirectory(dir, warn);
	    return this;
	}

	public Path addDirectories(String dirs) {
	    return addDirectories(dirs, warn);
	}

	private void addDirectory(String dir, boolean warn) {
	    if (! new File(dir).isDirectory()) {
		if (warn)
		    log.warning(Position.NOPOS,
				"dir.path.element.not.found", dir);
		return;
	    }

	    for (String direntry : new File(dir).list()) {
		String canonicalized = direntry.toLowerCase();
		if (canonicalized.endsWith(".jar") ||
		    canonicalized.endsWith(".zip"))
		    addFile(dir + File.separator + direntry, warn);
	    }
	}

	public Path addFiles(String files, boolean warn) {
	    if (files != null)
		for (String file : new PathIterator(files, emptyPathDefault))
		    addFile(file, warn);
	    return this;
	}

	public Path addFiles(String files) {
	    return addFiles(files, warn);
	}

	private void addFile(String file, boolean warn) {
	    if (contains(file)) {
		/* Discard duplicates and avoid infinite recursion */
	    } else if (! new File(file).exists()) {
		if (warn)
		    log.warning(Position.NOPOS,
				"path.element.not.found", file);
	    } else {
		super.add(file);
		if (expandJarClassPaths && isZip(file))
		    addJarClassPath(file, warn);
	    }
	}

	// Adds referenced classpath elements from a jar's Class-Path
	// Manifest entry.  In some future release, we may want to
	// update this code to recognize URLs rather than simple
	// filenames, but if we do, we should redo all path-related code.
	private void addJarClassPath(String jarFileName, boolean warn) {
	    try {
		String jarParent = new File(jarFileName).getParent();
		JarFile jar = new JarFile(jarFileName);

		try {
		    Manifest man = jar.getManifest();
		    if (man == null) return;

		    Attributes attr = man.getMainAttributes();
		    if (attr == null) return;

		    String path = attr.getValue(Attributes.Name.CLASS_PATH);
		    if (path == null) return;

		    for (StringTokenizer st = new StringTokenizer(path);
			 st.hasMoreTokens();) {
			String elt = st.nextToken();
			if (jarParent != null)
			    elt = new File(jarParent, elt).toString();
			addFile(elt, warn);
		    }
		} finally {
		    jar.close();
		}
	    } catch (IOException e) {
		log.error(Position.NOPOS,
			  "error.reading.file", jarFileName, e);
	    }
	}
    }

    private Path computeBootClassPath() {
	String optionValue;
	Path path = new Path();

	path.addFiles(options.get("-Xbootclasspath/p:"));

	if ((optionValue = options.get("-endorseddirs")) != null)
	    path.addDirectories(optionValue);
	else
	    path.addDirectories(System.getProperty("java.endorsed.dirs"), false);

	if ((optionValue = options.get("-bootclasspath")) != null)
	    path.addFiles(optionValue);
	else
	    // Standard system classes for this compiler's release.
	    path.addFiles(System.getProperty("sun.boot.class.path"), false);

	path.addFiles(options.get("-Xbootclasspath/a:"));

	// Strictly speaking, standard extensions are not bootstrap
	// classes, but we treat them identically, so we'll pretend
	// that they are.
	if ((optionValue = options.get("-extdirs")) != null)
	    path.addDirectories(optionValue);
	else
	    path.addDirectories(System.getProperty("java.ext.dirs"), false);

	return path;
    }

    private Path computeUserClassPath() {
	String cp = options.get("-classpath");

	// CLASSPATH environment variable when run from `javac'.
	if (cp == null) cp = System.getProperty("env.class.path");

	// If invoked via a java VM (not the javac launcher), use the
	// platform class path
	if (cp == null && System.getProperty("application.home") == null)
	    cp = System.getProperty("java.class.path");

	// Default to current working directory.
	if (cp == null) cp = ".";

	return new Path()
	    .expandJarClassPaths(true) // Only search user jars for Class-Paths
	    .emptyPathDefault(".")     // Empty path elt ==> current directory
	    .addFiles(cp);
    }

    private Path computeSourcePath() {
	String sourcePathArg = options.get("-sourcepath");
	if (sourcePathArg == null)
	    return null;

	return new Path().addFiles(sourcePathArg);
    }

    /** The actual effective locations searched for sources */
    private Path sourceSearchPath;

    public Collection<String> sourceSearchPath() {
	if (sourceSearchPath == null) {
	    lazy();
	    sourceSearchPath = sourcePath != null ? sourcePath : userClassPath;
	}
	return Collections.unmodifiableCollection(sourceSearchPath);
    }

    /** The actual effective locations searched for classes */
    private Path classSearchPath;

    public Collection<String> classSearchPath() {
	if (classSearchPath == null) {
	    lazy();
	    classSearchPath = new Path();
	    classSearchPath.addAll(bootClassPath);
	    classSearchPath.addAll(userClassPath);
	}
	return Collections.unmodifiableCollection(classSearchPath);
    }

    /** Is this the name of a zip file? */
    private static boolean isZip(String name) {
	return new File(name).isFile();
    }

    private void printVerbose(String key, String arg) {
	Log.printLines(log.noticeWriter,
		       Log.getLocalizedString("verbose." + key, arg));
    }

}
