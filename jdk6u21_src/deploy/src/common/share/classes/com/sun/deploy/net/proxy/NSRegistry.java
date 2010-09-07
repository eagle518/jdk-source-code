/*
 * @(#)NSRegistry.java	1.19 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net.proxy;

import java.io.EOFException;
import java.io.IOException;
import java.io.File;
import java.io.RandomAccessFile;
import java.util.StringTokenizer;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;


/**
 * Provides simple read-only access to the (binary) Netscape registry file.
 * The registry file, typically C:\WINNT\nsreg.dat on windows or
 * ~/.netscape/registry on Unix, contains configuration information for
 * Netscape applications.  The registry is organized like the windows registry:
 * it's tree structured and nodes (called "keys") have a list of name/value
 * pairs called entries.
 * <p>
 * The format of the file doesn't seem to be officially documented however
 * it's not complicated and can be easily derived from the Mozilla sources
 * that provide access to it.  See the references below for more information.
 * <p>
 * To use this class you must create an NSRegistry object and open
 * a valid registry file.  To look up a value (the value of an "entry") you
 * provide it's path; here's an example that prints the current version
 * of Communicator.
 * <pre>
 * NSRegistry reg = new NSRegistry().open(new File("c:\WINNT\nsreg.dat"));
 * System.out.println(reg.get("Version Registry/Netscape/Communicator/Version"));
 * reg.close();
 * </pre>
 * The NSRegistry.dump() method prints the contents of the entire registry.
 *
 * @version 1.2, 03/11/00
 */
final class NSRegistry
{
    private static final int magic = 0x76644441;
    private RandomAccessFile in = null;
    private int rootOffset;
    private int majorVersion, minorVersion;

    /**
     * Each entry in the registry file, beginning at rootOffset, is a record
     * formatted like the REGDESC struct below, however note that the last
     * field is not part of this record.  Each record is exactly 32 bytes long.
     * This struct definition was lifted directly from the mozilla sources, see
     * <p>
     * <a href="http://lxr.mozilla.org/mozilla/source/modules/libreg/src/reg.h">
     * source/modules/libreg/src/reg.h
     * </a>
     * <pre>
     *  typedef struct _desc {
     *      REGOFF  location;   // this object's offset (for verification)
     *	    REGOFF  name;       // name string
     *	    uint16  namelen;    // length of name string (including terminator)
     *	    uint16  type;       // node type (key, or entry style)
     *	    REGOFF  left;       // next object at this level (0 if none)
     *	    REGOFF  down;       // KEY: first subkey        VALUE: 0
     *	    REGOFF  value;      // KEY: first entry object  VALUE: value string
     *	    uint32  valuelen;   // KEY: 0  VALUE: length of value data
     *	    uint32  valuebuf;   // KEY: 0  VALUE: length available
     *	    REGOFF  parent;     // the node on the immediate level above
     *	} REGDESC;
     * </pre>
     * The REGTYPE_ENTRY constant (0x0010) comes from NSReg.h, see
     * <a href="http://lxr.mozilla.org/mozilla/source/modules/libreg/include/NSReg.h">
     * source/modules/libreg/include/NSReg.h
     * </a>
    */
    private static final class Record {
	int name, namelen, type, left, down, value;
	long valuelen, valuebuf;
	public boolean isKey() {
	    return (type & 0x0010) != 0;  // REGTYPE_ENTRY, see NSReg.h
	}
	public boolean isEntry() {
	    return !isKey();
	}
    }

    /**
     * Read a Record at the specified offset.  Throws an IOException if
     * the "location" field in the record doesn't match its offset
     * in the file.
     */
    private Record readRecord(long offset) throws IOException {
	in.seek(offset);
	if ((long)readInt() != offset) {
	    throw new IOException("invalid offset for record [" + offset + "]");
	}
	Record r = new Record();
	r.name = readInt();
	r.namelen = readUnsignedShort();
	r.type = readUnsignedShort();
	r.left = readInt();
	r.down = readInt();
	r.value = readInt();
	r.valuelen = readUnsignedInt();
	r.valuebuf = readUnsignedInt();
	return r;
    }
    
