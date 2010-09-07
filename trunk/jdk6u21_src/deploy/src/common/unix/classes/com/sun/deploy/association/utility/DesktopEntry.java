/*
 * @(#)DesktopEnry.java 
 * Created on April 15, 2005, 9:29 AM
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;
import java.net.URI;
import java.util.Properties;
import java.util.Iterator;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import com.sun.deploy.util.Trace;

/**
 *
 * Represents a Desktop Entry in Unix desktop environments.
 *
 * <p> An instance of this class represents a desktop entry as defined by <a
 * href="http://www.freedesktop.org/Standards/desktop-entry-spec"">
 * <i>freedesktop.org/Standards/desktop-entry-spec</i></a>
 * The latest version of this spec is 0.9.4.
 * This class provides constructors for creating desktop entries from
 * URI forms or file pathname, methods for accessing the
 * various components of an instance, and methods for normalizing, resolving,
 * and relativizing desktop entry instances.  
 *
 * <h4>Example Desktop Entry </h4>
 * <p>
 * Both the KDE and GNOME desktop environments have adopted a similar format 
 * for "desktop entries". 
 * </p>
 *
 * @(#)DesktopEntry.java	1.6 10/03/24
 */
public class DesktopEntry {
    
    public static final String DEFAULT_GROUP = "Desktop Entry";
    
    private String group = null;
      
    private Properties entries = null;;

    /**
     * <p>
     * Standard Keys
     * <a link="http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s04.html">
     * Recognized desktop entrykeys</a>
     * </p>
     */
    private static String[] DESKTOP_ENTRY_KEYS = new String[]{
                "Type",
                "Version",
                "Encoding",
                "Name",
                "GenericName",
                "NoDisplay",
                "Comment",
                "Icon",
                "Hidden",
                "FilePattern",
                "TryExec",
                "Exec",
                "Path",
                "Terminal",
                "SwallowTitle",
                "SwallowExec",
                "Actions",
                "MimeType",
                "SortOrder",
                "Dev",
                "FSType",
                "MountPoint",
                "ReadOnly",
                "UnmountIcon",
                "URL",
                "Categories ",
                "OnlyShowIn",
                 "NotShowIn",
                "StartupNotify",
                "StartupWMClass"                
    };
    
    /** 
     * Creates a new instance of DesktopEntry.
     */
    public DesktopEntry() {
        this(DEFAULT_GROUP);
    }

    /** 
     * Creates a new instance of DesktopEntry.
     */
    public DesktopEntry(String group) {
        this.group = group;
        entries = new Properties();
        set("Version", "1.0");
    }
    
    /**
     * Gets the type(the 'Type' field) of this desktop entry
     * 
     * @return the type string
     */
    public String getGroup(){
        return group;
    }
    
    /**
     * Sets the group string of this desktop entry
     * @param the group string
     */
    public void setGroup(String group){
        this.group = group;
    }
    
    /**
     * Gets the type(the 'Type' field) of this desktop entry
     * 
     * @return the type string
     */
    public String getType(){
        return get("Type");
    }
    
    /**
     * Sets the type(the 'Type' field) of this desktop entry
     * There'return 4 types of desktop entires:
     * Application, Link, FSDevice, Directory
     * 
     * @param the type string
     */
    public void setType(String type){
        set("Type", type);
    }
    
    /**
     * Sets the encoding(the 'Encoding' field) of this desktop entry.
     * (UTF-8 or Legacy-Mixed)
     * 
     * @param the encoding string
     */
    public void setEncoding(String encoding){
        set("Encoding", encoding);
    }

    /**
     * Gets the encoding(the 'Encoding' field) of this desktop entry.
     * (UTF-8 or Legacy-Mixed)
     * 
     * @return the encoding string
     */
    public String getEncoding(){
        return get("Encoding");
    }

    /**
     * Sets the name(The 'Name' field) of this desktop entry
     * 
     * @param the name string
     */
    public void setName(String name){
        set("Name", name);
    }

    /**
     * Gets the name(The 'Name' field) of this desktop entry
     * 
     * @return the name string
     */
    public String getName(){
        return get("Name");
    }
    
    /**
     * Gets the generic name(The 'GenericName' field) of this desktop entry
     * 
     * @return the generic name string
     */
    public String getGenericName(){
        return get("GenericName");
    }

