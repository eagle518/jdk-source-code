/*
 * @(#)ClassesFromClasspath.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.lang.reflect.Method;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/* This class represents class names which are placed in the given PATH.
 * The constructor ClassesFromClasspath() creates object for CLASSPATH
 * The constructor ClassesFromClasspath(path) creates object for path
 * The method nextClassName returns qualified name of the next class.
 * The method clear() starts from beginning.
 * The method int printErrors(PrintWriter out) prints all errors and warning  
 * to the PrintWriter out and returns number of the errors.*/

/** This class represents class names which are placed in the given PATH. **/
public class ClassesFromClasspath {

    /** includes errors and warnings which are occurred. **/
    private Vector errors;
    /** number of the errors **/
    private int sizeErrors;
    /** includes PATH entries. **/
    private Vector entries;
    /** the index of the current entry**/
    private int pos;
    /** current entry.**/
    private Path currentEntry;
    /** determines if non-java packages will be scanned.**/
    private boolean isAllTracked = false;
    /** determines if warning will be printed.**/
    private boolean isIgnorableTrack = false;

    /** creates ClassesFromClasspath with system.class.path + CLASSPATH as PATH
     *  @param isIgnorableTrack determines if warning will be printed **/
    public ClassesFromClasspath(boolean isIgnorableTrack) throws SecurityException {
	this(getClasspath(), isIgnorableTrack);
    }

    /** creates ClassesFromClasspath with given PATH
     *  @param path given PATH 
     *  @param isIgnorableTrack determines if warning will be printed. **/
    public ClassesFromClasspath(String path, boolean isIgnorableTrack) {
        this.isIgnorableTrack = isIgnorableTrack; 
        entries = new Vector();
        pos = 0;
        sizeErrors = 0;
	errors = new Vector();
        currentEntry = null;
        
        Vector pathEntries = new Vector();
	//creates Hashtable with ZipFiles and directories from path.
	while (path != null && path.length() > 0) {
	    String s;
	    int index = path.indexOf(File.pathSeparatorChar);
	    if (index == -1) {
		s = path;
		path = null;
	    } else {
		s = path.substring(0, index);
		path = path.substring(index + 1);
	    }
	    // remove trailing separator, if necessary
	    if (s.endsWith(File.separator))
		s = s.substring(0, s.length() - File.separator.length());
	    File pathEntry = new File(s);
            String canonicalEntry = s;
            try {
                canonicalEntry = pathEntry.getCanonicalPath();
            } catch (IOException e) {
            }
               
            if (pathEntries.contains(canonicalEntry))
                continue;//remove duplicates entries from CLASSPATH
            else
                pathEntries.addElement(canonicalEntry);
	    if (pathEntry.isDirectory())
                entries.addElement(new DirE(pathEntry));
	    else {
                try {
                    entries.addElement(new ZipE(pathEntry));
                } catch (IOException e) {
                    String error = "Ignoring " + pathEntry.getAbsolutePath() + ": " + e;
                    if (!errors.contains(error) && isIgnorableTrack)
                        errors.addElement(error);
                }
	    }
	}
    }

    /** determinate if the throws clause can be tracked in
     *  the current version. **/
    public boolean isThrowsTracked() {
        try {
            Class c = javasoft.sqe.tests.api.SignatureTest.ClassesFromClasspath.class;
            Method met = c.getDeclaredMethod("getCurrentClass", new Class[0]);
            Class exep[] = met.getExceptionTypes();
            if ((exep == null) || (exep.length == 0))
                return false;
            else
                return true;
        } catch (Throwable t) {
            errors.addElement("Can not track that Method.getExceptionTypes()" +
                              " works correctly. " + t + " thrown.");
            sizeErrors++;
            return false;
        }
    }

    /** returns system CLASSPATH concatenated with CLASSPATH.**/
    public static String getClasspath() {
        String sysPath = System.getProperty("java.sys.class.path");
        String path = System.getProperty("java.class.path");
        if ((sysPath == null) || sysPath.equals(""))
            return path;
        else
            return sysPath + File.pathSeparatorChar + path;
    }
        
    /** Prints all errors to the PrintWriter and returns number of the errors.**/
    public int printErrors(PrintWriter out) {
        if (out == null)
            return sizeErrors;
	for (int i = 0; i < errors.size(); i++) {
	    out.println((String)errors.elementAt(i));
	}
	return sizeErrors;
    }

    /** the clear context and removes position of current class to the begin **/
    public void clear() {
	pos = 0;
        currentEntry = null;
    }

    /** returns the name of the next class which found in the PATH.**/
    public String nextClassName() {
        if (isAllTracked) {
            return nextClassNameFromAll();
        }
        String name;
        while ((name = nextClassNameFromAll()) != null) {
            if (name.startsWith("java."))
                return name;
        }
        return null;
    }
            
    /** returns the name of the next class which found in the PATH.**/
    private String nextClassNameFromAll() {
        String name = null;
        for (;((currentEntry == null) ||
               ((name = currentEntry.nextClassName()) == null)) &&
                 (pos < entries.size()); pos++) {
            currentEntry = (Path)entries.elementAt(pos);
            currentEntry.clear();
        }
        return name;
    }

    /** open current class file as stream **/
    public InputStream getCurrentClass() throws IOException {
        if (currentEntry == null)
            return null;
        else
            return currentEntry.getCurrentClass();
    }

    public InputStream findClass(String name)
        throws IOException, ClassNotFoundException {
        for (int i = 0; i < entries.size(); i++) {
            Path temp = (Path)entries.elementAt(i);
            try {
                InputStream classFile = temp.findClass(name);
                return classFile;
            } catch (ClassNotFoundException e) {
            }
        }
        throw new ClassNotFoundException(name);
    }
                

