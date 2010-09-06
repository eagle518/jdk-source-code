/**
 * @(#)JavadocTool.java	1.41 04/05/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import java.io.*;

import java.util.Collection;

import com.sun.tools.javac.code.*;
import com.sun.tools.javac.code.Symbol.*;
import com.sun.tools.javac.comp.*;
import com.sun.tools.javac.jvm.ClassReader;
import com.sun.tools.javac.jvm.ClassWriter;
import com.sun.tools.javac.util.Paths;
import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.tree.Tree.*;
import com.sun.tools.javac.util.*;

import com.sun.javadoc.LanguageVersion;
import static com.sun.javadoc.LanguageVersion.*;

/**
 *  This class could be the main entry point for Javadoc when Javadoc is used as a
 *  component in a larger software system. It provides operations to
 *  construct a new javadoc processor, and to run it on a set of source
 *  files.
 *  @author Neal Gafter
 */
public class JavadocTool extends com.sun.tools.javac.main.JavaCompiler {
    DocEnv docenv;
    
    final Context context;
    final Messager messager;
    final JavadocClassReader reader;
    final JavadocEnter enter;
    final Annotate annotate;
    private final Paths paths;
    
    /**
     * Construct a new JavaCompiler processor, using appropriately
     * extended phases of the underlying compiler.
     */
    protected JavadocTool(Context context) {
        super(context);
	this.context = context;
        messager = Messager.instance0(context);
	reader = JavadocClassReader.instance0(context);
	enter = JavadocEnter.instance0(context);
	annotate = Annotate.instance(context);
	paths = Paths.instance(context);
    }
    
    /**
     * For javadoc, the parser needs to keep comments. Overrides method from JavaCompiler.
     */
    protected boolean keepComments() {
        return true;
    }

    /**
     *  Construct a new javadoc tool.
     */
    public static JavadocTool make0(Context context) {
	Messager messager = null;
        try {
	    // force the use of Javadoc's class reader
	    JavadocClassReader.preRegister(context);

	    // force the use of Javadoc's own enter phase
	    JavadocEnter.preRegister(context);

	    // force the use of Javadoc's own member enter phase
	    JavadocMemberEnter.preRegister(context);

	    // force the use of Javadoc's own todo phase
	    JavadocTodo.preRegister(context);

	    // force the use of Messager as a Log
	    messager = Messager.instance0(context);

            return new JavadocTool(context);
        } catch (CompletionFailure ex) {
            messager.error(Position.NOPOS, ex.getMessage());
            return null;
        }
    }
    
    public RootDocImpl getRootDocImpl(String doclocale,
                                      String encoding,
                                      ModifierFilter filter,
                                      List<String> javaNames,
                                      List<String[]> options,
                                      boolean breakiterator,
                                      List<String> subPackages,
                                      List<String> excludedPackages,
				      boolean docClasses,
				      boolean legacyDoclet,
                      boolean quiet) {
	docenv = DocEnv.instance(context);
	docenv.showAccess = filter;
    docenv.quiet = quiet;
	docenv.breakiterator = breakiterator;
        docenv.setLocale(doclocale);
        docenv.setEncoding(encoding);
	docenv.docClasses = docClasses;
	docenv.legacyDoclet = legacyDoclet;
	reader.sourceCompleter = docClasses ? null : this;
        
        ListBuffer<String> names = new ListBuffer<String>();
        ListBuffer<Tree> classTrees = new ListBuffer<Tree>();
        ListBuffer<Tree> packTrees = new ListBuffer<Tree>();
        
        try {
            for (List<String> it = javaNames; it.nonEmpty(); it = it.tail) {
                String name = it.head;
                if (!docClasses && name.endsWith(".java") && new File(name).exists()) {
                    docenv.notice("main.Loading_source_file", name);
                    Tree tree = parse(name);
                    classTrees.append(tree);
                } else if (isValidPackageName(name)) {
		    names = names.append(name);
                } else if (name.endsWith(".java")) {
		    docenv.error(null, "main.file_not_found", name);;
		} else {
		    docenv.error(null, "main.illegal_package_name", name);
		}
            }

            if (!docClasses) {
		// Recursively search given subpackages.  If any packages
		//are found, add them to the list.
		searchSubPackages(subPackages, names, excludedPackages);
            
		// Parse the packages
		for (List<String> packs = names.toList(); packs.nonEmpty(); packs = packs.tail) {
		    // Parse sources ostensibly belonging to package.
		    parsePackageClasses(packs.head, packTrees, excludedPackages);
		}
            
		if (messager.nerrors() != 0) return null;
            
		// Enter symbols for all files
		docenv.notice("main.Building_tree");
		enter.main(classTrees.toList().appendList(packTrees.toList()));
	    }
        } catch (Abort ex) {}
        
        if (messager.nerrors() != 0) return null;
        
	if (docClasses)
	    return new RootDocImpl(docenv, javaNames, options);
	else
	    return new RootDocImpl(docenv, listClasses(classTrees.toList()), names.toList(), options);
    }

    /** Is the given string a valid package name? */
    boolean isValidPackageName(String s) {
	int index;
	while ((index = s.indexOf('.')) != -1) {
	    if (!isValidClassName(s.substring(0, index))) return false;
	    s = s.substring(index+1);
	}
	return isValidClassName(s);
    }


    private final static char pathSep = File.pathSeparatorChar;
    