    /**
     * Sets the generic name(The 'GenericName' field) of this desktop entry
     * 
     * @param the generic name string
     */
    public void setGenericName(String genericName){
        set("GenericName", genericName);
    }

    /**
     * Gets the program to execute ,possibly with arguments
     * (The 'Exec' field) of this desktop entry
     * 
     * @return the program to execute 
     */
    public String getExec(){
        return get("Exec");
    }

    /**
     * Sets the program to execute(The 'Exec' field) of this desktop entry
     * 
     * @param the program to execute
     */
    public void setExec(String program){
        set("Exec", program);
    }

    /**
     * Gets the icon
     * (The 'Icon' field) of this desktop entry
     * 
     * @return the icon
     */
    public String getIcon(){
        return get("Icon");
    }

    /**
     * Sets the icon(The 'Icon' field) of this desktop entry
     * 
     * @param the icon
     */
    public void setIcon(String icon){
        set("Icon", icon);
    }


    /**
     * Gets the terminal status
     * (The 'Terminal' field) of this desktop entry
     * 
     * @return the terminal status
     */
    public boolean getTerminal(){
        return Boolean.parseBoolean(get("Terminal"));
    }

    /**
     * Sets the terminal(The 'Terminal' field) of this desktop entry
     * 
     * @param the icon
     */
    public void setTerminal(boolean terminal){
        set("Terminal", String.valueOf(terminal));
    }

    /**
     * Gets the categories
     * (The 'Categories' field) of this desktop entry
     * 
     * @return the categories
     */
    public String getCategories(){
        return get("Categories");
    }

    /**
     * Sets the categories(The 'Categories' field) of this desktop entry
     * For example: 
     *
     * @param the categories
     */
    public void setCategories(String categories){
        set("Categories", categories);
    }

    /**
     * Gets the comment
     * (The 'Comment' field) of this desktop entry
     * 
     * @return the comment
     */
    public String getComment(){
        return get("Comment");
    }

    /**
     * Sets the comment(The 'Comment' field) of this desktop entry
     * For example: 
     *
     * @param the comment
     */
    public void setComment(String comment){
        set("Comment", comment);
    }

    /**
     * Gets the path
     * (The 'Path' field) of this desktop entry
     * 
     * @return the path
     */
    public String getPath(){
        return get("Path");
    }

    /**
     * Sets the path(The 'path' field) of this desktop entry
     *
     * @param the categories
     */
    public void setPath(String path){
        set("Path", path);
    }
    
    /**
     * if value == null, remove the property.
     */
    public void set(String key, String value){
        if( value == null){
            entries.remove(key);
        }else{
            entries.setProperty(key, value);
        }
    }
    
    public void set(String key, String locale, String value){
        set(key + "[" + locale + "]", value);
    }

    public String get(String key, String locale){
        return get(key + "[" + locale + "]");
    }

    public String get(String key){
        return entries.getProperty(key);
    }
    
    public void load(String content) {
        
        try{
           entries.load( new ByteArrayInputStream( content.getBytes()));                 
        }catch(IOException ioe){
           Trace.ignoredException(ioe);
           //never get here 
        }
        
    }
    
    /**
     * Creates a DesktopEntry by parsing the given string 
     *
     * @param content  the content of the desktop entry
     */
    public static DesktopEntry create(String content){
        return create(DEFAULT_GROUP, content);
    }

    /**
     * Creates a DesktopEntry by parsing the given string 
     *
     * @param group group name
     *
     * @param content  the content of the desktop entry
     */
    public static DesktopEntry create(String group, String content){
        if( null == group || group.trim().equals("")){
            group = DEFAULT_GROUP;
        }
        
        DesktopEntry entry = new DesktopEntry(group);
        entry.load(content);
        return entry;
    }

    /**
     * Returns the content of this DesktopEntry as a string.
     */
    public String toString(){
        StringBuffer sb = new StringBuffer();
        if( null == group){
            sb.append("[Desktop Entry]\n");
        }else{
            sb.append("["+ group + "]\n");
        }
        Iterator it = entries.keySet().iterator();
        String key ;
        while(it.hasNext()){
            key = (String)it.next();
            sb.append(key);
            sb.append("=");
            sb.append(entries.getProperty((String)key));
            sb.append("\n");
        }

        return sb.toString();
    }
    
    
}