    /** specify, that non-java packages will be scanned. **/
    public void setAllClasses() {
        isAllTracked = true;
    }

    /** This interface represent path entry **/
    private interface Path {
        /** returns name of the next class in this path entry **/
        String nextClassName();
        /** open current class file as stream **/
        InputStream getCurrentClass() throws IOException;
        /** the clear context and removes position of current class
         *  to the begin **/
        void clear();
        public InputStream findClass(String name)
            throws IOException, ClassNotFoundException;
        
    }

    /** This class represents path entry which is directory **/
    private class DirE implements Path {
        /** the path of the path entry **/
        String path;
        /** contains all classes placed in this directory **/
        Vector classes;
        /** index of the last founded class. **/
        int pos;
        /** the name of the last founded class. **/
        String lastName;

        /** creates new DirE for given directory.
         *  @param dir given directory. **/
        DirE(File dir) {
            this.path = dir.getAbsolutePath();
            classes = new Vector();
            pos = -1;
            scanDir(dir, "");
            lastName = null;
        }

        /** found all classes placed in the given directory included
         *  placed in the subdirectories
         *  @param dir given directory
         *  @param pkg the package name which contains this directory
         *  as sub-package. **/
        private void scanDir(File dir, String pkg) {
            try {
                String[] files = dir.list();
                if (files == null)
                    return;
                for (int i = 0; i < files.length; i++) {
                    File f = new File(dir, files[i]);
                    if (files[i].endsWith(".class")) {
                        String prefName = pkg.equals("") ? pkg : (pkg + ".");
                        classes.addElement(prefName +
                                           files[i].substring(0, files[i].length() - 6));
                    } else if (f.isDirectory())  
                        scanDir(f, pkg.equals("") ? files[i] : (pkg + "." + files[i]));
                }
            } catch (SecurityException e) {
                String error = "The Security constraints does not allow" +
                    " to track " + pkg;
                if (!errors.contains(error))
                    errors.addElement(error);
                sizeErrors++;
            }
        }

        /** returns name of the next class **/
        public String nextClassName() {
            pos++;
            if (pos < classes.size())
                lastName = (String)classes.elementAt(pos);
            else
                lastName = null;
            return lastName;
        }

        /** open current class as stream. **/
        public InputStream getCurrentClass() throws IOException {
            if ((pos < 0) || (lastName == null))
                throw new IOException("The current class not exist.");
            String name = (String)classes.elementAt(pos);
            if (File.separator.length() == 1)
                name = path +  File.separator +
                       name.replace('.', File.separator.charAt(0)) + ".class";
            else {
                String temp = path;
                while (name.indexOf('.') > 0) {
                    int n = name.indexOf('.');
                    temp = temp + File.separator + name.substring(0, n);
                    if (n >= name.length() - 1)
                        break;
                    name = name.substring(n + 1);
                }
                name = temp;
            }
            File cl = new File(name);
            return new FileInputStream(cl);
        }

        /** removes current position to the begin of scanned classes. **/
        public void clear() {
            pos = -1;
            lastName = null;
        }
        public InputStream findClass(String name)
            throws IOException, ClassNotFoundException {
            if (classes.contains(name)) {
                if (File.separator.length() == 1)
                    name = path +  File.separator +
                        name.replace('.', File.separator.charAt(0)) + ".class";
                else {
                    String temp = path;
                    while (name.indexOf('.') > 0) {
                        int n = name.indexOf('.');
                        temp = temp + File.separator + name.substring(0, n);
                        if (n >= name.length() - 1)
                            break;
                        name = name.substring(n + 1);
                    }
                    name = temp;
                }
                File cl = new File(name);
                return new FileInputStream(cl);
            } else {
                throw new ClassNotFoundException(name);
            }
        }
                
    }

    /** This class represent path entry which is zip or jar file **/
    private class ZipE implements Path {
        /** specified zip file. **/
        ZipFile zipfile;
        /** Enumeration of the classes placed in this zip file.**/
        Enumeration entries;
        /** entry which represents last founded class. **/
        ZipEntry currentEntry;
        /** name of the last founded classes. **/
        String lastName;

        /** Creates new ZipE for given zip or jar file. **/
        ZipE(File zipfile) throws IOException {
            this.zipfile = new ZipFile(zipfile);
            entries = this.zipfile.entries();
            currentEntry = null;
            lastName = null;
        }

        /** returns name of the next class. **/
        public String nextClassName() {
            String name = "";
            while (!name.endsWith(".class") && entries.hasMoreElements()) {
                currentEntry = (ZipEntry)entries.nextElement();
                name = currentEntry.getName();
            }
            if (!name.endsWith(".class")) 
                lastName = null;
            else
                lastName = name.substring(0, name.length() - 6).replace('/', '.');
            return lastName;
        }

        /** open last founded class as stream. **/
        public InputStream getCurrentClass() throws IOException {
            if ((currentEntry == null) || (lastName == null))
                throw new IOException("The current class not exist.");
            return zipfile.getInputStream(currentEntry);
        }

        /** removes the current position to the begin of the classes. **/
        public void clear() {
            entries = this.zipfile.entries();
            currentEntry = null;
            lastName = null;
        }

        public InputStream findClass(String name)
            throws IOException, ClassNotFoundException {
            ZipEntry zipEntry = zipfile.getEntry(name.replace('.', '/') +
                                                 ".class");
            if (zipEntry == null)
                throw new ClassNotFoundException();
            else
                return zipfile.getInputStream(zipEntry);
        }
    }
}
