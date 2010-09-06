/*
 * @(#)Environment.java	1.11 03/12/19
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
import com.sun.tools.example.debug.bdi.*;

public class Environment {

    private SourceManager sourceManager;
    private ClassManager classManager;
    private ContextManager contextManager;
    private MonitorListModel monitorListModel;
    private ExecutionManager runtime;

    private PrintWriter typeScript;

    private boolean verbose;

    public Environment() {
	this.classManager = new ClassManager(this);
	//### Order of the next three lines is important!  (FIX THIS)
	this.runtime = new ExecutionManager();
	this.sourceManager = new SourceManager(this);
	this.contextManager = new ContextManager(this);
	this.monitorListModel = new MonitorListModel(this);
    }

    // Services used by debugging tools.
    
    public SourceManager getSourceManager() {
	return sourceManager;
    }

    public ClassManager getClassManager() {
	return classManager;
    }

    public ContextManager getContextManager() {
	return contextManager;
    }

    public MonitorListModel getMonitorListModel() {
	return monitorListModel;
    }

    public ExecutionManager getExecutionManager() {
	return runtime;
    }

    //### TODO:
    //### Tools should attach/detach from environment
    //### via a property, which should call an 'addTool'
    //### method when set to maintain a registry of
    //### tools for exit-time cleanup, etc.  Tool
    //### class constructors should be argument-free, so
    //### that they may be instantiated by bean builders.
    //### Will also need 'removeTool' in case property
    //### value is changed.
    //
    // public void addTool(Tool t);
    // public void removeTool(Tool t);

     public void terminate() {
	 System.exit(0);
     }

    // public void refresh();    // notify all tools to refresh their views


    // public void addStatusListener(StatusListener l);
    // public void removeStatusListener(StatusListener l);

    // public void addOutputListener(OutputListener l);
    // public void removeOutputListener(OutputListener l);

    public void setTypeScript(PrintWriter writer) {
	typeScript = writer;
    }

    public void error(String message) {
	if (typeScript != null) {
	    typeScript.println(message);
	} else {
	    System.out.println(message);
	}
    }
    
    public void failure(String message) {
	if (typeScript != null) {
	    typeScript.println(message);
	} else {
	    System.out.println(message);
	}
    }

    public void notice(String message) {
	if (typeScript != null) {
	    typeScript.println(message);
	} else {
	    System.out.println(message);
	}
    }

    public OutputSink getOutputSink() {
	return new OutputSink(typeScript);
    }

    public void viewSource(String fileName) {
	//### HACK ###
	//### Should use listener here.
	com.sun.tools.example.debug.gui.GUI.srcTool.showSourceFile(fileName);
    }

    public void viewLocation(Location locn) {
	//### HACK ###
	//### Should use listener here.
	//### Should we use sourceForLocation here?
	com.sun.tools.example.debug.gui.GUI.srcTool.showSourceForLocation(locn);
    }

    //### Also in 'ContextManager'.  Do we need both?

    public boolean getVerboseFlag() {
	return verbose;
    }

    public void setVerboseFlag(boolean verbose) {
	this.verbose = verbose;
    }

}
