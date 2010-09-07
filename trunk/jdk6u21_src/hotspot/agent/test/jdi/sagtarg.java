/*
 * Copyright (c) 2002, 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// This is the target debuggee for sagtest.java.
// It just waits which lets the test call all the JDI
// methods on it.

import java.util.List;

interface MyInterface {
    public void myMethod();
}


abstract class MySuper implements MyInterface {
}

class sagtarg extends MySuper {
    public static void main(String[] args){
        String stringVar = "localVar1";
        int    intVar = 89;
        List<String> genVar = null;
        System.out.println("Howdy!");
        String myStr = "";
        synchronized(myStr) {
            try {
                myStr.wait();
            } catch (InterruptedException ee) {
            }
        }
        System.out.println("Goodbye from sagtarg!");
    }

    public void myMethod() {
    }
}
