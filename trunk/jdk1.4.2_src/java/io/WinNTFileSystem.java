/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)WinNTFileSystem.java	1.5 03/01/23
 */

package java.io;

/**
 * Unicode-aware FileSystem for Windows NT/2000.
 * 
 * @author Konstantin Kladko
 * @version 1.5, 03/01/23
 * @since 1.4
 */
class WinNTFileSystem extends Win32FileSystem {

    protected native String canonicalize0(String path)
                                                throws IOException;

    /* -- Attribute accessors -- */

    public native int getBooleanAttributes(File f);
    public native boolean checkAccess(File f, boolean write);
    public native long getLastModifiedTime(File f);
    public native long getLength(File f);


    /* -- File operations -- */

    public native boolean createFileExclusively(String path)
	                                       throws IOException;
    public native boolean delete(File f);
    public synchronized native boolean deleteOnExit(File f);
    public native String[] list(File f);
    public native boolean createDirectory(File f);
    public native boolean rename(File f1, File f2);
    public native boolean setLastModifiedTime(File f, long time);
    public native boolean setReadOnly(File f);
    native String getDriveDirectory(int drive);
    private static native void initIDs();

    static {
	    initIDs();
    }
}
