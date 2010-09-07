/*
 * @(#)FlexFilter.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import java.io.*;
import java.util.*;

public class FlexFilter implements FilenameFilter {

    private Vector descriptors = new Vector();
    private boolean filesOnly = true;
    private String wildCard = "*";

    /*  This class with take strings, and create a filename filter
	based on them.  The strings can have *'s for wildcards. */

    public FlexFilter() {}

    public void addDescriptor(String s) {
	descriptors.addElement(s.trim());
    }

    public void addDescriptors(String[] s) {
	for(int i = 0;i<s.length;i++)
	    descriptors.addElement(s[i].trim());
    }
	
    public void clearDescriptors() {
	descriptors = new Vector();
    }

    public boolean accept(File dir, String name) {
	if(new File(dir,name).isDirectory())
	    return !filesOnly;			
				
	for(int i = 0;i<descriptors.size();i++) {
	    boolean test = testFile((String)descriptors.elementAt(i),name);
	    if(test) return true;
	}
	return false;
    }

    protected boolean testFile(String desc, String file) {

	desc.trim();
	file.trim();
	int star = desc.indexOf(wildCard);
	if(star == 0) {  //  Wild card is first
	    String match = desc.substring(star+1);
	    if(file.endsWith(match)) return true;
	    else return false;
	}
	else if(star > 0 && ( star < (file.length()-2))) {  //  Wild card is in the middle
	    String beginMatch = desc.substring(0,star);
	    String endMatch = desc.substring(star+1);
	    if(file.startsWith(beginMatch) && file.endsWith(endMatch)) return true;
	    else return false;
	}
	else if(star == (file.length()-1)) { // Wild card is at the end
	    String beginMatch = desc.substring(star);
	    if(file.startsWith(beginMatch)) return true;
	    else return false;

	}
	return desc.equals(file);
    }
    public void setFilesOnly(boolean b) { filesOnly  = b; }
    public boolean isFilesOnly() { return filesOnly; }
	
    public void setWildCard(String s) 
    { 
	wildCard = s; 
    }
    public String getWildCard() 
    { 
	return wildCard; 
    }
    public String toString() 
    {
	String str = "FlexFilter:\n  Descriptors";
	for(int i = 0;i<descriptors.size();i++) 
	    {
		str += "\n"+(String)descriptors.elementAt(i);
	    }
	str += "\nFiles Only:  "+(filesOnly?"True":"False");
	return str;
    }
    public int getDescriptorCount()
    {
	return descriptors.size();
    }
}