    /**
     * Read a UTF8 string at the specified offset.  Note that nchars does
     * not include a terminating null character.
     */
    private String readString(long offset, long nchars) throws IOException {
	in.seek(offset);
	StringBuffer sb = new StringBuffer();
	for(int i = 0; i < nchars; i++) {
	    sb.append((char)(in.read()));
	}
	return sb.toString();
    }

    /**
     * Read a signed short at the current offset.  The byte ordering matches
     * what's used by the Netscape registry on all platforms.
     */
    private short readShort() throws IOException {
	int ch1 = in.read();
	int ch2 = in.read();
	if ((ch1 | ch2) < 0) {
	     throw new EOFException();
	}
	return (short)((ch2 << 8) + (ch1 << 0));
    }

    /**
     * Read an unsigned short at the current offset.  The byte ordering matches
     * what's used by the Netscape registry on all platforms.
     */
    private int readUnsignedShort() throws IOException {
	int ch1 = in.read();
	int ch2 = in.read();
	if ((ch1 | ch2) < 0) {
	     throw new EOFException();
	}
	return (ch2 << 8) + (ch1 << 0);
    }

    /**
     * Read an integer at the current offset.  The byte ordering matches
     * what's used by the Netscape registry on all platforms.
     */
    private int readInt() throws IOException {
	int ch1 = in.read();
	int ch2 = in.read();
	int ch3 = in.read();
	int ch4 = in.read();
	if ((ch1 | ch2 | ch3 | ch4) < 0) {
	     throw new EOFException();
	}
	return ((ch4 << 24) + (ch3 << 16) + (ch2 << 8) + (ch1 << 0));
    }

    /**
     * Read an unsigned integer at the current offset.  The byte ordering
     * matches what's used by the Netscape registry on all platforms.
     */
    private long readUnsignedInt() throws IOException {
	int ch1 = in.read();
	int ch2 = in.read();
	int ch3 = in.read();
	int ch4 = in.read();
	if ((ch1 | ch2 | ch3 | ch4) < 0) {
	     throw new EOFException();
	}
	return ((ch4 << 24) + (ch3 << 16) + (ch2 << 8) + (ch1 << 0));
    }


    /**
     * Open the (binary) Netscape registry file (usually C:\WINNT\nsreg.dat
     * on Windows) and read its header.  Return <code>this</code> if the
     * file was opened and the header was read successfully, null
     * otherwise.  This method must be called, before calling anything else.
     *<p>
     * The layout of the file header is only documented in the Mozilla
     * sources, see
     * <p>
     * <a href="http://lxr.mozilla.org/mozilla/source/modules/libreg/src/reg.h">
     * source/modules/libreg/src/reg.h
     * </a>
     * <p>
     * Here's the header struct itself:
     *    typedef struct _hdr {
     *       uint32  magic;      // must equal MAGIC_NUMBER
     *       uint16  verMajor;   // major version number
     *       uint16  verMinor;   // minor version number
     *       REGOFF  avail;      // next available offset
     *       REGOFF  root;       // root object
     *    } REGHDR;
     */
    NSRegistry open(File regFile) {
	if (in != null) {
	    return this;
	}
	try {
	    in = new RandomAccessFile(regFile, "r");

	    if (readInt() != magic) {
		throw new IOException("not a valid Netscape Registry File");
	    }
	    majorVersion = readUnsignedShort();
	    minorVersion = readUnsignedShort();
	    in.skipBytes(4);
	    rootOffset = readInt();

	    return this;
	}
	catch (IOException e) {
	    return null;
	}
    }


    /**
     * Close the Netscape registry file.
     */
    void close() {
	if (in != null) {
	    try {
		in.close();
	    }
	    catch(IOException exc) {
	    }
	    in = null;
	}
    }


