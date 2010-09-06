/*
 * @(#)ThreadInfo.java	1.11 03/12/19
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

package com.sun.tools.example.debug.bdi;

import com.sun.jdi.*;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;

//### Should handle target VM death or connection failure cleanly.

public class ThreadInfo {

    private ThreadReference thread;
    private int status;

    private int frameCount;

    Object userObject;  // User-supplied annotation.

    private boolean interrupted = false;

    private void assureInterrupted() throws VMNotInterruptedException {
	if (!interrupted) {
	    throw new VMNotInterruptedException();
	}
    }
    
    ThreadInfo (ThreadReference thread) {
	this.thread = thread;
	this.frameCount = -1;
    }

    public ThreadReference thread() {
	return thread;
    } 

    public int getStatus() throws VMNotInterruptedException {
        assureInterrupted();
	update();
	return status;
    }

    public int getFrameCount() throws VMNotInterruptedException {
        assureInterrupted();
	update();
	return frameCount;
    }

    public StackFrame getFrame(int index) throws VMNotInterruptedException {
        assureInterrupted();
	update();
	try {
	    return thread.frame(index);
	} catch (IncompatibleThreadStateException e) {
	    // Should not happen
	    interrupted = false;
	    throw new VMNotInterruptedException();
	}
    }

    public Object getUserObject() {
	return userObject;
    }

    public void setUserObject(Object obj) {
	userObject = obj;
    }

    // Refresh upon first access after cache is cleared.

    void update() throws VMNotInterruptedException {
	if (frameCount == -1) {
	    try {
		status = thread.status();
		frameCount = thread.frameCount();
	    } catch (IncompatibleThreadStateException e) {
		// Should not happen
		interrupted = false;
		throw new VMNotInterruptedException();
	    }
	}
    }
    
    // Called from 'ExecutionManager'.
    
    void validate() {
	interrupted = true;
    }

    void invalidate() {
	interrupted = false;
	frameCount = -1;
	status = ThreadReference.THREAD_STATUS_UNKNOWN;
    }
    
}
