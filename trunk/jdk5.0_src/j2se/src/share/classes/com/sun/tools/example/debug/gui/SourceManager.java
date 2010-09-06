/*
 * @(#)SourceManager.java	1.11 03/12/19
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

import com.sun.tools.example.debug.event.*;
import com.sun.tools.example.debug.bdi.*;

/**
 * Manage the list of source files.
 * Origin of SourceListener events.
 */
public class SourceManager {

    //### TODO: The source cache should be aged, and some cap
    //### put on memory consumption by source files loaded into core.

    private List sourceList;
    private SearchPath sourcePath;

    private Vector sourceListeners = new Vector();

    private Map classToSource = new HashMap();

    private Environment env;

    /**
     * Hold on to it so it can be removed.
     */
    private SMClassListener classListener = new SMClassListener();
    
    public SourceManager(Environment env) {
	this(env, new SearchPath(""));
    }
    
    public SourceManager(Environment env, SearchPath sourcePath) {
        this.env = env;
	this.sourceList = new LinkedList();
	this.sourcePath = sourcePath;
        env.getExecutionManager().addJDIListener(classListener);
    }

    /**
     * Set path for access to source code.
     */
    public void setSourcePath(SearchPath sp) {
	sourcePath = sp;
	// Old cached sources are now invalid.
	sourceList = new LinkedList();
	notifySourcepathChanged();
        classToSource = new HashMap();
    }

    public void addSourceListener(SourceListener l) {
	sourceListeners.addElement(l);
    }

    public void removeSourceListener(SourceListener l) {
	sourceListeners.removeElement(l);
    }

    private void notifySourcepathChanged() {
	Vector l = (Vector)sourceListeners.clone();
	SourcepathChangedEvent evt = new SourcepathChangedEvent(this);
	for (int i = 0; i < l.size(); i++) {
	    ((SourceListener)l.elementAt(i)).sourcepathChanged(evt);
	}
    }

    /**
     * Get path for access to source code.
     */
    public SearchPath getSourcePath() {
	return sourcePath;
    }
    
    /**
     * Get source object associated with a Location.
     */
    public SourceModel sourceForLocation(Location loc) {
	return sourceForClass(loc.declaringType());
    }

    /**
     * Get source object associated with a class or interface.
     * Returns null if not available.
     */
    public SourceModel sourceForClass(ReferenceType refType) {
        SourceModel sm = (SourceModel)classToSource.get(refType);
        if (sm != null) {
            return sm;
        }
        try {
            String filename = refType.sourceName();
            String refName = refType.name();
            int iDot = refName.lastIndexOf('.');
            String pkgName = (iDot >= 0)? refName.substring(0, iDot+1) : "";
            String full = pkgName.replace('.', File.separatorChar) + filename;
            File path = sourcePath.resolve(full);
            if (path != null) {
                sm = sourceForFile(path);
                classToSource.put(refType, sm);
                return sm;
            }
            return null;
        } catch (AbsentInformationException e) {
            return null;
        }
    }

    /**
     * Get source object associated with an absolute file path.
     */
    //### Use hash table for this?
    public SourceModel sourceForFile(File path) {
        Iterator iter = sourceList.iterator();
        SourceModel sm = null;
        while (iter.hasNext()) {
            SourceModel candidate = (SourceModel)iter.next();
            if (candidate.fileName().equals(path)) {
                sm = candidate;
		iter.remove();    // Will move to start of list.
                break;
            }
        }
        if (sm == null && path.exists()) {
	    sm = new SourceModel(env, path);
        }
        if (sm != null) { 
            // At start of list for faster access
            sourceList.add(0, sm);  
        }
	return sm;
    }
    
    private class SMClassListener extends JDIAdapter 
                                   implements JDIListener {

        public void classPrepare(ClassPrepareEventSet e) {
            ReferenceType refType = e.getReferenceType();
            SourceModel sm = sourceForClass(refType);
            if (sm != null) {
                sm.addClass(refType);
            }
	}

	public void classUnload(ClassUnloadEventSet e) {
            //### iterate through looking for (e.getTypeName()).
            //### then remove it.
	}
    }
}
