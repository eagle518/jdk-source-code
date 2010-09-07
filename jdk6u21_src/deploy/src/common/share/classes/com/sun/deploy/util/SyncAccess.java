/*
 * @(#)SyncAccess.java	1.3 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.util.*;
import java.lang.*;

public class SyncAccess {

    // Operations
    public static final int READ_OP        = 2;
    public static final int WRITE_OP       = 4;

    // Modes
    public static final int SHARED_READ_MODE = 8;
    
    public SyncAccess(int mode) {
        this.lockedOP = 0;
        this.syncObj  = new Object();
        this.mode     = mode;
    } 

    public Lock lock(int op) {
        int wait_mask = 0;
        if ( op==READ_OP && (mode&SHARED_READ_MODE)!= 0 ) {
            wait_mask = WRITE_OP;
        } else {
            wait_mask = READ_OP | WRITE_OP;
        }
        return new Lock ( wait_mask, op );
    }

    public class Lock {

        private Lock(int wait_mask, int op) {
            this.op = op;
            acquireLock(wait_mask, op);
        }

        public void release() {
            releaseLock(op);
        }

        private int op;
    }

    private void acquireLock(int wait_mask, int op) {
        synchronized ( syncObj ) {
            while ( (lockedOP & wait_mask)!=0 ) {
                try {
                    syncObj.wait();
                } catch (InterruptedException ie) {}
            }
            lockedOP |= op;
        }
    }

    private void releaseLock(int op) {
        synchronized ( syncObj ) {
            lockedOP &= ~op;
            syncObj.notifyAll();
        }
    }

    private int lockedOP;
    private Object syncObj;
    private int mode;
}

