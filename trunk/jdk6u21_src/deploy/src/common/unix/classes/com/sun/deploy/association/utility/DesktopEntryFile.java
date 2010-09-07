/*
 * @(#)DesktopEnryFile.java 
 * Created on April 15, 2005, 9:29 AM
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;
import java.io.File;
import java.io.StringReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
/**
 *
 * Represents a desktop entry file in Unix desktop environments.
 *
 * This class provides constructors for creating {@link DesktopEntry} from
 * URI forms or file pathname, methods for reading or writing the
 * a desktop entry instance, and methods for normalizing, resolving,
 * and a relativizing desktop entry instance.  
 * 
 * Since the most important group which explicitly needs to be supported 
 * is "[Desktop Entry]", this class only supports this single group.
 * It might support other group with desktop entry spec updates.
 *
 *
 * @(#)DesktopEntryFile.java	1.8 10/03/24
 */
public class DesktopEntryFile {
    
    static {
        //load gnome vfs lib
        Config.getInstance().loadDeployNativeLib(); 
    }
    
    private String uri = null;
    private ArrayList desktopEntries = null;

    
    /** 
     * Creates a new instance of DesktopEntryFile from a GNOME URI reference.
     * <p>
     * This constructor construct a desktop entry file from a GNOME URI reference.
     * The URI might be a standard URI or an Gnome extended URI including
     * pedefined Gnome VFS URI :
     * <ul>
     *    <li>applications:///  :User defined Applications </li>
     *    <li>applications-all-users:/// :System wide defined Applications></li>
     * </ul>
     *
     * For example: 
     *    new DesktopEntryFile("file:///home/foo/foo.desktop");
     *    new DesktopEntryFile("applications:///Associates/gedit.desktop");
     * </p>
     *
     * @param  uri  The URI string referenced to a desktop entry
     *
     * @throws  NullPointerException
     *          If <tt>uri</tt> is <tt>null</tt>
     *
     * @throws  IllegalArgumentException
     *          If the given string violates URI's RFC 
     */
    
    public DesktopEntryFile(String uri){
        if(null == uri){
            throw new NullPointerException("uri is null!");
        }
        if( uri.indexOf("://") == 0){
            throw new IllegalArgumentException("Invalid URI[" + uri + "]!");
        }
        this.uri = uri;
        desktopEntries = new ArrayList();
        Trace.println("new DesktopEntryFile uri = [" + uri + "]" , TraceLevel.BASIC);
    }
    

    /** 
     * Creates a new instance of DesktopEntryFile from a file.
     * This method works as if by invoking the 
     * {@link #DesktopEntryFile(String) constructor}
     * and {@link File#toURI()}
     * For example: new DesktopEntryFile(new File("/home/foo/Desktop/foo.desktop"));
     *
     * @param  file   The file to be parsed into a URI
     *
     * @throws  NullPointerException
     *          If <tt>file</tt> is <tt>null</tt>
     */
    public DesktopEntryFile(File file) {
        this(file.toURI().toString());
    }

    
    public String getParent(){
        String path = uri.toString();
	int index = path.lastIndexOf(File.separator);
        int prefixLength = path.indexOf("://") + 3; // file://
	if (index < prefixLength) {
	    if ((prefixLength > 0) && (path.length() > prefixLength))
		return path.substring(0, prefixLength);
	    return "/";
	}
	return path.substring(0, index);    
    }

    private DesktopEntryFile getParentDesktopEntryFile() {
        String parent = getParent();
	return (parent == null) ? null : (new DesktopEntryFile(parent));    
    }    
    /**
     * Reads and returns the desktop entry of the desktop entry file
     *
     * @return the desktop entry collection.
     *
     * @throws  IOException
     *          If an I/O error has occurred, or the file doesn't exist.
     */
    public Collection readEntries()
                         throws IOException {
        readEntryInternal();    
        
        return desktopEntries;
    }
    
    /**
     * Writes the given DesktopEntry object to the desktop entry file
     * 
     * @param DesktopEntry the desktop entry to write to the entry file
     *
     * @throws  IOException
     *          If an I/O error has occurred
     */
    public void writeEntry(DesktopEntry desktopEntry)
                         throws IOException {
        ArrayList l = new ArrayList();
        l.add(desktopEntry);
        writeEntries(l);
    } 
    