    /**
     * Recursively find a matching subkey (relative to the parent Record) until
     * the last token in pathElts is reached.  The last token must match
     * an entry in parent.
     */
    private String get(Record parent, StringTokenizer pathElts)  throws IOException
    {
	String name = pathElts.nextToken();

	/* If this isn't the last path element then find a subkey with
	 * a matching name.
	 */
	if (pathElts.hasMoreTokens()) {
	    int childOffset = parent.down;
	    while (childOffset != 0) {
		Record key = readRecord(childOffset);
		if (name.equals(readString(key.name, key.namelen - 1))) {
		    return get(key, pathElts);
		}
		else {
		    childOffset = key.left;
		}
	    }
	    return null;
	}

	/* Name is the last path element so find an entry with a matching name
	 * and return its value.
	 */
	else {
	    int entryOffset = parent.value;
	    while (entryOffset != 0) {
		Record entry = readRecord(entryOffset);
		if (name.equals(readString(entry.name, entry.namelen - 1))) {
		    return readString(entry.value, entry.valuelen - 1);
		}
		else {
		    entryOffset = entry.left;
		}
	    }
	    return null;
	}
    }


    /**
     * Return the value of the specified path.  A path resembles a UNIX file
     * path, forward slashes ('/') are used to separate path elements.  The
     * elements of the path must match the names of registry keys and the final
     * path element must match the name of a registry entry.  Paths always
     * implicitly begin with a slash.   Here's an example that returns
     * the version of Navigator:
     * <pre>
     * NSRegistry reg = new NSRegistry().open();
     * reg.get("Version Registry/Netscape/Communicator/Version");
     * reg.close();
     * </pre>
     */
    String get(String path)
    {
	StringTokenizer pathElts = new StringTokenizer(path, "/");
	try {
	    return get(readRecord(rootOffset), pathElts);
	}
	catch(IOException e) {
	    return null;
	}
    }


    /**
     * Recursively walk the registry tree beginning with the record at
     * parentOffset and print each key and entry.  Keys are printed first,
     * then the entries.
     */
    private void dump(String indent, int parentOffset) throws IOException
    {
	Record key = readRecord(parentOffset);
	System.out.println(indent + readString(key.name, key.namelen - 1));
	int childOffset = key.down;
	while (childOffset != 0) {
	    dump(indent + "    ", childOffset);
	    childOffset = readRecord(childOffset).left;
	}
	int entryOffset = key.value;
	while (entryOffset != 0) {
	    Record entry = readRecord(entryOffset);
	    String name = readString(entry.name, entry.namelen - 1);
	    String value = readString(entry.value, entry.valuelen - 1);
	    System.out.println(indent + "[" + name + " = " + value + "]");
	    entryOffset = entry.left;
	}
    }


    /**
     * Dump the contents of the registry to the standard output.  The
     * name of each key is, indented to show its level in the tree.  Entries
     * are indented similarly and are formatted like this:
     * <pre>
     * [name = value]
     * </pre>
     */
    void dump() {
	try {
	    dump("", rootOffset);
	}
	catch(IOException e) {
	    e.printStackTrace();
	}
    }


    public String toString() {
	String s = getClass().getName() + "@" + Integer.toHexString(hashCode());
	if (in != null) {
	    s += "[version " + majorVersion + "." + minorVersion + "]";
	}
	else {
	    s += "[closed]";
	}
	return s;
    }

    /**
     * Return the location of the "prefs.js" user profile file in the
     * netscape registry or null if we can't figure that out.  This method
     * should work with versions 4.5 - 4.7 of Navigator.
     */
    private static String getNS45PrefsFilePath(String systemDir) 
    {
	String path = null;

	try
	{
	    File registryFile = new File(systemDir, "nsreg.dat");
	    NSRegistry reg = new NSRegistry().open(registryFile);
	    if (reg != null) {
		String user = reg.get("Common/Netscape/ProfileManager/LastNetscapeUser");
		if (user != null) {
		    path = reg.get("Users/" + user + "/ProfileLocation");
		}
		reg.close();
	    }
	    
	    if (path != null)
		path = path + File.separator + "prefs.js";
	}
	catch(Exception e)
	{
	    Trace.printException(e, ResourceManager.getMessage("net.proxy.nsprefs.error"),
				 ResourceManager.getMessage("net.proxy.error.caption"));
	}

	return path;
    }

    private static void showProxyErrorDialog()
    {
	UIFactory.showErrorDialog(null, 
	    ResourceManager.getMessage("net.proxy.nsprefs.error"),
	    ResourceManager.getMessage("net.proxy.error.caption"));
    }
}

