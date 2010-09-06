/*
 * @(#)Context.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;

class Context {
    
    static final int outer = 0;
    static final int readingReply = 1;
    static final int writingCommand = 2;

    final String whereJava;
    final String whereC;

    int state = outer;
    private boolean inEvent = false;

    Context() {
        whereJava = "";
        whereC = "";
    }

    private Context(String whereJava, String whereC) {
        this.whereJava = whereJava;
        this.whereC = whereC;
    }

    Context subcontext(String level) {
        Context ctx;
        if (whereC.length() == 0) {
            ctx = new Context(level, level);
        } else { 
            ctx = new Context(whereJava + "." + level, whereC + "_" + level);
        }
        ctx.state = state;
        ctx.inEvent = inEvent;
        return ctx;
    }

    private Context cloneContext() {
        Context ctx = new Context(whereJava, whereC);
        ctx.state = state;
        ctx.inEvent = inEvent;
        return ctx;
    }        

    Context replyReadingSubcontext() {
        Context ctx = cloneContext();
        ctx.state = readingReply;
        return ctx;
    }

    Context commandWritingSubcontext() {
        Context ctx = cloneContext();
        ctx.state = writingCommand;
        return ctx;
    }

    Context inEventSubcontext() {
        Context ctx = cloneContext();
        ctx.inEvent = true;
        return ctx;
    }

    boolean inEvent() {
        return inEvent;
    }

    boolean isWritingCommand() {
        return state == writingCommand;
    }

    boolean isReadingReply() {
        return state == readingReply;
    }
}