    /**
     * search all directories in path for subdirectory name. Add all
     * .java files found in such a directory to args.
     */
    private void parsePackageClasses(String name,
                                     ListBuffer<Tree> trees,
                                     List<String> excludedPackages) {
        if (excludedPackages.contains(name)) {
            return;
        }
        boolean hasFiles = false;
        docenv.notice("main.Loading_source_files_for_package", name);
        name = name.replace('.', File.separatorChar);
	for (String pathname : paths.sourceSearchPath()) {
            File f = new File(pathname, name);
            String names[] = f.list();
            // if names not null, then found directory with source files
            if (names != null) {
                String dir = f.getAbsolutePath();
                if (!dir.endsWith(File.separator))
                    dir = dir + File.separator;
                for (int j = 0; j < names.length; j++) {
                    if (isValidJavaSourceFile(names[j])) {
                        String fn = dir + names[j];
                        // messager.notice("main.Loading_source_file", fn);
                        trees.append(parse(fn));
                        hasFiles = true;
                    }
                }
            }
        }
        if (!hasFiles)
            messager.warning(null, "main.no_source_files_for_package", 
			     name.replace(File.separatorChar, '.'));
    }
    
    /**
     * Recursively search all directories in path for subdirectory name.
     * Add all packages found in such a directory to packages list.
     */
    private void searchSubPackages(List<String> subPackages,
                                   ListBuffer<String> packages,
                                   List<String> excludedPackages) {
	// FIXME: This search path is bogus.
	// Only the effective source path should be searched for sources.
	// Only the effective class path should be searched for classes.
	// Should the bootclasspath/extdirs also be searched for classes?
	java.util.List<String> pathnames = new java.util.ArrayList<String>();
	if (paths.sourcePath() != null)
	    for (String elt : paths.sourcePath())
		pathnames.add(elt);
	for (String elt : paths.userClassPath())
	    pathnames.add(elt);

        for (String subPackage : subPackages)
            searchSubPackage(subPackage, packages, excludedPackages, pathnames);
    }
    
    /**
     * Recursively search all directories in path for subdirectory name.
     * Add all packages found in such a directory to packages list.
     */
    private void searchSubPackage(String packageName,
				  ListBuffer<String> packages,
				  List<String> excludedPackages,
				  Collection<String> pathnames) {
        if (excludedPackages.contains(packageName))
            return;

        String packageFilename = packageName.replace('.', File.separatorChar);
        boolean addedPackage = false;
	for (String pathname : pathnames) {
            File f = new File(pathname, packageFilename);
            String filenames[] = f.list();
            // if filenames not null, then found directory
            if (filenames != null) {
		for (String filename : filenames) {
                    if (!addedPackage
                            && (isValidJavaSourceFile(filename) ||
				isValidJavaClassFile(filename))
                            && !packages.contains(packageName)) {
                        packages.append(packageName);
                        addedPackage = true;
                    } else if (isValidClassName(filename) &&
			       (new File(f, filename)).isDirectory()) {
                        searchSubPackage(packageName + "." + filename,
					 packages, excludedPackages, pathnames);
                    }
                }
            }
        }
    }

    /**
     * Return true if given file name is a valid class file name.
     * @param file the name of the file to check.
     * @return true if given file name is a valid class file name
     * and false otherwise.
     */
    private static boolean isValidJavaClassFile(String file) {
        if (!file.endsWith(".class")) return false;
        String clazzName = file.substring(0, file.length() - ".class".length());
        return isValidClassName(clazzName);
    }

    /**
     * Return true if given file name is a valid Java source file name.
     * @param file the name of the file to check.
     * @return true if given file name is a valid Java source file name
     * and false otherwise.
     */
    private static boolean isValidJavaSourceFile(String file) {
        if (!file.endsWith(".java")) return false;
        String clazzName = file.substring(0, file.length() - ".java".length());
        return isValidClassName(clazzName);
    }

    /** Are surrogates supported?
     */
    final static boolean surrogatesSupported = surrogatesSupported();
    private static boolean surrogatesSupported() {
        try {
            boolean b = Character.isHighSurrogate('a');
            return true;
        } catch (NoSuchMethodError ex) {
            return false;
        }
    }

    /**
     * Return true if given file name is a valid class name
     * (including "package-info").
     * @param clazzname the name of the class to check.
     * @return true if given class name is a valid class name
     * and false otherwise.
     */
    public static boolean isValidClassName(String s) {
        if (s.length() < 1) return false;
	if (s.equals("package-info")) return true;
        if (surrogatesSupported) {
            int cp = s.codePointAt(0);
            if (!Character.isJavaIdentifierStart(cp))
                return false;
            for (int j=Character.charCount(cp); j<s.length(); j+=Character.charCount(cp)) {
                cp = s.codePointAt(j);
                if (!Character.isJavaIdentifierPart(cp))
                    return false;
            }
        } else {
            if (!Character.isJavaIdentifierStart(s.charAt(0)))
                return false;
            for (int j=1; j<s.length(); j++)
                if (!Character.isJavaIdentifierPart(s.charAt(j)))
                    return false;
        }
        return true;
    }
    
    /**
     * From a list of top level trees, return the list of contained class definitions
     */
    List<ClassDef> listClasses(List<Tree> trees) {
        ListBuffer<ClassDef> result = new ListBuffer<ClassDef>();
	for (Tree t : trees) {
            if (t.tag == Tree.TOPLEVEL) {
		for (Tree def : ((TopLevel)t).defs) {
                    if (def.tag == Tree.CLASSDEF)
                        result.append((ClassDef)def);
                }
            }
        }
        return result.toList();
    }
    
}