    /**
     * Writes the given DesktopEntry object to the desktop entry file
     * 
     * @param DesktopEntry the desktop entry to write to the entry file
     *
     * @throws  IOException
     *          If an I/O error has occurred
     */
    public void writeEntries(Collection entries)
                         throws IOException {
        this.desktopEntries = new ArrayList(entries);
        writeEntryInternal();
    }     
    
    /**
     * Tests whether the file exists.
     * 
     * @return true if the file exists.
     */
    public boolean exists(){
        return gnome_vfs_file_exists(uri.toString());
    }
    
    /**
     * Deletes the vfs file or vfs directory.
     *
     * @return true if and only if the vfs file or directory is successfully deleted;
     * false otherwise.
     *    
     */
    public boolean delete() {
        return gnome_vfs_delete_file(uri.toString());
    }
    
    /**
     * Delete all parent empty vfs directory until non-empty directory.
     *
     * @return true if and only if the directory is successfully deleted;
     * false otherwise.
     *    
     */
    public boolean deleteToNonEmptyParent() {
        //delete directories
        DesktopEntryFile path = this;
        while( null != path 
                && path.exists() ){
            
            //Top directory, such as file://
            if( path.getURI().matches(".*://")){ 
                break;
            }
            if( path.delete() ){
                Trace.println("file deleted " + path.toString() , TraceLevel.BASIC);
            }else{
                break;
            }
            path = path.getParentDesktopEntryFile();
        }
        return true;
    }
    
    /**
     * Mkdir
     *
     * @return true if and only if the directory is successfully created;
     * false otherwise.
     *    
     */
    public boolean mkdir() throws IOException {
        return gnome_vfs_mkdir(uri.toString());
    }    
    
    
    /**
     * Reads a native file referenced by uri, and returns the content
     * as String.
     */
    private static native String gnome_vfs_read_file(String uri);

    /**
     * Vfs Mkdir 
     */
    private static native boolean gnome_vfs_mkdir(String uri);
    
    /**
     *Tests whether the file exists.
     */
    private static native boolean gnome_vfs_file_exists(String uri);
    
    /**
     * Writes a native file referenced by uri.
     */
    private static native boolean gnome_vfs_write_file(String uri, String content);

    /**
     * Ulinks a native file referenced by uri.
     */
    private static native boolean gnome_vfs_delete_file(String uri);
    
    /**
     * Ensure gnome vfs library has been loaded
     */
    private static native void ensure_load_gnome_vfs_lib();
    
    /**
     * Reads entry internally
     * @throws  IOException
     *          If an I/O error has occurred
     */
    private void readEntryInternal()
                         throws IOException {
        BufferedReader br = new BufferedReader(
                new StringReader(gnome_vfs_read_file(uri.toString())));
        String line = null;
        DesktopEntry entry = null;
        int len = 0;
        while(true){
            line = br.readLine();
            if(null == line){
                break;
            }
            if(line.matches("^\\s*\\[.*\\]\\s*")){
                if( null != entry){ 
                    //Null operations
                }else{  //New entry begins
                    entry = new DesktopEntry(trimBracket(line));
                    desktopEntries.add(entry);
                }
            }else{
                if( null != entry){
                   entry.load(line);
                }
            }
        }
    }
    
    private static String trimBracket(String src){
        if(null == src){
            return "";
        }
        int len = src.length();
        if(len > 1){
            return src.trim().substring(1, len -1);
        }else{
            return "";            
        }
    }

    /**
     * Writes entry internally
     * @throws  IOException
     *          If an I/O error has occurred
     */
    private void writeEntryInternal()
                         throws IOException {
        
        //create directories
        DesktopEntryFile path = getParentDesktopEntryFile();
        ArrayList l = new ArrayList();
        while( null != path && !path.exists() ){
            l.add(path);
            path = path.getParentDesktopEntryFile();
        }
        
        for(int i = l.size() - 1 ; i >= 0; i--){
            path = (DesktopEntryFile)l.get(i);
            Trace.println("writeEntryInternal mkdir " + path.toString() , TraceLevel.BASIC);
            path.mkdir();
        }
        
        Iterator it;
        StringBuffer sb = new StringBuffer();
        it = desktopEntries.iterator();
        while(it.hasNext()){
            sb.append(it.next());
            sb.append("\n");
            
        }
        Trace.println("gnome_vfs_write_file [" + uri.toString() + "]" , TraceLevel.BASIC);
        boolean b = gnome_vfs_write_file(uri.toString(), sb.toString());

    }
    
    public String getURI(){
        return uri;
    }
    
    public String toString(){
        return "DesktopEntryFile[" + uri + "]";
    }
    
}
