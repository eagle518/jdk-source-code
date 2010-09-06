/*
 * @(#)ThreadGroupIterator.java	1.14 03/12/19
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

package com.sun.tools.example.debug.tty;

import com.sun.jdi.ThreadGroupReference;
import com.sun.jdi.ThreadReference;
import java.util.List;
import java.util.Stack;
import java.util.ArrayList;
import java.util.Iterator;
            
/**
 * Descend the tree of thread groups.
 * @author Robert G. Field
 */
class ThreadGroupIterator implements Iterator {
    private final Stack stack = new Stack();

    ThreadGroupIterator(List tgl) {
        push(tgl);
    }

    ThreadGroupIterator(ThreadGroupReference tg) {
        List tgl = new ArrayList();
        tgl.add(tg);
        push(tgl);
    }

    ThreadGroupIterator() {
        this(Env.vm().topLevelThreadGroups());
    }

    private Iterator top() {
        return (Iterator)stack.peek();
    }

    /**
     * The invariant in this class is that the top iterator
     * on the stack has more elements.  If the stack is 
     * empty, there is no top.  This method assures
     * this invariant.
     */
    private void push(List tgl) {
        stack.push(tgl.iterator());
        while (!stack.isEmpty() && !top().hasNext()) {
            stack.pop();
        }
    }

    public boolean hasNext() {
        return !stack.isEmpty();
    }

    public Object next() {
        return nextThreadGroup();
    }

    public ThreadGroupReference nextThreadGroup() {
        ThreadGroupReference tg = (ThreadGroupReference)top().next();
        push(tg.threadGroups());
        return tg;
    }

    public void remove() {
        throw new UnsupportedOperationException();
    }

    static ThreadGroupReference find(String name) {
        ThreadGroupIterator tgi = new ThreadGroupIterator();
        while (tgi.hasNext()) {
            ThreadGroupReference tg = tgi.nextThreadGroup();
            if (tg.name().equals(name)) {
                return tg;
            }
        }
        return null;
    }

}
            
