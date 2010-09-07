/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)WinNTFileSystem.java	1.9 04/03/20
 */

package java.io;

/**
 * Unicode-aware FileSystem for Windows NT/2000.
 * 
 * @author Konstantin Kladko
 * @version 1.9, 04/03/20
 * @since 1.4
 */
class WinNTFileSystem extends Win32FileSystem {

    protected native String canonicalize0(String path)
                                                throws IOException;
    protected native String canonicalizeWithPrefix0(String canonicalPrefix,
                                                    String pathWithCanonicalPrefix)
                                                throws IOException;

    /* -- Attribute accessors -- */

    public native int getBooleanAttributes(File f);
    public native boolean checkAccess(File f, boolean write);
    public native long getLastModifiedTime(File f);
    public native long getLength(File f);


    /* -- File operations -- */

    public native boolean createFileExclusively(String path)
	                                       throws IOException;
    protected native boolean delete0(File f);
    public synchronized native boolean deleteOnExit(File f);
    public native String[] list(File f);
    public native boolean createDirectory(File f);
    protected native boolean rename0(File f1, File f2);
    public native boolean setLastModifiedTime(File f, long time);
    public native boolean setReadOnly(File f);
    protected native String getDriveDirectory(int drive);
    private static native void initIDs();

    static {
	    initIDs();
    }
}
