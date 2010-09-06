/*
 * @(#)SourceModel.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.gui;

import java.io.*;
import java.util.*;

import com.sun.jdi.*;
import com.sun.jdi.request.*;

import com.sun.tools.example.debug.bdi.*;

import javax.swing.*;

/**
 * Represents and manages one source file.
 * Caches source lines.  Holds other source file info.
 */
public class SourceModel extends AbstractListModel {

    private File path;

    boolean isActuallySource = true;

    private List classes = new ArrayList();

    private Environment env;

    // Cached line-by-line access.

    //### Unify this with source model used in source view?
    //### What is our cache-management policy for these?
    //### Even with weak refs, we won't discard any part of the
    //### source if the SourceModel object is reachable.
    /**
     * List of Line.
     */
    private List sourceLines = null;

    public static class Line {
        public String text;
        public boolean hasBreakpoint = false;
        public ReferenceType refType = null;
        Line(String text) {
            this.text = text;
        }
        public boolean isExecutable() {
            return refType != null;
        }
        public boolean hasBreakpoint() {
            return hasBreakpoint;
        }
    };
	
    // 132 characters long, all printable characters.
    public static final Line prototypeCellValue = new Line(
                                        "abcdefghijklmnopqrstuvwxyz" + 
    					"ABCDEFGHIJKLMNOPQRSTUVWXYZ" +
    					"ABCDEFGHIJKLMNOPQRSTUVWXYZ" +
    					"1234567890~!@#$%^&*()_+{}|" +
    					":<>?`-=[];',.XXXXXXXXXXXX/\\\"");

    SourceModel(Environment env, File path) {
        this.env = env;
	this.path = path;
    }
    
    public SourceModel(String message) {
        this.path = null;
        setMessage(message);
    }

    private void setMessage(String message) {
        isActuallySource = false;
        sourceLines = new ArrayList();
        sourceLines.add(new Line(message));
    }

    // **** Implement ListModel  *****

    public Object getElementAt(int index) {
        if (sourceLines == null) {
            initialize();
        }
        return sourceLines.get(index);
    }

    public int getSize() {
        if (sourceLines == null) {
            initialize();
        }
        return sourceLines.size();
    }

    // ***** Other functionality *****

    public File fileName() {
	return path;
    }

    public BufferedReader sourceReader() throws IOException {
	return new BufferedReader(new FileReader(path));
    }

    public Line line(int lineNo) {
	if (sourceLines == null) {
	    initialize();
	}
	int index = lineNo - 1; // list is 0-indexed
	if (index >= sourceLines.size() || index < 0) {
	    return null;
	} else {
	    return (Line)sourceLines.get(index);  
	}
    }

    public String sourceLine(int lineNo) {
        Line line = line(lineNo);
	if (line == null) {
	    return null;
	} else {
	    return line.text;  
	}
    }

    void addClass(ReferenceType refType) {
        // Logically is Set
        if (classes.indexOf(refType) == -1) {
            classes.add(refType);
            if (sourceLines != null) {
                markClassLines(refType);
            }
        }
    }

    /**
     * @return List of currently known {@link com.sun.jdi.ReferenceType} 
     * in this source file.
     */
    public List referenceTypes() {
        return Collections.unmodifiableList(classes);
    }

    private void initialize() {
        try {
            rawInit();
        } catch (IOException exc) {
            setMessage("[Error reading source code]");
        }
    }

    public void showBreakpoint(int ln, boolean hasBreakpoint) {
        line(ln).hasBreakpoint = hasBreakpoint;
        fireContentsChanged(this, ln, ln);
    }

    public void showExecutable(int ln, ReferenceType refType) {
        line(ln).refType = refType;
        fireContentsChanged(this, ln, ln);
    }

    /**
     * Mark executable lines and breakpoints, but only
     * when sourceLines is set.
     */
    private void markClassLines(ReferenceType refType) {
        List methods = refType.methods();
        for (Iterator mit = methods.iterator(); mit.hasNext();) {
            Method meth = (Method)mit.next();
            try {
                List lines = meth.allLineLocations();
                for (Iterator lit = lines.iterator(); lit.hasNext();) {
                    Location loc = (Location)lit.next();
                    showExecutable(loc.lineNumber(), refType);
                }       
            } catch (AbsentInformationException exc) {
                // do nothing
            }
        }             
        List bps = env.getExecutionManager().eventRequestManager().breakpointRequests();
        for (Iterator it = bps.iterator(); it.hasNext();) {
            BreakpointRequest bp = (BreakpointRequest)it.next();
            if (bp.location() != null) {
                Location loc = bp.location();
                if (loc.declaringType().equals(refType)) {
                    showBreakpoint(loc.lineNumber(),true);
                }
            }
        }
    }

    private void rawInit() throws IOException {
	sourceLines = new ArrayList();
	BufferedReader reader = sourceReader();
	try {
	    String line = reader.readLine();
	    while (line != null) {
		sourceLines.add(new Line(expandTabs(line)));
		line = reader.readLine();
	    }
	} finally {
	    reader.close();
	}
        for (Iterator it = classes.iterator(); it.hasNext();) {
            markClassLines((ClassType)it.next());
        }
    }

    private String expandTabs(String s) {
	int col = 0;
	int len = s.length();
	StringBuffer sb = new StringBuffer(132);
	for (int i = 0; i < len; i++) {
	    char c = s.charAt(i);
	    sb.append(c);
	    if (c == '\t') {
		int pad = (8 - (col % 8));
		for (int j = 0; j < pad; j++) {
		    sb.append(' ');
		}
		col += pad;
	    } else {
		col++;
	    }
	}
	return sb.toString();
    }

}
