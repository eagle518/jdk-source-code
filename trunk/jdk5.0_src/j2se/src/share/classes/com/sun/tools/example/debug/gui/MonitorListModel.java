/*
 * @(#)MonitorListModel.java	1.8 03/12/19
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

import java.util.*;

import javax.swing.AbstractListModel;

public class MonitorListModel extends AbstractListModel {

    private final List monitors = new ArrayList();
	
    MonitorListModel(Environment env) {
	
	// Create listener.
	MonitorListListener listener = new MonitorListListener();
	env.getContextManager().addContextListener(listener);

        //### remove listeners on exit!
    }
	
    public Object getElementAt(int index) {
        return monitors.get(index);
    }
	
    public int getSize() {
        return monitors.size();
    }

    public void add(String expr) {
        monitors.add(expr);
        int newIndex = monitors.size()-1;  // order important
        fireIntervalAdded(this, newIndex, newIndex);
    }

    public void remove(String expr) {
        int index = monitors.indexOf(expr);
        remove(index);
    }

    public void remove(int index) {
        monitors.remove(index);
        fireIntervalRemoved(this, index, index);
    }

    public List monitors() {
        return Collections.unmodifiableList(monitors);
    }

    public Iterator iterator() {
        return monitors().iterator();
    }

    private void invalidate() {
        fireContentsChanged(this, 0, monitors.size()-1);
    }

    private class MonitorListListener implements ContextListener {

	public void currentFrameChanged(CurrentFrameChangedEvent e) {
	    invalidate();
	}
    }
}
