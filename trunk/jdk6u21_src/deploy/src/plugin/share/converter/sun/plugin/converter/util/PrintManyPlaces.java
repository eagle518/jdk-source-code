/*
 * @(#)PrintManyPlaces.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.util;

import java.io.*;
import java.util.*;

public class PrintManyPlaces extends OutputStream {

    private Vector places = new Vector();

    public void addPlace(OutputStream o) {
	places.addElement(o);
    }
		
    public void removePlace(OutputStream o ) {
	places.removeElement(o);
    }

    public void write(int b) throws IOException {

	OutputStream o ;
	for(int i = 0;i<places.size();i++) {
	    o = (OutputStream)places.elementAt(i);
	    o.write(b);
	    o.flush();
	}
    }
    public synchronized void closeAll() {
	try {
	    OutputStream o ;
	    for(int i = 0;i<places.size();i++) {
		o = (OutputStream)places.elementAt(i);
		if(o != System.out)
		    o.close();
	    }
	}
	catch(IOException e) {
	    e.printStackTrace();
	}
    }
		
    public int countPlaces() {
	return places.size();
    }
}
