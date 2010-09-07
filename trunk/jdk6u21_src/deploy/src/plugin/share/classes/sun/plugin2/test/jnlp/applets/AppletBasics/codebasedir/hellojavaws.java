/*  Copyright (c) 1999 by Sun Microsystems, Inc.
 *  All rights reserved.

 *  ident @(#)hellojavaws.java 

 *  This code is provided "AS IS", without any warranty of any kind.
 *  Sun is under no obligation to provide maintenance or support for
 *  this code or provide future updates thereto.

 *  SUN DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED 
 *  WARRANTIES, INCLUDING BUT NOT LIMITED TO, WARRANTIES OF NON-
 *  INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
 *  OR ANY WARRANTIES ARISING FROM A COURSE OF DEALING, USAGE OR TRADE
 *  PRACTICE.
 */

public class hellojavaws implements SimpleAppletTestIf1 {
    public static void main (String [] args) {

        String testName = System.getProperty("jnlp.TESTNAME");
        if (testName == null) {
            testName = "HELLOJAWS";
        }

        System.out.println("STATUS: " + testName + " START");

        hellojavaws h1 = new hellojavaws();
        h1.start();
        boolean result = h1.getTestResult();
        
        if(result) {
            System.out.println("STATUS: " + testName + " PASSED");
        } else {
            System.out.println("STATUS: " + testName + " FAILED");
        }

        try {
            Thread.sleep(4000);
        } catch (InterruptedException ignore) {
        }

        System.exit(0);
    }

    private boolean test_result = false;

    public hellojavaws() {
    }

    public boolean getTestResult() { return test_result; }

    public void start()
    {
        test_result = test();
        String res = test_result?T_PASSED:T_FAILED;
		System.out.println(T_TAG+" "+res);
    }

    public boolean test ()
    {
        return true;
    }
}
